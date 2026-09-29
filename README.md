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

This library uses CMake as its build system. You need to have PS2DEV and PS2SDK properly set up before building, plus a native host C compiler (`cc`, `gcc`, or `clang`) for the IRX image tooling.

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
- `libps2_drivers_img.a` - External-image flavor with no embedded IRX payload arrays
- `ps2_drivers_irximg_bootstrap.elf` - Standalone USB bootstrap for launchers that reset the IOP before starting the real ELF
- `ps2_drivers.irximg` - A deterministic, validated container with the canonical IRX set
- Sample executables in `build/samples/*/` directories (if BUILD_SAMPLES=ON)

The current `libps2_drivers.a` behavior is unchanged and still embeds the IRX payloads selected by the final application link. The optional `libps2_drivers_img.a` flavor keeps those payloads out of the ELF and loads them from `ps2_drivers.irximg` through temporary EE staging memory.

The image tooling can be checked explicitly with:

```bash
cmake --build . --target ps2_drivers_irximg_check
cmake --build . --target ps2_drivers_irximg_stage_check
cmake --build . --target ps2_drivers_irximg_inspect
cmake --build . --target ps2_drivers_img_payload_check
```

After installation:

- both libraries are under `$PS2SDK/ports/lib/`;
- headers are under `$PS2SDK/ports/include/`;
- pkg-config metadata is available as `ps2_drivers` and `ps2_drivers-img`;
- the companion image and `ps2_drivers_irximg_bootstrap.elf` are installed under `$PS2SDK/ports/share/ps2_drivers/`.

### External IRX image flavor

The external flavor is useful when EE RAM is more valuable than keeping IRX source bytes permanently embedded in the application ELF.

The required order is:

1. link against `libps2_drivers_img.a`;
2. deploy `ps2_drivers.irximg` beside the ELF and run with that directory as the current working directory;
3. call one of the `ps2_drivers_img_stage_*()` helpers **before** resetting the IOP;
4. reset/sync the IOP and apply the usual SBV patches;
5. call the existing `init_*_driver()` APIs normally.

For example:

```c
#include <ps2_drivers_img.h>
#include <ps2_joystick_driver.h>

if (ps2_drivers_img_stage_default(
        PS2_DRIVER_REQ_JOYSTICK) != PS2_DRIVERS_IMG_OK) {
    /* handle error before resetting the IOP */
}

/* SifIopReset / SifIopSync / SifInitRpc / SBV patches */

if (init_joystick_driver(true) != JOYSTICK_INIT_STATUS_OK) {
    /* handle driver initialization error */
}
```

The staging API expands driver requirements into their transitive IRX dependencies and deduplicates them. Each module's temporary EE buffer is freed immediately after its `SifExecModuleBuffer()` call returns.

For applications that use `init_only_boot_ps2_filesystem_driver()`, there is no need to duplicate boot-device detection or filesystem dependency logic. Stage the current boot filesystem plus only the application's additional drivers:

```c
if (ps2_drivers_img_stage_default_for_current_boot(
        PS2_DRIVER_REQ_AUDIO |
        PS2_DRIVER_REQ_JOYSTICK) != PS2_DRIVERS_IMG_OK) {
    /* handle error before resetting the IOP */
}

/* reset/sync the IOP and apply SBV patches */

init_only_boot_ps2_filesystem_driver();
init_audio_driver();
init_joystick_driver(true);
```

If the application uses `init_ps2_filesystem_driver()` instead, use `ps2_drivers_img_stage_default_for_all_filesystems(additional_requirements)`.

Some ELF loaders reset the IOP before transferring control to the application. When an ELF is launched from `mass:` this removes the USB filesystem before the application can stage `ps2_drivers.irximg`. For that launch mode, deploy `ps2_drivers_irximg_bootstrap.elf` as the entry ELF instead of embedding an emergency copy of the USB drivers in the real application.

Place an `elf_path.ini` beside the bootstrap containing the real ELF path, for example:

```text
MVS.elf
```

The bootstrap embeds only `iomanX`, `fileXio`, `bdm`, `bdmfs_fatfs`, `usbd`, and `usbmass_bd`. It restores `mass:`, then starts the configured ELF with PS2SDK's `LoadELFFromFileWithPartitionNoReset()`. The real application can therefore stage `ps2_drivers.irximg` before performing its own definitive IOP reset, while remaining free of embedded IRX payloads itself. The target may alternatively be supplied as the bootstrap's first argument; remaining arguments are forwarded to the target ELF.

The convenience API also provides:

- `ps2_drivers_img_stage_default()` for an explicit requirement set using the standard relative image name;
- `ps2_drivers_img_stage_for_boot_device()` and `ps2_drivers_img_stage_default_for_boot_device()` when the boot device is already known;
- `ps2_drivers_img_stage_for_path()` and `ps2_drivers_img_stage_default_for_path()` when a launch path is already known;
- `ps2_drivers_img_requirements_for_boot_device()`, `ps2_drivers_img_requirements_for_path()`, and `ps2_drivers_img_requirements_for_current_boot()` when an application only wants the resolved mask.
- `ps2_drivers_img_error_string()` for readable diagnostics without duplicating the error-code switch.

After a successful initial stage, the image flavor remembers only the image path. If a driver genuinely unloads its IOP modules and later needs to initialize them again, call:

```c
deinit_joystick_driver(true);

if (ps2_drivers_img_restage_last() != PS2_DRIVERS_IMG_OK) {
    /* the remembered image source must currently be accessible */
}

init_joystick_driver(true);
```

`ps2_drivers_img_restage_last()` repeats the last successful staging request. `ps2_drivers_img_restage(requirements)` remains available when a different set is needed. Neither retains IRX payloads between initializations. `ps2_drivers_img_discard_staged()` clears temporary buffers while preserving the remembered source path; call `ps2_drivers_img_forget_source()` when the path should also be released.

The API deliberately uses a relative filename rather than a concrete device prefix. The current convention is that `ps2_drivers.irximg` lives beside the ELF and that the ELF directory is the process working directory. An IOP reset may remove the filesystem/device stack that made that directory accessible, so initial staging must still happen before reset. A later explicit restage is only possible once the same relative path is accessible again.

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
