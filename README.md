# 3DS Patcher

OpenPak's 3DS Patcher.

| | |
| --- | --- |
| Console | Nintendo 3DS |
| Kind | HOME Menu app, shipped as a CIA (`nimbus.cia`; also `nimbus.3dsx` for the Homebrew Launcher) |
| Download | [Releases](https://github.com/openpak/3ds-patcher/releases) (`vX.Y.Z` tags) |
| Based on | [Pretendo Nimbus](https://github.com/PretendoNetwork/nimbus) (GPL-3.0 as upstream) |
| Licence | GPL-3.0, see [LICENSE](LICENSE) |

## What it does

- Switches a custom-firmware 3DS between Nintendo's servers and OpenPak's, and back: pick **Nintendo** or **OpenPak**, then reboot.
- Keeps Nintendo's account and OpenPak's account in separate friend slots, so switching never deletes either.
- Installs the OpenPak plugin (`nimbus.3gx`) and the Juxt certificate, and the sysmodule and Miiverse game patches when they are included. Anything it replaces is backed up as `<name>.original` in `/3ds/openpak/system`, and selecting Nintendo puts the exact original back.
- The app carries those files inside itself, so a new CIA is a full update: the first start applies them and offers a reboot.
- Under the hood: the HTTP, socket (NNCS), account, friends and Miiverse patches swap `nintendo.net` / `nintendowifi.net` for the same-length `openpak.org`, and the SSL patch disables root-CA verification for those names.

## Install

1. Install `nimbus.cia` from the [release](https://github.com/openpak/3ds-patcher/releases) with FBI (or the Homebrew Launcher with `nimbus.3dsx`).
2. Start **OpenPak 3DS Patcher** from the HOME Menu and select **OpenPak**, then **Reboot now**.
3. Sign in with your OpenPak account on the console's account settings, and press **Y** in the app to turn on the plugin when you start a game session.
4. To go back, start the app, select **Nintendo** and reboot.

Needs Luma3DS 13 or newer with "Enable loading external FIRMs and modules" and "Enable game patching" on (hold SELECT at boot); the app checks and says so. Full steps and the manual route are in [deploy](docs/deploy.md).

## Build

    docker/build.sh

The IPS patches need the six decrypted sysmodule dumps; see [deploy](docs/deploy.md). `tests/run.sh` checks the backup and rollback rules on the host.

## Docs

- [Protocol](docs/protocol.md)
- [Deploy](docs/deploy.md)
- [Provenance](docs/provenance.md)

## Licence

See [LICENSE](LICENSE).

PRDs: openpak/prds (private)
