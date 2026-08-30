#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <string.h>
#include <page.h>

EFI_STATUS LoadELF(char* Buffer, void** EntryPoint) {
    EFI_STATUS Status;
    Elf64_Ehdr* Header = (Elf64_Ehdr*)Buffer;

    if (Header->e_ident[EI_MAG0] != ELFMAG0 || 
        Header->e_ident[EI_MAG1] != ELFMAG1 ||
        Header->e_ident[EI_MAG2] != ELFMAG2 ||
        Header->e_ident[EI_MAG3] != ELFMAG3) {
        Print(L"[ELF ] Kernel file is not a valid ELF file\r\n");
        return EFI_LOAD_ERROR;
    }

    if (Header->e_ident[EI_CLASS] != ELFCLASS64) {
        Print(L"[ELF ] Kernel ELF is 32-bit while heroic is 64-bit\r\n");
        return EFI_LOAD_ERROR;
    }

    if (Header->e_ident[EI_DATA] != ELFDATA2LSB) {
        Print(L"[ELF ] Kernel ELF is MSB\r\n");
        return EFI_LOAD_ERROR;
    }

    if (Header->e_type != ET_EXEC) {
        Print(L"[ELF ] Kernel ELF is not executable\r\n");
        return EFI_LOAD_ERROR;
    }

    if (Header->e_machine != EM_X86_64) {
        Print(L"[ELF ] Kernel ELF is not compiled for x86_64\r\n");
        return EFI_LOAD_ERROR;
    }

    if (Header->e_version != EV_CURRENT) {
        Print(L"[ELF ] Kernel ELF has an old ELF version\r\n");
        return EFI_LOAD_ERROR;
    }

    Elf64_Phdr* ProgramHeader = (Elf64_Phdr*)&Buffer[Header->e_phoff];

    while ((UINTN)ProgramHeader -
           (UINTN)&Buffer[Header->e_phoff] <
           Header->e_phentsize *
           Header->e_phnum) {
        if (ProgramHeader->p_type == PT_LOAD) {
            EFI_VIRTUAL_ADDRESS VAddress = ALIGN_DOWN_4KB(ProgramHeader->p_vaddr);
            UINTN PageOffset = ProgramHeader->p_vaddr - VAddress;
            UINTN PageCount = ALIGN_UP_4KB(PageOffset + ProgramHeader->p_memsz) / 0x1000;

            EFI_PHYSICAL_ADDRESS PAddress = 0;

            Status = uefi_call_wrapper(BS->AllocatePages,
                4,
                AllocateAnyPages,
                EfiLoaderData,
                PageCount,
                &PAddress);

            EFI_PHYSICAL_ADDRESS Address = PAddress + PageOffset;

            if (EFI_ERROR(Status)) {
                Print(L"[ELF ] Could not allocate Program Header\r\n");
                return Status;
            }

            memcpy((void*)Address,
                   &Buffer[ProgramHeader->p_offset],
                   ProgramHeader->p_filesz);

            memset((void*)Address + ProgramHeader->p_filesz,
                   0,
                   ProgramHeader->p_memsz - ProgramHeader->p_filesz);

            Status = MapPages(VAddress, Address, PageCount);
            if (EFI_ERROR(Status)) {
                Print(L"[ELF ] MapPages failed with %r\r\n", Status);
                return Status;
            }
        }

        ProgramHeader++;
    }

    Print(L"[ELF ] Successfully loaded Kernel. Entry point: 0x%016lx\r\n", Header->e_entry);

    *EntryPoint = (void*)Header->e_entry;
    return EFI_SUCCESS;
}