# IRX image bootstrap ELF

`ps2_drivers_irximg_bootstrap.elf` is a small launcher for applications using
the external `ps2_drivers.irximg` flavor when their original ELF launcher
resets the IOP before transferring control.

The bootstrap embeds only the USB mass-storage filesystem stack (`iomanX`,
`fileXio`, `bdm`, `bdmfs_fatfs`, `usbd`, and `usbmass_bd`). It restores `mass:`,
then launches the real application with PS2SDK's
`LoadELFFromFileWithPartitionNoReset()`. The real application can therefore
stage `ps2_drivers.irximg` before performing its own clean IOP reset.

Pass the target ELF as the first argument. If no argument is supplied, the
bootstrap reads the first line of `elf_path.ini` from its current directory.
Relative targets are resolved against that current directory before loading, so
`MVS.elf` becomes a device-qualified path such as `mass:/NJEMU-MVS/MVS.elf` or
`host:/MVS.elf`.
Additional arguments after the target path are forwarded to the target ELF.
