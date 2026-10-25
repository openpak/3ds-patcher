# 3DS Patcher

OpenPak's 3DS Patcher.

| | |
| --- | --- |
| Console | Nintendo 3DS |
| Kind | Account manager (`nimbus.cia`, `nimbus.3dsx`), plugin (`nimbus.3gx`) and IPS patches |
| Download | [Releases](https://github.com/openpak/3ds-patcher/releases) (`vX.Y.Z` tags) |
| Based on | [Pretendo Nimbus](https://github.com/PretendoNetwork/nimbus) (GPL-3.0 as upstream) |
| Licence | GPL-3.0, see [LICENSE](LICENSE) |

## What it does

- Swaps `nintendo.net` / `nintendowifi.net` for the same-length `openpak.org` in the HTTP, socket (NNCS), account, friends and Miiverse patches.
- Disables root-CA verification in the SSL patch, so the console reaches OpenPak's servers for those names.
- Offers the **Pretendo** button as the OpenPak account path.

## Install

1. Copy `3ds/nimbus/` from the release (or your own `out/combined_out/`) to the root of the SD card.
2. Install `nimbus.cia` with FBI (or FBI Reloaded), run it and select the **Pretendo** (= OpenPak) account.
3. Enable the Rosalina "Plugin Loader" (`nimbus.3gx`).

Full steps, including the Luma3DS settings, are in [deploy](docs/deploy.md).

## Build

    docker/build.sh

The IPS patches need the six decrypted sysmodule dumps; see [deploy](docs/deploy.md).

## Docs

- [Protocol](docs/protocol.md)
- [Evidence](docs/evidence.md)
- [Deploy](docs/deploy.md)
- [Notes](docs/notes.md)
- [Upstream README](docs/upstream.md)
- [Docs index](docs/README.md)

## Licence

See [LICENSE](LICENSE).
