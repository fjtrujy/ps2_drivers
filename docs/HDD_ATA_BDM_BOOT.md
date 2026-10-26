# Internal HDD FAT/exFAT through ATA/BDM

This is an **explicit alternative** to APA/PFS, not a change to the default
internal HDD stack. The installed PS2SDK must provide `ata_bd.irx`, an ATAD
build configured with `ATA_ENABLE_BDM`. The IRX container and embedded library
carry this driver separately from `ps2atad.irx`.

## Storage modes

| Storage | Requirements | Filesystem path | Bootstrap |
| --- | --- | --- | --- |
| Internal HDD APA/PFS | `PS2_DRIVER_REQ_HDD` | `pfs:/` (APA partition from `hdd0:`) | `BOOT_HDD.ELF` |
| Internal HDD FAT/exFAT | `PS2_DRIVER_REQ_ATA_BDM` | `massN:/` | `BOOT_ATA.ELF` |
| USB and MX4SIO | Existing USB/MX4SIO requirements | `massN:/` | `BOOT.ELF` |

ATA/BDM stages `iomanX`, `fileXio`, `ps2dev9`, `bdm`, `bdmfs_fatfs`, and
`ata_bd`. Its IOP initialization order is fileXio, DEV9, BDM, BDMFS_FATFS,
and lastly ATA/BDM. An external IRX application consumes the modules with
`init_fileXio_driver()`, `init_dev9_driver()`, `init_bdm_driver()` and
`init_ata_bdm_driver(false)` (or calls `init_ata_bdm_driver(true)` if it has
not initialized dependencies yet).

Do **not** combine `PS2_DRIVER_REQ_HDD` and `PS2_DRIVER_REQ_ATA_BDM`.
`ps2atad.irx` and `ata_bd.irx` implement the same `atad` module, and loading
both is unsafe. The image staging APIs reject this combination. A clean IOP
reset is required when switching modes. This implementation deliberately does
not attempt simultaneous APA/PFS and exFAT mounts on the same ATA bus.

## Distinguishing ATA from USB

BDM exposes ATA-backed partitions as `mass:`, `mass0:`, and other `massN:`
paths, just like USB and MX4SIO. **Do not use `getBootDeviceID(massN:)` to
select internal ATA**: its legacy USB/removable meaning is unchanged. Choose
ATA explicitly via a launcher, application setting, or another trusted source.
For this reason, the dedicated bootstrap forwards the marker
`--ps2-drivers-ata-bdm` to the launched application.

For an application launched by that bootstrap:

```c
#include <ps2_ata_bdm_launch.h>
#include <ps2_drivers_img.h>

ps2_ata_bdm_launch_context_t launch;
int status = ps2_ata_bdm_launch_capture_current(&launch, argc, argv);

if (status == PS2_ATA_BDM_LAUNCH_OK) {
    status = ps2_drivers_img_stage_default_for_ata_bdm(
        PS2_DRIVER_REQ_POWEROFF |
        PS2_DRIVER_REQ_AUDIO | PS2_DRIVER_REQ_JOYSTICK);
    if (status != PS2_DRIVERS_IMG_OK) {
        /* Report the failure without resetting the IOP. */
    } else {
        /* Reset/synchronize IOP, initialize RPC, apply SBV patches. */
        status = ps2_ata_bdm_launch_restore(&launch);
        if (status != PS2_ATA_BDM_LAUNCH_OK) {
            /* Report the missing volume / failed driver initialization. */
        }
        /* Initialize the application's staged audio and pad drivers. */
    }
}
```

For programs that choose the physical ATA transport in their own settings,
`ps2_ata_bdm_launch_capture_explicit(&launch, directory)` records the
already-known `massN:` directory without requiring the launcher marker. This
is **not** transport autodetection. Both capture variants reject non-`massN:`
directories; an unmarked ordinary USB launch does not opt into ATA.

## Bootstrap packaging

Install the generic `ps2_drivers_irximg_ata_bootstrap.elf` as `BOOT_ATA.ELF`
alongside the usual external `ps2_drivers.irximg`, `elf_path.ini`, and real
application ELF. The same target-resolution and no-reset ELF handoff are used
by the existing USB and APA/PFS launchers. If the launch filesystem survives
the ELF browser handoff, the ATA launcher reuses it. Otherwise it starts the
ATA/BDM stack and waits for the captured `massN:` directory to reappear.
It passes the transport marker before the application's forwarded arguments.

The bootstrap intentionally does not bundle USB drivers and does not guess
whether a missing mass volume belongs to USB or ATA. Ensure an ATA-formatted
HDD is physically present and the chosen `massN:` slot is stable across a
reset. The bootstrap and restore helper report failure if it is not.

## Validation

Run `ps2_drivers_ata_bdm_launch_check`, `ps2_drivers_irximg_stage_check` and
`ps2_drivers_irximg_check`. The tests assert correct IRX identities, reject
conflicting ATAD modes, verify the transport marker and malformed directories,
and preserve the existing APA/PFS and USB closure tests. Also build the
three bootstrap ELFs, inspect artifact sizes, and confirm the original USB and
APA/PFS ELF bytes are unchanged.

Compilation and host staging tests are **not** proof of a mounted exFAT
volume. Real ATA/BDM filesystem access and game/application boot require an
isolated exFAT HDD test in PCSX2 or physical PS2 hardware. Do not claim an
exFAT runtime pass until that test has actually completed.

An isolated PCSX2 test with an MBR-formatted 1 GiB exFAT HDD image exercised
both `ata_bdm_sample.elf` and the complete external-IRX handoff. The test
launcher initialized ATA/BDM and entered `mass0:/BOOT_ATA.ELF`; that bootstrap
loaded `mass0:/ATA_TEST.ELF` with no IOP reset. The target recognized the
transport marker, staged seven external IRX modules (including poweroff),
reset the IOP, restored the filesystem, consumed all staged modules and
successfully opened `mass0:/`. PCSX2 reported `ata0p1 mounted to fatfs` before
and after the reset. The image and profile are separate from NJEMU's isolated
APA/PFS test image and the user's normal PCSX2 HDD configuration.

PCSX2 emitted an ATA `0xe0` warning during shutdown, after successful file
access. Physical hardware shutdown behavior and actual NJEMU/game loading from
exFAT remain untested. The ATA bootstrap has only been tested in the
filesystem-already-live handoff path; its missing-filesystem restoration path
also requires validation with a launcher which resets the IOP.
