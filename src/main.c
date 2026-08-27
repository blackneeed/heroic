#include <efi.h>
#include <efilib.h>
#include <mmap.h>
#include <disk.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;
    
    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    Print(L"[DISK] Volume handle obtained\r\n");

    char* Buffer;
    uint64_t ReadSize;

    Status = ReadFile(Volume, L"kernel.elf", &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    if (Buffer[0] == 0x7F && Buffer[1] == 'E' && Buffer[2] == 'L' && Buffer[3] == 'F') {
        Print(L"[DISK] Kernel file is a valid ELF file\r\n");
    } else {
        Print(L"[DISK] Kernel file is not a valid ELF file\r\n");
        return EFI_LOAD_ERROR;
    }

    EFI_MEMORY_DESCRIPTOR* Map;
    uint64_t MapSize, MapKey;
	uint64_t DescriptorSize;
	uint32_t DescriptorVersion;    

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