#!/bin/sh
set -eu
cd "$(dirname "$0")"
ARM_CC="${ARM_CC:-$PWD/tools/arm-toolchain/bin/arm-none-eabi-gcc}"
ARM_OBJCOPY="${ARM_OBJCOPY:-${ARM_CC%gcc}objcopy}"
if [ ! -x "$ARM_CC" ]; then echo 'ARM compiler missing. See README.md for toolchain setup.' >&2; exit 1; fi
python3 tools/make_assets.py
python3 tools/story.py
python3 tools/gba_assets.py
python3 tools/gba_rooms.py
python3 tools/gba_story.py
python3 tools/gba_music.py
mkdir -p build
"$ARM_CC" -DPOLARITY_ADVANCE -mcpu=arm7tdmi -marm -O2 -ffreestanding -fno-builtin -Wall -Wextra -Isrc -Isrc/gba -nostdlib -Wl,-T,src/gba/link.ld,-Map,build/polarity-gba.map -o build/polarity.elf src/gba/start.s src/gba/main.c src/gba/audio.c src/gba/runtime.c src/engine.c src/gba/generated/levels.c src/city.c src/gba/generated/story.c src/gba/generated/assets.c src/gba/generated/music.s -lgcc
"$ARM_OBJCOPY" -O binary build/polarity.elf build/polarity.gba
"${ARM_CC%gcc}nm" -n build/polarity.elf > build/polarity-gba.sym
python3 tools/gba_header.py
cp build/polarity.gba web/polarity.gba
