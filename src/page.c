#define PAGE_C_IMPL
#include <page.h>
#include <efi.h>
#include <efilib.h>
#include <string.h>
#include <stdint.h>

uint64_t *pml4;

static inline uint64_t va_offset(uint64_t va) {
    return va & 0xFFFULL;
}

static inline uint64_t pt_index(uint64_t va) {
    return va >> 12 & 0x1FFULL;
}

static inline uint64_t pd_index(uint64_t va) {
    return va >> 21 & 0x1FFULL;
}

static inline uint64_t pdp_index(uint64_t va) {
    return va >> 30 & 0x1FFULL;
}

static inline uint64_t pml4_index(uint64_t va) {
    return va >> 39 & 0x1FFULL;
}

static EFI_STATUS AllocatePageTable(uint64_t **Table) {
    EFI_PHYSICAL_ADDRESS Address = 0;

    EFI_STATUS Status = uefi_call_wrapper(
        BS->AllocatePages,
        4,
        AllocateAnyPages,
        EfiLoaderData,
        1, // 512 * sizeof(uint64_t) = 4096 = 1 page
        &Address
    );

    if (EFI_ERROR(Status))
        return Status;

    *Table = (uint64_t *)Address;

    memset(*Table, 0, 0x1000);

    return EFI_SUCCESS;
}


EFI_STATUS PageInit(uint64_t HighestPhysAddr) {
    EFI_STATUS Status;
    Status = AllocatePageTable(&pml4);
    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] Could not allocate pool for PML4\r\n");
        return Status;
    }

    memset(pml4, 0, 512 * sizeof(uint64_t));

    int PageCount = (HighestPhysAddr + 0x1FFFFF) / 0x200000;
    MapHugePages(0xFFFF800000000000, 0x0000000000000000, PageCount);

    return Status;
}

EFI_STATUS SetupToPD(uint64_t VAddress, uint64_t Address, uint64_t **pd_out, uint64_t page_align) {
    if (!pml4) {
        Print(L"[PAGE] SetupToPD: PML4 not initialized, call PageInit() first\r\n");
        return EFI_NOT_READY;
    }

    if (VAddress % page_align != 0 || Address % page_align != 0) {
        Print(L"[PAGE] SetupToPD: VAddress and Address must be page-aligned (0x%lx), vaddr = 0x%lx, addr = 0x%lx\r\n", page_align, VAddress, Address);
        return EFI_INVALID_PARAMETER;
    }

    uint64_t *pdpt;
    uint64_t *pd;
    EFI_STATUS Status;

    if (pml4[pml4_index(VAddress)] & Present) {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PML4E already present for VAddress 0x%lx\r\n", VAddress);
        #endif
        pdpt = (uint64_t*)(pml4[pml4_index(VAddress)] & ~0xFFFULL);
    } else {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PML4E not present for VAddress 0x%lx, allocating new PDPT\r\n", VAddress);
        #endif
        Status = AllocatePageTable(&pdpt);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] SetupToPD: Could not allocate pool for PDPT\r\n");
            return Status;
        }

        memset(pdpt, 0, 512 * sizeof(uint64_t));
        pml4[pml4_index(VAddress)] = (uint64_t)pdpt | Present | ReadWrite;
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PML4E %d set to PDPT at 0x%lx\r\n", pml4_index(VAddress), pdpt);
        #endif
    }

    if (pdpt[pdp_index(VAddress)] & Present) {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PDPTE entry already present for VAddress 0x%lx\r\n", VAddress);
        #endif
        pd = (uint64_t*)(pdpt[pdp_index(VAddress)] & ~0xFFFULL);
    } else {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PDPTE not present for VAddress 0x%lx, allocating new PD\r\n", VAddress);
        #endif
        Status = AllocatePageTable(&pd);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] SetupToPD: Could not allocate pool for PD\r\n");
            return Status;
        }
        
        memset(pd, 0, 512 * sizeof(uint64_t));
        pdpt[pdp_index(VAddress)] = (uint64_t)pd | Present | ReadWrite;
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] SetupToPD: PDPTE %d set to PD at 0x%lx\r\n", pdp_index(VAddress), pd);
        #endif
    }

    *pd_out = pd;
    return EFI_SUCCESS;
}

EFI_STATUS _MapPage(uint64_t VAddress, uint64_t Address, BOOLEAN multiple) {
    uint64_t *pd;
    EFI_STATUS Status = SetupToPD(VAddress, Address, &pd, 0x1000ULL);

    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] MapPage: SetupToPD failed for VAddress 0x%lx\r\n", VAddress);
        return Status;
    }

    uint64_t *pt;

    if (pd[pd_index(VAddress)] & Present) {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] MapPage: PDE already present for VAddress 0x%lx\r\n", VAddress);
        #endif
        pt = (uint64_t*)(pd[pd_index(VAddress)] & ~0xFFFULL);
    } else {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] MapPage: PDE not present for VAddress 0x%lx, allocating new PT\r\n", VAddress);
        #endif
        Status = AllocatePageTable(&pt);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapPage: Could not allocate pool for PT\r\n");
            return Status;
        }
        
        memset(pt, 0, 512 * sizeof(uint64_t));
        pd[pd_index(VAddress)] = (uint64_t)pt | Present | ReadWrite;
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] MapPage: PDE %d set to PT at 0x%lx\r\n", pd_index(VAddress), pt);
        #endif
    }

    if (pt[pt_index(VAddress)] & Present) {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] MapPage: PTE already present for VAddress 0x%lx, overwriting\r\n", VAddress);
        #endif
    }

    pt[pt_index(VAddress)] = (Address | Present | ReadWrite) & ~NoExecute;
    #if defined(PAGE_DEBUG_ALL)
    Print(L"[PAGE] MapPage: PTE %d set to 0x%lx (0x%lx -> 0x%lx) \r\n", pt_index(VAddress), Address, Address, VAddress);
    #else
    if (!multiple) {
        Print(L"[PAGE] 4KB 0x%016lx -> 0x%016lx\r\n", Address, VAddress);
    }
    #endif

    return EFI_SUCCESS;
}

EFI_STATUS MapPage(uint64_t VAddress, uint64_t Address) {
    return _MapPage(VAddress, Address, FALSE);
}

EFI_STATUS MapPages(uint64_t VAddress, uint64_t Address, int PageCount) {
    for (uint64_t i = 0; i < PageCount; i++) {
        EFI_STATUS Status = _MapPage(VAddress + i * 0x1000ULL, Address + i * 0x1000ULL, TRUE);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapPage failed for VAddress 0x%lx, Address 0x%lx\r\n", VAddress + i * 0x1000ULL, Address + i * 0x1000ULL);
            return Status;
        }
    }

    Print(L"[PAGE] 0x%016lx-0x%016lx -> 0x%016lx-0x%016lx\r\n", Address, Address + PageCount * 0x1000ULL, VAddress, VAddress + PageCount * 0x1000ULL);

    return EFI_SUCCESS;
}

EFI_STATUS _MapHugePage(uint64_t VAddress, uint64_t Address, BOOLEAN multiple) {
    uint64_t *pd;
    EFI_STATUS Status = SetupToPD(VAddress, Address, &pd, 0x200000ULL);

    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] MapHugePage: SetupToPD failed for VAddress 0x%lx\r\n", VAddress);
        return Status;
    }

    if (pd[pd_index(VAddress)] & Present) {
        #ifdef PAGE_DEBUG_ALL
        Print(L"[PAGE] MapHugePage: PDE already present for VAddress 0x%lx, overwriting\r\n", VAddress);
        #endif
    }
    
    pd[pd_index(VAddress)] = (Address | Present | ReadWrite | PageSize) & ~NoExecute;
    #if defined(PAGE_DEBUG_ALL)
    Print(L"[PAGE] MapHugePage: PDE %d set to 0x%lx (0x%lx -> 0x%lx) \r\n", pd_index(VAddress), Address, Address, VAddress);
    #else
    if (!multiple) {
        Print(L"[PAGE] 2MB 0x%016lx -> 0x%016lx\r\n", Address, VAddress);
    }
    #endif

    return EFI_SUCCESS;
}

EFI_STATUS MapHugePage(uint64_t VAddress, uint64_t Address) {
    return _MapHugePage(VAddress, Address, FALSE);
}

EFI_STATUS MapHugePages(uint64_t VAddress, uint64_t Address, int PageCount) {
    for (uint64_t i = 0; i < PageCount; i++) {
        EFI_STATUS Status = _MapHugePage(VAddress + i * 0x200000ULL, Address + i * 0x200000ULL, TRUE);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapHugePage failed for VAddress 0x%lx, Address 0x%lx\r\n", VAddress + i * 0x200000ULL, Address + i * 0x200000ULL);
            return Status;
        }
    }

    Print(L"[PAGE] 0x%016lx-0x%016lx -> 0x%016lx-0x%016lx\r\n", Address, Address + PageCount * 0x200000ULL, VAddress, VAddress + PageCount * 0x200000ULL);

    return EFI_SUCCESS;
}