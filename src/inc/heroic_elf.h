#include <efi.h>
#pragma once

EFI_STATUS LoadELF(void* Buffer, void** EntryPoint);