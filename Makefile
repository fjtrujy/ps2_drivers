EE_LIB = libps2_drivers.a

## EE LIBS
UNPACKED_FOLDER = unpacked_lib
LIBS += -lmtap -lpadx
LIBS_NAME = $(LIBS:-l%=lib%.a)

# IRX libs
IRX_FILES += mtapman.irx padman.irx
EE_OBJS += $(IRX_FILES:.irx=_irx.o)

# Helpers
EE_INCS += -Iinclude
EE_OBJS += src/ps2_joystick_driver.o

all: unpack
all: EE_OBJS += $(shell find ${UNPACKED_FOLDER} -name '*.o')
all: $(EE_LIB) 

unpack:
	mkdir -p ${UNPACKED_FOLDER};\
	for lib in ${LIBS_NAME}; do \
        ${EE_AR} -x $(PS2SDK)/ee/lib/$$lib  --output=${UNPACKED_FOLDER};\
    done;

clean:
	rm -rf ${UNPACKED_FOLDER}
	rm -f $(EE_LIB) $(EE_OBJS)

# IRX files
%_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/$*.irx $@ $*_irx

#Include preferences
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp