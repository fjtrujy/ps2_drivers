# IRX image bootstrap ELF

`ps2_drivers_irximg_bootstrap.elf` is a small launcher for applications using
the external `ps2_drivers.irximg` flavor when their original ELF launcher
resets the IOP before transferring control.

The bootstrap embeds only the USB mass-storage filesystem stack (`iomanX`,
`fileXio`, `bdm`, `bdmfs_fatfs`, `usbd`, and `usbmass_bd`). If the launch
filesystem is still accessible (for example after a wLaunchELF handoff), it
reuses that stack. Otherwise it restores `mass:` from the embedded modules.
It then launches the real application with PS2SDK's
`LoadELFFromFileWithPartitionNoReset()`. The real application can therefore
stage `ps2_drivers.irximg` before performing its own clean IOP reset.

Pass the target ELF as the first argument. If no argument is supplied, the
bootstrap reads the first line of `elf_path.ini` from its current directory.
Relative targets are resolved against that current directory before loading, so
`MVS` becomes a device-qualified path such as `mass:/NJEMU-MVS/MVS` or
`host:/MVS`.
Additional arguments after the target path are forwarded to the target ELF.

## Internal HDD / APA/PFS

`ps2_drivers_irximg_hdd_bootstrap.elf` is a separate launcher for an internal
HDD. Unlike the USB bootstrap, it requires the original APA partition name in
the launch arguments, e.g. `hdd0:__common:pfs:/NJEMU/MVS/BOOT_HDD.ELF`, while
the working directory is a mounted `pfs:/NJEMU/MVS/`. It fails with a clear
error if either piece of information is missing.

The HDD variant resets the IOP, installs the fileXio/DEV9/ATAD/HDD/PFS stack,
remounts the original partition, restores the directory, and launches the
configured target without another reset. A partition-qualified argument is
passed to the target so it can preserve that information for a later reset.
The USB variant's module set and its target-argument convention are unchanged.

Some HDD launchers place the original partition-qualified bootstrap path in
`argv[1]`. The HDD variant treats those arguments as launch context, not as a
target ELF; it then reads `elf_path.ini` or uses a separate explicit target.

The application is responsible for staging its own external IRX dependencies
before any further reset. See `docs/HDD_PFS_BOOT.md` for the application API.

For an HDD-only deployment that must expose `BOOT.ELF`, rename/copy the **HDD**
bootstrap as `BOOT.ELF` on the APA/PFS volume. The regular USB bootstrap under
that filename cannot restore the PFS mount and will fail. The HDD bootstrap
accepts wLaunchELF's partition-qualified argument plus its `-r`/`-nr`
launcher flag, and reconstructs the PFS working directory when the initial
newlib cwd is malformed.

## Internal HDD / FAT/exFAT

`ps2_drivers_irximg_ata_bootstrap.elf` is an optional third launcher for an
ATA-backed BDM `massN:` volume. It reuses the launch filesystem when available
or loads fileXio, DEV9, BDM, BDMFS_FATFS and `ata_bd` in dependency order.
It forwards `--ps2-drivers-ata-bdm` to its target so the application can
distinguish the internal ATA transport from USB or MX4SIO, which also use
`massN:` device paths. An ordinary USB `BOOT.ELF` never forwards this marker.
The application must use the explicit ATA/BDM staging and launch-context APIs
for subsequent resets. See `docs/HDD_ATA_BDM_BOOT.md`.
