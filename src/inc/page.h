#pragma once
#include <stdint.h>
#include <efi.h>
#ifdef PAGE_C_IMPL
#define Present      (1ULL << 0)
#define ReadWrite    (1ULL << 1)
#define PageSize     (1ULL << 7)
#define NoExecute    (1ULL << 63)
#endif

EFI_STATUS PageInit(uint64_t HighestPhysAddr);
EFI_STATUS MapPage(uint64_t VAddress, uint64_t Address);
EFI_STATUS MapPages(uint64_t VAddress, uint64_t Address, int PageCount);
EFI_STATUS MapHugePage(uint64_t VAddress, uint64_t Address);
EFI_STATUS MapHugePages(uint64_t VAddress, uint64_t Address, int PageCount);