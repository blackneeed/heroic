#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE *Volume)
{
  EFI_LOADED_IMAGE *loaded_image = NULL;
  EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

  EFI_STATUS Status = uefi_call_wrapper(BS->HandleProtocol, 3, image, &lipGuid, (void **) &loaded_image); // get loaded image protocol
  goto end;

  *Volume = LibOpenRoot(loaded_image->DeviceHandle); // get volume handle

  end:
  return Status; 
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_FILE_HANDLE Volume;
    EFI_STATUS Status = GetVolume(ImageHandle, &Volume);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get volume\n");
        return Status;
    }

    Print(L"Got volume!\n");
    
    return EFI_SUCCESS;
}