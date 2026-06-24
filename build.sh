#!/usr/bin/env bash
set -e

if [ "$(uname -s)" = "Darwin" ]; then
  EXT=dylib
  # On macOS the abiv0 variants need a macOS arm64 build of wooting_analog_common.a
  # (the repo ships a Linux ELF copy). The Wooting Analog SDK >= 0.7 uses C-plugin
  # ABI v1, so abiv1 is the variant it loads and abiv0 is best-effort here.
  ABIV0_REQUIRED=0
else
  EXT=so
  ABIV0_REQUIRED=1
fi

# build_variant <sun-target> <out-name> <out-dir> <required>
build_variant() {
  local target="$1" outname="$2" outdir="$3" required="$4"
  if sun "$target"; then
    mkdir -p "$outdir"
    mv "lib${target}.$EXT" "$outdir/${outname}.$EXT"
  elif [ "$required" = "1" ]; then
    echo "ERROR: required build '$target' failed" >&2
    exit 1
  else
    echo "WARN: skipping '$target' (build failed; not required on this platform)" >&2
  fi
}

# Build universal-analog-plugin
build_variant abiv0 abiv0 universal-analog-plugin "$ABIV0_REQUIRED"
build_variant abiv1 abiv1 universal-analog-plugin 1

# Build universal-analog-plugin-with-wooting-device-support
build_variant abiv0-pluswooting abiv0 universal-analog-plugin-with-wooting-device-support "$ABIV0_REQUIRED"
build_variant abiv1-pluswooting abiv1 universal-analog-plugin-with-wooting-device-support 1
