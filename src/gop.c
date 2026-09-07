#include <efi.h>
#include <efilib.h>
#include <gop.h>

EFI_STATUS GetGOP(EFI_GRAPHICS_OUTPUT_PROTOCOL **GOP) {
    EFI_STATUS Status;

    EFI_GUID GraphicsOutputProtocolGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    Status = uefi_call_wrapper(BS->LocateProtocol,
        3,
        &GraphicsOutputProtocolGUID,
        NULL,
        GOP);

    if (EFI_ERROR(Status)) {
        Print(L"[GOP ] LocateProtocol failed with %r\r\n", Status);
        return Status;
    }
}

EFI_STATUS QueryModeInfo(EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP, UINTN *NativeMode, UINTN *ModeCount) {
    EFI_STATUS Status;
    
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo = NULL;
    UINTN ModeInfoSize = 0;

    Status = uefi_call_wrapper(GOP->QueryMode,
        4,
        GOP,
        GOP->Mode->Mode, &ModeInfoSize, &ModeInfo);

    if (Status == EFI_NOT_STARTED)
        Status = uefi_call_wrapper(GOP->SetMode, 2, GOP, 0);
    
    if (EFI_ERROR(Status)) {
        Print(L"[GOP ] Unable to get native mode: %r\r\n", Status);
        return Status;
    }

    *NativeMode = GOP->Mode->Mode;
    *ModeCount = GOP->Mode->MaxMode;
    
    Print(L"[GOP ] Native mode number: %lu\r\n", *NativeMode);
    Print(L"[GOP ] Listing modes...\r\n");

    for (UINTN i = 0; i < *ModeCount; i++) {
        Status = uefi_call_wrapper(GOP->QueryMode,
            4,
            GOP,
            i,
            &ModeInfoSize,
            &ModeInfo);

        if (EFI_ERROR(Status)) {
            Print(L"[GOP ] Failed to fetch mode (number: %d): %r\r\n", i, Status);
            continue;
        }

        const CHAR16* FormatString;

        switch (ModeInfo->PixelFormat) {
            case PixelRedGreenBlueReserved8BitPerColor:
                FormatString = L"RGBX32";
                break;
            case PixelBlueGreenRedReserved8BitPerColor:
                FormatString = L"BGRX32";
                break;
            case PixelBitMask:
                FormatString = L"BitMask";
                break;
            case PixelBltOnly:
                FormatString = L"BltOnly";
                break;
            case PixelFormatMax:
                FormatString = L"FormatMax";
                break;
            default:
                FormatString = L"Unknown";
                break;
        }

        Print(L"[GOP ] %d: %dx%d %s\r\n",
        i,
        ModeInfo->HorizontalResolution,
        ModeInfo->VerticalResolution,
        FormatString);
    }

    return Status;
}

