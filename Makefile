EE_LIB = libps2_drivers.a
EE_SRC_DIR = src/
UNPACKED_DIR = unpacked_lib

## EE LIBS

LIBS += -lmtap -lpadx
LIBS_NAME = $(LIBS:-l%=lib%.a)

# IRX libs
IRX_FILES += mtapman.irx padman.irx
EE_OBJS += $(IRX_FILES:.irx=_irx.o)

# Helpers
EE_INCS += -Iinclude

JOYSTICK_DRIVER_OBJS = internals_ps2_joystick_driver.o init_ps2_joystick_driver.o deinit_ps2_joystick_driver.o
EE_OBJS += $(JOYSTICK_DRIVER_OBJS)

## ALL ACTIONS
all: prepare
all: unpack
all: EE_OBJS += $(shell find ${UNPACKED_DIR} -name '*.o')
all: $(EE_LIB) 

prepare:
	mkdir -p ${UNPACKED_DIR};\

unpack:
	for lib in ${LIBS_NAME}; do \
        ${EE_AR} -x $(PS2SDK)/ee/lib/$$lib  --output=${UNPACKED_DIR};\
    done;

clean:
	rm -rf ${UNPACKED_DIR}
	rm -f $(EE_LIB) $(EE_OBJS)

# IRX files
%_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/$*.irx $@ $*_irx

# EE FUNCTIONS OBJECTS
EE_C_COMPILE = $(EE_CC) $(EE_CFLAGS)

%_ps2_joystick_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_joystick_driver $(EE_SRC_DIR)ps2_joystick_driver.c -c -o $(EE_OBJS_DIR)$@

#Include preferences
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp