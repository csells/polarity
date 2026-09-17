# POLARITY: Storm Courier — Advance

[Play the Advance edition](https://polarity-storm-courier.csells.chatgpt.site) · [Play the Color original](https://polarity-storm-courier.csells.chatgpt.site/color/) · [Game Boy Color source release](https://github.com/csells/polarity/tree/gameboy-color)

An original precision platformer running as a **native Game Boy Advance cartridge** in mGBA WebAssembly. Every dash flips cyan ↔ amber. Cyan carries signal; amber carries power. Reconnect six neighborhoods in Lumen, meet the people waiting for your deliveries, and bring their city home.

The Advance edition is composed for a 240 × 160 screen: 32px animated courier sprites, 16px terrain, a two-axis camera, seven original 128-color scene paintings, an illustrated city atlas, 64px resident portraits, restoration lighting, and six original 32-second stereo soundtrack arrangements. The jump, dash, wall-jump, buffering and precision tuning remain exactly as family-tested. The first delivery teaches the basics through safe play.

Eighteen Advance routes are 1,472 screen pixels wide and contain three checkpoint plazas each, two charge sockets, and an optional lost letter. Matching charge activates a socket, restores its bridges, and brings the delivery beacon closer to life. Both links open the exit. Checkpoints and links survive retries. Finish three deliveries to restore a neighborhood; the branching map brings the routes back together at Storm Heart.

| Location | Feature | Unlock |
| --- | --- | --- |
| Spark District | Jump, directional dash, polarity and charge gates | Start here |
| Iron Foundry | Sentries: stomp or dash through | Restore Spark District |
| Hanging Gardens | Climbing ropes and canopy routes | Restore Spark District |
| Cloud Works | Springs and freight platforms | Restore Iron Foundry |
| Wind Observatory | Updrafts and aerial landings | Restore Hanging Gardens |
| Storm Heart | Timed coils and combined mechanics | Restore both upper branches |

## Controls

| Action | GBA / touch | Keyboard | Standard controller |
| --- | --- | --- | --- |
| Move / aim | D-pad | Arrows / WASD | D-pad / left stick |
| Jump | A | Z / Space | Bottom face button |
| Dash + flip | B or R shoulder | X / Shift / E | Right or left face button, or RB |
| City map | L shoulder, or Up + Select | M / Q | LB, or Up + Back / Select |
| Retry / read collected letter | Select | R | Back / View / Select |
| Pause | Start | Enter | Start / Options |

On an 8BitDo SNES layout, bottom **B** jumps and **Y / A** dashes. Standard mappings also support **L** for the map and **R** for dash. For devices reporting raw buttons, axes or HID hats, use **8BitDo / controller setup** to teach the eight essential controls; Up + Select still opens the map on these calibrated layouts. Layouts persist for that controller/mode in the browser. Windows / X-input mode is useful where the model supports it. The SN30 2.4G for SNES Classic connects directly to a PC over USB; its console receiver is not a PC USB receiver. [8BitDo manuals](https://support.8bitdo.com/).

Controller input can start the game even when the browser keeps audio locked. Tap **Enable Sound** if needed. Disconnecting or leaving the page pauses play. The touch D-pad supports diagonal aiming and simultaneous jump/dash touches; landscape puts controls beside the screen. The native pause menu offers A to resume and B for the map.

On the city map, A opens the room chooser and B resumes. Left/right select unlocked rooms. Select reads a collected letter. Completed locations remain replayable.

## Saves and editions

**Advance starts fresh. There is no save portability between Color and Advance.** The editions have distinct cartridge save signatures and browser storage keys. Both persist progress locally in the same browser on the same site; saves do not sync between devices. Clearing browser data removes saves. Storage errors are shown in the player.

This reimagining uses Advance save version 2 (`polarity-advance-save-v2`). Its expanded routes begin a fresh adventure; the earlier Advance save remains in its original browser key and is not imported.

The original Color release is tagged **`gameboy-color`** at `2ed40f6b44584ca54b2b96a299e33c72896f8b62`. Its original cartridge and player remain at `/color/`, using their existing Color save. The Advance ROM is `web/polarity.gba`; the archived Color ROM is `web/color/polarity.gbc`.

## Build and run

The ready-to-play cartridges and browser emulator binaries are included. To serve them locally:

```sh
python3 server.py
# Open http://localhost:8787
```

To rebuild the Advance cartridge, install the pinned relocatable [xPack ARM toolchain](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/tag/v15.2.1-1.1) into this checkout and compile:

```sh
./tools/setup-arm.sh
./build.sh
```

The installer verifies the release SHA-256 checksum. Alternatively set `ARM_CC` and `ARM_OBJCOPY` to an existing ARM bare-metal GCC toolchain. Python 3 is the only normal build dependency for assets. Original image-generated artwork is checked in under `assets/gba/source/`; the checked-in packed palettes and tiles make normal builds independent of image tools. To reprocess the artwork, install Pillow 12.3.0 and run `python3 tools/prepare_gba_art.py`. Terrain and original PCM scores are generated from authored Python sources. Generated C/PCM files and the compiler are ignored by Git. The ROM is approximately 6.7 MB and uses 32 KiB SRAM.

The renderer uses GBA mode 0: an 8bpp painted backdrop, 4bpp architectural and collision layers, a HUD, and hardware sprites. It budgets 16 KiB for terrain/UI, 40 KiB for scene tiles, 8 KiB for screen maps, and a separate 32 KiB for OBJ art. DMA3 uploads maps/OAM at VBlank. DMA1 and DMA2 stream left/right 16,384 Hz PCM through Direct Sound A/B; timer interrupts reset both channels together at the 32-second loop. Swept PSG tones, noise and short arpeggios provide responsive effects. Hot rendering and physics functions execute from fast internal RAM; HUD updates clear only the rows they use. Simulation advances once per native 59.73 Hz frame, with a native overrun counter checked by cartridge replays. The browser clock runs independently of animation callbacks, with frame-aligned input events so quick taps survive delayed browser scheduling. The ARM startup sets stacks, copies fast code into internal RAM and initializes data/BSS; the build writes and validates the cartridge header.

`npm ci` installs browser test dependencies and the pinned mGBA package; `npm run build` packages the static player, both cartridges and the existing hosting worker into `dist/`.

For the legacy GBC build, install [GBDK 2020 4.5.0](https://github.com/gbdk-2020/gbdk-2020/releases/tag/4.5.0) at `tools/gbdk/` and run `./build-gbc.sh`. Its engine and story data remain shared source; its renderer and audio live in `src/main.c` and `src/music.c`.

## Verification

Build first, then generate deterministic routes and run the actual GBA cartridge:

```sh
mkdir -p artifacts
cc -O2 -DPOLARITY_ADVANCE -o tools/solve tests/solve.c src/engine.c src/gba/generated/levels.c
for r in $(seq 0 17); do ./tools/solve "$r" --letter; done
cc -O2 -DPOLARITY_ADVANCE -o /tmp/polarity-gba-layout tests/gba-layout.c src/engine.c src/city.c src/gba/generated/levels.c
/tmp/polarity-gba-layout
node tests/gba-entry.cjs
node tests/gba-replay.cjs
node tests/gba-audio.cjs
```

The replay operates the same vendored mGBA core as the web player. It checks all eighteen letters and routes, branching unlocks, resident deliveries, restoration scenes, the ending, replay selection, SRAM reload, rejection of Color saves, and one physics update per frame. Audio checks cover six distinct arrangements, clipping and playback across a complete DMA loop. Screenshots and a soundtrack sample are written to ignored `artifacts/`.

Native checks:

```sh
cc -O2 -o /tmp/polarity-movement tests/movement.c src/engine.c src/levels.c && /tmp/polarity-movement
cc -O2 -o /tmp/polarity-city tests/city.c src/city.c && /tmp/polarity-city
cc -O2 -o /tmp/polarity-adventure tests/adventure.c src/engine.c src/levels.c && /tmp/polarity-adventure
python3 tests/rooms.py
node tests/controller-profile.cjs
```

The golden movement fingerprint is **`ee887670`**, covering 1,440 frames of movement and the original mechanics. City tests include the retained Color save decoder; the Advance cartridge accepts only its separate save signature/version.

Browser tests require Google Chrome and `npm ci`. Start `python3 -m http.server 8790 --directory web` in another terminal, then:

```sh
POLARITY_URL=http://localhost:8790 node tests/gba-timing.cjs
POLARITY_URL=http://localhost:8790 node tests/gba-browser.cjs
POLARITY_URL=http://localhost:8790 node tests/controller.cjs
POLARITY_URL=http://localhost:8790 node tests/8bitdo.cjs
POLARITY_URL=http://localhost:8790 node tests/touch.cjs
```

On a busy Mac, prefix a browser test with `taskpolicy -a env` before its `POLARITY_URL=...` assignment to use application scheduling. Background scheduling can otherwise dominate frame-rate measurements. The timing assertion remains unchanged.

These exercise actual browser emulation, isolated saves/reload, shoulder controls, the Color archive, simulated standard and raw 8BitDo reports, touch diagonals, pause/mute, focus/disconnection handling, and phone layouts. Physical controller firmware and real GBA hardware have not been tested here. Older `replay.cjs`, `restoration.cjs` and `save-and-map.cjs` target the archived Color build and its symbol file.

## Source and credits

- `src/engine.c`, `src/city.c`: portable movement and progression
- `src/gba/`: native ARM startup, hardware renderer, game flow and audio
- `tools/rooms.py`, `tools/gba_rooms.py`, `tools/story.py`: authored routes, Advance compositions, residents and letters
- `tools/prepare_gba_art.py`, `tools/gba_assets.py`: artwork conversion, hardware palettes, metatiles and sprite packing
- `tools/gba_music.py`: original stereo score synthesis
- `web/app.js`: mGBA adapter, controls, audio and independent Advance saves
- `web/color/`: preserved Color player and cartridge

Advance emulation is [mGBA](https://github.com/mgba-emu/mgba) via [mGBA-wasm](https://github.com/wasm-gaming/mGBA-wasm), MPL-2.0; license and source links are in `web/vendor/mgba/`. The Color emulator is [binjgb](https://github.com/binji/binjgb), copyright Ben Smith, MIT licensed; its license is retained. Hardware implementation references include [Tonc](https://gbadev.net/tonc/). Game code, characters, level layouts, artwork and music are original to this project. No assets from Celeste, Mega Man or Mario are used.

The district paintings, city atlas, courier sheet and resident sheet were created
for Polarity using image generation, then quantized and packed for GBA hardware.
The original PNGs, conversion script and packed output are all included. The
terrain metatiles, camera, animation selection, level adaptations, palette
lighting and stereo score are authored in this repository.
