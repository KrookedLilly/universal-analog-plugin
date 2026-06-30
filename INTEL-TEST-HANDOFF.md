# Intel-Mac validation handoff — macOS UAP `macos-1.0.0`

Resume doc for finishing the macOS Universal Analog Plugin release. Written 2026-06-25 on an
**Apple Silicon** Mac; the remaining gate is a **runtime test on an Intel Mac**, which you'll do in
a fresh session on that machine. This doc is committed to `KrookedLilly/universal-analog-plugin`
(branch `macos`) so you can clone/pull it on the Intel Mac and continue.

> New session, read this first, then `macos-spike/RELEASE-HANDOFF.md` for the fuller history.

---

## Where things stand

- ✅ macOS UAP fork **works** — verified in-game (Minecraft, Prism) with a Keychron Q5 HE on
  **Apple Silicon**. Soup fixes **merged upstream** ([calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180)).
- ✅ Signed + notarized installer built: **`UAP-macOS-1.0.0.pkg`** — **universal (`arm64` + `x86_64`)**,
  stapled, Gatekeeper-accepted.
- ⛔ **THIS STEP: validate the `x86_64` slice at runtime on a real Intel Mac.** Only `arm64` has been
  runtime-verified. Compilation of `x86_64` is clean, but that is not the same as running.
- ⛔ After Intel passes: **publish the GitHub Release** `macos-1.0.0`, then update the Unity asset docs.

## The artifact — you do NOT rebuild on Intel

`UAP-macOS-1.0.0.pkg` is **universal**, so the same file installs and runs on Intel. It is
**gitignored** (installers ship via Release, not the repo), so it will **not** be in your clone.
**Transfer the existing pkg** to the Intel Mac (AirDrop / iCloud / USB) rather than rebuilding.

- On the Apple Silicon Mac it's at: `universal-analog-plugin-mac/UAP-macOS-1.0.0.pkg`
- **SHA-256 (verify after transfer):** `204b37625727b7bfc74a51d0450f50754b69539fbaccf25396b677e34676ca97`
  - check on Intel: `shasum -a 256 UAP-macOS-1.0.0.pkg`

(If you ever must rebuild instead: `macos-spike/build-pkg.sh` does sign+notarize+staple, but it
needs the universal dylib + your Developer ID certs + the `uap-notary` keychain profile on that
machine. Transferring the pkg is far simpler and tests the exact shipping artifact.)

## Prereqs on the Intel Mac

1. **Wooting Analog SDK runtime** installed (the plugin slots into it). Copy both dylibs and clear quarantine:
   ```bash
   sudo cp libwooting_analog_sdk.dylib     /usr/local/lib/
   sudo cp libwooting_analog_wrapper.dylib /usr/local/lib/
   sudo xattr -d com.apple.quarantine /usr/local/lib/libwooting_analog_sdk.dylib
   sudo xattr -d com.apple.quarantine /usr/local/lib/libwooting_analog_wrapper.dylib
   ```
2. A **non-Wooting** HE keyboard in analog mode (Keychron Q5 HE = the reference board). Testing a
   non-Wooting board is the point — it exercises the Soup command-write path our fork fixed.
   (Razer is out on macOS — needs Windows-only Synapse.)

## Test procedure

1. Verify the transferred pkg's SHA-256 (above), then **double-click to install** (it self-escalates;
   installs `universal-analog-plugin/abiv1.dylib` into `/usr/local/share/WootingAnalogPlugins/`).
2. **Grant Input Monitoring** to whatever launches the game (System Settings → Privacy & Security →
   Input Monitoring) — e.g. Prism Launcher. This is the #1 "no input" cause; relaunch after granting.
3. Drive it with the **non-Wooting** board. Easiest = the same vehicle used on arm64: a Minecraft
   instance with the HE-Keyboard mod (Prism). Confirm:
   - **analog WASD** (partial press = partial walk speed), and
   - **clean exit** (no crash on quit — the #4 Soup bug was a teardown crash; confirm it's absent on x86_64 too).
4. Optionally confirm the loaded dylib is the universal one: `lipo -archs /usr/local/share/WootingAnalogPlugins/universal-analog-plugin/abiv1.dylib` → `x86_64 arm64`.

### Pass criteria
Analog input from the non-Wooting board works **and** the game exits cleanly on the Intel Mac.

## After Intel passes — publish the release

From a machine that has the pkg + `gh` authed (Intel is fine if you set up `gh` there; otherwise do
it back on the Apple Silicon Mac):

```bash
cd <repo root of the universal-analog-plugin clone>
gh release create macos-1.0.0 \
  UAP-macOS-1.0.0.pkg \
  --repo KrookedLilly/universal-analog-plugin \
  --target macos \
  --title "macOS build macos-1.0.0" \
  --notes-file macos-spike/RELEASE-NOTES-macos-1.0.0.md
```

Release notes are already written at `macos-spike/RELEASE-NOTES-macos-1.0.0.md` (says "Intel
runtime validated" — true once this test passes). Install guide: `macos-spike/INSTALL-macOS.md`.

## After the release is live — Unity asset

The sibling **Unity HE-Keyboard asset** gains non-Wooting macOS support from this same fork (no code
change). Its handoff is at that repo's root: `unity/assets/he-keyboard-asset/HANDOFF-native-sdk-and-macOS-fixes.md`.
Update its docs/listing to point at the now-published release. (Hold until the release is live so the
download link is real and Intel is confirmed.)

## If Intel FAILS

Capture the failure (console output, crash log, whether the plugin loaded at all) and stop before
publishing. The x86_64 slice would need investigation — start from `macos-spike/FINDINGS.md` (the 4
Soup bugs) and check whether the failure is arch-specific (e.g. struct layout, calling convention)
vs. an environment issue (SDK not installed, Input Monitoring not granted). Do NOT cut the release
with a broken Intel slice — the release notes claim Intel works.

## Key pointers

- `macos-spike/RELEASE-HANDOFF.md` — full release/packaging history + status.
- `macos-spike/FINDINGS.md` — engineering write-up of the 4 Soup macOS HID bugs.
- `macos-spike/INSTALL-macOS.md` — end-user install guide.
- `macos-spike/build-pkg.sh` — sign/notarize/staple script (only if rebuilding).
- Forks: `KrookedLilly/universal-analog-plugin` (branch `macos`) + `KrookedLilly/Soup` (branch `macos`).
- SDK is **0.8.0 → ABI v1**, so `abiv1.dylib` is the loaded variant.
