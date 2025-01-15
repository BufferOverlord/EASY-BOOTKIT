#include <efi.h>
#include <efilib.h>
#define ENTRY_POINT efi_main
EFI_SYSTEM_TABLE* GlobalSystemTable = NULL;


#define SEARCH_BASE (UINT8*)0x9       // Startadresse
#define SEARCH_SIZE 0x100000000
const UINT8 Signature2[] = { 0x48, 0xB8, 0x77, 0xBE, 0x9F, 0x1A, 0x2F, 0xDD, 0x24, 0x06, 0x49, 0xF7, 0xE1 };

#define SIGNATURE_SIZE (sizeof(Signature2))

void FindSignature(UINT8* MemoryBase, UINTN MemorySize, const UINT8* Signature, UINTN SignatureSize, UINT8** FoundAddress) {
    for (UINT8* ptr = MemoryBase; ptr < MemoryBase + MemorySize - SignatureSize; ++ptr) {
        if (CompareMem(ptr, Signature, SignatureSize) == 0) {
            *FoundAddress = ptr;  
            return;               
        }
    }
    *FoundAddress = NULL;
}




static const EFI_GUID ProtocolGuid
= { 0x1a32111a, 0xee4f, 0x1826, {0x4b, 0x1a, 0x40, 0xb7, 0xff, 0x7f, 0x00, 0xf5} };


static EFI_STATUS EFIAPI efi_unload(IN EFI_HANDLE ImageHandle) {

    return EFI_ACCESS_DENIED;
}


typedef struct _DummyProtocalData {
    UINTN blank;
} DummyProtocalData;


typedef EFI_STATUS(EFIAPI* EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE, UINTN);
static EFI_EXIT_BOOT_SERVICES OriginalExitBootServices = NULL;




static EFI_STATUS EFIAPI HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey) {
    gBS->ExitBootServices = OriginalExitBootServices;
UINT8* FoundAddress = NULL;
    EFI_INPUT_KEY Key;
    FindSignature(SEARCH_BASE, SEARCH_SIZE, Signature2, SIGNATURE_SIZE, &FoundAddress);

    if (FoundAddress != NULL) {
        Print(L"Signature found at address: %p\n", FoundAddress);
    } else {
        Print(L"Signature not found in the specified memory range.\n");
    }

        GlobalSystemTable->ConOut->OutputString(GlobalSystemTable->ConOut, L"Press any key to continue...\r\n");

    while (GlobalSystemTable->ConIn->ReadKeyStroke(GlobalSystemTable->ConIn, &Key) == EFI_NOT_READY) {
        
    }
    
    GlobalSystemTable->ConOut->OutputString(GlobalSystemTable->ConOut, L"Key pressed! Exiting...\r\n");

    return OriginalExitBootServices(ImageHandle, MapKey);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);


    EFI_LOADED_IMAGE* LoadedImage = NULL;
    EFI_STATUS status = BS->OpenProtocol(ImageHandle, &LoadedImageProtocol,
        (void**)&LoadedImage, ImageHandle,
        NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR(status))
    {
        Print(L" Open protocol Failed: %d\n", status);
        return status;
    }


    DummyProtocalData dummy = { 0 };
    status = LibInstallProtocolInterfaces(
        &ImageHandle, &ProtocolGuid,
        &dummy, NULL);

    if (EFI_ERROR(status))
    {
        Print(L"interface create Failed: %d\n", status);
        return status;
    }
    LoadedImage->Unload = (EFI_IMAGE_UNLOAD)efi_unload;

    OriginalExitBootServices = gBS->ExitBootServices;
    gBS->ExitBootServices = HookedExitBootServices;
    return EFI_SUCCESS;
}
