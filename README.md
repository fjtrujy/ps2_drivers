# ps2_drivers
A library for making easier the usage of the IO drivers (`.IRX` + `EE .a`)

## MOTIVATION
This library will make way easier the load/unload and initialization process of the Drivers that requires `IOP` (using an `IRX` file) and `EE` (static library `.a`).

As a side project, I was trying to port `SDL2` for `PlayStation 2`, and meanwhile I was working in this I faced that was so difficult to implement `SDL_main`, and that was the trigger to work on this.

Currenty situation implies:
- First of all the developer need to understand how IOP + EE works, making more difficult to be an atractive platform to new joiners.
- Makefile is more complex. IRX files need to be either embedded with `bin2s` or `bin2o`
- A lot of boiler plate is needed, defining several extern libraries
- It force developers to do copy & paste everytime.
- Error control too messy.

With the solution that I propose within this library, the developer, just need to link the library, and init the desired libraries.

## EXAMPLE
Let me put an example, where we compare `Before` vs `After`. I'm not going to put the error control in the before version otherwise will be too long :)

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


### CONCLUSION
I have tried to follow the KISS concept meanwhile I was implementing this library

## THANKS