#pragma once
#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE *Volume);
uint64_t FileSize(EFI_FILE_HANDLE FileHandle);
EFI_STATUS ReadFile(EFI_FILE_HANDLE Volume, uint16_t *FileName, char **Buffer, uint64_t *ReadSize);