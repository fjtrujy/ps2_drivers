IOP_PREFIX := $(PS2DEV)/iop/bin/mipsel-none-elf-
IOP_CC := $(IOP_PREFIX)gcc
IOP_STRIP := $(IOP_PREFIX)strip
PS2SDK ?= $(PS2DEV)/ps2sdk
CFLAGS := -D_IOP -fno-builtin -G0 -Os -Wall -Werror -gdwarf-2 -gz -msoft-float -mno-explicit-relocs \
	-DIOMANX_OLD_NAME_COMPATIBILITY=0 -DIOMANX_OLD_NAME_ADDDELDRV=0 \
	-I../common -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include
IMPORT_CFLAGS := $(CFLAGS) -fno-toplevel-reorder
LDFLAGS := -nostdlib -dc -r

.PHONY: all clean
all: irx/cacheio.irx
obj/cacheio.o: src/cacheio.c ../common/cacheio-common.h
	@mkdir -p $(@D)
	$(IOP_CC) $(CFLAGS) -c $< -o $@
obj/build-imports.c: src/irx_imports.h src/imports.lst
	@mkdir -p $(@D)
	printf '%s\n' '#include "irx_imports.h"' > $@
	cat src/imports.lst >> $@
obj/imports.o: obj/build-imports.c
	$(IOP_CC) $(IMPORT_CFLAGS) -Isrc -c $< -o $@
irx/cacheio.notiopmod.elf: obj/cacheio.o obj/imports.o $(PS2SDK)/iop/lib/libbdm.a
	@mkdir -p $(@D)
	$(IOP_CC) -T$(PS2SDK)/iop/startup/linkfile -Os -o $@ $^ $(LDFLAGS) -lgcc
irx/cacheio.notiopmod.stripped.elf: irx/cacheio.notiopmod.elf
	$(IOP_STRIP) --strip-unneeded --remove-section=.pdr --remove-section=.comment --remove-section=.mdebug.abi32 --remove-section=.gnu.attributes -o $@ $<
irx/cacheio.irx: irx/cacheio.notiopmod.stripped.elf
	$(PS2SDK)/bin/srxfixup --rb --irx1 --allow-zero-text -o $@ $<
clean:
	rm -rf obj irx
