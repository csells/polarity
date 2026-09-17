# POLARITY: Storm Courier

An original Game Boy Color precision platformer and a mobile web player.

Every dash flips cyan ↔ amber. Gates matching your charge are safe. Explore **six locations with three rooms each** on a branching world map. Every room scrolls across four screens, with three checkpoint flags, unlimited retries, and automatic saved progress.

| Location | New feature | Unlock |
| --- | --- | --- |
| Spark District | Directional dash and charge gates | Start here |
| Iron Foundry | Patrolling sentries: stomp or dash through | Clear Spark District |
| Hanging Gardens | Ropes: hold up/down to climb, jump to let go | Clear Spark District |
| Cloud Works | Launch springs | Clear Iron Foundry |
| Wind Observatory | Rising wind currents | Clear Hanging Gardens |
| Storm Heart | Timed coils: dim is safe, bright is dangerous | Clear both Cloud Works and Wind Observatory |

Each group introduces its mechanic, varies it, and combines it with earlier skills. Reach the ring to advance. After three deliveries, return to the map and choose another location. Completed locations can be replayed. The map shows room completion and remembers your current checkpoint in each unfinished location.

## Play

[Play POLARITY in your browser](https://polarity-storm-courier.csells.chatgpt.site) — public, with keyboard, touch, and controller support.

Run `python3 server.py`, then open port 8787. The web player includes multitouch controls, keyboard and gamepad input, sound effects, pause, fullscreen, and a ROM download.

- Arrows / WASD: move and aim the dash
- Z / Space: jump (hold for height)
- X / Shift: dash and flip charge
- Enter: pause
- R: retry from the last flag
- M: world map (or use the World Map button)

Standard browser-mapped controllers (USB or Bluetooth): D-pad / left stick to move and aim; bottom face button (Xbox A / PlayStation Cross) to jump; right or left face button (B / Circle or X / Square) to dash; Start / Options to start, pause and resume; Back / View / Share to retry. Press a button after connecting so the browser detects it. If audio remains locked, tap **Enable Sound** once. Disconnecting the active controller pauses the game.

For 8BitDo SN30/SF30-style layouts, bottom **B** jumps and **Y / A** dashes; **START** pauses and **SELECT** retries; **UP + SELECT** opens the map. Use Windows / X-input mode where supported. If the browser does not recognize the layout, open **8BitDo / controller setup** and teach its eight controls. This supports D-pads reported as buttons, digital axes, or HID hats and saves the layout per controller/mode in this browser. The SN30 2.4G model for SNES Classic must connect directly to a PC by USB; its console receiver is not a PC USB receiver. See [8BitDo SN30 support](https://support.8bitdo.com/faq/sn30.html) and [SN30 2.4G support](https://support.8bitdo.com/faq/sn30-2-4-g.html).

`node tests/controller-profile.cjs` tests binding interpretation. `node tests/8bitdo.cjs` tests simulated raw controller reports through the real browser player, including calibration, diagonals, pause/retry, and persistence. These tests do not establish physical compatibility of every controller firmware or Windows driver.

On hardware: D-pad to move, A to jump, B to dash, Start to pause, Select to retry, Up + Select for the map. The hardware pause menu also offers A to resume and B for the map.

## Build

`./build.sh` generates tile art and levels, compiles C with GBDK 2020 4.5.0, and produces `web/polarity.gbc`. The compiler is not included in this repository. Download the [GBDK 2020 4.5.0 release](https://github.com/gbdk-2020/gbdk-2020/releases/tag/4.5.0) for your operating system and extract it so the compiler is at `tools/gbdk/bin/lcc`. The ready-to-play ROM is included in `web/polarity.gbc`.

The ROM is 32 KiB, targets Game Boy Color hardware, and uses an MBC5 cartridge with 8 KiB of battery-backed save RAM. The browser stores cartridge saves locally. Return in the same browser on the same site to continue; saves do not sync between devices. Clearing browser data removes the save. A visible message reports storage failures.

## Verification

Run `mkdir -p artifacts` before the checks below. Browser tests require Google Chrome, `npm install`, a running `python3 -m http.server 8788 --directory web`, and a local ROM build for symbol files.

`cc -O2 -o tools/solve tests/solve.c src/engine.c src/levels.c && ./tools/solve` searches for a route through every room using the actual C physics engine, producing replayable inputs in `artifacts/`.

`node tests/replay.cjs` replays all eighteen room solutions in the actual cartridge, stepping its CPU to input polls, navigating the branching map, and checking all completed locations survive reload. `node tests/save-and-map.cjs` checks real flag activation, retry, map return/resume, and checkpoint persistence through reload. `cc -O2 -o /tmp/adventure-tests tests/adventure.c src/engine.c src/levels.c && /tmp/adventure-tests` checks the new mechanics directly. `node tests/touch.cjs` checks simultaneous touch controls, diagonal dashes, pause, sound, and phone layouts. `node tests/controller.cjs` verifies standard controller inputs through the browser Gamepad API using simulated controller reports, including start with locked audio, pause/resume, disconnect/reconnect, dead zones, and focus safety. Physical USB/Bluetooth hardware requires a device/browser that exposes the standard mapping. Install the dev dependency with `npm install`.

## Files

- `src/engine.c`: fixed-point movement, collision and charge rules
- `tools/make_assets.py`: original pixel art and eighteen authored scrolling room layouts
- `src/main.c`: Game Boy renderer, input, sound and game flow
- `web/`: standalone static site; no production server dependencies

## Credits

The web emulator is [binjgb](https://github.com/binji/binjgb), copyright Ben Smith, MIT licensed. Its license is included in `web/vendor/LICENSE-binjgb`. The cartridge is built with [GBDK 2020](https://gbdk.org/). Original game code, room layouts, and pixel artwork created for this project. No assets from Celeste, Mega Man, or Mario are used.
