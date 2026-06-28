#!/bin/bash
# PicoDualProbe — Verification Script
# Tests SWD and JTAG functionality against real targets.
#
# Prerequisites:
#   - PicoDualProbe firmware flashed on a Pico (GP2=SWCLK, GP3=SWDIO, GP4=TDI, GP5=TDO, GP6=nRST)
#   - OpenOCD 0.12+ installed
#   - For SWD test: another RP2040/Pico connected (SWCLK→GP2, SWDIO→GP3, GND→GND)
#   - For JTAG test: NEORV32 board connected (TCK→GP2, TMS→GP3, TDI→GP4, TDO→GP5, GND→GND)

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROBE_DIR="$(dirname "$SCRIPT_DIR")"
CFG_DIR="$PROBE_DIR/scripts/openocd"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

pass() { echo -e "${GREEN}PASS${NC}: $1"; }
fail() { echo -e "${RED}FAIL${NC}: $1"; }
info() { echo -e "${YELLOW}INFO${NC}: $1"; }

echo "============================================"
echo " PicoDualProbe Verification"
echo "============================================"
echo ""

# Check OpenOCD
if ! which openocd > /dev/null 2>&1; then
    fail "OpenOCD not found. Install: sudo apt install openocd"
    exit 1
fi
OPENOCD_VER=$(openocd --version 2>&1 | head -1)
info "OpenOCD: $OPENOCD_VER"

# Check probe is connected
info "Looking for PicoDualProbe..."
if ! lsusb 2>/dev/null | grep -qi "CMSIS-DAP"; then
    fail "No CMSIS-DAP probe found. Is the PicoDualProbe plugged in?"
    echo "  Expected: 'PicoDualProbe (CMSIS-DAP)' in lsusb output"
    exit 1
fi
pass "CMSIS-DAP probe detected"

# ── SWD Test ──
echo ""
echo "--- SWD Test (RP2040 target) ---"
info "Connect PicoDualProbe to target RP2040:"
info "  Probe GP2 (SWCLK) → Target SWCLK"
info "  Probe GP3 (SWDIO) → Target SWDIO"
info "  Probe GND        → Target GND"
echo ""
read -p "Press Enter when wired, or 's' to skip SWD test: " SWD_CHOICE

if [ "$SWD_CHOICE" != "s" ]; then
    SWD_OUTPUT=$(openocd -f "$CFG_DIR/picodualprobe-swd.cfg" -f target/rp2040.cfg -c "init; exit" 2>&1) || true
    if echo "$SWD_OUTPUT" | grep -q "0x0bc12477"; then
        pass "SWD: RP2040 DPIDR 0x0bc12477 detected"
    elif echo "$SWD_OUTPUT" | grep -q "Cortex-M0"; then
        pass "SWD: Cortex-M0+ core found (DPIDR not printed but core detected)"
    else
        fail "SWD: Could not detect RP2040 target"
        echo "$SWD_OUTPUT" | tail -20
    fi
else
    info "SWD test skipped"
fi

# ── JTAG Test ──
echo ""
echo "--- JTAG Test (NEORV32 target) ---"
info "Connect PicoDualProbe to NEORV32 JTAG:"
info "  Probe GP2 (TCK)  → NEORV32 jtag_tck"
info "  Probe GP3 (TMS)  → NEORV32 jtag_tms"
info "  Probe GP4 (TDI)  → NEORV32 jtag_tdi"
info "  Probe GP5 (TDO)  → NEORV32 jtag_tdo"
info "  Probe GP6 (nRST) → NEORV32 reset (optional)"
info "  Probe GND        → NEORV32 GND"
echo ""
read -p "Press Enter when wired, or 's' to skip JTAG test: " JTAG_CHOICE

if [ "$JTAG_CHOICE" != "s" ]; then
    echo ""
    info "Running JTAG chain scan..."
    JTAG_OUTPUT=$(openocd -f "$CFG_DIR/jtag-scan.cfg" 2>&1) || true
    
    if echo "$JTAG_OUTPUT" | grep -qi "tap.*0x[0-9a-f]\{8\}"; then
        TAP_ID=$(echo "$JTAG_OUTPUT" | grep -i "tap" | grep -oP '0x[0-9a-f]{8}' | head -1)
        if [ "$TAP_ID" = "0xFFFFFFFF" ] || [ "$TAP_ID" = "0x00000000" ]; then
            fail "JTAG: Bad IDCODE $TAP_ID — check wiring (TDO floating or stuck)"
        else
            pass "JTAG: IDCODE $TAP_ID detected"
        fi
    else
        fail "JTAG: No TAP found on scan chain"
        echo "$JTAG_OUTPUT" | tail -20
    fi
else
    info "JTAG test skipped"
fi

echo ""
echo "============================================"
echo " Verification complete"
echo "============================================"
