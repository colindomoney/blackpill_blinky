# STM32F411 BlackPill — Complete Development Guide
WeAct STM32F411CEU6 (MiniF4) · macOS · STM32CubeIDE · ST-LINK V3SET

---

## Hardware

| Item | Detail |
|---|---|
| MCU | STM32F411CEU6 — Cortex-M4, 100 MHz, 512 KB flash, 128 KB SRAM |
| Board | WeAct MiniSTM32F4x1 (BlackPill) |
| Programmer | ST-LINK V3SET |
| SWD wiring | SWDIO → SWDIO, SWDCLK → SWDCLK, GND → GND |
| Power | BlackPill self-powered via USB — do **not** connect 3V3 from ST-LINK |

> The ST-LINK will report `Voltage: 0.00V` when Vref is unwired. This is normal and does not affect flashing or debugging.

---

## Project Structure

| Path | Purpose |
|---|---|
| `blackpill_blinky.ioc` | STM32CubeMX project — source of truth for pin config |
| `Core/` | CubeMX-generated HAL init, IRQ handlers, `main.c` |
| `App/` | Application code (not touched by CubeMX) |
| `Drivers/` | ST HAL + CMSIS (generated, do not edit) |
| `Debug/` | IDE build output — auto-generated, do not edit |
| `STM32F411CEUx_FLASH.ld` | Linker script |

---

## Environment Setup

### 1. STM32CubeIDE

Download and install from ST:
https://www.st.com/en/development-tools/stm32cubeide.html

Default install path: `/Applications/STM32CubeIDE.app`

### 2. STM32CubeProgrammer

Download and install from ST:
https://www.st.com/en/development-tools/stm32cubeprog.html

Default install path: `/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/`

### 3. Shell Paths (`~/.zshrc`)

Add the following — adjust version strings if your CubeIDE install differs:

```zsh
# ARM GNU toolchain (bundled with STM32CubeIDE)
export PATH="/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/\
com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.macos64_1.0.100.202602081740/\
tools/bin:$PATH"

# STM32CubeProgrammer CLI
export STLINK_CLI="/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/\
STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI"

# GDB (same toolchain as above)
export ARM_GDB="/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/\
com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.macos64_1.0.100.202602081740/\
tools/bin/arm-none-eabi-gdb"

# Convenience aliases
alias stlink-probe='$STLINK_CLI --list'
alias stlink-connect='$STLINK_CLI -c port=SWD'
```

Reload: `source ~/.zshrc`

### 4. OpenOCD

```zsh
brew install openocd
```

> **macOS note:** Use `interface/stlink-dap.cfg` — not `interface/stlink.cfg`. The DAP interface is required for ST-LINK V3 on macOS. The Homebrew `stlink` package (`st-info`, `st-flash`) does not support macOS as of v1.8.0 — use the ST CLI or OpenOCD instead.

---

## STM32CubeIDE Workflow

### Creating a Project

1. **File → New → STM32 Project**
2. Search for `STM32F411CE`, select it
3. Name your project, choose C, Executable, STM32Cube
4. CubeMX opens — configure peripherals (clock, GPIO, UART, etc.)
5. **Project → Generate Code** — populates `Core/` and `Drivers/`

### Pin & Clock Configuration (CubeMX)

- Open the `.ioc` file to reconfigure at any time
- BlackPill onboard LED: **PC13**, active low
- HSE: 25 MHz crystal on-board; configure PLL for 100 MHz SYSCLK
- After changes: **Project → Generate Code** — only `Core/Src/main.c` init sections are overwritten; code between `/* USER CODE BEGIN */` and `/* USER CODE END */` is preserved

### Building in the IDE

- **Project → Build Project** (`Cmd+B`)
- Output lands in `Debug/` — the Makefile there is auto-generated and is the canonical CLI build system
- First build after cloning: **Project → Build** to generate `Debug/` before using CLI

### Generating a Makefile Project (CLI builds)

The IDE generates a full GNU Makefile in `Debug/`. To build from the CLI without opening the IDE, the `Debug/` folder must exist (i.e. built at least once in the IDE):

```zsh
make -C Debug          # incremental build
make -C Debug clean    # clean
make -C Debug clean all  # full rebuild
```

If `Debug/` is deleted: open the IDE and do **Project → Build** to regenerate it.

---

## Probing the ST-LINK

### List connected probes

```zsh
$STLINK_CLI --list
```

### Connect and identify target

```zsh
$STLINK_CLI -c port=SWD
```

Expected output for the BlackPill:

```
ST-LINK SN  : 001700313234510533353533
ST-LINK FW  : V3J17M10B6S1
Board       : STLINK-V3SET
Voltage     : 0.00V          ← normal, Vref unwired
Device ID   : 0x431
Device name : STM32F411xC/E
NVM size    : 512 KBytes
Device CPU  : Cortex-M4
```

---

## Flashing

### Via STM32CubeProgrammer CLI (recommended on macOS)

```zsh
# Flash ELF (handles addresses automatically)
$STLINK_CLI -c port=SWD -w Debug/blackpill_blinky.elf -v -rst

# Flash binary at specific address
$STLINK_CLI -c port=SWD -w Debug/blackpill_blinky.bin 0x08000000 -v -rst

# Flash HEX
$STLINK_CLI -c port=SWD -w Debug/blackpill_blinky.hex -v -rst
```

Flags: `-v` = verify after write · `-rst` = reset MCU when done

### Via OpenOCD

```zsh
openocd \
  -f interface/stlink-dap.cfg \
  -f target/stm32f4x.cfg \
  -c "program Debug/blackpill_blinky.elf verify reset exit"
```

### Via IDE

**Run → Run As → STM32 C/C++ Application** — builds, flashes, and launches debug in one step.

---

## Debugging

Requires two terminals. Keep Terminal 1 running for the duration of the session.

### Terminal 1 — OpenOCD GDB Server

```zsh
openocd -f interface/stlink-dap.cfg -f target/stm32f4x.cfg
```

Successful output:

```
Info : STLINK V3J17M10B6S1 (API v3) VID:PID 0483:374F
Info : Target voltage: 3.291018
Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
Info : [stm32f4x.cpu] target has 6 breakpoints, 4 watchpoints
Info : Listening on port 3333 for gdb connections
```

### Terminal 2 — GDB Session

```zsh
$ARM_GDB Debug/blackpill_blinky.elf \
  -ex "target extended-remote :3333" \
  -ex "monitor reset halt" \
  -ex "load" \
  -ex "monitor reset init" \
  -ex "break main" \
  -ex "continue"
```

Use `extended-remote` (not `remote`) — survives target resets without restarting GDB.

### Essential GDB Commands

| Command | Effect |
|---|---|
| `monitor reset halt` | Halt the MCU |
| `monitor reset init` | Reset and run to main |
| `load` | Flash current ELF |
| `b main` | Breakpoint at function |
| `b Core/Src/main.c:42` | Breakpoint at file:line |
| `info registers` | Dump core registers |
| `x/10xw 0x20000000` | Examine 10 words of SRAM |
| `p variable_name` | Print variable value |
| `continue` | Run |
| `stepi` | Step one instruction |
| `nexti` | Step over one instruction |
| `finish` | Run to end of current function |
| `monitor shutdown` | Stop OpenOCD |
| `q` | Quit GDB |

### Debugging in STM32CubeIDE

**Run → Debug As → STM32 C/C++ Application** — full graphical debugger with register view, memory browser, live expressions, and RTOS thread awareness.

---

## Read / Dump Flash

```zsh
# Read entire flash to file
$STLINK_CLI -c port=SWD -r8 0x08000000 0x80000 dump.bin

# Read a specific region (e.g. first 4 KB)
$STLINK_CLI -c port=SWD -r8 0x08000000 0x1000 sector0.bin
```

---

## Mass Erase

```zsh
$STLINK_CLI -c port=SWD -e all
```

---

## Memory Map Reference (STM32F411)

| Region | Start | Size |
|---|---|---|
| Flash | `0x08000000` | 512 KB |
| SRAM | `0x20000000` | 128 KB |
| Peripherals | `0x40000000` | — |
| CoreSight / Debug | `0xE0000000` | — |
| DBGMCU_IDCODE | `0xE0042000` | Device ID register |

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `st-info --probe` → Found 0 | stlink-org v1.8.0 dropped macOS support | Use `$STLINK_CLI` or OpenOCD |
| OpenOCD `Error: open failed` with `stlink.cfg` | macOS USB driver conflict | Use `stlink-dap.cfg` instead |
| `Voltage: 0.00V` | Vref (3V3) not wired to ST-LINK | Normal for bus-powered BlackPill, ignore |
| CubeMX overwrites my code | Code outside USER CODE blocks | Always put app code between `/* USER CODE BEGIN */` markers |
| `Debug/` missing | Never built in IDE | Do **Project → Build** in CubeIDE once |
| ST-LINK COM LED flashing red, not enumerating | ST-LINK firmware needs update | Open STM32CubeIDE — it will prompt to update firmware automatically |

---

## Quick Reference

```zsh
# Probe
$STLINK_CLI --list
$STLINK_CLI -c port=SWD

# Build
make -C Debug

# Flash
$STLINK_CLI -c port=SWD -w Debug/blackpill_blinky.elf -v -rst

# Debug (Terminal 1)
openocd -f interface/stlink-dap.cfg -f target/stm32f4x.cfg

# Debug (Terminal 2)
$ARM_GDB Debug/blackpill_blinky.elf -ex "target extended-remote :3333" -ex "monitor reset halt" -ex "load" -ex "break main" -ex "continue"
```
