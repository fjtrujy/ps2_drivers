EE_LIB = libps2_drivers.a
EE_SRC_DIR = src/
UNPACKED_DIR = unpacked_lib

## EE LIBS
LIBS += -lfileXio
LIBS += -lmtap -lpadx
LIBS += -laudsrv
LIBS += -lpoweroff
LIBS += -lmouse
LIBS_NAME = $(LIBS:-l%=lib%.a)

# IRX libs
IRX_FILES += sio2man.irx
IRX_FILES += iomanX.irx fileXio.irx
IRX_FILES += mcman.irx mcserv.irx
IRX_FILES += bdm.irx bdmfs_fatfs.irx usbd.irx usbmass_bd.irx
IRX_FILES += cdfs.irx
IRX_FILES += ps2dev9.irx ps2atad.irx ps2hdd.irx ps2fs.irx
IRX_FILES += mtapman.irx padman.irx
IRX_FILES += libsd.irx audsrv.irx
IRX_FILES += poweroff.irx
IRX_FILES += ps2mouse.irx
EE_OBJS += $(IRX_FILES:.irx=_irx.o)

# Helpers
EE_INCS += -Iinclude
SIO2MAN_DRIVER_OBJS = internals_ps2_sio2man_driver.o init_ps2_sio2man_driver.o deinit_ps2_sio2man_driver.o
FILEXIO_DRIVER_OBJS = internals_ps2_fileXio_driver.o init_ps2_fileXio_driver.o deinit_ps2_fileXio_driver.o
MEMCARD_DRIVER_OBJS = internals_ps2_memcard_driver.o init_ps2_memcard_driver.o deinit_ps2_memcard_driver.o
USBD_DRIVER_OBJS = internals_ps2_usbd_driver.o init_ps2_usbd_driver.o deinit_ps2_usbd_driver.o
USB_DRIVER_OBJS = internals_ps2_usb_driver.o init_ps2_usb_driver.o deinit_ps2_usb_driver.o
CDFS_DRIVER_OBJS = internals_ps2_cdfs_driver.o init_ps2_cdfs_driver.o deinit_ps2_cdfs_driver.o
DEV9_DRIVER_OBJS = internals_ps2_dev9_driver.o init_ps2_dev9_driver.o deinit_ps2_dev9_driver.o
HDD_DRIVER_OBJS = internals_ps2_hdd_driver.o init_ps2_hdd_driver.o deinit_ps2_hdd_driver.o \
	mount_hdd_partition_ps2_hdd_driver.o mount_current_partition_ps2_hdd_driver.o \
	umount_hdd_partition_ps2_hdd_driver.o umount_current_partition_ps2_hdd_driver.o
FILESYSTEM_DRIVER_OBJS = waitUntilDeviceIsReady_ps2_filesystem_driver.o rootDevicePath_ps2_filesystem_driver.o getBootDeviceID_ps2_filesystem_driver.o \
	__internal_deinit_ps2_filesystem_driver.o deinit_ps2_filesystem_driver.o init_ps2_filesystem_driver.o
JOYSTICK_DRIVER_OBJS = internals_ps2_joystick_driver.o init_ps2_joystick_driver.o deinit_ps2_joystick_driver.o
AUDIO_DRIVER_OBJS = internals_ps2_audio_driver.o init_ps2_audio_driver.o deinit_ps2_audio_driver.o
POWEROFF_DRIVER_OBJS = internals_ps2_poweroff_driver.o init_ps2_poweroff_driver.o deinit_ps2_poweroff_driver.o
MOUSE_DRIVER_OBJS = internals_ps2_mouse_driver.o init_ps2_mouse_driver.o deinit_ps2_mouse_driver.o

EE_OBJS += $(SIO2MAN_DRIVER_OBJS) $(FILEXIO_DRIVER_OBJS) $(MEMCARD_DRIVER_OBJS) $(USBD_DRIVER_OBJS) $(USB_DRIVER_OBJS) $(CDFS_DRIVER_OBJS) \
	$(DEV9_DRIVER_OBJS) $(HDD_DRIVER_OBJS) $(FILESYSTEM_DRIVER_OBJS) $(JOYSTICK_DRIVER_OBJS) $(AUDIO_DRIVER_OBJS) $(POWEROFF_DRIVER_OBJS) \
	$(MOUSE_DRIVER_OBJS)

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

%_ps2_usbd_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_usbd_driver $(EE_SRC_DIR)ps2_usbd_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_usb_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_usb_driver $(EE_SRC_DIR)ps2_usb_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_cdfs_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_cdfs_driver $(EE_SRC_DIR)ps2_cdfs_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_dev9_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_dev9_driver $(EE_SRC_DIR)ps2_dev9_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_hdd_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_hdd_driver $(EE_SRC_DIR)ps2_hdd_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_filesystem_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_filesystem_driver $(EE_SRC_DIR)ps2_filesystem_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_joystick_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_joystick_driver $(EE_SRC_DIR)ps2_joystick_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_audio_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_audio_driver $(EE_SRC_DIR)ps2_audio_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_poweroff_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_poweroff_driver $(EE_SRC_DIR)ps2_poweroff_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_mouse_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_mouse_driver $(EE_SRC_DIR)ps2_mouse_driver.c -c -o $(EE_OBJS_DIR)$@

#Include preferences
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp