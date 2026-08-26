#include <efi.h>
#include <efilib.h>

// idk why i jumped straight to doing disk stuff
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

void wait_for_key(EFI_SYSTEM_TABLE *SystemTable) {
    UINTN index;
    EFI_INPUT_KEY key;

    uefi_call_wrapper(
        BS->WaitForEvent,
        3,
        1,
        &SystemTable->ConIn->WaitForKey,
        &index
    );

    uefi_call_wrapper(
        SystemTable->ConIn->ReadKeyStroke,
        2,
        SystemTable->ConIn,
        &key
    );
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"Hello, World!\n");

    wait_for_key(SystemTable);

    return EFI_SUCCESS;
}