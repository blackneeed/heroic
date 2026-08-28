#include <efi.h>
#include <efilib.h>
#include <mmap.h>
#include <disk.h>
#include <heroic_elf.h>
#include <page.h>

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

    uint64_t HighestPhysAddr, CurrMapOffset, CurrMapIndex = 0;

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

    Status = FetchMemoryMap(&Map, &MapSize, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        Print(L"[MMAP] FetchMemoryMap failed with %r!\r\n", Status);
        return Status;
    }

    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Print(L"[BS] ExitBootServices failed with %r!\r\n", Status);
        return Status;
    }

    return Status;
}