# Changelog — nn-nimbus

Generated from git history on 2026-09-15. `git log` stays the source
of truth; this file is the readable summary.

## v2.2.0 — 2026-11-04

- The app is **OpenPak 3DS Patcher**: one status screen with the same flow and wording as the Switch Patcher
  (OPENPAK / NINTENDO pill, Nintendo and OpenPak cards with `Active`, `A Select`, `B Exit`,
  `OpenPak selected. Reboot to apply?` with `A Reboot now` / `B Later`, `Failed: <reason>`). The Pretendo
  artwork and wording are gone.
- Backup and rollback: files OpenPak replaces are kept as `.original`/`.absent` in `/3ds/openpak/system`,
  selecting Nintendo restores them exactly, and a file changed by someone else is left alone.
- The plugin and Juxt certificate (and the game patches, when built) travel inside the CIA and are applied on
  first start, so a CIA install is a complete update. The old `/3ds/nimbus/update` folder still works.
- Selecting the network already in use re-applies it; the unconditional "Failed to apply Azahar configuration"
  message on success is fixed.
- `tests/run.sh` checks the backup and rollback rules on the host.

## v2.1.2 — 2026-11-04

- ci: runner image pinned, checkout@v5
- The per-console patchers renamed; docs point at docs.openpak.org; docs trimmed to README + protocol + deploy

- ci: release builds on `v*.*.*` tags only (was `openpak-v*`), CI on manual dispatch,
  artifacts kept 3 days [ef8dcd6, c4977bd]; docs commits

## openpak-v1 — 2026-09-12

- openpak.org names (same length as pretendo.cc) in every patch [c9c7f25]
- Socket patch resolves `*.openpak.org` and the NNCS names to the OpenPak box on the console;
  resolver symbols defined (`openpak_server_ip` was undefined), inline length walk instead of
  strlen [c349410, 270eff0, 7128b35]
- Build via docker (`docker/build.sh`), CTRPF built serially (Makefile races under -j), CI and
  `openpak-v*` releases [7128b35, 270a960, c94dca6]
- README: OpenPak usage — the Pretendo button is the OpenPak path; DS3-1 steps [d85bc0d]

## v2.1.1 — 2026-06-09



## v2.1.0 — 2026-02-15



## v2.0.2 — 2025-12-19



## v2.0.1 — 2025-12-13



## v2.0.0 — 2025-12-06



## v2.0.0-rc1 — 2025-08-18



## v1.6.1 — 2024-07-02



## v1.6.0 — 2024-04-07



## v1.5.0 — 2023-10-10



## v1.0.3 — 2023-07-24



## v1.1.0 — 2023-07-24



## v1.0.2 — 2023-07-06



## v1.0.0 — 2022-01-29



## v2.1.2 — 2022-01-29



## v2.1.2 — 2022-01-29

- Merge pull request #4 from shutterbug2000/alt-nasc-account-patches [4bcb6ab]
- Remove the env vars for act and nasc urls, we don't need them [8c8139d]
- Merge pull request #3 from shutterbug2000/alt-nasc-account-patches [9c228ea]
- lolwhoops [4efaf58]
- Make current account detection more reliable [f67f62b]
- Update README [2a289d8]
- Updated app metadata [0456305]
- explicitly set Pretendo URLs in patches [f6af7fd]
- Merge pull request #2 from PretendoNetwork/alt-nasc-account-patches [16375c0]
- Merge pull request #1 from shutterbug2000/alt-nasc-account-patches [350963d]
- whoops removed this on accident [3806c59]
- Fancy UI for Pretendo [25ca345]
- Add act account management [580b334]
- Add nasc and account patches [aca8a67]
- Create test env account [38a6447]
- Fix NascEnvironment [83a7d4f]
- Set client sdk version [5e3c607]
- Initial commit [9b16945]

