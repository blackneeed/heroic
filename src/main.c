#include <efi.h>
#include <efilib.h>

// idk why i jumped straight to doing disk stuff
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

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR* Map, UINTN* MapSize, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion) {
    EFI_STATUS Status;

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) return Status;
    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MapSize, (void**)&Map);
    if (EFI_ERROR(Status)) return Status;
    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
    return Status;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_MEMORY_DESCRIPTOR* Map = NULL;
    UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;    
    EFI_STATUS Status;

    Status = FetchMemoryMap(Map, &MapSize, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) return Status;

    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);

    return Status;
}