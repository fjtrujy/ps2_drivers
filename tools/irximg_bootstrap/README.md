# Universal external-IRX bootstrap

Build **one** `ps2_drivers_irximg_bootstrap.elf` and distribute it as
`BOOT.ELF` on USB, MX4SIO, HDD APA/PFS and ATA/BDM FAT/exFAT storage.
No other storage-specific bootstrap target is required.

Install alongside the application's executable, `elf_path.ini` (first line
names that executable) and `ps2_drivers.irximg`. The application stages its
own external driver requirements **before** its IOP reset. The bootstrap
links only the drivers needed to recover the launch storage; it never loads
all the transports into IOP simultaneously.

## Behavior

- **APA/PFS HDD:** wLaunchELF forwards a qualified
  `hdd0:<partition>:pfs:/.../BOOT.ELF` path. The bootstrap recovers the
  original PFS directory even when newlib's initial cwd is malformed,
  restores that exact partition, then forwards the qualified target path.
- **USB/MX4SIO/ATA:** All share the `massN:` namespace. From a clean IOP,
  the bootstrap tries USB, then MX4SIO, then ATA-backed BDM. Each failed
  candidate is discarded with another complete IOP reset, so only the
  successful storage stack remains. The selected transport must restore the
  same `massN:` directory; no arbitrary remapping to a different volume.
  Successful ATA initialization forwards the marker
  `--ps2-drivers-ata-bdm` so the application can restore ATA/BDM after its
  own reset. USB/MX4SIO receive no ATA marker.
- **Host:** The existing host filesystem is used without a reset.

Launcher `-r`/`-nr` arguments are launch metadata, not target filenames.
The resolved target is entered with `LoadELFFromFileWithPartitionNoReset()`.

## Limitations

Automatically selecting the first provider that restores a `massN:` path
can be ambiguous if multiple media simultaneously expose the *same path*.
In particular, `mass0:` does not encode the transport. For deterministic
results, test with one launch device connected and ensure that the selected
filesystem contains the intended `elf_path.ini`. An unreachable or remapped
mass volume is reported as an error, not silently substituted.

The universal APA/PFS HDD path passed in both PCSX2 and on a physical PS2
through the standalone wLaunchELF HDD smoke-test package. USB FAT32 and
ATA/BDM exFAT end-to-end paths passed in PCSX2. MX4SIO, physical USB/ATA
and mixed-device scenarios still need independent hardware verification.
