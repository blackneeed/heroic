#include <elf.h>
#include <efi.h>
#include <efilib.h>
#include <mmap.h>
#include <disk.h>
#include <string.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;
    
    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[DISK] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    Print(L"[DISK] Volume handle obtained\r\n");

    char* Buffer;
    uint64_t ReadSize;

    Status = ReadFile(Volume, L"kernel.elf", &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[DISK] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    Elf64_Ehdr* hdr = (void*)Buffer;

    if (hdr->e_ident[EI_MAG0] == ELFMAG0 && 
        hdr->e_ident[EI_MAG1] == ELFMAG1 &&
        hdr->e_ident[EI_MAG2] == ELFMAG2 &&
        hdr->e_ident[EI_MAG3] == ELFMAG3) {
        Print(L"[ELF] Kernel file is a valid ELF file\r\n");
    } else {
        Print(L"[ELF] Kernel file is not a valid ELF file\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64) {
        Print(L"[ELF] Kernel ELF is 32-bit while heroic is 64-bit\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        Print(L"[ELF] Kernel ELF is MSB\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_type != ET_EXEC) {
        Print(L"[ELF] Kernel ELF is not executable\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_machine != EM_X86_64) {
        Print(L"[ELF] Kernel ELF is not compiled for x86_64\r\n");
        return EFI_LOAD_ERROR;
    }

    if (hdr->e_version != EV_CURRENT) {
        Print(L"[ELF] Kernel ELF has an old ELF version\r\n");
        return EFI_LOAD_ERROR;
    }

    Elf64_Phdr* Phdr = (Elf64_Phdr*)&Buffer[hdr->e_phoff];

    while ((uint64_t)Phdr - (uint64_t)&Buffer[hdr->e_phoff] < hdr->e_phentsize * hdr->e_phnum) {
        if (Phdr->p_type == PT_LOAD) {
            int PageCount = (Phdr->p_memsz + 0xFFF) / 0x1000;

            uint64_t Address = 0;

            Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, PageCount, &Address);
            if (EFI_ERROR(Status)) {
                Print(L"[ELF] Could not allocate pages for Phdr\r\n");
                return Status;
            }

            memcpy((void*)Address, (void*)&Buffer[Phdr->p_offset], Phdr->p_filesz);

            Print(L"[ELF] PHDR type PT_LOAD Vaddr 0x%lx Paddr 0x%lx loaded at 0x%lx (no paging yet)\r\n", Phdr->p_vaddr, Phdr->p_paddr, Address);
        } else {
            Print(L"[ELF] Ignoring program header with type %d\r\n", Phdr->p_type);
        }

        Phdr++;
    }

    EFI_MEMORY_DESCRIPTOR* Map;
    uint64_t MapSize, MapKey;
	uint64_t DescriptorSize;
	uint32_t DescriptorVersion;    

    Status = FetchMemoryMap(&Map, &MapSize, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        Print(L"[MMAP] FetchMemoryMap failed with %r!\r\n", Status);
        return Status;
    }

    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Print(L"[BS] ExitBootServices failed with %r!\r\n", Status);
        return Status;
    }

    return Status;
}