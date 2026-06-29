# PicoDualProbe Bug: CMSIS-DAP Vendor Endpoint Fails on Second OpenOCD Session

**Date:** 2026-06-29
**Firmware:** debugprobe_on_zero.uf2 (Jun 29 16:42 build, with watchdog dap_edpt_reset fix)
**Probe:** Waveshare RP2040-Zero, serial 50443405A87B281C
**Target:** NeoRV32 RISC-V on Alchitry Pt V2 (XC7A100T)
**OpenOCD:** 0.12.0
**Config:** `debugprobe/scripts/openocd/neorv32.cfg`

---

## Symptoms

| Connection # | Result | Error |
|-------------|--------|-------|
| 1st (after replug) | ✅ Works | CPU halted, PC read, GDB server started |
| 2nd | ❌ Fails | `CMSIS-DAP command CMD_DAP_JTAG_SEQ failed` |
| 3rd | ❌ Fails | `CMSIS-DAP command CMD_INFO failed` (USB stack dead) |
| After replug | ✅ Works again | Cycle repeats |

## What Works

- First connection after USB replug: full JTAG scan, IDCODE read, RISC-V examine, halt, PC read, GDB server
- Board UART survives if OpenOCD does `resume` before `shutdown`
- Probe USB stays visible on `lsusb` even after failure (device doesn't disconnect)

## What Fails

The CMSIS-DAP vendor endpoint (bulk USB endpoint) doesn't properly reset its internal state when OpenOCD disconnects and reconnects. The second session's first JTAG sequence command (`CMD_DAP_JTAG_SEQ`) fails, indicating the DAP firmware's JTAG state machine or USB buffer is in a bad state.

By the third attempt, even `CMD_INFO` (the simplest CMSIS-DAP command) fails, meaning the USB vendor endpoint itself is dead.

## NeoRV32 JTAG DTM Specifics

From `neorv32_debug_dtm.vhd` (RISC-V debug spec 0.13/1.0 compatible):

### TAP Parameters
- **IR length:** 5 bits (OpenOCD config: `irlen 5`)
- **IDCODE:** `0x00000001` (version=0, part=0x0000, manid=0x000)
- **DTMCS register:** `0x00000071` (hardcoded)
  - Bits [3:0] = 0x1 → `abits` = 1? No — see below
  - Actually: `x"00000071"` = `0b00000000_00000000_00000000_01110001`
  - abits = bits[3:0] = 0x1 → **1-bit DMI address** (wrong, see note)
  - Actually the DTMCS is 32 bits: version[3:0]=1, abits[9:4]=0x7, idle[18:10]=0, dmireset[16]=0, dmihardreset[17]=0, 0=rest
  - **abits = 7** → 7-bit DMI address (matches `dmi.addr` being 7 bits in VHDL)
  - **version = 1** → debug spec 1.0

### DMI (Debug Module Interface)
- **dmi register:** 41 bits = 7-bit addr + 32-bit data + 2-bit op/status
- **IR instructions:**
  - `00001` = IDCODE
  - `10000` = DTMCS (status/control)
  - `10001` = DMI (access debug module)
  - `11111` = BYPASS

### DTMCS Reset Operations
- **dmireset** (bit 16): clears sticky error, resets DMI state
- **dmihardreset** (bit 17): full hard reset of DMI controller

### Key Observation
The NeoRV32 DTM has a `busy` flag in the DMI controller. If OpenOCD exits mid-operation (while `busy=1`), the DMI controller stays in a busy state. The next OpenOCD session tries to scan JTAG, but the DTM's DMI controller is still waiting for a response from the debug module.

**The fix on the OpenOCD side:** before shutdown, write DTMCS with `dmireset=1` (bit 16) to clear the DMI state. Or write `dmihardreset=1` (bit 17) for a full reset.

### The Probe Side

The PicoDualProbe's CMSIS-DAP firmware (`tusb_edpt_handler.c`) uses a circular buffer with `DAP_PACKET_COUNT=8` and `DAP_PACKET_SIZE=512`. When OpenOCD disconnects:

1. `tud_unmount_cb` fires → deletes DAP task, UART task, autobaud task
2. `was_configured = 0`
3. When OpenOCD reconnects → `tud_mount_cb` fires → recreates all tasks
4. The USB stack calls `dap_edpt_reset` (zeros buffers) and `dap_edpt_open` (queues first OUT transfer)

The problem: between unmount and mount, the **DAP library internal state** (in `CMSIS_DAP/` source) is NOT reset. The DAP library has its own JTAG state machine that persists across USB sessions. The `dap_edpt_reset` only zeros the USB buffers, not the DAP library's internal JTAG/SWD state.

## Potential Fixes

### Fix 1: Call DAP_Setup() on reconnect (probe firmware)
In `tud_mount_cb`, before creating the DAP task, call `DAP_Setup()` to reinitialize the DAP library:
```c
void tud_mount_cb(void)
{
  if (!was_configured) {
    DAP_Setup();  // reinitialize DAP library state
    // ... create tasks ...
  }
}
```

### Fix 2: Write DTMCS dmihardreset before shutdown (OpenOCD config)
Add to the OpenOCD config:
```
init
halt
# ... do stuff ...
# Before shutdown, reset the DTM:
riscv set_prefer_sba off
resume
shutdown
```
Or use a telnet script that sends `dtmcs_write 0x20000` (bit 17 = dmihardreset) before `shutdown`.

### Fix 3: Keep OpenOCD running (workaround)
Don't let OpenOCD exit between sessions. Run it as a persistent daemon with telnet port 4444. The MCP server connects to telnet, sends commands, disconnects from telnet — but OpenOCD stays running, keeping the USB endpoint alive.

This is the approach in the PROBE-INTEGRATION-PLAN.md — OpenOCD as a background service.

## Test Results

### Clean resume + shutdown (Fix 2 attempt)
```
openocd -f safe.cfg -c "init" -c "halt" -c "reg pc" -c "resume" -c "shutdown"
→ First session: ✅ PC=0x31c
→ Board UART after: ✅ alive (resume worked)
→ Second session: ❌ CMD_DAP_JTAG_SEQ failed
```
Resume fixes the board, but the probe still can't reconnect.

### Watchdog dap_edpt_reset (Fix 1 attempt)
```
Added dap_edpt_reset(0) to dev_mon watchdog before tud_init
→ No improvement: second connection still fails
```
The watchdog never fires because USB is fine — it's the DAP library state that's broken.

## Next Steps

1. **Try Fix 1** — add `DAP_Setup()` to `tud_mount_cb` in main.c, rebuild, reflash, test
2. **Try Fix 3** — run OpenOCD as persistent daemon, test multiple telnet sessions
3. **If neither works** — the bug is in the CMSIS-DAP library itself (`CMSIS_DAP/CMSIS/DAP/Firmware/`), need to trace what state persists

## Files

- Probe firmware: `debugprobe/src/main.c`, `debugprobe/src/tusb_edpt_handler.c`
- DAP config: `debugprobe/include/DAP_config.h` (DAP_PACKET_SIZE=512, DAP_PACKET_COUNT=8)
- OpenOCD config: `debugprobe/scripts/openocd/neorv32.cfg`
- NeoRV32 DTM: `neorv32-main/rtl/core/neorv32_debug_dtm.vhd`
- NeoRV32 DM: `neorv32-main/rtl/core/neorv32_debug_dm.vhd`