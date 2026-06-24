# Installing the Universal Analog Plugin on macOS

This lets **non-Wooting** Hall-effect keyboards (Keychron, DrunkDeer, Madlions, …) deliver
analog/variable-pressure input on macOS, through the Wooting Analog SDK. Apps built on that SDK
— like the HE-Keyboard Minecraft mod or a Unity project — then see your keyboard.

If you have a **Wooting** board, you don't need this plugin at all — Wooting works natively. This
is only for other brands.

> ❌ **Razer** analog keyboards are not supported on macOS (they need Razer Synapse, Windows-only).

---

## Step 1 — Install the Wooting Analog SDK runtime

The plugin slots *into* the Wooting SDK, so the SDK must be installed first. It's two native
libraries that go in `/usr/local/lib/`.

1. Download the latest macOS `.tar.gz` from the
   [Wooting Analog SDK releases](https://github.com/WootingKb/wooting-analog-sdk/releases).
2. Copy both dylibs into place and clear the quarantine flag (macOS blocks unsigned downloaded
   dylibs until you do):

   ```bash
   sudo cp libwooting_analog_sdk.dylib     /usr/local/lib/
   sudo cp libwooting_analog_wrapper.dylib /usr/local/lib/
   sudo xattr -d com.apple.quarantine /usr/local/lib/libwooting_analog_sdk.dylib
   sudo xattr -d com.apple.quarantine /usr/local/lib/libwooting_analog_wrapper.dylib
   ```

## Step 2 — Install the Universal Analog Plugin

1. Download **`UAP-macOS-1.0.0.pkg`** from this release.
2. Double-click it and follow the installer (it asks for your password — it writes into a
   system directory). It installs to:

   ```
   /usr/local/share/WootingAnalogPlugins/universal-analog-plugin/abiv1.dylib
   ```

   The installer is signed and notarized, so Gatekeeper won't complain.

## Step 3 — Grant Input Monitoring  ⚠️ the important one

macOS won't let *anything* read your keyboard at the HID level without **Input Monitoring**
permission — and it's granted to the **app that launches the game**, not to "java" and not to the
plugin.

1. **System Settings → Privacy & Security → Input Monitoring**
2. Turn it on for your launcher:
   - **Prism Launcher** → grant *Prism Launcher*
   - **Official Minecraft launcher** → grant the *Minecraft* app
   - Launching from a terminal (e.g. `./gradlew runClient`) → grant *Terminal* / *iTerm*
3. If the app was already running, quit and relaunch it after granting.

> If movement does nothing, this is almost always the cause. The plugin loads fine but reads
> zero pressure until the launcher has Input Monitoring.

## Step 4 — Launch and verify

Start your game/app the normal way. In the HE-Keyboard Minecraft mod, open
**Mods → HE-Keyboard → Config** — the status panel should show the SDK loaded and at least one
device connected. In-world, a partial WASD press = partial walk speed.

---

## Troubleshooting

- **No analog movement at all** → Input Monitoring (Step 3). Re-check it's the *launcher* that's
  granted, and relaunch.
- **SDK "not found" in the mod's status panel** → the Wooting dylibs aren't in `/usr/local/lib/`
  or weren't `xattr`-cleared (Step 1).
- **Device not detected, but Input Monitoring is on** → confirm the keyboard is in analog mode and
  is a supported brand (Keychron / DrunkDeer / Madlions / other command-based HE). Razer is not
  supported on macOS.
- **Works in dev but not after install** (developers) → make sure you're loading the installed
  `abiv1.dylib`, not a stale build elsewhere.

## Uninstall

```bash
sudo rm -rf /usr/local/share/WootingAnalogPlugins/universal-analog-plugin
```
(Leaves the Wooting SDK itself in place.)
