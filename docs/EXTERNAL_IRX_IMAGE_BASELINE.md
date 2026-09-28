# External IRX Image E0 Baseline

Baseline captured on 2026-09-28 before the external-image runtime refactor.

## Toolchain

- Branch: `external-irx-image`
- Baseline commit: `c5d9d49` (`Add external IRX image flavor plan`)
- EE compiler: `mips64r5900el-ps2-elf-gcc (GCC) 15.2.0`
- Installed PS2SDK did not expose Git commit metadata.
- Build configuration: CMake, `BUILD_SAMPLES=ON`
- Existing embedded flavor only; no runtime code changes are included in these measurements.

The archive sizes are recorded only for context:

| Archive | Bytes |
| --- | ---: |
| `libps2_drivers_impl.a` | 721,700 |
| `libps2_drivers.a` | 2,690,740 |

Archive size is not an EE-memory measurement. Static archive extraction means an application normally links only the IRX objects referenced by the driver code it uses.

## Installed IRX payload sizes

| IRX | Bytes |
| --- | ---: |
| `sio2man.irx` | 5,241 |
| `iomanX.irx` | 10,401 |
| `fileXio.irx` | 5,637 |
| `mcman.irx` | 72,101 |
| `mcserv.irx` | 8,197 |
| `bdm.irx` | 10,745 |
| `bdmfs_fatfs.irx` | 36,205 |
| `usbd.irx` | 26,241 |
| `usbmass_bd.irx` | 12,681 |
| `mx4sio_bd.irx` | 11,841 |
| `cdfs.irx` | 10,705 |
| `ps2dev9.irx` | 10,977 |
| `ps2atad.irx` | 10,653 |
| `ps2hdd.irx` | 23,701 |
| `ps2fs.irx` | 42,477 |
| `mtapman.irx` | 7,781 |
| `padman.irx` | 36,741 |
| `libsd.irx` | 15,813 |
| `audsrv.irx` | 19,389 |
| `poweroff.irx` | 3,489 |
| `ps2mouse.irx` | 8,453 |
| `ps2kbd.irx` | 14,853 |
| `ps2cam.irx` | 11,005 |
| `netman.irx` | 13,093 |
| `smap.irx` | 14,173 |
| `ps2ip-nm.irx` | 99,261 |
| `ps2ips.irx` | 4,765 |
| **Total** | **546,619** |

## Embedded sample baselines

`IRX bytes` is the sum of the actual `*_irx` data symbols present in each final ELF, obtained from `mips64r5900el-ps2-elf-nm -S -t d`. It is the closest build-time estimate of the steady-state EE memory that the image flavor can reclaim for that sample.

| Sample | ELF file bytes | text | data | bss | IRX bytes |
| --- | ---: | ---: | ---: | ---: | ---: |
| `alldrivers_sample` | 1,810,708 | 153,388 | 375,768 | 91,488 | **369,175** |
| `alldrivers_verbose_sample` | 1,835,360 | 158,876 | 375,768 | 92,128 | **369,175** |
| `filesystem_sample` | 1,864,804 | 173,112 | 307,808 | 67,832 | **301,292** |
| `hdd_sample` | 1,708,616 | 161,676 | 157,112 | 60,360 | **150,796** |
| `listdir_sample` | 1,965,768 | 183,212 | 285,224 | 68,744 | **278,746** |
| `mtap_sample` | 1,418,744 | 157,044 | 55,948 | 28,704 | **49,763** |
| `network_ee_sample` | 2,364,516 | 286,108 | 44,476 | 280,600 | **38,243** |
| `network_iop_sample` | 1,617,136 | 169,852 | 148,544 | 42,040 | **142,269** |
| `poweroff_sample` | 1,350,148 | 151,260 | 9,644 | 35,768 | **3,489** |
| `select_test_ee` | 2,271,684 | 278,372 | 44,464 | 248,520 | **38,243** |
| `select_test_iop` | 1,661,732 | 173,444 | 148,544 | 42,040 | **142,269** |
| `showiopinfo_sample` | 1,886,820 | 171,444 | 343,064 | 90,760 | **336,494** |
| `tcp_burst_client_ee` | 2,280,144 | 279,868 | 44,464 | 248,520 | **38,243** |
| `tcp_server_ee` | 2,265,168 | 276,932 | 44,464 | 248,520 | **38,243** |
| `udp_echo_server_ee` | 2,277,720 | 281,004 | 44,464 | 248,520 | **38,243** |
| `wav_sample` | 1,450,032 | 157,420 | 41,372 | 50,080 | **35,202** |

Representative linked IRX sets:

- `mtap_sample`: `sio2man`, `mtapman`, `padman` = 49,763 bytes.
- `wav_sample`: `libsd`, `audsrv` = 35,202 bytes.
- `network_ee_sample`: `netman`, `ps2dev9`, `smap` = 38,243 bytes.
- `network_iop_sample`: `netman`, `ps2dev9`, `ps2ip-nm`, `ps2ips`, `smap` = 142,269 bytes.
- `filesystem_sample`: 16 IRXs = 301,292 bytes.
- `alldrivers_sample`: 19 IRXs = 369,175 bytes.

## Initial E1 image result

The first version-1 image produced from the same 27 installed IRXs is:

- raw payload bytes: 546,619;
- `ps2_drivers.irximg` bytes: 547,456;
- format overhead including table and 16-byte payload alignment: 837 bytes;
- entries: 27;
- manifest CRC32: `0e786b2d`.

Two independent pack operations produced byte-identical files.
The validator also rejected both a truncated image and a payload-corrupted image; the latter failed on the affected IRX CRC.

## E0 conclusion

The optimization is worthwhile even for moderately sized driver sets. For the current samples, the theoretical steady-state EE saving ranges from 3,489 bytes for the poweroff-only case to 369,175 bytes for the all-drivers sample.

The external flavor must still be compared against this baseline after E2/E3. In particular:

- embedded flavor link granularity must remain unchanged;
- the external flavor should remove the corresponding `*_irx` data symbols from final ELFs;
- any provider metadata/code overhead must be measured against these payload savings;
- peak pre-reset staging memory is separate from steady-state memory and must be measured later on PCSX2 and real hardware.

## E2 embedded-provider regression check

The E2 descriptor/executor refactor was compared against a freshly built E1 archive using the same toolchain and sample object files.

A pre-existing sample-build issue was found during this check: sample link commands searched `$PS2SDK/ports/lib` before the current build directory, so `-lps2_drivers` could resolve to an already installed copy instead of the archive just built in the checkout. The sample CMake files were changed to link the build-tree `libps2_drivers.a` by explicit path so future local and CI validation exercises the intended archive.

After correcting the link target:

- every sample retained the exact same set of linked `*_irx` payload symbols;
- `.data` delta was 0 bytes for every sample;
- `.bss` delta was 0 bytes for every sample;
- `.text` overhead ranged from 64 to 224 bytes depending on the number of referenced descriptors/call sites.

Representative loaded-size deltas:

| Sample | text delta | data delta | bss delta |
| --- | ---: | ---: | ---: |
| `poweroff_sample` | +64 | 0 | 0 |
| `wav_sample` | +80 | 0 | 0 |
| `mtap_sample` | +96 | 0 | 0 |
| `network_iop_sample` | +96 | 0 | 0 |
| `filesystem_sample` | +192 | 0 | 0 |
| `alldrivers_sample` | +208 | 0 | 0 |
| `alldrivers_verbose_sample` | +224 | 0 | 0 |

The descriptor objects remain one-per-IRX archive members. Linking a driver therefore still pulls only the descriptor and payload for IRXs referenced by that driver path; the refactor does not introduce a central table that retains unrelated IRX payloads.

## E3/E4 external-image measurements

The external provider was validated on the host against the generated image before runtime testing. The dependency expansion produced these staged sets with the active PS2SDK:

| Requirement | Modules | Staged bytes |
| --- | ---: | ---: |
| `PS2_DRIVER_REQ_JOYSTICK` | 3 | 49,763 |
| `PS2_DRIVER_REQ_AUDIO` | 2 | 35,202 |
| `PS2_DRIVER_REQ_IOPIP` | 5 | 142,269 |
| `PS2_DRIVER_REQ_FILESYSTEM_ALL` | 16 | 301,292 |

The staging test also verifies that a second staging attempt is rejected while a set is active and that `ps2_drivers_img_discard_staged()` returns both the staged-module and staged-byte counters to zero.

Both `libps2_drivers_img.a` and the dedicated external-flavor sample ELF are checked with `nm` and contain no defined `*_irx` or `size_*_irx` payload symbols.

For a direct same-source comparison using the joystick dependency set:

| Flavor | text | data | bss | Loaded total | Embedded IRX bytes |
| --- | ---: | ---: | ---: | ---: | ---: |
| Embedded reference | 149,676 | 55,948 | 28,128 | 233,752 | 49,763 |
| External image | 158,564 | 6,156 | 28,128 | 192,848 | 0 |

The first external implementation therefore removes all 49,763 raw joystick IRX bytes while adding 8,888 bytes of parser/staging/executor text. The net loaded-size reduction for this sample is **40,904 bytes**.

These are build-time loaded-section measurements, not a substitute for runtime EE heap measurements. PCSX2 and real-hardware testing must still verify the full pre-reset staging -> IOP reset -> module execution -> staged-buffer release lifecycle, including startup from the practical boot media listed in the plan.
