# Family playtest follow-up

Preserve the validated movement tuning and the idle camera fix. Keep Color and
Advance saves separate; preserve existing Advance progress.

- [x] Investigate multi-minute emulator crashes using real-time browser playback,
  audio/save allocation measurements, and native cartridge endurance checks.
  Reproduce before attributing a cause; retain any unresolved device-specific gap.
- [x] Make later routes harder through authored obstacles and combinations;
  keep the first delivery safe and checkpoints usable.
- [x] Give sentries detailed animated silhouettes that match their hitboxes.
- [x] Replace the dash's harsh buzz with a short, layered electrical whoosh.
- [x] Write clear Advance-specific setup, objectives, radio and delivery dialogue
  that explain the storm, the courier's task and the consequences of each repair.
- [x] Verify changed routes in the native emulator, audio, browser controls,
  save preservation, idle rendering and unchanged movement fingerprint.
- [x] Publish and verify the public release.

## Evidence

Pending crash report details: device/browser and freeze/reload/error behavior.
- Original player: ten real-time minutes in Chrome and ten in WebKit without
  a crash; twenty accelerated native minutes without a cartridge freeze. WASM
  memory stayed at 64 MiB. These runs do not disprove the reported crash.
- A stalled device-audio-clock fixture retained 1,200 native sound sources after
  twenty emulated seconds. The updated player caps the queue at twelve, detaches
  finished sources, and clears pending audio on pause/mute. The regression failed
  before the change and passes afterward. This is a reproduced resource-growth
  failure, not a confirmed attribution of the family's crash.
- Injecting an emulator exception previously left a frozen player and repeated
  timer exceptions. The recovery regression now verifies a stopped timer,
  copyable report and reload with the same saved checkpoint, even if the failed
  core no longer accepts key updates. Reports stay local in `polarity-last-error`.
- Actual-cartridge replay completes all eighteen revised rooms and letters with
  no deaths or frame overruns. All checkpoint positions remain safe, the first
  delivery remains hazard-free, and the movement fingerprint remains `ee887670`.
- Browser timing: 59.5 fps with animation callbacks throttled to 5 Hz. Phone touch,
  standard controllers, raw 8BitDo axes/hats, pause, save/reload and independent
  Color storage passed. The Color ROM hash is unchanged.
- Native audio: six distinct stereo scores, no clipping, loop-boundary playback;
  the isolated new dash peaks at 9,216/32,767 and ends after 102 ms. Preview WAVs
  now use the emulator's actual output rate.
- Reviewed native screenshots of the briefing, first delivery and new sentry;
  new dialogue is checked against the 30-column display at generation time.

## Still open

- [ ] Identify the specific crash reported by Chris and Donna. Device/browser,
  freeze versus tab-reload behavior, and any new error report are pending.
  Keep this plan active rather than describing the crash as conclusively fixed.
- [x] Updated player: ten real-time minutes in Chrome (35,857 frames), no errors,
  64 MiB WASM, about 4–8 MiB JS heap, one to three pending sound sources; two
  real-time minutes in WebKit with active audio and a bounded queue.

## Public release

Published Sites version 9 from `705f5a8295a58e121fc73b6de91c21e97dce590c`.
The public ROM and player JavaScript match the verified local files byte for byte.
Public Chrome checks passed keyboard recovery after an injected failure, saved
checkpoint reload, GBA shoulders, phone/landscape layout and independent Color
storage. ROM SHA-256:
`4485baf17cae85508c2f8d25d3f5e83405f5c7e09e209d4e0c15ff2f95ab5809`.
The specific reported crash remains open as described above.

## Background-crash diagnostics follow-up

Chris reports crashes most often while the game is idle in the background.
The exact underlying browser/emulator failure remains unconfirmed.

- [x] Add an always-available JSON report download, including recent lifecycle,
  audio, memory and console/error evidence, game SRAM, a last healthy mGBA
  snapshot, cartridge SHA-256 and deployed player-code identity.
- [x] Persist bounded local history (three sessions, eighty events each), retain
  the latest recorded failure across reloads, and show storage failures clearly.
- [x] Suspend background input polling as well as emulation; preserve controller
  release-before-resume behavior after returning to the page.
- [x] Finish regression checks and publish the diagnostic recorder.

The diagnostic acceptance test covers actual tab focus loss, injected
freeze/resume events (automation forces visibility to remain visible), a runtime
exception, and an actual isolated Chrome renderer crash. The downloaded emulator
snapshot is loaded and advanced in the same native mGBA core. Storage-quota
failure, console warnings and unhandled promise rejections are also exercised.
No claim is made that the family's specific crash was reproduced.

Published Sites version 10 from `a0ffd35de676780969e755dcff237af2382492f3`.
The diagnostic acceptance test also passed against the public site, including
report download after a killed renderer, reloading an emulator snapshot in mGBA,
and storage-quota failure. Published player files match local source byte for
byte; the ROM remains unchanged. Deployed player identity:
`613b305ab429247b9ea923969f0836dd05d881a246e7139226c38e7d1fbf1cb7`.
Local controller, 8BitDo, touch, audio, runtime-recovery, timing, and GBA browser
regressions passed. Controller tests were repeated after the final background
input guards; holding Start while returning cannot accidentally unpause.

Browser discard semantics: [Chrome Page Lifecycle API](https://developer.chrome.com/docs/web-platform/page-lifecycle-api).
