# External IRX Image Runtime Validation

This document records E5/E6 validation for the external IRX image flavor.

## Environment

- Date: 2026-09-28
- Branch: `external-irx-image`
- PS2 toolchain: `/Users/fjtrujy/toolchains/ps2/ps2dev`
- PCSX2: v2.9.91
- Runtime image source used by emulator tests: `host:ps2_drivers.irximg`

The `host:` source is intentionally useful for emulator validation because it remains accessible after the IOP reset. It does **not** prove that USB, MX4SIO, HDD/PFS, optical-disc, or memory-card sources remain accessible after reset; those devices still require the image to be staged before resetting the IOP.

## E5 dependency-closure coverage

The host staging test exercises every public driver requirement independently, plus the combined filesystem profile. It verifies:

- the complete transitive module closure;
- deduplication;
- expected module count;
- non-zero staged byte count;
- rejection of a second staging attempt while payloads are still staged;
- execution-style release of every staged module back to zero bytes/modules.

Current measured closures:

| Requirement | Modules | Staged bytes |
| --- | ---: | ---: |
| sio2man | 1 | 5,241 |
| fileXio | 2 | 16,038 |
| memcard | 3 | 85,539 |
| BDM | 2 | 46,950 |
| USBD | 1 | 26,241 |
| USB mass storage | 4 | 85,872 |
| MX4SIO | 4 | 64,032 |
| CDFS | 1 | 10,705 |
| DEV9 | 1 | 10,977 |
| HDD | 8 | 150,796 |
| joystick | 3 | 49,763 |
| audio | 2 | 35,202 |
| poweroff | 1 | 3,489 |
| mouse | 2 | 34,694 |
| keyboard | 2 | 41,094 |
| camera | 2 | 37,246 |
| netman | 1 | 13,093 |
| SMAP | 1 | 14,173 |
| EEIP | 3 | 38,243 |
| IOPIP | 5 | 142,269 |
| all filesystem drivers | 16 | 301,292 |

These byte totals match the corresponding IRX payload sums from the E0 baseline.

## E5 PCSX2 runtime validation

The dedicated `irximg_sample` was built against `libps2_drivers_img.a`. Its post-link assertion confirms that no embedded `*_irx` payload symbols are present in the external-flavor ELF.

Runtime sequence:

1. open `host:ps2_drivers.irximg`;
2. stage `PS2_DRIVER_REQ_JOYSTICK`;
3. reset/sync the IOP and apply SBV patches;
4. call `init_joystick_driver(true)`;
5. let each `SifExecModuleBuffer()` release its EE staging allocation.

PCSX2 registered the external modules after the reset:

- `sio2man 1.02`;
- `mtapman 1.02`;
- `padman 1.02`.

This proves that the image-backed descriptors and staged EE buffers survive the IOP reset and are valid inputs to the normal driver initialization path.

The sample itself checks that staged module count and staged byte count both reach zero after initialization.

## E6 restage/reinit semantics

The external provider now remembers only the successful image path. It does **not** retain any IRX payload after that module has been executed.

Public API:

```c
int ps2_drivers_img_restage(uint32_t driver_requirements);
void ps2_drivers_img_forget_source(void);
```

The remembered path is replaced only after a successful explicit `ps2_drivers_img_stage()`. A failed staging attempt does not destroy the previous usable source path.

`ps2_drivers_img_restage()`:

- fails with `PS2_DRIVERS_IMG_ERR_NO_SOURCE` until an initial stage has succeeded;
- requires all previous staging buffers to have been consumed or discarded;
- reopens the remembered image path and stages the requested driver closure again;
- still validates image bounds and per-module CRCs;
- does not keep module payloads resident after execution.

`ps2_drivers_img_discard_staged()` discards only temporary payload buffers. The remembered source path remains available for an explicit restage. Call `ps2_drivers_img_forget_source()` when even that small retained path string is no longer wanted.

### Host validation

For every requirement set listed above, the host test now performs:

1. initial stage;
2. release every staged module as the runtime executor would;
3. verify zero staged modules/bytes;
4. `ps2_drivers_img_restage()`;
5. verify the same module count and exact byte count;
6. discard;
7. finally forget the source and verify restaging fails with `PS2_DRIVERS_IMG_ERR_NO_SOURCE`.

### PCSX2 reinitialization validation

The joystick sample additionally exercises a real unload/restage/reinit sequence:

1. initialize `sio2man`, `mtapman`, and `padman` from staged image data;
2. confirm the staged payload count reaches zero;
3. call `deinit_joystick_driver(true)`, which unloads the joystick IRXs and its SIO2MAN dependency;
4. call `ps2_drivers_img_restage(PS2_DRIVER_REQ_JOYSTICK)`;
5. initialize the joystick driver again;
6. confirm staged payloads return to zero again.

PCSX2's IOP registration log contains two complete registrations of each image-provided module:

```text
sio2man version 1.02  # first initialization
mtapman version 1.02
padman version 1.02

sio2man version 1.02  # after unload + restage
mtapman version 1.02
padman version 1.02
```

This validates the intended E6 semantics without retaining IRX bytes permanently in EE RAM.

## Hardware coverage still required

The remaining E5 work is source-device validation on a real PS2. The runtime architecture is specifically designed so these tests do not require reopening the image immediately after reset.

Still to validate on hardware where practical:

- `host:` through ps2link;
- `mass:` USB;
- `mx4sio:`;
- `mc0:` / `mc1:`;
- `cdfs:` / `cdrom0:`;
- HDD/PFS.

For each source the pass condition is: the complete required IRX set is staged before reset, initialization succeeds after reset without reading the image again, and staged EE bytes return to zero after module execution.

Restaging after an unload is only available while the remembered image source is accessible at that moment. For media whose driver disappears across an IOP reset, applications may need to re-establish that filesystem stack before calling `ps2_drivers_img_restage()`, or avoid unloading modules they intend to use for the rest of the process.
