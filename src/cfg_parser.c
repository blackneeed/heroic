#include <efi.h>
#include <efilib.h>
#include <stdlib.h>
#include <disk.h>

char base_characters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

UINT64 ParseUINT64(const char *StartPtr, UINTN Length, UINTN Base)
{
    UINT64 Curr = 0;

    for (UINTN i = 0; i < Length; i++)
        for (UINTN x = 0; x < Base; x++)
            if (base_characters[x] == StartPtr[i]) {
                Curr = Curr * Base + x;
                break;
            }

    return Curr;
}

EFI_STATUS LoadConfig(EFI_HANDLE ImageHandle, UINT16* FileName) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE Volume;
    Status = GetVolume(ImageHandle, &Volume);

    if (EFI_ERROR(Status)) {
        Print(L"[CFG ] GetVolume failed with %r!\r\n", Status);
        return Status;
    }
    
    UINT8* Buffer;
    UINTN ReadSize;

    Status = ReadFile(Volume, FileName, &Buffer, &ReadSize);
    if (EFI_ERROR(Status)) {
        Print(L"[CFG ] ReadFile failed with %r!\r\n", Status);
        return Status;
    }

    Print(L"[CFG ] Succesfully read %s\r\n", FileName);

    FreePool(Buffer);

    return Status;
}