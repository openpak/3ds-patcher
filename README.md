# OpenPak nn-nimbus — 3DS patches and account manager for OpenPak

Fork of Pretendo's nimbus. The Luma patches swap `nintendo.net` / `nintendowifi.net` for
the same-length `openpak.org` in the HTTP, socket (NNCS), account, friends and Miiverse
modules, and the SSL patch disables root-CA verification, so the console reaches OpenPak's
servers for those names.

## Building (OpenPak)

One command, no local toolchain:

    docker/build.sh

That builds the toolchain image once (`docker/tools.Dockerfile` — devkitARM, libctru,
CTRPluginFramework, 3gxtool, armips, makerom, bannertool, tex3ds, flips), then builds the
plugin (`nimbus.3gx`), the app (`nimbus.cia`, `nimbus.3dsx`) and — when the six sysmodule
dumps are in place — the IPS patches, into `out/`.

The patches cannot be built from source alone: they are IPS diffs against decrypted
sysmodule code that only a real console can provide. Dump the six modules per
`DECOMPRESSING.md` and copy each dump to `patches/<module>/code.bin` (act, friends, http,
socket, ssl, miiverse), then re-run `docker/build.sh`. The dumps are gitignored — decrypted
Nintendo code never leaves your console.

Releases: push a `vX.Y.Z` tag and CI attaches `nimbus.cia`, `nimbus.3dsx` and
`nimbus.3gx` to a GitHub release. Patch builds stay local, for the same reason.

---

# Nimbus
## Pretendo account manager for the 3DS

Upstream README below. On OpenPak: **the "Pretendo" button is the OpenPak path** — the same
unofficial-environment selection, pointed at `account.openpak.org` / `nasc.openpak.org` by the
patches. (The on-screen Pretendo artwork is upstream's; renaming it is cosmetic and has not
been done.)

## Usage (OpenPak)

1. Grab `nimbus.cia` (or `nimbus.3dsx`) and `nimbus.3gx` from the `releases` (see the git tags) page
2. Copy `3ds/nimbus/` from the release (or your own `out/combined_out/`) to the root of your 3DS SD card
3. Install `nimbus.cia` with FBI (or FBI Reloaded)
4. Reboot holding SELECT and ensure "Enable loading external FIRMs and modules" and "Enable game patching" are both on (Luma3DS 13.0 or higher)
5. Run Nimbus and select the **Pretendo** (= OpenPak) account, then sign in
6. Enable the plugin: Rosalina menu → "Plugin Loader" → "Enabled"

Steps 4 and 6 are only needed for the full patch set; once the six IPS patches are built and
copied to `3ds/nimbus/update/`, Luma applies them at boot. Until then the account manager
still runs and can carry an account — the game-facing pieces need the patches.

## Usage (upstream Pretendo)

1. Grab the latest app and IPS patches from the [upstream Releases](https://github.com/PretendoNetwork/nimbus/releases) page
2. Extract to the root of your 3DS SD card
3. Install the Nimbus homebrew using FBI (or FBI Reloaded) if using the CIA build
4. Run the Nimbus homebrew and select either to use a Pretendo or Nintendo account
     - If it doesn't work, reboot your 3DS while holding SELECT and ensure that "Enable loading external FIRMs and modules" and "Enable game patching" are both turned on, as well as ensuring that your Luma3DS version is 13.0 or higher.
5. Enable the Nimbus plugin by entering into the Rosalina menu and setting the "Plugin Loader" to "Enabled"

## Building

1. Clone the repository recursively using `git clone https://github.com/PretendoNetwork/nimbus --recursive`
    - If you have cloned the repository previously, please run `git pull` and `make clean` while in the nimbus folder to avoid errors and broken files
    - On top of that, if you cloned it before 1.0.2 released, you might also need to run `git submodule update --init --recursive` while in the nimbus folder
2. Install devkitARM, libctru 2.5.0 or later, [CTRPluginFramework](https://gitlab.com/thepixellizeross/ctrpluginframework), [3gxtool](https://gitlab.com/thepixellizeross/3gxtool), [armips](https://github.com/Kingcom/armips), [makerom](https://github.com/3DSGuy/Project_CTR), [bannertool](https://github.com/Steveice10/bannertool) and [flips](https://github.com/Alcaro/Flips)
3. Copy [decompressed `code.bin`](https://github.com/PretendoNetwork/nimbus/blob/main/DECOMPRESSING.md) files from the act, friends, http, miiverse, socket and ssl sysmodules in their respective `patches` directories (any Miiverse code.bin works for the miiverse module)
4. Run `make`

## Credits

Thanks to:

- [pinklimes](https://github.com/gitlimes) for the CIA version banner
- [TraceEntertains](https://github.com/TraceEntertains) for making a CIA version of Nimbus and maintaining the project
- [DaniElectra](https://github.com/DaniElectra) for making the 3DS HTTP and Socket patches and maintaining the project
- [SciresM](https://github.com/SciresM) for making the 3DS SSL patches
- [zaksabeast](https://github.com/zaksabeast) for the original 3ds-Friend-Account-Manager and all the research into the friends and act system titles
- [shutterbug2000](https://github.com/shutterbug2000) for the GUI
- [libctru](https://github.com/devkitPro/libctru) for the `frda.c` base, homebrew template, and other library functions (and thanks to citro2d for part of a system font function)
- [Universal-Core](https://github.com/Universal-Team/Universal-Core) for the string drawing functions
- [Fangal-Airbag](https://github.com/Fangal-Airbag) for making the account switcher GUI support button controls
- All other 3DS researchers
