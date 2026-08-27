#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE *Volume)
{
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    EFI_STATUS Status = uefi_call_wrapper(BS->HandleProtocol, 3, image, &lipGuid, (void **) &loaded_image); // get loaded image protocol
    if (EFI_ERROR(Status)) {
        return Status;
    }

    *Volume = LibOpenRoot(loaded_image->DeviceHandle); // get volume handle

    return Status; 
}

uint64_t FileSize(EFI_FILE_HANDLE FileHandle)
{
    uint64_t ret;
    EFI_FILE_INFO *FileInfo;

    FileInfo = LibFileInfo(FileHandle);
    ret = FileInfo->FileSize;

    FreePool(FileInfo);

    return ret;
}

EFI_STATUS ReadFile(EFI_FILE_HANDLE Volume, uint16_t *FileName, char **Buffer, uint64_t *ReadSize)
{
    EFI_FILE_HANDLE FileHandle;
    EFI_STATUS Status = uefi_call_wrapper(Volume->Open, 5, Volume, &FileHandle, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] Open failed with %r!\r\n", Status);
        return Status;
    }

    Print(L"[DISK] Opened file: %s\r\n", FileName);

    *ReadSize = FileSize(FileHandle);

    Print(L"[DISK] Size of file %s: %lu bytes\r\n", FileName, *ReadSize);

    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *ReadSize, (void**)Buffer);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] AllocatePool failed with %r (attempt to allocate %lu bytes as a buffer for file %s)!\r\n", Status, *ReadSize, FileName);
        return Status;
    }

    Status = uefi_call_wrapper(FileHandle->Read, 3, FileHandle, ReadSize, *Buffer);
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] Read failed with %r!\r\n", Status);
        return Status;
    }

    return Status;
}