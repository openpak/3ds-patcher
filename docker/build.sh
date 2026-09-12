#!/usr/bin/env bash
# Build nn-nimbus the way OpenPak builds it.
#
#   docker/build.sh
#
# Builds the toolchain image once (docker/tools.Dockerfile), then builds inside
# it: the plugin (.3gx), the app (CIA + 3DSX), and — when decrypted sysmodule
# dumps are in place — the six IPS patches. Artifacts land in out/.
set -euo pipefail
cd "$(dirname "$0")/.."

docker build -f docker/tools.Dockerfile -t nn-nimbus-tools:latest .

exec docker run --rm -v "$PWD":/src nn-nimbus-tools:latest bash docker/build-inner.sh
