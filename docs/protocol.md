# Protocol: 3ds-patcher

Fork of Pretendo's nimbus. The Luma patches swap `nintendo.net` / `nintendowifi.net` for
the same-length `openpak.org` in the HTTP, socket (NNCS), account, friends and Miiverse
modules, and the SSL patch disables root-CA verification, so the console reaches OpenPak's
servers for those names.
