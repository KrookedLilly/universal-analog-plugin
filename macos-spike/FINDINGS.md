# macOS Universal Analog Plugin spike — findings

**Date:** 2026-06-23/24
**Outcome:** ✅ **GO.** The Universal Analog Plugin works on macOS. A real keypress on a
Keychron Q5 HE produces analog values through the full chain the HE-Keyboard mod uses
(`libwooting_analog_wrapper` → `libwooting_analog_sdk` → our plugin → Soup → keyboard),
verified with `read_test` (`RESULT: SUCCESS`) and a clean process exit.

## What was needed

The plugin itself needed only **build/packaging** changes — but Soup's brand-new macOS HID
layer (`calamity-inc/Soup`, the library the plugin delegates all device I/O to) had **four
real bugs** that broke any command-based analog keyboard (Keychron / DrunkDeer / Madlions)
on macOS. Wooting boards would have worked already (input-report only, no command writes).

### Build / packaging (in this fork)
- **Sun build tool** built from source on macOS (the documented PHP bootstrap was
  unavailable; compiled `suncli` directly).
- **`.sun` descriptors**: added a macOS-conditional `linker_arg -framework IOKit` /
  `-framework CoreFoundation` block (Soup's macOS HID path needs them; Soup declares them
  nowhere). One `linker_arg` per token — Sun quotes each arg, so `-framework IOKit` must be
  two entries.
- **`build.sh`**: Darwin branch emitting `.dylib`; `abiv0` is best-effort on macOS (needs a
  macOS arm64 build of the Rust `wooting_analog_common.a`, which the repo ships ELF-only).
- **ABI**: the installed Wooting SDK is **0.8.0**, which uses **C-plugin ABI v1** (bare
  symbol names). So **`abiv1.dylib` is the variant that loads**; `abiv0` is not needed on
  this SDK. (SDK 0.7 used ABI v0 — a red herring from a stale Homebrew cellar on disk.)

### Soup macOS HID fixes (`soup/hwHid.cpp`, `soup/hwHid.hpp`)
1. **`getAll()` closed the `IOHIDManager`** right after opening devices — and
   `IOHIDManagerClose` closes every device the manager opened. Retained device refs stayed
   non-null (so `havePermission()` lied) but `IOHIDDeviceSetReport` failed with
   `kIOReturnNotOpen`. **Fix:** keep a single persistent `IOHIDManager` for the process
   (hidapi's model); never close it.
2. **`sendReport()` included the leading report-id byte** in the buffer passed to
   `IOHIDDeviceSetReport`. macOS takes the report id as a separate arg; for unnumbered
   reports (id 0) the leading byte must be stripped or the report runs one byte over its
   declared length and is rejected. **Fix:** strip it (matches hidapi's `set_report`).
3. **`hwHid::reset()` called `IOHIDDeviceClose()`** on a shared `IOHIDDeviceRef`. The
   plugin re-runs `getAll()` every ~1s (`discover_devices(false)`); the discarded duplicate
   wrapper's destructor closed the device out from under the active poll thread (and tore
   down its input callback), so input died after ~1s. **Fix:** don't close in `reset()`;
   the persistent manager owns open-state. Just `CFRelease` our ref.
4. **`hwHid::reset()` called `IOHIDDeviceUnscheduleFromRunLoop(CFRunLoopGetCurrent())`.**
   `reset()` can run on a different thread than the one that scheduled the device (static
   destructor at dylib unload, or device-removal on the discovery thread); on the wrong /
   finalizing thread this crashes on a freed run-loop mode (bus error / NSException at
   `wooting_analog_uninitialise`). **Fix:** don't unschedule in `reset()` — it only runs
   when the device is going away, so the callback won't fire again.

There is also a defensive reopen-on-`kIOReturnNotOpen`-and-retry in `sendReport()` (a
transient seen once); harmless and rarely hit once #1/#3 are in place.

## How it was diagnosed

Standalone Soup-linked probes isolated each layer without the SDK/JVM:
- `kcprobe` — Soup-direct, main thread → confirmed HID send/read after fixes #1/#2.
- `kcprobe2` — poll on a background thread → ruled out threading.
- `kcprobe3` — poll thread + 1 Hz re-discovery thread → reproduced & confirmed fix #3.
- `read_test` — links the real wrapper, exercises the full SDK path → final confirmation
  (+ caught the #4 teardown crash once it called `uninitialise()`).

## Verification status

- ✅ `read_test` (full SDK path): `RESULT: SUCCESS`, analog depth values, clean exit.
- ✅ **In-game (Minecraft via Prism + HE-Keyboard mod): confirmed** — analog WASD on the
  Keychron Q5 HE, clean game exit (no teardown crash). This is the real end-to-end result.
- ✅ **Apple Silicon AND Intel** both runtime-verified: the universal `abiv1.dylib`'s x86_64 slice
  was validated on a real Intel Mac via the Minecraft mod (2026-06-30) — analog WASD + clean exit.
  Released as GitHub `macos-1.0.0` on `KrookedLilly/universal-analog-plugin`.
- ✅ Keychron Q5 HE (primary, daily driver). DrunkDeer A75 not yet retested post-fix
  (same code path; expected to work). Razer Huntsman V3 Pro remains **out** on macOS
  (needs Razer Synapse, which is Windows-only).

## Remaining / future work

- **`abiv0` for older SDKs**: build `wooting_analog_common.a` for `aarch64-apple-darwin`
  (rustup + `cargo build --target aarch64-apple-darwin`) if support for SDK ≤0.7 is wanted.
  Not needed for SDK 0.8.
- ✅ **Upstreamed + merged**: the four Soup fixes are in **[calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180)**
  (squashed to one clean commit; the defensive reopen-retry was dropped). Merged upstream. An
  official UAP macOS release built against the updated Soup will eventually supersede our fork build.
- **Install ergonomics**: codesigning/notarization, a user-facing install script, and the
  HE-Keyboard mod's v2 device-status docs.
- **`build.sh` end users** need the `sun` build tool on PATH (or `suncli` symlinked).

## Install recap (what's deployed on this machine)

- Plugin: `/usr/local/share/WootingAnalogPlugins/universal-analog-plugin/abiv1.dylib`
- SDK + wrapper: `/usr/local/lib/libwooting_analog_sdk.dylib`, `libwooting_analog_wrapper.dylib`
- macOS **Input Monitoring** granted to the launching terminal (the JVM/Minecraft will need
  it too, granted to whatever launches the game).
