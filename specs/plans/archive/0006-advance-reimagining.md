# Advance reimagining

The earlier Advance port did not satisfy the requested redesign. Preserve the
family-tested movement timing and the cyan/amber restoration premise; reconsider
the rest against the actual 240x160 screen and hardware budget.

- Original district paintings, courier animation, resident art and city atlas.
- 32px courier and 16px world metatiles. Logical physics stays unchanged; screen
  motion doubles. Two-axis camera with direction anticipation and instant retry.
- Advance-specific 92x18 collision maps: 18 routes, three safe checkpoint courts
  per route, area-specific rehearsal spaces. First route teaches without hazards.
- Mode 0: 128-color scene, foreground, architecture, HUD, hardware sprites.
  Explicit 64KiB BG / 32KiB OBJ VRAM budgets. Rendering must not drop simulation
  frames; verify in mGBA with deterministic input replays.
- 64px independently sampled portraits; inhabited city map; warm restoration
  lighting and contextual in-room teaching. A clear silhouette and readable
  hazards take precedence over background detail.
- Six original 32-second stereo PCM arrangements with different orchestration,
  longer phrasing and cross-channel reflections. PSG jump/dash/charge effects.
- Keep source-aware keyboard, multitouch and calibrated 8BitDo controller input;
  keep the native emulator and independent browser clock already validated.
- New Advance room geometry uses Advance save version 2 and its own storage key.
  No save conversion from Color or the earlier Advance layout.
- Publish the actual tested cartridge and matching web player to the existing
  public site. Preserve the tagged Color release and its cartridge bytes.

Verify: exact ROM replays through all 18 deliveries and letters, progressive
unlocks and ending; native frame cadence; stereo audio across its full loop;
browser reload/save, short taps, controllers, touch and portrait/landscape sizing;
inspect real emulator screenshots, not only the high-resolution source art.

## Completed and verified — 2026-09-17

Published at https://polarity-storm-courier.csells.chatgpt.site from source
`d189ed55e50a5952f19e0a8b18c19bf850aeca77` (Sites version 7).

- Actual mGBA replay: all 18 rooms, both sockets and all 18 letters, branch
  unlocks, resident deliveries, restorations, ending, replay and native SRAM
  reload. Zero deaths and zero gameplay frame overruns in the scripted routes.
- Movement fingerprint remains `ee887670`. All 54 checkpoint respawns are safe;
  all three Advance checkpoint positions round-trip through the save codec.
- Native entry latch and pause-panel clearing pass regression checks.
- Six stereo scores are distinct, non-silent and unclipped; playback continues
  across the 32-second loop. Jump, dash and restoration cues use native PSG.
- Standard gamepad and raw 8BitDo axes/hat calibration, saved mappings,
  simultaneous inputs, disconnect/reconnect, pause and retry pass in Chrome.
- Multitouch, diagonal dash, release, mute and portrait/landscape layouts pass.
- Public browser: independent Color/Advance storage, checkpoint reload and
  shoulder controls pass. 5ms taps survive throttled animation callbacks;
  measured 59.9 emulated frames/sec on the live site.
- Public ROM SHA-256 matches the tested cartridge:
  `1036571883228aebca4583428116e57e786262e17d73f3a910e3fead3f0faa87`.
  The Color archive remains byte-identical to the `gameboy-color` tag.

The busy Mac initially throttled background test processes; final real-time
checks used macOS application scheduling (`taskpolicy -a`). Layout captures
use software-composited headless Chrome. No physical GBA or controller firmware
was tested. Original generated artwork and its packed hardware assets are
checked in; ordinary builds require no image-generation service.
