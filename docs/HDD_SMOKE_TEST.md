# Standalone APA/PFS HDD boot smoke test

This example checks the real **wLaunchELF -> HDD BOOT.ELF -> application ->
IOP reset -> HDD remount** sequence. It uses only `ps2_drivers`; it requires
no emulator, ROMs, cache, or other NJEMU files.

## Build and copy

Configure `ps2_drivers` using the PS2SDK toolchain with `BUILD_SAMPLES=ON` and
build the `hdd_smoke_package` target:

```sh
cmake -S . -B build -DBUILD_SAMPLES=ON
cmake --build build --target hdd_smoke_package -j
```

The complete, ready-to-copy package is at
`build/samples/hdd_boot_smoke/HDD_SMOKE.zip`. An unpacked copy is at
`build/samples/hdd_boot_smoke/HDD_SMOKE/`. The ZIP has only these six files:

| File | Function |
| --- | --- |
| `BOOT.ELF` | **Universal bootstrap**, built from `ps2_drivers_irximg_bootstrap.elf` |
| `HDD_SMOKE.ELF` | Standalone test application, linked with the external-IRX flavor |
| `elf_path.ini` | Names `HDD_SMOKE.ELF` as the target |
| `probe.txt` | Known contents verified both before and after IOP reset |
| `ps2_drivers.irximg` | Staged IRX payloads, including the six APA/PFS modules and poweroff |
| `README.txt` | Short hardware instructions |

Copy **all six files together** into one directory on an *existing APA/PFS*
internal HDD partition, for example `hdd0:__common/PS2DRIVERS/`. Launch
**`BOOT.ELF` from that HDD directory in wLaunchELF's file browser**. Do not
launch `HDD_SMOKE.ELF` directly: it expects the original APA partition
context forwarded by the generic HDD bootstrap.

`BOOT.ELF` is the **same universal executable** used for USB, MX4SIO
and both internal HDD modes. There are no longer separate bootloader binaries. The smoke test only reads files and does not format,
modify or overwrite partitions.

## Expected output

The test keeps a text screen visible and also prints to the PCSX2 console:

```text
[hdd-smoke] PS2_DRIVERS HDD/PFS SMOKE TEST
[hdd-smoke] Partition: hdd0:__common
[hdd-smoke] Directory: pfs:/PS2DRIVERS
[hdd-smoke] Before reset read OK: pfs:/PS2DRIVERS/probe.txt
[hdd-smoke] Staged 7 modules (... bytes)
[hdd-smoke] Resetting IOP...
[hdd-smoke] IOP reset and HDD/PFS restore OK
[hdd-smoke] After reset read OK: pfs:/PS2DRIVERS/probe.txt
[hdd-smoke] PASS: bootstrap, image staging, IOP reset,
[hdd-smoke] HDD remount, and file read succeeded.
```

The partition and directory reflect the installation's actual location.
If any step fails, the screen displays `FAIL` followed by the failed stage and
numeric status. Both PASS and FAIL remain on screen until the console is
reset or powered off. For a particularly early failure before the test ELF
loads, enable wLaunchELF's IOP logging and check the selected `BOOT.ELF`.

## Validation

This sample passed in PCSX2 2.9.x against a new, isolated **4 GiB APA/PFS
disk image** containing a `SMOKE` partition. Only the six packaged files
were installed for the test, alongside a separate wLaunchELF binary used to
launch them. The `BOOT.ELF` was selected by wLaunchELF's HDD launcher. The
log confirmed that both marker-file reads succeeded, seven required IRXs
were staged and consumed, and the original partition was remounted after the
IOP reset. NJEMU content was not present on the test disk.

**Real-hardware baseline:** the previous, HDD-specific version of this
package was run from an internal HDD on a physical PS2 using wLaunchELF. The tester reported that it worked
and reached the final message, `Test complete. Power off or reset the PS2.`
That message is printed only after both marker-file reads and the full IOP
reset/remount sequence have passed. This confirms the original APA/PFS HDD
bootstrap path on actual hardware, independently of NJEMU. The new universal
BOOT.ELF has passed the equivalent PCSX2 test but awaits physical validation.

This does **not** validate NJEMU's own integration, ROM loading, or the
separate ATA/BDM FAT/exFAT mode on physical hardware.
