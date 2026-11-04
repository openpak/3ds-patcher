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

1. Install `nimbus.cia` from the release with FBI (or run `nimbus.3dsx` from the Homebrew Launcher).
2. Hold SELECT while booting and make sure "Enable loading external FIRMs and modules" and "Enable game patching" are both on (Luma3DS 13.0 or higher). The app shows both and says which is off.
3. Start **OpenPak 3DS Patcher** from the HOME Menu. The top screen shows the status pill (`OPENPAK` or `NINTENDO`) and how many OpenPak files are installed.
4. Select **OpenPak** (A, or tap the card). The message `OpenPak selected. Reboot to apply?` appears: **A** reboots now, **B** later.
5. Sign in with your OpenPak account in the console's account settings. Press **Y** in the app to turn the plugin loader on for the session.
6. To return to Nintendo: start the app, select **Nintendo**, reboot. The files OpenPak replaced are restored exactly.

Selecting the network already in use re-applies it (`OpenPak setup updated. Reboot to apply?`). **X** unlinks the OpenPak account from the console.

### What is backed up

OpenPak's files live in `/3ds/openpak/system` as `<name>.managed`. Before one replaces a file of yours, your file is copied to `<name>.original` (or `<name>.absent` is written when there was none). Selecting Nintendo restores those exactly. A file changed by somebody else after OpenPak wrote it is kept and the app says `System files changed; originals kept in /3ds/openpak/system`. Files covered: the five sysmodule patches in `/luma/sysmodules`, the three Miiverse `code.ips` in `/luma/titles`, `/luma/plugins/nimbus.3gx` and `/3ds/juxt-prod.pem`.

## Patch status

The six IPS patches are built from the maker's own decrypted dumps and are not in the release. When they are built (`docker/build.sh`), they are carried inside the CIA like the plugin and installed by the same switch.

## Appendix: manual route (developers)

Copy `3ds/nimbus/` from your own `out/combined_out/` to the SD card root and install `nimbus.cia` with FBI. Files in `/3ds/nimbus/update/` (with the file names of the old layout) are picked up and staged by the app on its next start, with the same backups. Without the patches the account switch still works; the game-facing pieces need them.
