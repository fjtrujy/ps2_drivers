# ps2_drivers
A library for making it easier to use IO drivers (`.IRX` + `EE .a`)

## MOTIVATION
This library will make it way easier to load/unload, and initialization the process of Drivers that requires `IOP` (using an `IRX` file) and `EE` (static library `.a`).

As a side project, I was trying to port `SDL2` for `PlayStation 2`, and meanwhile I was working on this since I found that it was difficult to implement `SDL_main` as was.

Without this the currenty situation assumes:
- The developer need to understand how IOP + EE works, making it a less atractive platform to newcomers.
- Makefile are more complex. IRX-files need to be either embedded with `bin2s` or `bin2o`.
- A lot of boiler plate is needed, for defining several extern libraries.
- It force developers to do copy & paste everytime.
- Error handeling is too messy.

With the solution that I propose within this library, the developer, just need to link the library, and init the desired libraries.

## EXAMPLE
Let me put an example, where we compare `Before` vs `After`. I'm not going to put the error handeling in the before version otherwise will be too long :)

### BEFORE
`Makefile`
```make

IRX_FILES += sio2man.irx
IRX_FILES += mtapman.irx padman.irx

EE_OBJS += $(IRX_FILES:.irx=_irx.o)

LIBS += -lmtap -lpadx

# IRX files
%_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/$*.irx $@ $*_irx
```

`main.c`
```c

extern unsigned char sio2man_irx[] __attribute__((aligned(16)));
extern unsigned int size_sio2man_irx;

extern unsigned char mtapman_irx[] __attribute__((aligned(16)));
extern unsigned int size_mtapman_irx;

extern unsigned char padman_irx[] __attribute__((aligned(16)));
extern unsigned int size_padman_irx;

void load_modules(void) {
    SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&mtapman_irx, size_mtapman_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, NULL, NULL);
}

void start_libraries(void) {
    mtapInit();
    padInit(0);
}

int main(int argc, char **argv) {
    .......
    load_modules();
    start_libraries();
}
```

### AFTER
`Makefile`
```make
LIBS += -lps2_drivers
```

`main.c`
```c
int main(int argc, char **argv) {
    .......
    init_joystick_driver(true);
}
```

### ALL DRIVERS EXAMPLE
This example will prepare, the next drivers:
- Memory Card 
- FileXio
- USB
- CDFS
- Sound
- Joystick
- Poweroff
- HDD

`Makefile`
```make
LIBS += -lps2_drivers
```

`main.c`
```c
static void prepare_drivers() {
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver(true);
    init_cdfs_driver();
    init_joystick_driver(true);
    init_audio_driver();
    init_poweroff_driver();
    init_hdd_driver(true, true);
}

int main(int argc, char **argv) {
    prepare_drivers();
}
```

### CONCLUSION
I tried to follow the KISS concept while implementing this library

## THANKS
