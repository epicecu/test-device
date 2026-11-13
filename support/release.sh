#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=$(tr -d '\r\n' < "$root/VERSION")

case "$version" in
  0|0.*[!0-9.]*|*[!0-9.]*|*.*.*.*|.*|*.|*..*)
    echo "VERSION must be stable SemVer X.Y.Z." >&2
    exit 1
    ;;
esac

major=${version%%.*}
remainder=${version#*.}
minor=${remainder%%.*}
patch=${remainder#*.}
firmware_version=$((major * 1000000 + minor * 1000 + patch))

validate() {
  tag=${RELEASE_TAG:-}
  test "$tag" = "v$version" || {
    echo "Release tag must equal v$version." >&2
    exit 1
  }
  test "$major" -le 4294 && test "$minor" -le 999 && test "$patch" -le 999 || {
    echo "VERSION cannot be represented in Common1 firmware_version." >&2
    exit 1
  }
  grep -Fq -- "-D PROGRAMMOR_FIRMWARE_VERSION=$firmware_version" "$root/platformio.ini" || {
    echo "PlatformIO firmware version does not match VERSION." >&2
    exit 1
  }
  git -C "$root" fetch --quiet origin main
  tagged_commit=$(git -C "$root" rev-list -n 1 "$tag")
  git -C "$root" merge-base --is-ancestor "$tagged_commit" origin/main || {
    echo "Release tag must identify a commit already reachable from origin/main." >&2
    exit 1
  }
  printf '%s\n' "$version"
}

package() {
  source="$root/.pio/build/teensy41/firmware.hex"
  destination="$root/release/programmor-test-device-teensy41-v$version.hex"
  test -f "$source"
  mkdir -p "$root/release"
  cp "$source" "$destination"
  (cd "$root/release" && sha256sum "$(basename "$destination")" > "$(basename "$destination").sha256")
}

case "${1:-}" in
  validate) validate ;;
  package) package ;;
  *) echo "Use: support/release.sh validate|package" >&2; exit 2 ;;
esac
