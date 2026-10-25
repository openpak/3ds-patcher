# Deploy: 3ds-patcher

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

## Usage (OpenPak)

1. Grab `nimbus.cia` (or `nimbus.3dsx`) and `nimbus.3gx` from the `releases` (see the git tags) page
2. Copy `3ds/nimbus/` from the release (or your own `out/combined_out/`) to the root of your 3DS SD card
3. Install `nimbus.cia` with FBI (or FBI Reloaded)
4. Reboot holding SELECT and ensure "Enable loading external FIRMs and modules" and "Enable game patching" are both on (Luma3DS 13.0 or higher)
5. Run Nimbus and select the **Pretendo** (= OpenPak) account, then sign in
6. Enable the plugin: Rosalina menu → "Plugin Loader" → "Enabled"
