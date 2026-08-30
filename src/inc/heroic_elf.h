#include <efi.h>
#pragma once

EFI_STATUS LoadELF(char* Buffer, void** EntryPoint);