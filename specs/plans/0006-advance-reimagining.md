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
