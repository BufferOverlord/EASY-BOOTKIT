#include <efi.h>
#include <efilib.h>
#define ENTRY_POINT efi_main










#define SEARCH_BASE (UINT8*)0x0     // Startadresse
#define SEARCH_SIZE 0x100000000      // 4GB

const UINT8 Signature2[] = {
    0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x01, 0x00, 0x00, 0x0E, 0x1F, 0xBA, 0x0E,
    0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C,
    0xCD, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70,
    0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63,
    0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65,
    0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20,
    0x44, 0x4F, 0x53, 0x20, 0x6D, 0x6F, 0x64, 0x65
};

#define SIGNATURE_SIZE (sizeof(Signature2))

void FindSignature(UINT8* MemoryBase, UINTN MemorySize, const UINT8* Signature, UINTN SignatureSize, UINT8** FoundAddress) {
    for (UINT8* ptr = MemoryBase; ptr < MemoryBase + MemorySize - SignatureSize; ++ptr) {
        if (CompareMem(ptr, Signature, SignatureSize) == 0) {
            *FoundAddress = ptr;  // Set the found address
            return;  // Signature found, exit function
        }
    }
    *FoundAddress = NULL;  // No signature found, set found address to NULL
}












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




static EFI_STATUS EFIAPI HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey) {
    gBS->ExitBootServices = OriginalExitBootServices;
    UINT8* foundAddress = NULL;


    FindSignature(SEARCH_BASE, SEARCH_SIZE, Signature2, SIGNATURE_SIZE, &foundAddress);
    
    if (foundAddress != NULL) {
        ST->ConOut->SetAttribute(ST->ConOut, EFI_BACKGROUND_GREEN | EFI_WHITE);
    } else {
        ST->ConOut->SetAttribute(ST->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
    }


	
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
