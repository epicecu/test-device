#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
table_dir=${TABLE_DIR:-"$root/../table"}
expected="$table_dir/proto/table/v1/table.proto"
actual="$root/device-config/protobuf/table/v1/table.proto"

test -f "$expected" || {
  echo "TABLE_DIR must identify EpicECU Table 1.0.0." >&2
  exit 1
}
test "$(git -C "$table_dir" rev-parse HEAD)" = 98f8b9a8d5721caeda80860fb6a607480d89c00a || {
  echo "TABLE_DIR must be pinned to EpicECU Table commit 98f8b9a8d5721caeda80860fb6a607480d89c00a." >&2
  exit 1
}
cmp "$expected" "$actual"
