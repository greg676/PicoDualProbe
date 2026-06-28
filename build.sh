#!/bin/bash
# Build PicoDualProbe firmware
# Usage: ./build.sh [pico|zero] [clean]
#
# Prerequisites:
#   - Pico SDK at /mnt/data-1tb/ai-design-studio/pico-sdk
#   - arm-none-eabi-gcc in PATH (system, not Xilinx)
#   - SEGGER tools NOT in PATH (they break picotool build)

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PICO_SDK_PATH="${PICO_SDK_PATH:-/mnt/data-1tb/ai-design-studio/pico-sdk}"

# Strip SEGGER from PATH (its 'cc' breaks ARM builds)
export PATH=$(echo "$PATH" | tr ':' '\n' | grep -v segger | tr '\n' ':')

TARGET="${1:-pico}"
CLEAN=0
if [ "$1" = "clean" ] || [ "$2" = "clean" ]; then
    CLEAN=1
fi

case "$TARGET" in
    zero)
        BUILD_DIR="$SCRIPT_DIR/build-zero"
        CMAKE_FLAGS="-DDEBUG_ON_ZERO=1 -DPICO_BOARD=pico"
        OUTPUT="debugprobe_on_zero.uf2"
        ;;
    pico)
        BUILD_DIR="$SCRIPT_DIR/build"
        CMAKE_FLAGS="-DDEBUG_ON_PICO=1 -DPICO_BOARD=pico"
        OUTPUT="debugprobe_on_pico.uf2"
        ;;
    clean)
        rm -rf "$SCRIPT_DIR/build" "$SCRIPT_DIR/build-zero"
        echo "Cleaned both build directories."
        exit 0
        ;;
    *)
        echo "Usage: $0 [pico|zero] [clean]"
        exit 1
        ;;
esac

if [ "$CLEAN" -eq 1 ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake \
    $CMAKE_FLAGS \
    -DCMAKE_C_COMPILER=/usr/bin/arm-none-eabi-gcc \
    -DCMAKE_CXX_COMPILER=/usr/bin/arm-none-eabi-g++ \
    -DCMAKE_ASM_COMPILER=/usr/bin/arm-none-eabi-gcc \
    -DPICOTOOL_FETCH_FROM_GIT_PATH=/mnt/data-1tb/ai-design-studio/picotool-build \
    "$SCRIPT_DIR"

make -j$(nproc)

echo ""
echo "=== Build complete: $TARGET ==="
ls -la "$BUILD_DIR/$OUTPUT"
echo ""
echo "To flash: hold BOOTSEL, copy $OUTPUT to RPI-RP2 drive"
