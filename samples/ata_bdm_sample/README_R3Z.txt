PS2_DRIVERS - INTERNAL exFAT HDD / wLaunchELF-R3Z SMOKE TEST

Extract all five files together into an existing exFAT partition on
the internal PS2 HDD:

  BOOT.ELF             universal ps2_drivers bootstrap
  ATA_TEST.ELF         standalone ATA/BDM diagnostic
  elf_path.ini         selects ATA_TEST.ELF
  ps2_drivers.irximg   external driver modules
  README_R3Z.txt       this file

In wLaunchELF-R3Z, open ata0:/ or ata1:/, navigate to the test folder,
and launch BOOT.ELF (not ATA_TEST.ELF directly).

The launcher should recognize the ataN: path, start ATA/BDM, locate the
same directory as a massN: volume and hand off to ATA_TEST.ELF with the
ATA transport marker. The sample stages the drivers, resets the IOP,
restores ATA/BDM, and shows "PASS" on screen. The result remains visible
until you power off or reset the PS2.

This is a read-only test. It does not format or modify the HDD.
If you see a black screen or return to the browser, capture the R3Z
loader log and the ps2_drivers bootstrap log for diagnosis.

Do not assume ata0: always maps to mass0:. Multiple volumes containing
identical elf_path.ini targets are rejected to prevent wrong-disk boot.
