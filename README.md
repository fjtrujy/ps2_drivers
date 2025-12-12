# ps2_drivers

A library for making it easier to use IO drivers (`.IRX` + `EE .a`) on PlayStation 2.

## MOTIVATION

This library will make it way easier to load/unload and initialize the process of Drivers that requires `IOP` (using an `IRX` file) and `EE` (static library `.a`).

As a side project, I was trying to port `SDL2` for `PlayStation 2`, and meanwhile, I was working on this since I found that it was difficult to implement `SDL_main` as was.

Without this, the current situation assumes:

- The developer needs to understand how IOP + EE works, making it a less attractive platform to newcomers.
- Makefiles are more complex. IRX files need to be either embedded with `bin2s` or `bin2o`.
- A lot of boilerplate is needed, for defining several extern libraries.
- It forces developers to copy & paste every time.
- Error handling is too messy.

With the solution that I propose within this library, the developer just needs to link the library and initiate the desired libraries.

## AVAILABLE DRIVERS

| Driver | Function | Description |
|--------|----------|-------------|
| Audio | `init_audio_driver()` | Sound output via audsrv |
| Camera | `init_camera_driver(bool)` | EyeToy camera support |
| CDFS | `init_cdfs_driver()` | CD/DVD filesystem |
| DEV9 | `init_dev9_driver()` | Expansion bay device (HDD/Network) |
| FileXio | `init_fileXio_driver()` | Extended file I/O |
| Filesystem | `init_ps2_filesystem_driver()` | Combined filesystem driver (all storage devices) |
| HDD | `init_hdd_driver(bool, bool)` | Hard disk drive support |
| Joystick | `init_joystick_driver(bool)` | Controller input with multitap support |
| Keyboard | `init_keyboard_driver(bool)` | USB keyboard support |
| Memory Card | `init_memcard_driver(bool)` | Memory card access |
| Mouse | `init_mouse_driver(bool)` | USB mouse support |
| MX4SIO | `init_mx4sio_driver(bool)` | SD card via memory card slot adapter |
| Netman | `init_netman_driver()` | Network manager |
| Poweroff | `init_poweroff_driver()` | Power-off callback handling |
| SIO2MAN | `init_sio2man_driver()` | Serial I/O manager (dependency for many drivers) |
| SMAP | `init_smap_driver()` | Network adapter (Ethernet) |
| USB | `init_usb_driver(bool)` | USB mass storage support |
| USBD | `init_usbd_driver()` | USB host controller |

Most drivers accept a `bool` parameter to automatically initialize their dependencies.

## BUILDING

This library uses CMake as its build system. You need to have PS2DEV and PS2SDK properly set up before building.

### Prerequisites

```bash
export PS2DEV=/path/to/ps2dev
export PS2SDK=$PS2DEV/ps2sdk
export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin
```

### Build and Install

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the library and samples
make -j$(nproc)

# Install to PS2SDK
make install
```

### Build Options

- `BUILD_SAMPLES` (default: ON) - Build sample projects

To disable building samples:

```bash
cmake .. -DBUILD_SAMPLES=OFF
```

### Build Output

The build process will generate:
- `libps2_drivers.a` - The main library combining all drivers and PS2SDK libraries
- Sample executables in `build/samples/*/` directories (if BUILD_SAMPLES=ON)

After installation, the library will be available at `$PS2SDK/ports/lib/` and headers at `$PS2SDK/ports/include/`.

## EXAMPLE

Let me put an example, where we compare `Before` vs `After`. I'm not going to put the error handling in the before version otherwise will be too long :)

### BEFORE

`CMakeLists.txt`

```cmake
# Complex setup required for IRX embedding
add_custom_command(OUTPUT sio2man_irx.c
    COMMAND ${BIN2C} ${PS2SDK}/iop/irx/sio2man.irx sio2man_irx.c sio2man_irx)
add_custom_command(OUTPUT mtapman_irx.c
    COMMAND ${BIN2C} ${PS2SDK}/iop/irx/mtapman.irx mtapman_irx.c mtapman_irx)
add_custom_command(OUTPUT padman_irx.c
    COMMAND ${BIN2C} ${PS2SDK}/iop/irx/padman.irx padman_irx.c padman_irx)

target_sources(your_target PRIVATE sio2man_irx.c mtapman_irx.c padman_irx.c)
target_link_libraries(your_target PRIVATE mtap padx)
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
    // ...
    load_modules();
    start_libraries();
}
```

### AFTER

`CMakeLists.txt`

```cmake
target_link_libraries(your_target PRIVATE ps2_drivers)
```

`main.c`

```c
#include <ps2_joystick_driver.h>

int main(int argc, char **argv) {
    // ...
    init_joystick_driver(true);  // true = auto-init dependencies
}
```

### ALL DRIVERS EXAMPLE

This example initializes the most common drivers:

- FileXio (extended file I/O)
- Memory Card
- USB Mass Storage
- CDFS (CD/DVD)
- Joystick (with multitap)
- Audio
- Poweroff
- HDD

`CMakeLists.txt`

```cmake
target_link_libraries(your_target PRIVATE ps2_drivers)
```

`main.c`

```c
#include <ps2_all_drivers.h>

static void init_drivers() {
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver(true);
    init_cdfs_driver();
    init_joystick_driver(true);
    init_audio_driver();
    init_poweroff_driver();
    init_hdd_driver(true, true);
}

static void deinit_drivers() {
    deinit_poweroff_driver();
    deinit_audio_driver();
    deinit_joystick_driver(false);
    deinit_usb_driver(false);
    deinit_cdfs_driver();
    deinit_memcard_driver(true);
    deinit_hdd_driver(false);
    deinit_fileXio_driver();
}

int main(int argc, char **argv) {
    init_drivers();
    // Your code here
    deinit_drivers();
    return 0;
}
```

### FILESYSTEM DRIVER EXAMPLE

For a simpler approach, use the unified filesystem driver that handles all storage devices:

```c
#include <ps2_filesystem_driver.h>

int main(int argc, char **argv) {
    // Initialize all filesystem drivers (MC, USB, CDFS, HDD, MX4SIO)
    init_ps2_filesystem_driver();

    // Wait for a specific device to be ready
    waitUntilDeviceIsReady("mass:");

    // Your code here

    deinit_ps2_filesystem_driver();
    return 0;
}
```

### CONCLUSION

I tried to follow the KISS concept while implementing this library.

## SAMPLES

The repository includes several sample projects to demonstrate driver usage:

| Sample | Description |
|--------|-------------|
| `alldrivers_sample` | Basic initialization of all common drivers |
| `alldrivers_verbose_sample` | Verbose output during driver initialization |
| `filesystem_sample` | Filesystem driver usage |
| `hdd_sample` | HDD partition mounting |
| `listdir_sample` | Directory listing example |
| `mtap_sample` | Multitap controller support |
| `poweroff_sample` | Power-off callback handling |
| `wav_sample` | Audio playback example |
| `basic_elf_loader` | ELF loader example |
| `showiopinfo_sample` | IOP module information |

Build samples with:

```bash
cmake .. -DBUILD_SAMPLES=ON
make
```

## LICENSE

Licensed under GNU Library General Public License version 2. See LICENSE file for details.

## THANKS
