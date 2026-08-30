#include <efi.h>
#include <efilib.h>

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, UINT16* FileName, void** KernelEntry);