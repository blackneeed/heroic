#pragma once
#include <efi.h>

EFI_STATUS LoadELF(char* Buffer, void** EntryPoint);