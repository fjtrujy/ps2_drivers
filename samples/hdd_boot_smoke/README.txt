PS2_DRIVERS STANDALONE HDD/PFS SMOKE TEST

Copy the five test files (and optionally this README) to ONE directory on your
APA/PFS-formatted internal PS2 HDD (e.g. hdd0:__common/PS2DRIVERS/):

  BOOT.ELF          <-- universal bootstrap for USB / MX4SIO / HDD
  HDD_SMOKE.ELF     <-- standalone test, no NJEMU assets
  elf_path.ini      <-- contains HDD_SMOKE.ELF
  probe.txt         <-- file access check
  ps2_drivers.irximg

Launch BOOT.ELF from the HDD directory using wLaunchELF's file
browser. Do not launch HDD_SMOKE.ELF directly: BOOT.ELF preserves
the original partition context and restores the PFS filesystem.

On the screen, look for:
  Before reset: probe.txt read OK
  IOP reset and HDD/PFS restore OK
  After reset: probe.txt read OK
  PASS: bootstrap, image staging, IOP reset, ...

The screen remains visible until you power off or reset the console.
If it fails, the screen shows the failed stage and an error code.

This package tests APA/PFS internal HDDs, NOT ATA/BDM exFAT or USB.
It does not write or format the HDD.
