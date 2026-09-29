# cacheio

`cacheio` is an optional read-only extent-aware I/O path owned by `ps2_drivers`.
It is intended for workloads that repeatedly perform aligned random reads from
large files and want to avoid repeating filesystem cluster traversal on every
read.

The EE client opens a path through the IOP RPC service. The IOP module obtains
the file fragment list through `USBMASS_IOCTL_GET_FRAGLIST`, maps those extents
to the underlying block device, and services reads with PS2SDK's generic
`bd_defrag_read()` helper.

The implementation builds against an installed upstream PS2SDK. It does not
require PS2SDK's experimental FatFs FastSeek/CLMT optimization or a PS2SDK-owned
`cacheio` library/IRX.

The public EE API is installed as `cacheio.h`; `ps2_cacheio_driver.h` provides
the normal `ps2_drivers` lifecycle wrapper. The generated `cacheio.irx` is
included in both the embedded and external-image ps2_drivers flavors.
