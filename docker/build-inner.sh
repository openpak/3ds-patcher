#!/usr/bin/env bash
# Runs inside the container (see build.sh). Builds everything that can be built
# from source: the app (CIA + 3DSX) and the plugin (.3gx). The sysmodule IPS
# patches additionally need code.bin dumps from a real console; if all six are
# present the patches are built too and the full release layout is assembled.
set -euo pipefail
cd /src

echo "== tools =="
for t in armips flips makerom bannertool 3gxtool tex3ds; do
    command -v "$t" >/dev/null || { echo "missing tool: $t" >&2; exit 1; }
    printf '  %-12s %s\n' "$t" "$(command -v "$t")"
done

echo "== plugin (nimbus.3gx) =="
# serial on purpose: the recursive devkitPro Makefiles misorder the link under -j
make -C plugin clean >/dev/null
make -C plugin

echo "== app (nimbus.3dsx, nimbus.cia) =="
make -C app clean >/dev/null
make -C app

# The six dumps the patches need, and the module each one patches.
DUMPS=(
    patches/act/code.bin
    patches/friends/code.bin
    patches/http/code.bin
    patches/socket/code.bin
    patches/ssl/code.bin
    patches/miiverse/code.bin
)
missing=0
for f in "${DUMPS[@]}"; do
    [ -f "$f" ] || { echo "no dump: $f"; missing=1; }
done

if [ "$missing" = 0 ]; then
    echo "== patches (all six dumps present) =="
    make -C patches clean >/dev/null
    make -C patches -j"$(nproc)"
else
    echo "== patches SKIPPED =="
    echo "   The IPS patches are built from decrypted sysmodule code, which only a"
    echo "   real 3DS can provide. Dump the six modules (see DECOMPRESSING.md), copy"
    echo "   each one over the path listed above, and re-run docker/build.sh."
fi

echo "== assembling out/ =="
rm -rf out
UPDATE=out/combined_out/3ds/nimbus/update
mkdir -p "$UPDATE"
cp plugin/plugin.3gx "$UPDATE/nimbus.3gx"
touch "$UPDATE/update.txt"
cp patches/miiverse/juxt-prod.pem "$UPDATE/"

emit() { # module dir, then every title id it patches
    local d=$1; shift
    [ -f "patches/$d/out/code.ips" ] || return 0
    for t in "$@"; do cp "patches/$d/out/code.ips" "$UPDATE/$t.ips"; done
}
emit act      0004013000003802
emit friends  0004013000003202
emit http     0004013000002902
emit socket   0004013000002E02
emit ssl      0004013000002F02
emit miiverse 000400300000BC02 000400300000BD02 000400300000BE02

# the app and plugin, in each output flavour
for out in out/3dsx_out out/cia_out; do
    cp -r out/combined_out/. "$out/"
done
mkdir -p out/3dsx_out/3ds out/cia_out/cias out/combined_out/cias
cp app/nimbus.3dsx out/3dsx_out/3ds/
cp app/nimbus.3dsx out/combined_out/3ds/
cp app/nimbus.cia out/cia_out/cias/
cp app/nimbus.cia out/combined_out/cias/

echo "== done =="
find out -type f | sort
