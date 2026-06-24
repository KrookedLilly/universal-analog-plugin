#!/usr/bin/env bash
# Build the standalone read harness against the installed Wooting wrapper.
set -e
WRAP_DIR="${WRAP_DIR:-/usr/local/lib}"

if [ ! -e "$WRAP_DIR/libwooting_analog_wrapper.dylib" ]; then
  echo "ERROR: libwooting_analog_wrapper.dylib not found in $WRAP_DIR" >&2
  echo "Install the Wooting Analog SDK first (wrapper + sdk dylib in /usr/local/lib)." >&2
  exit 1
fi

clang read_test.c -o read_test \
  -L"$WRAP_DIR" -lwooting_analog_wrapper \
  -Wl,-rpath,"$WRAP_DIR"

echo "built ./read_test (WRAP_DIR=$WRAP_DIR)"
