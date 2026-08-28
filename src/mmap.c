#include <efi.h>
#include <efilib.h>

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, uint64_t* MapSize, uint64_t* MapKey, uint64_t* DescriptorSize, uint32_t* DescriptorVersion) {
    EFI_STATUS Status;

    *MapSize = 0;
    *Map = NULL;

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, NULL, MapKey, DescriptorSize, DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;

    *MapSize += *DescriptorSize * 2;

    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *MapSize, (void**)Map);
    if (EFI_ERROR(Status)) return Status;
    
    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, *Map, MapKey, DescriptorSize, DescriptorVersion);
    return Status;
}