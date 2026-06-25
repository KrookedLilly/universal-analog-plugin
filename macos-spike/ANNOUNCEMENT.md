# Announcement / changelog copy — macOS cross-vendor support

Reusable copy for the mod's store/listing pages, the Unity Asset Store listing, and social.
Framing per the release: the fix is now **merged upstream** ([Soup#180](https://github.com/calamity-inc/Soup/pull/180)),
and this remains an **interim KrookedLilly build** until the Universal Analog Plugin ships a macOS
release built against the updated Soup — that official release will replace this one.

---

## Short (changelog line)

> **macOS: non-Wooting Hall-effect keyboards now work.** Keychron, DrunkDeer, Madlions and other
> command-based HE boards now deliver analog WASD on macOS via our fixed Universal Analog Plugin.
> (Wooting boards already worked; Razer remains Windows-only.)

## Medium (store/listing blurb)

> ### macOS now supports non-Wooting HE keyboards
> Until now, HE-Keyboard on macOS only worked with Wooting boards — the community Universal Analog
> Plugin that adds other brands didn't function on the Mac. We tracked it down to four bugs in the
> plugin's macOS HID layer, fixed them, and verified analog WASD in-game with a Keychron Q5 HE.
>
> If you're on macOS with a Keychron, DrunkDeer, Madlions, or similar HE keyboard, install the
> Wooting Analog SDK plus our macOS Universal Analog Plugin build and you're set. The fixes are
> merged upstream ([Soup#180](https://github.com/calamity-inc/Soup/pull/180)); this is an interim
> build until the official plugin ships a macOS release that includes them.
>
> Install guide → [link to INSTALL-macOS.md / release]
> (Razer analog keyboards remain unsupported on macOS — they require Windows-only Synapse.)

## Long (blog / dev-update)

> **Cross-vendor analog input comes to macOS**
>
> HE-Keyboard reads your Hall-effect keyboard through the Wooting Analog SDK. Wooting boards are
> supported directly; every other brand rides in through the community
> [Universal Analog Plugin](https://github.com/AnalogSense/universal-analog-plugin) (UAP). On
> Windows and Linux that's worked for a while — but on macOS the plugin's device layer (the `Soup`
> HID library) simply didn't function for boards that need command writes, which is most non-Wooting
> HE keyboards (Keychron, DrunkDeer, Madlions).
>
> We dug in and found four distinct macOS HID bugs: the IOHIDManager being closed out from under
> its own devices, a report-ID byte not being stripped, a shared device handle being closed during
> routine re-discovery, and a run-loop being torn down from the wrong thread at shutdown. With those
> fixed, analog WASD works end-to-end — verified in Minecraft with a Keychron Q5 HE, clean startup
> and shutdown.
>
> The fixes are open-source and now merged upstream in
> [calamity-inc/Soup#180](https://github.com/calamity-inc/Soup/pull/180). Until they ship in an
> official UAP macOS release, macOS users of non-Wooting HE boards can use our signed, notarized build.
> The same fix also makes our Unity HE-Keyboard asset support these boards on macOS — same native
> SDK chain, no code change.
>
> Get it: [release link]. Razer analog stays Windows-only (it needs Synapse).

---

## Channels checklist (where to post)

- [ ] HE-Keyboard mod store/distribution listing (Modrinth/CurseForge/etc.) — medium blurb + install link
- [ ] HE-Keyboard `README.md` — already updated with the macOS fork note
- [ ] Unity Asset Store listing for the HE-Keyboard asset — medium blurb (note macOS now covered)
- [ ] Fork repo `README` / release page — short line linking the release
- [ ] Any social / community post — short line
