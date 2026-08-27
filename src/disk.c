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
