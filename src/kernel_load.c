#include <efi.h>
#include <efilib.h>
#include <disk.h>
#include <heroic_elf.h>

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, UINT16* FileName, void** KernelEntry) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    UINT8* Buffer;
    uint64_t ReadSize;

    Status = ReadFile(Volume, FileName, &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    Status = LoadELF(Buffer, KernelEntry);
    if (EFI_ERROR(Status)) {
        Print(L"[ELF] LoadELF failed with %r!\r\n", Status);
        return Status;
    }

    FreePool(Buffer);

    return Status;
}