# Internal HDD / APA/PFS boot with external IRX images

For an entirely independent end-to-end test without NJEMU files, use the
[standalone HDD smoke test](HDD_SMOKE_TEST.md). It produces a ready-to-copy
ZIP with an HDD-specific `BOOT.ELF`, test ELF and the external IRX image.

## Universal BOOT.ELF packaging

`ps2_drivers` now installs a **single**
`ps2_drivers_irximg_bootstrap.elf`. Copy it to the application's HDD
installation as `BOOT.ELF`, alongside `elf_path.ini`, the application and
`ps2_drivers.irximg`. The same bootstrap binary is also used on USB,
MX4SIO and ATA/BDM FAT/exFAT; separate `BOOT_HDD.ELF` and `BOOT_ATA.ELF`
files are no longer built.

The original APA/PFS path (e.g.
`hdd0:__common:pfs:/NJEMU/MVS/BOOT.ELF`) must be forwarded by wLaunchELF.
The universal bootstrap distinguishes APA/PFS from the `massN:` transports
using that path, recovers malformed initial newlib working directories,
ignores wLaunchELF's `-r`/`-nr` flags and mounts the correct partition after
its clean IOP reset. It then launches the real application without a second
reset and forwards the partition-qualified target path for any later resets.

**Validation:** The standalone HDD smoke test using the dedicated HDD-only
binary passed on physical PS2 hardware. The new universal executable passed
the identical standalone end-to-end handoff in PCSX2 on a fresh APA/PFS HDD
image. Physical verification of the *universal* executable is still pending;
this distinction matters for release readiness.

## Application API for subsequent IOP resets

Capture the launch context **before** resetting the IOP:

```c
#include <ps2_hdd_launch.h>
#include <ps2_drivers_img.h>

ps2_hdd_launch_context_t hdd_context;
int status = ps2_hdd_launch_capture_current(&hdd_context, argc, argv);
bool hdd_launch = status == PS2_HDD_LAUNCH_OK;

if (hdd_launch) {
    status = ps2_drivers_img_stage_default_for_boot_device(
        BOOT_DEVICE_HDD0,
        PS2_DRIVER_REQ_AUDIO | PS2_DRIVER_REQ_JOYSTICK);
} else {
    status = ps2_drivers_img_stage_default_for_current_boot(
        PS2_DRIVER_REQ_AUDIO | PS2_DRIVER_REQ_JOYSTICK);
}
if (status != PS2_DRIVERS_IMG_OK) {
    /* Handle staging failure without resetting the IOP. */
}

/* Reset/sync the IOP, initialize RPC, and apply the SBV patches here. */

if (hdd_launch &&
    ps2_hdd_launch_restore(&hdd_context) != PS2_HDD_LAUNCH_OK) {
    /* Handle a missing partition, driver error, or failed PFS mount. */
}

/* Initialize the application's remaining audio, pad, etc. drivers. */
```

For non-HDD boots, retain the application's existing filesystem initialization
sequence. `ps2_hdd_launch_capture()` also accepts an explicit cwd string for
callers that already saved it. `ps2_hdd_launch_format_argument()` produces a
partition-qualified PFS path when one ELF must hand control to another.

`getBootDeviceID("pfs:/...")` is intentionally unchanged: it still returns
`BOOT_DEVICE_UNKNOWN`. PFS identifies a *mounted filesystem*, not the APA
partition that must be remounted after IOP reset. Calling the generic
current-boot helper on a PFS cwd would therefore stage too few IRXs; once
launch context is captured, use `BOOT_DEVICE_HDD0` explicitly as shown.

The parser accepts `hdd0:` and `hdd:` launch paths, checks that a nonempty
partition precedes the PFS segment, validates the mounted PFS directory, and
rejects missing/malformed information without guessing a partition. The
context has fixed-capacity buffers and does not retain dynamic allocations.

The HDD runtime initializes `fileXio`, `poweroff`, `DEV9` and `HDD` without
BDM; then it mounts PFS and restores the captured directory. Explicit mounts
now register their mount status so the existing unmount helpers can release
them. The caller remains responsible for the usual lifecycle and IOP reset.

## Staging dependency correction

`PS2_DRIVER_REQ_HDD` now expands to exactly these six IRX modules:

- `iomanX`, `fileXio`, `ps2dev9`, `ps2atad`, `ps2hdd`, `ps2fs`.

It no longer requests `bdm` or `bdmfs_fatfs`: these are not dependencies of
the **APA/PFS** HDD driver chain. BDM is a general block-device filesystem
stack, however, and is also used for **internal ATA HDDs formatted as FAT or
exFAT**. Likewise, `init_hdd_driver(true, ...)` no longer initializes or
deinitializes BDM because this helper initializes APA/PFS only. The standalone
BDM, USB, MX4SIO, and cacheio requirement masks still request their own BDM
dependencies. All-filesystem staging retains BDM through USB/MX4SIO.

**Internal HDD FAT/exFAT uses a distinct, explicit ATA/BDM mode.** The image
contains `ata_bd.irx` (ATAD with `ATA_ENABLE_BDM`) and applications can select
`PS2_DRIVER_REQ_ATA_BDM`, load `init_ata_bdm_driver()`, and use the dedicated
ATA bootstrap and launch-context helpers. Simply restoring `bdm` and
`bdmfs_fatfs` to `PS2_DRIVER_REQ_HDD` would not connect the internal ATA
device to BDM. Do not combine `PS2_DRIVER_REQ_HDD` with
`PS2_DRIVER_REQ_ATA_BDM`: `ps2atad` and `ata_bd` both export ATAD and cannot
be loaded together. Switch formats by performing a clean IOP reset. See
[the ATA/BDM boot guide](HDD_ATA_BDM_BOOT.md).

With the current NJEMU integration-branch IRX set, the host tests measured:

| Requirement | Before | After |
| --- | ---: | ---: |
| HDD alone | 8 modules | 6 modules |
| HDD alone, staged bytes (current IRX set) | 152,004 | 104,206 |
| HDD + fileXio + poweroff + audio + joystick | 14 modules | 12 modules |
| All filesystems | 16 modules | 16 modules |

The original validation snapshot reported 150,796 bytes for the eight-module
HDD bundle using an earlier IRX revision. Here, the eight-module legacy
closure is reproduced by explicitly requesting `HDD | BDM`, giving 152,004
bytes with the current six-module HDD closure plus BDM/FATFS. The module
count is the stable expected dependency result. Host staging validation
asserts the exact module identities and no leftover modules, rather than
discarding known unused modules.

## Validation and limitations

Run `ps2_drivers_irximg_check`, `ps2_drivers_irximg_stage_check`,
`ps2_drivers_hdd_launch_check`, and `ps2_drivers_img_payload_check` with CMake.
The launch-context host test exercises valid and invalid APA/PFS arguments,
PFS mountpoint variants, and bounded argument formatting.

The HDD runtime was exercised in PCSX2 using NJEMU issue #23's isolated 8 GiB
APA/PFS image. A host-loaded diagnostic launcher entered the new HDD bootstrap,
which restored the mounted `__common` partition and handed execution to the
MVS core without another IOP reset. The resulting NJEMU log showed it
preserving and remounting `hdd0:__common`. This is a **handoff smoke test**,
not a substitute for recompiling NJEMU against the updated library and
validating physical PS2 hardware. In particular, different ELF browsers may
not pass original APA metadata; those will receive an explicit error rather
than an unreliable guessed partition.
