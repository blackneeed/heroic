#include <efi.h>
#include <efilib.h>
#include <disk.h>
#include <heroic_elf.h>

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, UINT16* FileName, void** KernelEntry) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[KRLD] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    UINT8* Buffer;
    UINTN ReadSize;

    Status = ReadFile(Volume, FileName, &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[KRLD] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    Status = LoadELF(Buffer, KernelEntry);
    if (EFI_ERROR(Status)) {
        Print(L"[KRLD] LoadELF failed with %r!\r\n", Status);
        return Status;
    }

    FreePool(Buffer);

    return Status;
}