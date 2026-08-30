#pragma once
#include <efi.h>
#include <efilib.h>

EFI_STATUS GetGOP(EFI_GRAPHICS_OUTPUT_PROTOCOL **GOP);
EFI_STATUS QueryModeInfo(EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP, UINTN *NativeMode, UINTN *ModeCount);