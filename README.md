# Blackpill (STM32F411) Blinky

Simple blinky example for the [WeAct STM32F411 MiniF4](https://github.com/WeActTC/MiniSTM32F4x1) (STM32F411CEU6).

---

## Project structure

| Path | Purpose |
|---|---|
| `blackpill_blinky.ioc` | STM32CubeMX project — source of truth for pin config |
| `Core/` | CubeMX-generated HAL init, IRQ handlers, main |
| `App/` | Application code (not touched by CubeMX) |
| `Drivers/` | ST HAL + CMSIS |
| `Debug/` | IDE build output — auto-generated, do not edit |
| `STM32F411CEUx_FLASH.ld` | Linker script |

---

## Prerequisites

### Toolchain

`arm-none-eabi-gcc` is bundled inside STM32CubeIDE. Add it to your shell profile (`~/.zshrc`):

```zsh
export PATH="/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/\
com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.macos64_1.0.100.202602081740/\
tools/bin:$PATH"
```

### OpenOCD

```zsh
brew install openocd
```

---

## Building from the CLI

The `Debug/` folder is auto-generated and kept up to date by STM32CubeIDE — it is the canonical build system for CLI use. Never edit it by hand.

```zsh
# build
make -C Debug

# clean build
make -C Debug clean && make -C Debug
```

If you nuke `Debug/` entirely, a Project → Build in the IDE recreates it from scratch.

---

## Flashing with STLink + OpenOCD

Connect the STLink programmer, then:

```zsh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "program Debug/blackpill_blinky.elf verify reset exit"
```

`verify` reads back flash to confirm the write. `reset exit` reboots the MCU and closes OpenOCD when done.

---

## Debugging with GDB + OpenOCD

Debugging requires two terminals.

**Terminal 1 — start OpenOCD** (exposes GDB server on :3333):

```zsh
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
```

**Terminal 2 — connect GDB:**

```zsh
arm-none-eabi-gdb Debug/blackpill_blinky.elf \
  -ex "target extended-remote :3333" \
  -ex "monitor reset halt" \
  -ex "load" \
  -ex "monitor reset init" \
  -ex "break main" \
  -ex "continue"
```

Use `extended-remote` rather than plain `remote` — it survives target resets and lets you re-flash without restarting GDB.

### Useful GDB commands

```
monitor reset halt     # halt the MCU
monitor reset init     # reset and run to main
load                   # flash the current elf
b app_main             # set a breakpoint by function name
info registers         # dump core registers
x/10xw 0x20000000     # examine 10 words of SRAM from base address
continue               # run
stepi                  # step one instruction
```

To quit: `monitor shutdown` then `q`.
