EE_SRC_DIR = src/
UNPACKED_DIR = unpacked_lib

NETWORK_IOP ?= 0 # Use NETWORK_IOP=1 to use IOP network drivers

ifeq ($(NETWORK_IOP),1)
	EE_CFLAGS += -DNETWORK_IOP
	EE_LIB = libps2_drivers_netIOP.a
else
	EE_LIB = libps2_drivers.a
endif

## EE LIBS
LIBS += -lfileXio
LIBS += -lmtap -lpadx
LIBS += -laudsrv
LIBS += -lpoweroff
LIBS += -lmouse -lkbd -lps2cam

ifeq ($(NETWORK_IOP),1)
	LIBS += -lps2ips
else
	LIBS += -lps2ip -lnetman
endif

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
IRX_FILES += ps2mouse.irx ps2kbd.irx ps2cam.irx

ifeq ($(NETWORK_IOP),1)
	IRX_FILES += ps2ips.irx ps2ip_nm.irx
endif

IRX_FILES += netman.irx smap.irx
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
KEYBOARD_DRIVER_OBJS = internals_ps2_keyboard_driver.o init_ps2_keyboard_driver.o deinit_ps2_keyboard_driver.o
CAMERA_DRIVER_OBJS = internals_ps2_camera_driver.o init_ps2_camera_driver.o deinit_ps2_camera_driver.o
NETMAN_DRIVER_OBJS = internals_ps2_netman_driver.o init_ps2_netman_driver.o deinit_ps2_netman_driver.o
SMAP_DRIVER_OBJS = internals_ps2_smap_driver.o init_ps2_smap_driver.o deinit_ps2_smap_driver.o
ifeq ($(NETWORK_IOP),1)
	IOPIP_DRIVER_OBJS = internals_ps2_iopip_driver.o init_ps2_iopip_driver.o deinit_ps2_iopip_driver.o
else
	EEIP_DRIVER_OBJS = internals_ps2_eeip_driver.o init_ps2_eeip_driver.o deinit_ps2_eeip_driver.o config_network_ps2_eeip_driver.o config_network_custom_ps2_eeip_driver.o
endif

EE_OBJS += $(SIO2MAN_DRIVER_OBJS) $(FILEXIO_DRIVER_OBJS) $(MEMCARD_DRIVER_OBJS) $(USBD_DRIVER_OBJS) $(USB_DRIVER_OBJS) $(CDFS_DRIVER_OBJS) \
	$(DEV9_DRIVER_OBJS) $(HDD_DRIVER_OBJS) $(FILESYSTEM_DRIVER_OBJS) $(JOYSTICK_DRIVER_OBJS) $(AUDIO_DRIVER_OBJS) $(POWEROFF_DRIVER_OBJS) \
	$(MOUSE_DRIVER_OBJS) $(KEYBOARD_DRIVER_OBJS) $(CAMERA_DRIVER_OBJS) $(NETMAN_DRIVER_OBJS) $(SMAP_DRIVER_OBJS) $(EEIP_DRIVER_OBJS) $(IOPIP_DRIVER_OBJS)

# Let's fail if warnings are found
EE_CFLAGS += -Werror

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

# ps2ip-nm.irx is a special case because "-" is not allowed in C identifiers
ps2ip_nm_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/ps2ip-nm.irx $@ ps2ip_nm_irx 

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

%_ps2_keyboard_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_keyboard_driver $(EE_SRC_DIR)ps2_keyboard_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_camera_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_camera_driver $(EE_SRC_DIR)ps2_camera_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_netman_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_netman_driver $(EE_SRC_DIR)ps2_netman_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_smap_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_smap_driver $(EE_SRC_DIR)ps2_smap_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_eeip_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_eeip_driver $(EE_SRC_DIR)ps2_eeip_driver.c -c -o $(EE_OBJS_DIR)$@

%_ps2_iopip_driver.o:
	$(EE_C_COMPILE) -DF_$*_ps2_iopip_driver $(EE_SRC_DIR)ps2_iopip_driver.c -c -o $(EE_OBJS_DIR)$@

#Include preferences
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp
