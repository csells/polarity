# POLARITY: Storm Courier

An original Game Boy Color precision platformer and a mobile web player.

Every dash flips cyan ↔ amber. Gates matching your charge are safe. Eight single-screen rooms introduce precision jumps, wall jumps, directional dashes, airborne refills, and alternating electric gates. Touch the lime ring to advance. Death restarts the current room instantly; retries are unlimited.

## Play

Run `python3 server.py`, then open port 8787. The web player includes multitouch controls, keyboard and gamepad input, sound effects, pause, fullscreen, and a ROM download.

- Arrows / WASD: move and aim the dash
- Z / Space: jump (hold for height)
- X / Shift: dash and flip charge
- Enter: pause
- R: retry room

Standard browser-mapped controllers (USB or Bluetooth): D-pad / left stick to move and aim; bottom face button (Xbox A / PlayStation Cross) to jump; right or left face button (B / Circle or X / Square) to dash; Start / Options to start, pause and resume; Back / View / Share to retry. Press a button after connecting so the browser detects it. If audio remains locked, tap **Enable Sound** once. Disconnecting the active controller pauses the game.

On hardware: D-pad to move, A to jump, B to dash, Start to pause, Select to retry.

## Build

`./build.sh` generates tile art and levels, compiles C with GBDK 2020 4.5.0, and produces `web/polarity.gbc`. The included local toolchain is macOS ARM64; on another platform, download the matching [GBDK release](https://github.com/gbdk-2020/gbdk-2020/releases/tag/4.5.0) into `tools/gbdk`.

The ROM is 32 KiB and targets Game Boy Color hardware. Progress lasts for the current session; refreshing the browser starts a new run.

## Verification

`cc -O2 -o tools/solve tests/solve.c src/engine.c src/levels.c && ./tools/solve` searches for a route through every room using the actual C physics engine, producing replayable inputs in `artifacts/`.

`node tests/browser.cjs` checks boot and desktop/mobile rendering. `node tests/replay.cjs` replays all eight room solutions in the actual cartridge, stepping its CPU to input polls. `node tests/touch.cjs` checks simultaneous touch controls, diagonal dashes, pause, sound, and phone layouts. `node tests/controller.cjs` verifies standard controller inputs through the browser Gamepad API using simulated controller reports, including start with locked audio, pause/resume, disconnect/reconnect, dead zones, and focus safety. Physical USB/Bluetooth hardware requires a device/browser that exposes the standard mapping. Install the dev dependency with `npm install`.

## Files

- `src/engine.c`: fixed-point movement, collision and charge rules
- `tools/make_assets.py`: original pixel art and eight room layouts
- `src/main.c`: Game Boy renderer, input, sound and game flow
- `web/`: standalone static site; no production server dependencies

## Credits

The web emulator is [binjgb](https://github.com/binji/binjgb), copyright Ben Smith, MIT licensed. Its license is included in `web/vendor/LICENSE-binjgb`. The cartridge is built with [GBDK 2020](https://gbdk.org/). Original game code, room layouts, and pixel artwork created for this project. No assets from Celeste, Mega Man, or Mario are used.
