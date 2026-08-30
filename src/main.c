#include <efi.h>
#include <efilib.h>
#include <mmap.h>
#include <boot_protocol.h>
#include <heroic_elf.h>
#include <page.h>
#include <kernel_load.h>
#include <string.h>

extern PAGE_ENTRY *PML4;
extern char transition_start[];
extern char transition_end[];

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;

    EFI_MEMORY_DESCRIPTOR* Map;
    UINTN MapSize;
    UINTN MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;    

    Status = FetchMemoryMap(&Map,
        &MapSize,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion);

    if (EFI_ERROR(Status)) {
        Print(L"[MMAP] FetchMemoryMap failed with %r!\r\n", Status);
        return Status;
    }

    EFI_PHYSICAL_ADDRESS HighestPhysicalAddress = GetHighestPhysicalAddress(MapSize,
        DescriptorSize,
        Map);

    Status = PageInit(HighestPhysicalAddress);
    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] PageInit failed with %r!\r\n", Status);
        return Status;
    }

    void *KernelEntry;

    LoadKernel(ImageHandle, L"kernel.elf", &KernelEntry);

    heroic_boot_protocol_data_t* BootProtocolData;

    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, sizeof(heroic_boot_protocol_data_t), (void**)&BootProtocolData);
    if (EFI_ERROR(Status)) {
        Print(L"[BOOT] Could not allocate boot protocol data structure: %r\r\n", Status);
        return Status;
    }

    BootProtocolData->magic = HEROIC_BOOT_PROTOCOL_MAGIC;
    BootProtocolData->size = sizeof(heroic_boot_protocol_data_t);

    BootProtocolData->hhdm = HHDM_BASE;

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
        : [pml4] "r"((uint64_t)PML4),
          [boot_prot_data] "r"(BootProtocolData->hhdm + (uint64_t)BootProtocolData),
          [kernel_entry] "r"((uint64_t)KernelEntry),
          [target]  "r"((uint64_t)TransitionPageAddress)
        : "rax", "rbx", "rcx", "memory"
    );

    return Status;
}