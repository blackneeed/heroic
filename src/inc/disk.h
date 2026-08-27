#pragma once
#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE *Volume);