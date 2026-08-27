#include <efi.h>
#include <efilib.h>

EFI_STATUS FetchMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, UINTN* MapSize, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion);