# External IRX Image Flavor Plan

## Goal

Add an optional ps2_drivers flavor that keeps IRX payloads out of the application ELF after startup.

The existing flavor remains the default and keeps its current convenience model: applications link `libps2_drivers.a`, the required IRX objects are pulled from the archive, and driver initialization calls `SifExecModuleBuffer()` on those embedded buffers.

The new flavor will instead ship a companion `ps2_drivers.irximg` file containing the IRX payloads. Before an IOP reset, the application stages the IRX modules it will need into temporary EE heap memory. After the reset, normal driver initialization executes those staged buffers. Each temporary EE buffer is released as soon as `SifExecModuleBuffer()` returns, so the IRX bytes do not remain resident in the application ELF for the lifetime of the process.

This is intended to reduce steady-state EE memory consumption without changing the public driver initialization APIs.

## Why the idea is viable

PS2SDK's current `SifExecModuleBuffer()` implementation:

1. receives an IRX image from EE memory;
2. allocates temporary IOP heap memory;
3. DMA-copies the IRX into IOP memory;
4. asks LOADFILE to load/start the module from that IOP buffer;
5. frees the temporary IOP heap allocation before returning.

Therefore the EE source buffer is no longer required once `SifExecModuleBuffer()` returns and can safely be released by the external-image provider.

The important bootstrapping constraint is the IOP reset. A reset removes the IOP-side filesystem stack that may be required to access USB, HDD, MX4SIO, and other devices. The companion image therefore cannot generally be opened for the first time after the reset. The required modules must be read into EE memory while the boot filesystem is still available, before resetting the IOP.

The lifecycle is consequently:

```
boot environment still owns usable filesystem drivers
        |
        v
open ps2_drivers.irximg
stage required IRXs into temporary EE heap buffers
        |
        v
SifIopReset / SifIopSync
reinitialize SIF + SBV patches
        |
        v
existing init_*_driver() calls
        |
        +--> execute staged IRX
        +--> SifExecModuleBuffer() returns
        +--> immediately free that IRX's EE staging buffer
        |
        v
all requested drivers initialized
steady-state EE memory no longer contains IRX payloads
```

## Important current behavior to preserve

The current archive contains every generated IRX object, but a final application ELF does not necessarily contain every IRX in the archive. Because each generated IRX is a separate object and driver code references only the modules it uses, normal static-link archive extraction pulls only referenced IRX objects.

The memory saving of the new flavor is therefore the sum of the IRX payloads actually linked by a given application, not automatically the size of every IRX in `libps2_drivers.a`.

The new implementation must preserve this granularity. In particular, do not replace the current module references with one central embedded-provider switch that references every IRX symbol, because that would cause all IRX objects to be pulled into embedded-flavor applications.

## Non-goals for the first implementation

- Do not remove or change the existing embedded flavor.
- Do not make IOP reset ownership mandatory inside ps2_drivers.
- Do not compress the image initially.
- Do not require applications to know raw IRX filenames or dependency ordering.
- Do not redesign the existing `init_*_driver()` / `deinit_*_driver()` public APIs.
- Do not optimize disk size at the expense of startup reliability.
- Do not assume the companion image remains accessible after the IOP reset.

## Proposed deliverables

Install both flavors side by side:

```
libps2_drivers.a          # existing embedded flavor, unchanged default
libps2_drivers_img.a      # external-image flavor
ps2_drivers.irximg        # companion IRX container
ps2_drivers.pc
ps2_drivers-img.pc
```

Naming can be revisited before implementation, but the two flavors should be link-selectable rather than requiring users to rebuild ps2_drivers with a mutually exclusive CMake option.

The external flavor should intentionally fail early if its image/staging API was not prepared correctly instead of silently falling back to embedded IRXs.

## Architecture

### 1. Introduce a module descriptor abstraction

Driver code should stop referring directly to `*_irx` arrays.

Each logical IRX gets a small descriptor object and drivers execute modules through a common helper. Conceptually:

```c
struct ps2_irx_module;

int ps2_irx_exec(
    const struct ps2_irx_module *module,
    unsigned int arg_len,
    const char *args,
    int *module_result);
```

Driver code becomes conceptually:

```c
extern const struct ps2_irx_module ps2_irx_sio2man;

__sio2man_id = ps2_irx_exec(
    &ps2_irx_sio2man,
    0,
    NULL,
    &__sio2man_ret);
```

Keep one descriptor object per IRX so normal archive extraction remains fine-grained.

### 2. Embedded provider

The embedded flavor defines each descriptor in the same object as, or directly next to, its one embedded IRX reference.

A descriptor must reference only its own payload:

```c
const struct ps2_irx_module ps2_irx_sio2man = {
    .id = PS2_IRX_SIO2MAN,
    .embedded_data = sio2man_irx,
    .embedded_size = size_sio2man_irx,
};
```

The embedded executor simply calls `SifExecModuleBuffer()`.

This preserves current behavior and current per-module linker selectivity.

### 3. Image provider

The external flavor defines the same module descriptor symbols but without IRX payload references. The descriptors contain stable module IDs used to find entries in `ps2_drivers.irximg`.

The image provider owns temporary staging state:

```c
struct ps2_irx_staged_module {
    enum ps2_irx_id id;
    void *data;
    uint32_t size;
};
```

When `ps2_irx_exec()` is called:

1. locate the staged buffer for the descriptor ID;
2. call `SifExecModuleBuffer()`;
3. release that module's staging buffer immediately after the synchronous call returns;
4. clear its staging record whether the module started successfully or failed;
5. return exactly the same module ID/result semantics expected by the existing driver code.

Releasing per-module allocations progressively is preferable to keeping one giant arena until every driver has initialized.

### 4. Public staging API

The public API should work in driver terms, not raw IRX terms.

Proposed first API shape:

```c
enum ps2_driver_requirement {
    PS2_DRIVER_REQ_SIO2MAN    = 1u << 0,
    PS2_DRIVER_REQ_FILEXIO    = 1u << 1,
    PS2_DRIVER_REQ_MEMCARD    = 1u << 2,
    PS2_DRIVER_REQ_BDM        = 1u << 3,
    PS2_DRIVER_REQ_USBD       = 1u << 4,
    PS2_DRIVER_REQ_USB        = 1u << 5,
    PS2_DRIVER_REQ_MX4SIO     = 1u << 6,
    PS2_DRIVER_REQ_CDFS       = 1u << 7,
    PS2_DRIVER_REQ_DEV9       = 1u << 8,
    PS2_DRIVER_REQ_HDD        = 1u << 9,
    PS2_DRIVER_REQ_JOYSTICK   = 1u << 10,
    PS2_DRIVER_REQ_AUDIO      = 1u << 11,
    PS2_DRIVER_REQ_POWEROFF   = 1u << 12,
    PS2_DRIVER_REQ_MOUSE      = 1u << 13,
    PS2_DRIVER_REQ_KEYBOARD   = 1u << 14,
    PS2_DRIVER_REQ_CAMERA     = 1u << 15,
    PS2_DRIVER_REQ_NETMAN     = 1u << 16,
    PS2_DRIVER_REQ_SMAP       = 1u << 17,
    PS2_DRIVER_REQ_EEIP       = 1u << 18,
    PS2_DRIVER_REQ_IOPIP      = 1u << 19,
};

int ps2_drivers_img_stage(
    const char *image_path,
    uint32_t driver_requirements);

void ps2_drivers_img_discard_staged(void);
size_t ps2_drivers_img_staged_bytes(void);
```

`ps2_drivers_img_stage()` expands the requested driver set to the complete transitive IRX dependency set, deduplicates modules, validates the image, and allocates/reads all required IRXs before returning.

A convenience profile may be useful:

```c
#define PS2_DRIVER_REQ_FILESYSTEM_ALL (...)
```

but it should be defined from the same dependency metadata rather than maintained independently.

### 5. Future automatic linked-module discovery

After the explicit driver-mask implementation is stable, investigate whether the EE GNU linker and PS2SDK link script can reliably support a linker-set section containing one small requirement record per linked module descriptor.

If proven reliable, `ps2_drivers_img_stage_linked(image_path)` could automatically stage exactly the IRXs referenced by the linked application without a driver mask.

This would be the ideal usability model, but it should not block the first implementation because custom-section retention/start-stop symbol behavior needs validation with the PS2 toolchain and `--gc-sections`.

## Image format

Use a small versioned binary container rather than a filesystem/archive format with unnecessary runtime dependencies.

Suggested filename:

```
ps2_drivers.irximg
```

Suggested version-1 format:

### Header

- 8-byte magic, e.g. `PS2IRX01`
- format version
- header size
- entry count
- entry table offset
- payload area offset
- total file size
- flags/reserved
- optional manifest checksum

### Entry

- stable numeric IRX ID
- payload offset
- payload size
- CRC32
- flags/reserved

IRX names do not need to be part of the runtime format if IDs are generated from one canonical manifest. A host-side inspection tool can map IDs back to names.

Use little-endian integer fields and validate all offsets/sizes against the file length before allocating.

Payloads may be 16-byte aligned in the image for deterministic layout, although the important requirement is that the EE staging allocations passed to `SifExecModuleBuffer()` are suitably aligned.

### Version 1 deliberately has no compression

Compression does not improve steady-state EE memory after staging buffers are freed. It only reduces disk size and potentially pre-reset peak staging size if the entire compressed data set is retained and decompressed module-by-module.

That optimization can be evaluated later after the uncompressed design is proven on real hardware.

## Canonical IRX manifest

Do not keep the IRX list duplicated across CMake, packer code, descriptor code, and dependency tables.

Create one manifest containing, for every IRX:

- stable module ID;
- PS2SDK IRX filename;
- generated C symbol name for the embedded flavor;
- optional human-readable name.

Generate from it where practical:

- CMake IRX build inputs;
- `ps2_drivers.irximg`;
- embedded descriptors;
- external descriptors;
- module ID enum/table used by the runtime;
- host-side image inspection output.

Driver-to-module dependencies can live in one adjacent dependency table.

Current IRXs to migrate from CMake:

```
sio2man
iomanX
fileXio
mcman
mcserv
bdm
bdmfs_fatfs
usbd
usbmass_bd
mx4sio_bd
cdfs
ps2dev9
ps2atad
ps2hdd
ps2fs
mtapman
padman
libsd
audsrv
poweroff
ps2mouse
ps2kbd
ps2cam
netman
smap
ps2ip-nm
ps2ips
```

## IOP reset integration

Keep low-level ownership flexible.

The minimum supported application sequence for the image flavor should be:

```c
if (ps2_drivers_img_stage(
        "mass:/ps2_drivers.irximg",
        PS2_DRIVER_REQ_FILEXIO |
        PS2_DRIVER_REQ_USB |
        PS2_DRIVER_REQ_JOYSTICK) < 0) {
    /* handle staging failure */
}

reset_IOP();

init_fileXio_driver();
init_usb_driver(true);
init_joystick_driver(true);
```

The existing samples duplicate `reset_IOP()`, so a later cleanup may introduce a shared optional helper for:

- `SifInitRpc(0)`;
- `SifIopReset()`;
- `SifIopSync()`;
- `SifInitRpc(0)`;
- `sbv_patch_enable_lmb()`;
- `sbv_patch_disable_prefix_check()`.

However, that helper is orthogonal to the image provider and should not be required to use it.

## Failure handling

Define explicit staging errors for:

- image not found;
- invalid magic/version;
- truncated/corrupt entry table;
- duplicate module ID;
- requested IRX missing from image;
- invalid offset/size;
- CRC failure;
- allocation failure;
- short read;
- staging already active with incompatible requirements.

If staging fails part-way through, free every buffer allocated by that staging attempt and leave the provider in a clean inactive state.

After reset, an attempt to initialize a driver whose IRX was not staged must fail deterministically with an image-provider error. It must not attempt to open the image unless an explicit on-demand mode is later designed and proven safe for that boot path.

After `SifExecModuleBuffer()`, free the staging buffer even on IRX startup failure; retrying after a failed load should require restaging that module.

## Deinit/reinit semantics

The embedded flavor can unload an IRX and later initialize it again because its source bytes remain in the ELF.

The external flavor loses that source buffer after execution. This is a real semantic difference that must be handled intentionally.

Version 1 options should be evaluated in this order:

1. Support the normal startup case first: stage -> reset -> initialize -> keep modules resident for process lifetime.
2. For modules that are unloaded and later reinitialized, provide an explicit restaging API while the backing filesystem is available.
3. Do not keep all staged IRX payloads forever merely to preserve transparent reinitialization, because that defeats the memory-saving objective.

Add documentation explaining that `deinit_*()` followed by `init_*()` in the image flavor may require restaging if the IOP module was actually unloaded.

A later API could provide:

```c
int ps2_drivers_img_restage(uint32_t driver_requirements);
```

using the image path retained by the provider, but only when the source device is currently usable.

## Build-system plan

### Keep legacy output unchanged

Current users linking `ps2_drivers` should see no source or behavior change.

### Add external image output

Build:

- a host-side image packer, preferably a small portable C or CMake-host tool so container builds remain deterministic;
- `ps2_drivers.irximg` from `${PS2SDK}/iop/irx/*.irx`;
- image-provider runtime objects;
- external-flavor module descriptors;
- `libps2_drivers_img.a`.

### Avoid duplicating EE support libraries unnecessarily

The final combined archive currently uses an MRI script to merge `ps2_drivers_impl` with required PS2SDK EE libraries.

Refactor the CMake composition so the common driver objects and EE support libraries can feed both final archive flavors without changing their public link behavior.

### Install and pkg-config

Install the image next to an appropriate PS2SDK data/share location rather than under `lib/`. If PS2SDK has no established ports data directory, define a project-specific install location such as:

```
${PS2SDK}/ports/share/ps2_drivers/ps2_drivers.irximg
```

Provide `ps2_drivers-img.pc` for the external archive. The pkg-config file cannot decide where an application deploys the image at runtime, so documentation and samples must make that explicit.

## Testing plan

### Host/build-time tests

Add a host-side image inspector/validator that can:

- print image version and module count;
- list ID/name/offset/size/CRC;
- detect malformed/truncated images.

Use it in CI immediately after building the image.

Add deterministic-image verification: two pack operations from the same IRX inputs must produce byte-identical output.

### Link-content regression tests

Build equivalent tiny applications against both flavors and inspect their ELF/map output.

Verify:

- embedded ELF contains the required IRX payload symbols/data;
- image-flavor ELF contains none of the `*_irx` payload arrays;
- linking one driver does not accidentally pull unrelated embedded IRXs;
- image flavor still links only the EE-side support actually required by the selected driver APIs.

Record exact ELF/data-size deltas.

### Runtime tests in PCSX2

Create a dedicated image-flavor sample with this sequence:

1. resolve/open image from the boot device;
2. stage selected drivers;
3. record free/available EE memory if a reliable metric is available;
4. reset/sync/patch IOP;
5. initialize drivers;
6. verify staged byte count falls to zero;
7. exercise each initialized subsystem.

Start with small dependency sets:

- joystick: sio2man + mtapman + padman;
- audio: libsd + audsrv;
- fileXio;
- USB storage;
- network.

Then test the all-filesystem profile.

### Real-hardware matrix

The main risk is not `SifExecModuleBuffer()`; it is whether the companion image can be read successfully before reset from each boot environment.

Test at least:

- host:/ps2link;
- mass:/USB;
- mx4sio:;
- mc0:/mc1: where practical;
- cdfs:/cdrom0:;
- HDD/PFS.

For each source, prove the image is completely staged before reset and that no post-reset access to the image is needed for the initial driver startup.

## Memory measurement

Do not rely only on archive size.

For representative applications, record:

- final ELF file size;
- loaded ELF `.text/.rodata/.data/.bss` sizes from map/size tooling;
- total bytes of IRX payloads selected;
- peak EE staging bytes immediately before reset;
- staged bytes after each module execution;
- steady-state EE bytes after initialization.

Expected steady-state saving for the image flavor should be close to the raw IRX bytes that would otherwise have been linked into the ELF, plus/minus alignment and section overhead.

Peak startup memory may temporarily be similar to or greater than the embedded flavor because the modules must exist in heap staging buffers before reset. That is acceptable only if the buffers are promptly reclaimed after loading. This trade-off must be measured on real hardware, especially for applications already close to the EE memory ceiling.

## Suggested implementation phases

### E0 - Baseline and measurements

- Build representative existing samples.
- Produce link maps and identify exactly which IRX payload objects each sample pulls.
- Record embedded-flavor ELF section sizes.
- Record raw IRX sizes from the active PS2SDK.
- Add the results to this document or a dedicated measurement document.

Exit criterion: quantified baseline before any architecture changes.

### E1 - Canonical manifest and image packer

- Introduce the canonical IRX manifest.
- Generate `ps2_drivers.irximg`.
- Add image validation/inspection.
- Add deterministic pack test in CI.

No runtime behavior changes yet.

Exit criterion: reproducible, validated image containing every current IRX.

### E2 - Module descriptor abstraction

- Replace direct IRX array use in driver sources with per-module descriptors/executor calls.
- Implement the embedded provider first.
- Verify every existing sample still behaves and links with the same IRX granularity.
- Compare ELF/map output to E0.

Exit criterion: embedded flavor has no functional or material size regression.

### E3 - External provider and staging API

- Add `ps2_drivers_img_stage()`.
- Add dependency expansion/deduplication.
- Add aligned per-module allocations.
- Add CRC and bounds validation.
- Add external executor with immediate post-exec free.
- Add explicit cleanup/error paths.

Exit criterion: a simple image-flavor sample can stage before reset and initialize one small driver set after reset.

### E4 - Build/install second flavor

- Produce `libps2_drivers_img.a` alongside the existing archive.
- Install image and pkg-config metadata.
- Add CI artifacts for both flavors.
- Add ELF symbol/section assertions proving IRX payloads are absent from image-flavor ELFs.

Exit criterion: consumers can choose either flavor without rebuilding ps2_drivers.

### E5 - Filesystem and dependency coverage

- Exercise fileXio, MC, USB, MX4SIO, CDFS, HDD, joystick, audio, and networking.
- Validate dependency closure tables against actual init paths.
- Test startup from each practical boot medium in PCSX2 and on real PS2 hardware.

Exit criterion: image flavor supports the same practical startup matrix as the embedded flavor, subject to documented pre-reset source availability.

### E6 - Reinit semantics

- Test every driver that can genuinely unload its IRX.
- Define/document which reinitialization paths require restaging.
- Add targeted `restage` support if needed.

Exit criterion: deinit/reinit behavior is explicit and tested rather than accidentally broken.

### E7 - Optional usability/peak-memory improvements

After the basic flavor is stable, evaluate independently:

- linker-set based automatic discovery of linked IRX requirements;
- loading one contiguous staging arena versus per-module allocations;
- compressed image payloads to lower disk size or pre-reset peak memory;
- a standard reusable IOP-reset helper;
- an application deployment helper that copies the companion image beside the ELF.

None of these should block E0-E6.

## Acceptance criteria

The feature is complete when:

- existing `libps2_drivers.a` consumers remain source-compatible and behavior-compatible;
- `libps2_drivers_img.a` links applications without embedding IRX payload arrays in their ELF;
- required IRXs can be staged from the companion image before IOP reset;
- existing driver init functions execute staged IRXs without API changes;
- each staging buffer is freed immediately after its module load attempt;
- malformed/missing images fail cleanly;
- dependency selection does not require users to know raw IRX internals;
- PCSX2 and real-hardware tests cover representative boot devices;
- measured steady-state EE memory reduction is documented;
- no unrelated IRXs are accidentally pulled into embedded applications.

## Recommended first implementation slice

Start with E0 and E1 only.

That gives us two things before touching driver runtime code:

1. exact measured memory/ELF savings available from the idea for real sample applications;
2. a deterministic, testable `ps2_drivers.irximg` format and packer.

After that, E2 can introduce the provider abstraction while using only the existing embedded backend. This isolates the riskiest refactor from the new runtime loading behavior and makes regressions much easier to identify.
