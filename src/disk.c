#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE ImageHandle, EFI_FILE_HANDLE *Volume)
{
    EFI_STATUS Status;

    EFI_LOADED_IMAGE *LoadedImage = NULL;
    EFI_GUID LoadedImageProtocolGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    Status = uefi_call_wrapper(BS->HandleProtocol,
        3,
        ImageHandle,
        &LoadedImageProtocolGUID,
        (void**)&LoadedImage);
    
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] GetVolume: HandleProtocol failed with %r\r\n", Status);
        return Status;
    }

    *Volume = LibOpenRoot(LoadedImage->DeviceHandle);

    return Status; 
}

UINTN FileSize(EFI_FILE_HANDLE FileHandle)
{
    UINTN Size;
    EFI_FILE_INFO *FileInfo;

    FileInfo = LibFileInfo(FileHandle);
    Size = FileInfo->FileSize;

    FreePool(FileInfo);

    return Size;
}

EFI_STATUS ReadFile(EFI_FILE_HANDLE Volume,
    UINT16 *FileName,
    UINT8 **Buffer,
    UINTN *ReadSize)
{
    EFI_FILE_HANDLE FileHandle;
    EFI_STATUS Status = uefi_call_wrapper(Volume->Open,
        5,
        Volume,
        &FileHandle,
        FileName,
        EFI_FILE_MODE_READ,
        EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] Open failed with %r!\r\n", Status);
        return Status;
    }

    *ReadSize = FileSize(FileHandle);

    Status = uefi_call_wrapper(BS->AllocatePool,
        3,
        EfiLoaderData,
        *ReadSize,
        (void**)Buffer);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] AllocatePool failed with %r!\r\n", Status);
        return Status;
    }

    Status = uefi_call_wrapper(FileHandle->Read,
        3,
        FileHandle,
        ReadSize,
        *Buffer);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] Read failed with %r!\r\n", Status);
        return Status;
    }

    return Status;
}