#!/usr/bin/env bash
set -e

if [ "$(uname -s)" = "Darwin" ]; then
  EXT=dylib
else
  EXT=so
fi

# Build universal-analog-plugin
sun abiv0
sun abiv1
mkdir -p universal-analog-plugin
mv "libabiv0.$EXT" universal-analog-plugin/abiv0.$EXT
mv "libabiv1.$EXT" universal-analog-plugin/abiv1.$EXT

# Build universal-analog-plugin-with-wooting-device-support
sun abiv0-pluswooting
sun abiv1-pluswooting
mkdir -p universal-analog-plugin-with-wooting-device-support
mv "libabiv0-pluswooting.$EXT" universal-analog-plugin-with-wooting-device-support/abiv0.$EXT
mv "libabiv1-pluswooting.$EXT" universal-analog-plugin-with-wooting-device-support/abiv1.$EXT
