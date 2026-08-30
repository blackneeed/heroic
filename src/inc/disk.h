#pragma once
#include <efi.h>
#include <efilib.h>

EFI_STATUS GetVolume(EFI_HANDLE ImageHandle, EFI_FILE_HANDLE *Volume);
UINTN FileSize(EFI_FILE_HANDLE FileHandle);
EFI_STATUS ReadFile(EFI_FILE_HANDLE Volume,
    UINT16 *FileName,
    UINT8 **Buffer,
    UINTN *ReadSize);