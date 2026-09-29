EE_CC := $(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-gcc
EE_AR := $(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-ar
PS2SDK ?= $(PS2DEV)/ps2sdk
CFLAGS := -D_EE -G0 -O2 -Wall -Werror -I../../include -I../common -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

.PHONY: all clean
all: lib/libcacheio.a
lib/libcacheio.a: obj/cacheio_rpc.o
	@mkdir -p $(@D)
	$(EE_AR) cru $@ $^
obj/cacheio_rpc.o: src/cacheio_rpc.c ../../include/cacheio.h ../common/cacheio-common.h
	@mkdir -p $(@D)
	$(EE_CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf obj lib
