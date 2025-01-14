# Based on https://wiki.osdev.org/GNU-EFI#Libraries
#
ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

CC              = gcc
OBJS            = EFI_MAIN.o
TARGET          = driver.efi

EFIINC          = gnu-efi/inc

EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol

LIB             = gnu-efi/$(ARCH)/lib
EFILIB          = gnu-efi/$(ARCH)/gnuefi

EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = gnu-efi/gnuefi/elf_$(ARCH)_efi.lds

CFLAGS          = $(EFIINCS) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args

ifeq ($(ARCH),x86_64)
        CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS)

all: prepare $(TARGET)

driver.so: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .rodata \
	-j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* \
	 -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $^ $@

.PHONY: clean prepare

prepare:
	@test ! -d gnu-efi && (git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi && make -C gnu-efi) || true

clean:
	rm -f $(TARGET) *.o *.so
