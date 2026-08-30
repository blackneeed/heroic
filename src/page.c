#define PAGE_C_IMPL
#include <page.h>
#include <efi.h>
#include <efilib.h>
#include <string.h>
#include <stdint.h>

PAGE_ENTRY *PML4;

static EFI_STATUS AllocatePageTable(PAGE_ENTRY **Table) {
    EFI_PHYSICAL_ADDRESS Address = 0;

    EFI_STATUS Status = uefi_call_wrapper(
        BS->AllocatePages,
        4,
        AllocateAnyPages,
        EfiLoaderData,
        1,
        &Address
    );

    if (EFI_ERROR(Status))
        return Status;

    *Table = (PAGE_ENTRY*)Address;

    memset(*Table, 0, 0x1000);

    return EFI_SUCCESS;
}

EFI_STATUS PageInit(EFI_PHYSICAL_ADDRESS HighestPhysAddr) {
    EFI_STATUS Status;
    Status = AllocatePageTable(&PML4);
    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] Could not allocate PML4\r\n");
        return Status;
    }

    UINTN PageCount = ALIGN_UP_2MB(HighestPhysAddr) / 0x200000;
    MapHugePages(HHDM_BASE, 0x0, PageCount);

    return Status;
}

EFI_STATUS SetupToPD(EFI_VIRTUAL_ADDRESS VAddress,
    EFI_PHYSICAL_ADDRESS Address,
    PAGE_ENTRY **PD,
    UINTN PageAlignment) {
    EFI_STATUS Status;

    if (!PML4) {
        Print(L"[PAGE] SetupToPD: PML4 not initialized, call PageInit() first\r\n");
        return EFI_NOT_READY;
    }

    if (VAddress % PageAlignment != 0 || Address % PageAlignment != 0) {
        Print(L"[PAGE] SetupToPD: VAddress and Address must be page-aligned (0x%lx), vaddr = 0x%lx, addr = 0x%lx\r\n", PageAlignment, VAddress, Address);
        return EFI_INVALID_PARAMETER;
    }

    PAGE_ENTRY *PML4E = &PML4[VADDR_PML4_INDEX(VAddress)];
    PAGE_ENTRY *PDPT;

    if (*PML4E & Present) {
        UINTN PDPTAddress = *PML4E & ~0xFFFULL;
        PDPT = (PAGE_ENTRY*)PDPTAddress;
    } else {
        Status = AllocatePageTable(&PDPT);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] SetupToPD: Could not allocate PDPT\r\n");
            return Status;
        }

        *PML4E = FLAG((UINTN)PDPT, 0);
    }

    PAGE_ENTRY *PDPTE = &PDPT[VADDR_PDP_INDEX(VAddress)];

    if (*PDPTE & Present) {
        UINTN PDAddress = *PDPTE & ~0xFFFULL;
        *PD = (PAGE_ENTRY*)PDAddress;
    } else {
        Status = AllocatePageTable(PD);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] SetupToPD: Could not allocate PD\r\n");
            return Status;
        }
        
        *PDPTE = FLAG((UINTN)*PD, 0);
    }

    return EFI_SUCCESS;
}

EFI_STATUS MapPage(EFI_VIRTUAL_ADDRESS VAddress, EFI_PHYSICAL_ADDRESS Address) {
    PAGE_ENTRY *PD;
    EFI_STATUS Status = SetupToPD(VAddress, Address, &PD, 0x1000ULL);

    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] MapPage: SetupToPD failed for VAddress 0x%lx\r\n", VAddress);
        return Status;
    }

    PAGE_ENTRY *PDE = &PD[VADDR_PD_INDEX(VAddress)];
    PAGE_ENTRY *PT;

    if (*PDE & Present) {
        UINTN PTAddress = *PDE & ~0xFFFULL;
        PT = (PAGE_ENTRY*)PTAddress;
    } else {
        Status = AllocatePageTable(&PT);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapPage: Could not allocate PT\r\n");
            return Status;
        }
        
        *PDE = FLAG((UINTN)PT, 0);
    }

    PAGE_ENTRY* PTE = &PT[VADDR_PT_INDEX(VAddress)];

    *PTE = FLAG(Address, 0);

    return EFI_SUCCESS;
}

EFI_STATUS MapPages(uint64_t VAddress, uint64_t Address, UINTN PageCount) {
    for (UINTN i = 0; i < PageCount; i++) {
        EFI_STATUS Status = MapPage(VAddress + i * 0x1000ULL, Address + i * 0x1000ULL);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapPage failed for VAddress 0x%lx, Address 0x%lx\r\n", VAddress + i * 0x1000ULL, Address + i * 0x1000ULL);
            return Status;
        }
    }

    Print(L"[PAGE] 0x%016lx-0x%016lx -> 0x%016lx-0x%016lx\r\n", Address, Address + PageCount * 0x1000ULL, VAddress, VAddress + PageCount * 0x1000ULL);

    return EFI_SUCCESS;
}

EFI_STATUS MapHugePage(uint64_t VAddress, uint64_t Address) {
    PAGE_ENTRY *PD;
    EFI_STATUS Status = SetupToPD(VAddress, Address, &PD, 0x200000ULL);

    if (EFI_ERROR(Status)) {
        Print(L"[PAGE] MapHugePage: SetupToPD failed for VAddress 0x%lx\r\n", VAddress);
        return Status;
    }

    PAGE_ENTRY *PDE = &PD[VADDR_PD_INDEX(VAddress)];

    *PDE = FLAG(Address, PageSize);

    return EFI_SUCCESS;
}

EFI_STATUS MapHugePages(uint64_t VAddress, uint64_t Address, UINTN PageCount) {
    for (UINTN i = 0; i < PageCount; i++) {
        EFI_STATUS Status = MapHugePage(VAddress + i * 0x200000ULL, Address + i * 0x200000ULL);
        if (EFI_ERROR(Status)) {
            Print(L"[PAGE] MapHugePage failed for VAddress 0x%lx, Address 0x%lx\r\n", VAddress + i * 0x200000ULL, Address + i * 0x200000ULL);
            return Status;
        }
    }

    Print(L"[PAGE] 0x%016lx-0x%016lx -> 0x%016lx-0x%016lx\r\n", Address, Address + PageCount * 0x200000ULL, VAddress, VAddress + PageCount * 0x200000ULL);

    return EFI_SUCCESS;
}