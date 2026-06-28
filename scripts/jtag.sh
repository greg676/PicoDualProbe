#!/bin/bash
# PicoDualProbe — JTAG wrapper with USB reset
# Usage: ./jtag.sh <config_file>
# Always resets the probe USB first to avoid CMD_INFO/CMD_DAP_JTAG_SEQ failures

PROBE_SERIAL="50443405A87B281C"
CONFIG="${1:-scripts/openocd/neorv32-validate.cfg}"

cd /mnt/data-1tb/ai-design-studio/debugprobe

# Reset the probe USB endpoint
echo "Resetting probe USB..."
usbreset 2e8a:000c 2>/dev/null
sleep 2

# Verify probe is back
if ! ls /dev/ttyACM* >/dev/null 2>&1; then
    echo "ERROR: Probe not found after reset"
    exit 1
fi

echo "Probe ready. Running OpenOCD with $CONFIG..."
timeout 15 openocd -f "$CONFIG" 2>&1

# Reset probe after use to clean up
echo "Cleaning up probe USB..."
usbreset 2e8a:000c 2>/dev/null
sleep 1
echo "Done."