#pragma once
#include <stdint.h>
#include <efi.h>

#ifdef PAGE_C_IMPL
#define Present      (1ULL << 0)
#define ReadWrite    (1ULL << 1)
#define PageSize     (1ULL << 7)
#define NoExecute    (1ULL << 63)

#define VADDR_OFFSET(vaddress)     ((vaddress)       & 0xFFFULL)
#define VADDR_PT_INDEX(vaddress)   ((vaddress) >> 12 & 0x1FFULL)
#define VADDR_PD_INDEX(vaddress)   ((vaddress) >> 21 & 0x1FFULL)
#define VADDR_PDP_INDEX(vaddress)  ((vaddress) >> 30 & 0x1FFULL)
#define VADDR_PML4_INDEX(vaddress) ((vaddress) >> 39 & 0x1FFULL)

#define FLAG(addr, extra) (((addr) | Present | ReadWrite | (extra)) & ~NoExecute)     
#endif

typedef UINT64 PAGE_ENTRY;

#define ALIGN_DOWN_4KB(addr) ((addr) & ~0xFFFULL)
#define ALIGN_DOWN_2MB(addr) ((addr) & ~0x1FFFFFULL)
#define ALIGN_UP_4KB(addr)   ALIGN_DOWN_4KB((addr) + 0xFFFULL)
#define ALIGN_UP_2MB(addr)   ALIGN_DOWN_2MB((addr) + 0x1FFFFFULL)

#define HHDM_BASE 0xFFFF800000000000

extern PAGE_ENTRY *PML4;

EFI_STATUS PageInit    (EFI_PHYSICAL_ADDRESS HighestPhysAddr);
EFI_STATUS MapPage     (EFI_VIRTUAL_ADDRESS VAddress, EFI_PHYSICAL_ADDRESS Address);
EFI_STATUS MapPages    (EFI_VIRTUAL_ADDRESS VAddress, EFI_PHYSICAL_ADDRESS Address, UINTN PageCount);
EFI_STATUS MapHugePage (EFI_VIRTUAL_ADDRESS VAddress, EFI_PHYSICAL_ADDRESS Address);
EFI_STATUS MapHugePages(EFI_VIRTUAL_ADDRESS VAddress, EFI_PHYSICAL_ADDRESS Address, UINTN PageCount);