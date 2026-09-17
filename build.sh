#!/bin/sh
set -eu
cd "$(dirname "$0")"
python3 tools/make_assets.py
mkdir -p build
./tools/gbdk/bin/lcc -msm83:gb -Wm-yC -Wl-yt0x00 -Wl-yo2 -Wm-ynPOLARITY -Wl-m -Wl-j -o "$PWD/build/polarity.gbc" src/main.c src/engine.c src/levels.c
cp build/polarity.gbc web/polarity.gbc
printf 'Built web/polarity.gbc (%s bytes)\n' "$(wc -c < web/polarity.gbc)"
