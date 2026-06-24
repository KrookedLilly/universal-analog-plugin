# Universal Analog Plugin — macOS build `macos-1.0.0`

First macOS build of the [Universal Analog Plugin](https://github.com/AnalogSense/universal-analog-plugin) (UAP). It brings analog/variable-pressure input from **command-based Hall-effect keyboards** (Keychron, DrunkDeer, Madlions, and similar) to macOS through the Wooting Analog SDK — so any app built on that SDK (e.g. the [HE-Keyboard Minecraft mod](https://github.com/KrookedLilly/minecraft-he-keyboard), or a Unity project using the Wooting SDK) gets non-Wooting HE keyboards on the Mac.

The stock upstream UAP doesn't work on macOS for these boards: its HID layer (the `Soup` library) had four macOS bugs that broke every keyboard needing command writes. This build is a KrookedLilly fork with those bugs fixed.

> **Interim build.** The four fixes are proposed upstream in **[calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180)**. Once they land in an official UAP release, that release supersedes this one. Until then, macOS users of non-Wooting HE boards need this build.

## What's in the download

- **`UAP-macOS-1.0.0.pkg`** — signed (Developer ID) + notarized installer. Drops a **universal (`arm64` + `x86_64`)** `abiv1.dylib` into the Wooting SDK plugin directory (`/usr/local/share/WootingAnalogPlugins/universal-analog-plugin/`).
  - SHA-256: `204b37625727b7bfc74a51d0450f50754b69539fbaccf25396b677e34676ca97`

## Requirements

- macOS on Apple Silicon **or** Intel.
- The **Wooting Analog SDK runtime** installed first (this plugin slots into it). See the install guide.
- A supported HE keyboard in analog mode.

## Install

See **[INSTALL-macOS.md](INSTALL-macOS.md)**. Short version:

1. Install the Wooting Analog SDK runtime.
2. Run `UAP-macOS-1.0.0.pkg`.
3. **Grant Input Monitoring** to whatever launches your game/app (this is the #1 "nothing happens" cause).

## Supported boards on macOS

✅ Keychron (Q-series HE, etc.), DrunkDeer, Madlions, and other command-based HE keyboards the Wooting SDK + UAP recognize. Wooting boards already work natively (with or without this plugin).

❌ **Razer** Huntsman analog stays unsupported on macOS — it requires Razer Synapse, which is Windows-only.

## Verified

Analog WASD in Minecraft (HE-Keyboard mod) with a **Keychron Q5 HE** on Apple Silicon, plus clean shutdown. Intel runtime validated separately on this release's `.pkg`.

## Source

- Plugin fork: [`KrookedLilly/universal-analog-plugin`](https://github.com/KrookedLilly/universal-analog-plugin) (branch `macos`)
- HID fixes: [`KrookedLilly/Soup`](https://github.com/KrookedLilly/Soup) (branch `macos`) → upstream PR [calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180)
