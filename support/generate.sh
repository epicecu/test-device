#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
generator=${NANOPB_GENERATOR:-"$root/.venv/bin/nanopb_generator"}
output="$root/firmware/src/generated"

if [ ! -x "$generator" ]; then
  echo "Nanopb generator is unavailable; run task setup first." >&2
  exit 1
fi

mkdir -p "$output"
PYTHONDONTWRITEBYTECODE=1 "$generator" \
  -I "$root/firmware/proto" \
  -D "$output" \
  -f "$root/firmware/proto/common.options" \
  -L '#include "vendor/nanopb/%s"' \
  --error-on-unmatched \
  "$root/firmware/proto/common.proto"

PYTHONDONTWRITEBYTECODE=1 "$generator" \
  -I "$root/device-config/protobuf" \
  -D "$output" \
  -f "$root/firmware/proto/test.options" \
  -L '#include "vendor/nanopb/%s"' \
  --error-on-unmatched \
  "$root/device-config/protobuf/test.proto"
