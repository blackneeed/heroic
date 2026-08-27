#include <efi.h>
#include <efilib.h>

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, uint64_t* MapSize, uint64_t* MapKey, uint64_t* DescriptorSize, uint32_t* DescriptorVersion);