#include <efi.h>
#include <efilib.h>

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR** Map,
                          UINTN* MapSize,
                          UINTN* MapKey,
                          UINTN* DescriptorSize,
                          UINT32* DescriptorVersion) {
    EFI_STATUS Status;

    *Map = NULL;
    *MapSize = 0;

    Status = uefi_call_wrapper(BS->GetMemoryMap,
        5,
        MapSize,
        Map,
        MapKey,
        DescriptorSize,
        DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;

    *MapSize += *DescriptorSize * 2;

    Status = uefi_call_wrapper(BS->AllocatePool,
        3,
        EfiLoaderData,
        *MapSize,
        (void**)Map);
    if (EFI_ERROR(Status)) return Status;
    
    Status = uefi_call_wrapper(BS->GetMemoryMap,
        5,
        MapSize,
        *Map,
        MapKey,
        DescriptorSize,
        DescriptorVersion);
    return Status;
}

EFI_PHYSICAL_ADDRESS GetHighestPhysicalAddress(UINTN MapSize,
    UINTN DescriptorSize,
    EFI_MEMORY_DESCRIPTOR* Map) {
    EFI_PHYSICAL_ADDRESS HighestAddress = 0;

    for (UINTN Offset = 0; Offset < MapSize; Offset += DescriptorSize) {
        UINTN EntryAddress = (UINTN)Map + Offset;
        EFI_MEMORY_DESCRIPTOR* Entry = (EFI_MEMORY_DESCRIPTOR*)EntryAddress;

        EFI_PHYSICAL_ADDRESS EntryEndAddress =
            Entry->PhysicalStart +
            Entry->NumberOfPages * 0x1000ULL;

        if (EntryEndAddress > HighestAddress)
            HighestAddress = EntryEndAddress;
    }

    return HighestAddress;
}
