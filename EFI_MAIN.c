#include <efi.h>
#include <efilib.h>
#define ENTRY_POINT efi_main

#define MEMORY_START 0x00000000
#define MEMORY_END   0x40000000
#define SIGNATURE {0x48, 0xB8, 0x77, 0xBE, 0x9F, 0x1A, 0x2F, 0xDD, 0x24, 0x06, 0x49, 0xF7, 0xE1}


static const EFI_GUID ProtocolGuid
	= { 0x1a32111a, 0xee4f, 0x1826, {0x4b, 0x1a, 0x40, 0xb7, 0xff, 0x7f, 0x00, 0xf5} };


static EFI_STATUS EFIAPI efi_unload(IN EFI_HANDLE ImageHandle) {

    return EFI_ACCESS_DENIED;
}





typedef struct _DummyProtocalData{
	UINTN blank;
} DummyProtocalData;


typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE, UINTN);
static EFI_EXIT_BOOT_SERVICES OriginalExitBootServices = NULL;


// Winload.exe --> OslpLogOsLaunch
static EFIAPI void winloadSignature(void) {
    static UINT8 signature[] = SIGNATURE;
    static UINTN sigLength = sizeof(signature);
    UINT8 *current = (UINT8 *)MEMORY_START;
    UINT8 *end = (UINT8 *)MEMORY_END - sigLength;

    BOOLEAN signatureFound = FALSE;

    while (current <= end) {
        UINTN i = 0;
        while (i < sigLength && current[i] == signature[i]) {
            i++;
        }
        if (i == sigLength) {
            signatureFound = TRUE;
            ST->ConOut->SetAttribute(ST->ConOut, EFI_BACKGROUND_GREEN | EFI_WHITE);
            break;
        }
        current++;
    }
}

static EFI_STATUS EFIAPI HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey) {
    gBS->ExitBootServices = OriginalExitBootServices;
    winloadSignature();
    return OriginalExitBootServices(ImageHandle, MapKey);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
     InitializeLib(ImageHandle, SystemTable);
    

EFI_LOADED_IMAGE *LoadedImage = NULL;
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



ST->ConOut->ClearScreen(ST->ConOut);
    OriginalExitBootServices = gBS->ExitBootServices;
    gBS->ExitBootServices = HookedExitBootServices;
    return EFI_SUCCESS;
}
