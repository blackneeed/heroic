#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <string.h>
#include <page.h>

EFI_STATUS LoadELF(void* Buffer_, void** EntryPoint) {
    char* Buffer = (char*)Buffer_;
    EFI_STATUS Status;
    Elf64_Ehdr* hdr = Buffer_;

    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || 
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3) {
        Print(L"[ELF ] Kernel file is not a valid ELF file\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64) {
        Print(L"[ELF ] Kernel ELF is 32-bit while heroic is 64-bit\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        Print(L"[ELF ] Kernel ELF is MSB\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_type != ET_EXEC) {
        Print(L"[ELF ] Kernel ELF is not executable\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_machine != EM_X86_64) {
        Print(L"[ELF ] Kernel ELF is not compiled for x86_64\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_version != EV_CURRENT) {
        Print(L"[ELF ] Kernel ELF has an old ELF version\r\n");
        return EFI_LOAD_ERROR;
    }

    Elf64_Phdr* Phdr = (Elf64_Phdr*)&Buffer[hdr->e_phoff];

    while ((uint64_t)Phdr - (uint64_t)&Buffer[hdr->e_phoff] < hdr->e_phentsize * hdr->e_phnum) {
        if (Phdr->p_type == PT_LOAD) {
            uint64_t PageOffset = Phdr->p_vaddr & 0xFFF;
            int PageCount = (PageOffset + Phdr->p_memsz + 0xFFF) / 0x1000;

            uint64_t VAddress = Phdr->p_vaddr & ~0xFFF;
            uint64_t Address = 0;

            Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, PageCount, &Address);
            if (EFI_ERROR(Status)) {
                Print(L"[ELF ] Could not allocate pages for Phdr\r\n");
                return Status;
            }

            memcpy((void*)(Address + PageOffset), (void*)&Buffer[Phdr->p_offset], Phdr->p_filesz); // cp filesz bytes from the file
            memset((void*)(Address + PageOffset + Phdr->p_filesz), 0, Phdr->p_memsz - Phdr->p_filesz); // zero out the extra memsz bytes

            Status = MapPages(VAddress, Address, PageCount);
            if (EFI_ERROR(Status)) {
                Print(L"[ELF ] MapPages failed with %r\r\n", Status);
                return Status;
            }
        }

        Phdr++;
    }

    Print(L"[ELF ] Successfully loaded Kernel. Entry point: 0x%016lx\r\n", hdr->e_entry);

    *EntryPoint = (void*)hdr->e_entry;
    return EFI_SUCCESS;
}