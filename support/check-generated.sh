#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT HUP INT TERM

cp -R "$root/firmware/src/generated" "$temporary/original"
"$root/support/generate.sh"
diff -ru "$temporary/original" "$root/firmware/src/generated"
