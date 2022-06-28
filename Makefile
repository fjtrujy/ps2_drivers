EE_LIB = libps2_drivers.a
EE_SRC_DIR = src/
UNPACKED_DIR = unpacked_lib

## EE LIBS
LIBS += -lfileXio
LIBS += -lmtap -lpadx
LIBS += -laudsrv
LIBS_NAME = $(LIBS:-l%=lib%.a)

# IRX libs
IRX_FILES += sio2man.irx
IRX_FILES += iomanX.irx fileXio.irx
IRX_FILES += mcman.irx mcserv.irx
IRX_FILES += usbd.irx bdm.irx bdmfs_vfat.irx usbmass_bd.irx
IRX_FILES += mtapman.irx padman.irx
IRX_FILES += libsd.irx audsrv.irx
EE_OBJS += $(IRX_FILES:.irx=_irx.o)

# Helpers
EE_INCS += -Iinclude
SIO2MAN_DRIVER_OBJS = internals_ps2_sio2man_driver.o init_ps2_sio2man_driver.o deinit_ps2_sio2man_driver.o
FILEXIO_DRIVER_OBJS = internals_ps2_fileXio_driver.o init_ps2_fileXio_driver.o deinit_ps2_fileXio_driver.o
MEMCARD_DRIVER_OBJS = internals_ps2_memcard_driver.o init_ps2_memcard_driver.o deinit_ps2_memcard_driver.o
USB_DRIVER_OBJS = internals_ps2_usb_driver.o init_ps2_usb_driver.o deinit_ps2_usb_driver.o
JOYSTICK_DRIVER_OBJS = internals_ps2_joystick_driver.o init_ps2_joystick_driver.o deinit_ps2_joystick_driver.o
AUDIO_DRIVER_OBJS = internals_ps2_audio_driver.o init_ps2_audio_driver.o deinit_ps2_audio_driver.o

EE_OBJS += $(SIO2MAN_DRIVER_OBJS) $(FILEXIO_DRIVER_OBJS) $(MEMCARD_DRIVER_OBJS) $(USB_DRIVER_OBJS) $(JOYSTICK_DRIVER_OBJS) $(AUDIO_DRIVER_OBJS)

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

install: all
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/include/
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/lib/
	cp -f include/* $(DESTDIR)$(PS2SDK)/ports/include/
	cp $(EE_LIB) $(DESTDIR)$(PS2SDK)/ports/lib/

# IRX files
%_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/$*.irx $@ $*_irx

# EE FUNCTIONS OBJECTS
EE_C_COMPILE = $(EE_CC) $(EE_CFLAGS)

%_ps2_sio2man_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_sio2man_driver $(EE_SRC_DIR)ps2_sio2man_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_fileXio_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_fileXio_driver $(EE_SRC_DIR)ps2_fileXio_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_memcard_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_memcard_driver $(EE_SRC_DIR)ps2_memcard_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_usb_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_usb_driver $(EE_SRC_DIR)ps2_usb_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_joystick_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_joystick_driver $(EE_SRC_DIR)ps2_joystick_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_audio_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_audio_driver $(EE_SRC_DIR)ps2_audio_driver.c -c -o $(EE_OBJS_DIR)$@

#Include preferences
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp