#include <efi.h>
#include <efilib.h>
#define ENTRY_POINT efi_main
EFI_SYSTEM_TABLE* GlobalSystemTable = NULL;



EFI_STATUS
EFIAPI
SearchMemorySignatureInFirst2GB(EFI_SYSTEM_TABLE* SystemTable, UINT8* Signature, UINTN SignatureSize) {
    EFI_STATUS Status;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    EFI_MEMORY_DESCRIPTOR* MemoryDescriptor;
    UINTN MemoryMapSize = 0;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    UINTN Index;
    UINTN MaxSearchSize = 2 * 1024 * 1024 * 1024;

    Status = SystemTable->BootServices->GetMemoryMap(
        &MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    MemoryMap = AllocatePool(MemoryMapSize);
    if (MemoryMap == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = SystemTable->BootServices->GetMemoryMap(
        &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    if (EFI_ERROR(Status)) {
        FreePool(MemoryMap);
        return Status;
    }

    MemoryDescriptor = (EFI_MEMORY_DESCRIPTOR*)MemoryMap;
    for (Index = 0; Index < MemoryMapSize / DescriptorSize; Index++) {
        EFI_MEMORY_TYPE MemType = MemoryDescriptor->Type;
        EFI_PHYSICAL_ADDRESS MemStart = MemoryDescriptor->PhysicalStart;
        UINTN MemSize = MemoryDescriptor->NumberOfPages * EFI_PAGE_SIZE;

        if (MemStart < MaxSearchSize && MemType != EfiConventionalMemory && MemType != EfiBootServicesData) {
            for (UINTN Offset = 0; Offset < MemSize - SignatureSize; Offset++) {
                UINT8* Addr = (UINT8*)(MemStart + Offset);
                if (CompareMem(Addr, Signature, SignatureSize) == 0) {
                    FreePool(MemoryMap);
                    return EFI_SUCCESS;
                }
            }
        }

        MemoryDescriptor = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemoryDescriptor + DescriptorSize);
    }

    FreePool(MemoryMap);
    return EFI_NOT_FOUND;
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
    EFI_STATUS Status;
    UINT8 Signature[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    UINTN SignatureSize = sizeof(Signature);

    Status = SearchMemorySignatureInFirst2GB(GlobalSystemTable, Signature, SignatureSize);

    if (EFI_ERROR(Status)) {
        GlobalSystemTable->ConOut->SetAttribute(GlobalSystemTable->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
    }
    else {
        GlobalSystemTable->ConOut->SetAttribute(GlobalSystemTable->ConOut, EFI_BACKGROUND_GREEN | EFI_WHITE);
    }





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



    ST->ConOut->ClearScreen(ST->ConOut);
    OriginalExitBootServices = gBS->ExitBootServices;
    gBS->ExitBootServices = HookedExitBootServices;
    return EFI_SUCCESS;
}
