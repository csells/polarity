# Lumen on Game Boy Advance

Preserve the family-tested GBC release with the public `gameboy-color` tag and a playable `/color/` archive. Build a real native ARM Game Boy Advance cartridge, not a browser recreation. Preserve the exact shared movement engine and story/progression while reauthoring presentation for 240x160, a hardware tiled renderer, layered parallax, weather, expressive sprites, readable HUD, district portraits, restoration celebrations and sampled original soundtrack. A/B retain jump/dash; R offers dash, L opens the atlas, Select retries/reads letters. Maintain standard and calibrated 8BitDo support and multitouch controls.

Use mGBA WebAssembly with BIOS HLE, local vendored runtime/license, deterministic stepping, and persistent SRAM. Advance starts fresh with a distinct save signature and storage key. User explicitly ruled out save portability between editions. Keep the Color archive accessible. Build reproducibly from source with a pinned ARM toolchain, standard GBA header and SRAM marker, VBlank updates and timer/DMA audio.

Verify movement fingerprint; native mechanics and saves; actual GBA cartridge boot, all 18 room solutions and letters, map/unlocks/dialogue/ending, save isolation and reload; real audio output; standard/8BitDo/touch and landscape layouts; rendered game art and public assets. Push the completed source and publish the exact tested cartridge to the existing public URL.

## Implementation verified

The Color tag is published and resolves to `2ed40f6b44584ca54b2b96a299e33c72896f8b62`; the archived Color ROM matches it byte for byte. Advance is a 1,844,716-byte native ARM cartridge. All eighteen optional-letter routes complete without deaths in mGBA, with one physics step per video frame, both charge links, branching unlocks, residents, restoration, ending, replay and SRAM reload. Movement fingerprint remains `ee887670`. Native pause/resume clears its panel. Six distinct PCM arrangements produce unclipped audio and continue across the 16-second DMA loop.

Browser verification passed independent save keys/formats and Color-save rejection, checkpoint/socket reload, R dash and L atlas, Color archive boot, standard controllers, raw 8BitDo axis/hat calibration, simultaneous touch/diagonal input, pause/mute, portrait/landscape, disconnection and focus handling. A timing regression throttles animation callbacks to 5 Hz: the independent emulation clock sustained 59.3 frames/second and registered 5ms taps. Input events are coalesced within an event and queued between frames; the launch button is held out until released to avoid accidental menu advancement.

Desktop, phone, title, rooms in all six districts, resident/restoration scenes, completed map and homecoming were inspected. The original movement engine is unchanged. Publication uses the established public site and GitHub main branch. Real hardware/physical controller firmware testing remains outside this automated verification; no such testing is claimed.
