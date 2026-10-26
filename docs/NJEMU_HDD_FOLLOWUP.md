# NJEMU follow-up: consume generic APA/PFS support from ps2_drivers

This describes the migration of NJEMU PR #25 after the generic code is merged
into the `njemu` branch of `ps2_drivers`. This repository makes **no changes**
to NJEMU itself. Do not modify NJEMU `resources/` or its FAST CACHE policy.

## Build and packaging

1. In NJEMU's PS2 CMake external-IRX branch, locate the installed
   `${PS2_DRIVERS_IRXIMG_BOOTSTRAP}` for `BOOT.ELF` and add a similar lookup
   for `${PS2SDK}/ports/share/ps2_drivers/ps2_drivers_irximg_hdd_bootstrap.elf`.
   Expose it as `PS2_DRIVERS_IRXIMG_HDD_BOOTSTRAP` to install rules.
2. Replace the local `${TARGET}_hdd_bootstrap` executable in NJEMU
   `CMakeLists.txt` (near the `BOOT_HDD` build rules) with that installed
   artifact. Retain the existing extensionless core, `elf_path.ini`,
   `ps2_drivers.irximg`, and `BOOT.ELF` packaging. Install the HDD launcher as
   `BOOT_HDD.ELF` only for the canonical external-driver MVS/CPS2 variants.
3. Delete `src/ps2/ps2_hdd_bootstrap.c` once both release packages build and
   the HDD launcher has passed PCSX2 validation. Do not rename or change the
   existing removable-storage `BOOT.ELF`.

## Runtime handoff

4. In `src/ps2/ps2_platform.c`, replace its local
   `ps2_hdd_launch_context_t` typedef and hand-written partition extraction
   inside `ps2_prepare_launch()` with the public `<ps2_hdd_launch.h>` and
   `ps2_hdd_launch_capture_current(&hdd_launch, argc, argv)`. Preserve
   NJEMU's `prepareLaunch` platform-driver hook, which is still responsible
   for capturing arguments before reset. Keep an NJEMU-local active flag or
   evaluate `capture_status == PS2_HDD_LAUNCH_OK`.
5. Replace `ps2_restore_hdd_filesystem()`'s fileXio/DEV9/HDD/PFS
   initialization and `chdir` code with
   `ps2_hdd_launch_restore(&hdd_launch)` after reset/sync and SBV patches.
   Retain NJEMU-specific log messages, failure policy, and cleanup sequencing.
6. In the application-stage path, continue using
   `ps2_drivers_img_stage_default_for_boot_device(BOOT_DEVICE_HDD0,
   application_requirements)` when the captured context is valid; PFS cwd
   still maps to `BOOT_DEVICE_UNKNOWN`, intentionally. Non-HDD launches
   continue using `ps2_drivers_img_stage_default_for_current_boot(...)`.
7. Remove the HDD-specific staged-BDM discard workaround in
   `src/ps2/ps2_input.c` (the branch checking two remaining modules and
   calling `ps2_drivers_img_discard_staged()`). Correct HDD staging now
   omits BDM/FATFS, so the normal strict zero-staged-module invariant should
   pass without a special exemption.
8. Keep NJEMU's shutdown behavior (unmount and deinitialize the subsystems
   it owns) and align it with the new explicit mount tracking in
   `ps2_drivers`. `umount_hdd_partition("pfs:")` can now release explicit
   mounts; the library's return codes are available for diagnostics.

## Preserve application-specific behavior

The following must remain in NJEMU:

- Its platform `prepareLaunch`/`init`/`shutdown` lifecycle and error policy.
- FAST CACHE runtime selection **only** for USB/MX4SIO, never for APA/PFS.
- `USE_CACHE` ROM streaming, ROM directories, configured cache size and
  emulator-specific filesystem paths.
- GUI, audio, controller, input and game initialization.

## Acceptance checklist

- Build MVS/CPS2 canonical release packages; verify identical
  USB/MX4SIO `BOOT.ELF` bytes and that both packages contain the generic
  `BOOT_HDD.ELF`, core, `elf_path.ini`, and external IRX image.
- Launch MVS and CPS2 from the isolated issue #23 APA/PFS image in PCSX2;
  verify the original partition argument survives both bootstrap and
  NJEMU's own IOP reset, with zero remaining staged modules.
- Launch MVS/USB and CPS2/MX4SIO from existing isolated tests if available.
- Confirm game ROM and BIOS reads, audio/pad/GUI and the existing desktop,
  PSP, PS2 and release CI suites. Perform physical internal-HDD validation
  with multiple ELF browsers before marking issue #23 fully resolved.

The `njemu` remote branch is an integration dependency of NJEMU's CI and
release workflows. Merge the ps2_drivers PR only after these checks, then
update NJEMU PR #25 in a distinct session to remove its now-redundant code.
