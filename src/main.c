#include <efi.h>
#include <efilib.h>
#include <mmap.h>
#include <disk.h>
#include <boot_protocol.h>
#include <heroic_elf.h>
#include <page.h>
#include <string.h>

extern uint64_t *pml4;
extern char transition_start[];
extern char transition_end[];

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;

    EFI_MEMORY_DESCRIPTOR* Map;
    uint64_t MapSize, MapKey;
	uint64_t DescriptorSize;
	uint32_t DescriptorVersion;    

    Status = FetchMemoryMap(&Map, &MapSize, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        Print(L"[MMAP] FetchMemoryMap failed with %r!\r\n", Status);
        return Status;
    }

    uint64_t HighestPhysAddr = 0;
    uint64_t CurrMapOffset = 0;
    uint64_t CurrMapIndex = 0;

    for (; CurrMapOffset < MapSize; CurrMapOffset += DescriptorSize) {
        uint64_t HighestPhysAddrOfEntry = Map[CurrMapIndex].PhysicalStart + Map[CurrMapIndex].NumberOfPages * 0x1000;
        if (HighestPhysAddrOfEntry > HighestPhysAddr) {
            HighestPhysAddr = HighestPhysAddrOfEntry;
        }
        CurrMapIndex++;
    }

    Status = PageInit(HighestPhysAddr);
    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] PageInit failed with %r!\r\n", Status);
        return Status;
    }

    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    char* Buffer;
    uint64_t ReadSize;

    Status = ReadFile(Volume, L"kernel.elf", &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    void* KernelEntry;

    Status = LoadELF(Buffer, &KernelEntry);
    if (EFI_ERROR(Status)) {
        Print(L"[ELF] LoadELF failed with %r!\r\n", Status);
        return Status;
    }

    heroic_boot_protocol_data_t* BootProtocolData;

    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, sizeof(heroic_boot_protocol_data_t), (void**)&BootProtocolData);
    if (EFI_ERROR(Status)) {
        Print(L"[BOOT] Could not allocate pool for boot protocol data structure: %r\r\n", Status);
        return Status;
    }

    BootProtocolData->magic = HEROIC_BOOT_PROTOCOL_MAGIC;
    BootProtocolData->size = sizeof(heroic_boot_protocol_data_t);

    BootProtocolData->hhdm = 0xFFFF800000000000;

    uint64_t* TransitionPageAddress;

    Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 1, (void**)&TransitionPageAddress);
    if (EFI_ERROR(Status)) {
        Print(L"[BOOT] Could not allocate page for transition: %r\r\n", Status);
        return Status;
    }

    MapPage((uint64_t)TransitionPageAddress, (uint64_t)TransitionPageAddress);

    Status = FetchMemoryMap(&Map, &MapSize, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        Print(L"[MMAP] FetchMemoryMap failed with %r!\r\n", Status);
        return Status;
    }

    BootProtocolData->memory_map = (uint64_t)Map;
    BootProtocolData->memory_map_size = MapSize;
    BootProtocolData->memory_map_descriptor_size = DescriptorSize;

    
    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Print(L"[BS] ExitBootServices failed with %r!\r\n", Status);
        return Status;
    }

    // start transition

    memcpy((void*)TransitionPageAddress, (void*)(&transition_start), ((uint64_t)&transition_end - (uint64_t)&transition_start));

    asm volatile (
        "mov %[pml4], %%rax\n\t"
        "mov %[boot_prot_data], %%rbx\n\t"
        "mov %[kernel_entry], %%rcx\n\t"
        "jmp *%[target]\n\t"
        :
        : [pml4] "r"((uint64_t)pml4),
          [boot_prot_data] "r"(BootProtocolData->hhdm + (uint64_t)BootProtocolData),
          [kernel_entry] "r"((uint64_t)KernelEntry),
          [target]  "r"((uint64_t)TransitionPageAddress)
        : "rax", "rbx", "rcx", "memory"
    );

    return Status;
}