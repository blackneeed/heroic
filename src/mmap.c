#include <efi.h>
#include <efilib.h>

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, UINTN* MapSize, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion) {
    EFI_STATUS Status;

    *MapSize = 0;
    *Map = NULL;

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, NULL, MapKey, DescriptorSize, DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;

    Print(L"[MMAP] MapSize: %lu\r\n", *MapSize);
    Print(L"[MMAP] DescriptorSize: %lu\r\n", *DescriptorSize);

    *MapSize += 2 * (*DescriptorSize); // just to be safe because we are doing a allocation to get the memory map and that may split an entry

    Print(L"[MMAP] New MapSize: %lu\r\n", *MapSize);


    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *MapSize, (void**)Map);
    if (EFI_ERROR(Status)) return Status;
    
    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, *Map, MapKey, DescriptorSize, DescriptorVersion);
    return Status;
}
