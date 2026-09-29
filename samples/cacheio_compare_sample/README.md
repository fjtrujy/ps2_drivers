# cacheio comparison

Build with the PS2 toolchain environment set:

```sh
cmake -S . -B build_cacheio_validate -DBUILD_SAMPLES=ON
cmake --build build_cacheio_validate --target cacheio_compare_runtime -j4
```

Use `cacheio_compare_runtime` (or the default build) so an IRX-only rebuild also
refreshes `ps2_drivers.irximg` beside the ELF. Launch that ELF through PCSX2 with
HostFs enabled and a USB mass image attached. The default input is
`mass:/NJEMU-58704ed5-MVS/cache/mslug3_cache/crom`; ELF arguments override the path
and random iteration count (default 4096). The input is read-only. Reports go to
`host:cacheio_compare_result.txt`, beside the ELF under PCSX2's HostFs root.

The test compares every complete 64 KiB block sequentially, saves reference
CRCs, then compares random reads against both POSIX bytes and the saved CRCs.
The final partial block, if any, is excluded. A block-70 diagnostic is retained
from the previous investigation. Success requires the final `RESULT: PASS code=0`
line, not simply successful initialization or intermediate progress.

Random I/O timings include POSIX seek+read versus cacheio read-at. Read order
alternates; CRC calculation, comparisons and report output are outside timed
regions. These are aggregate emulated EE timer ticks, not cold-media latency or
proof of hardware throughput. Divide by the reported ticks-per-second value.
The sample retains fileXio's default buffer size, as NJEMU currently does.
Actual SCSI command counts require separate instrumentation.

The prototype's current contract requires sector-aligned reads and a suitably
aligned EE buffer, each request at most 64 KiB. Keep files and media unchanged
while direct handles are open. Fragmented-file, reconnect, suspend/resume and
NJEMU fallback validation remain separate acceptance tests.

## Recorded PCSX2 run (2026-09-29)

`results/pcsx2-2026-09-29-correctness.txt` records the earlier sample: 1024
sequential byte comparisons and 4096 random CRC checks passed. This version
predates the final RESULT line and paired random timings.

`results/pcsx2-2026-09-29-paired64.txt` records the updated sample: the complete
sequential comparison plus 64 paired random byte comparisons passed. Aggregate
EE ticks give 316.13 ms/read POSIX and 18.16 ms/read cacheio (17.41x ratio).
This is a single warm-cache emulator run, excluding extent-map opening time;
there is no measured SCSI command count or hardware performance result yet.
Both runs used the pending BDM read-ahead error-handling change.
