# macOS UAP — Release/Packaging Handoff

Resume doc for shipping the macOS Universal Analog Plugin (UAP) to end users. The
engineering is **done and verified**; what's left is packaging, signing/notarizing,
distribution, and announcing. Written 2026-06-24 so work can continue after a context
compaction.

## TL;DR status

- ✅ macOS UAP **works end-to-end**, verified in real Minecraft (Prism) with a Keychron Q5 HE:
  analog WASD + clean game exit. Also fixes the **Unity asset** on macOS for free (same native
  SDK→plugin→Soup chain; no Unity code change needed).
- ✅ Four Soup macOS HID bugs fixed; **upstream PR open: [calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180)** (1 clean commit).
- ✅ Forks pushed: `KrookedLilly/universal-analog-plugin` (branch `macos`) + `KrookedLilly/Soup` (branch `macos`).
- ✅ he-keyboard repo docs reconciled on branch `docs/macos-uap-spike` (CLAUDE.md, README macOS note, `docs/klpm-he-keyboard-handoff.md`).
- ✅ **Universal (arm64+x86_64) dylib built clean** (`macos-spike/abiv1-universal.dylib`, gitignored):
  `lipo -archs` = `x86_64 arm64`, both frameworks linked, 0 warnings. x86_64 *compiles* clean;
  Intel *runtime* still to be verified on the Intel Mac. Rebuild command below.
- ⛔ NOT done: packaging (.pkg), code-sign/notarize, GitHub Release, install guide, announcement, Intel-Mac runtime test.

## Capabilities now available (the user)

- Apple Developer account, experienced with codesign/notarize.
- An **Intel Mac** for x86_64 testing (we'd only tested arm64).

## Key locations

- Plugin fork (clone): `/Users/krookedmac/Documents/Development/KrookedLilly/minecraft-mods/universal-analog-plugin-mac/` — branch `macos`.
- Built arm64 plugin (installed + verified): `universal-analog-plugin/abiv1.dylib`.
- Universal build output (in progress): `macos-spike/abiv1-universal.dylib` (+ `universal-build.log`).
- Sun build tool: `/Users/krookedmac/Documents/Development/KrookedLilly/minecraft-mods/Sun/suncli`.
- Installed on this machine: `/usr/local/share/WootingAnalogPlugins/universal-analog-plugin/abiv1.dylib`; SDK+wrapper in `/usr/local/lib/`.
- Diagnostic probes + findings: `macos-spike/` (`read_test.c`, `kcprobe*.cpp`, `FINDINGS.md`).
- he-keyboard repo: `/Users/krookedmac/Documents/Development/KrookedLilly/minecraft-mods/minecraft-he-keyboard/` (branch `docs/macos-uap-spike`).
- Unity asset (consumes same SDK in C#): `../he-keyboard-asset/Assets/KrookedLilly/HEKeyboard/`.

## Critical facts / gotchas

- **ABI:** installed Wooting SDK is **0.8.0 → C-plugin ABI v1**, so **`abiv1.dylib` is the variant that loads**. `abiv0` needs a macOS-arm64 build of the Rust `wooting_analog_common.a` (not needed for 0.8). `abiv1` is self-contained (inline FFI), which is why it builds universal in one clang pass.
- **Input Monitoring:** mandatory manual user step — grant it to the *launching app* (Prism Launcher; for `gradlew` it's the terminal). An installer **cannot** grant this. It's the #1 "no movement" cause → put it top of the install guide.
- **Razer** Huntsman V3 Pro stays **unsupported on macOS** (needs Windows-only Synapse).
- **Don't bundle the Wooting SDK** in our package (Wooting's to distribute; licensing). Install guide tells users to install Wootility/SDK first.
- **Clean-rebuild rule:** `reset()` is inline in `hwHid.hpp`; Sun under-tracks header deps, so after ANY `hwHid.hpp` change do `rm -rf int Soup/soup/int Soup/soup/bin` then rebuild (a stale `DigitalKeyboard.o` caused a teardown crash to survive a "fix").
- The defensive reopen-retry was **removed** (root causes #1/#3 cover it); current code is clean.

## Universal build — verify / reproduce

```bash
cd /Users/krookedmac/Documents/Development/KrookedLilly/minecraft-mods/universal-analog-plugin-mac
clang++ -std=c++17 -O3 -fPIC -arch arm64 -arch x86_64 -dynamiclib \
  -DABI_VERSION_TARGET=1 -I Soup \
  main.cpp $(find Soup/soup -name '*.cpp') \
  -framework IOKit -framework CoreFoundation \
  -o macos-spike/abiv1-universal.dylib
lipo -archs macos-spike/abiv1-universal.dylib   # expect: x86_64 arm64
otool -L macos-spike/abiv1-universal.dylib | grep -iE "IOKit|CoreFoundation"
```
(If clang universal ever misbehaves, fall back: build each arch and `lipo -create a.dylib b.dylib -output abiv1.dylib`.)

## Remaining plan

### Step A — Universal dylib ✅ DONE
Built `macos-spike/abiv1-universal.dylib` (`x86_64 arm64`, frameworks linked, 0 warnings).
This is the artifact to sign in Step B. (Rebuild with the command above if needed.)

### Step B — Package as signed + notarized `.pkg` (recommended over a zip)
A `.pkg` handles Gatekeeper (notarize+staple), the sudo install (pkg self-escalates), and ships
the universal dylib. Template (user supplies their Developer ID identities):

```bash
# 1. sign the dylib with the *Application* cert
codesign --force --options runtime --timestamp \
  --sign "Developer ID Application: <NAME> (<TEAMID>)" \
  macos-spike/abiv1-universal.dylib

# 2. stage the install tree (folder name must be 'universal-analog-plugin')
rm -rf /tmp/uap-stage && mkdir -p /tmp/uap-stage/universal-analog-plugin
cp macos-spike/abiv1-universal.dylib /tmp/uap-stage/universal-analog-plugin/abiv1.dylib

# 3. build the component pkg (installs into the Wooting plugin dir)
pkgbuild --root /tmp/uap-stage \
  --install-location /usr/local/share/WootingAnalogPlugins \
  --identifier com.krookedlilly.universalanalogplugin.macos \
  --version 1.0.0 \
  UAP-macOS-unsigned.pkg

# 4. sign the pkg with the *Installer* cert (different cert than step 1!)
productsign --sign "Developer ID Installer: <NAME> (<TEAMID>)" \
  UAP-macOS-unsigned.pkg UAP-macOS-1.0.0.pkg

# 5. notarize + staple
xcrun notarytool submit UAP-macOS-1.0.0.pkg --keychain-profile "<AC_PROFILE>" --wait
xcrun stapler staple UAP-macOS-1.0.0.pkg
spctl --assess --type install -vv UAP-macOS-1.0.0.pkg   # verify accepted
```
Dual-cert reminder: dylib → **Developer ID Application**; pkg → **Developer ID Installer**.

### Step C — Test on the Intel Mac
Install the pkg on the Intel machine, grant Input Monitoring, launch the mod, verify analog WASD
+ clean exit. Validates x86_64 (we only verified arm64).

### Step D — Distribute
One **GitHub Release** on `KrookedLilly/universal-analog-plugin` holding the signed pkg — single
source linked from both the mod and the Unity asset.

### Step E — Install guide + announcement
- macOS install guide: install Wooting SDK → run the pkg → **grant Input Monitoring** → launch.
- Changelog/announcement for the mod's store listing + the Unity Asset Store listing. Framing:
  "interim KrookedLilly build while the fix is under review upstream ([Soup#180]); will be
  replaced by the official plugin once merged."

## Open decisions (ask the user on resume)

1. `.pkg` installer (recommended) vs plain signed zip?
2. Version string (e.g. `macos-1.0.0`) and release home — fork repo release, the mod's release page, or both?
3. Intel-Mac test before or after signing?

## Cross-references

- Full engineering writeup: `macos-spike/FINDINGS.md`.
- Spec/plan: he-keyboard `docs/superpowers/specs|plans/2026-06-23-macos-universal-analog-plugin-spike.md`.
- Product handoff entry: he-keyboard `docs/klpm-he-keyboard-handoff.md` (2026-06-24).
- Upstream PR: https://github.com/calamity-inc/Soup/pull/180
