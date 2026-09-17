# The city wakes up

User validation: Chris, his son and grandson have played and like the mechanics, precision, and jump/dash interaction. Preserve that movement exactly. Do not retune acceleration, jump height/cut, gravity, dash speed/duration, wall jumps, coyote time or input buffering.

Deliver a cohesive playable adventure in the existing actual GBC cartridge and public web player:
- Eighteen independently composed four-screen rooms, grouped into six locations. Each group teaches, develops and combines its mechanic; safe checkpoint plazas stay at columns 20/40/60.
- Cyan signal sockets and amber power sockets tie polarity to every restoration. Matching charge activates a socket without harming the player; both open the delivery beacon. Relays survive retries and saves.
- One optional lost letter per room rewards exploration without blocking progression. Keep letters permanently; expose room replay selection.
- A spatially navigated city map, visible restoration, distinct scenery, walking/jumping sprites and clear phase colors.
- Short radio messages while playing, residents at delivery, location restoration celebrations, and a homecoming ending. Avoid long mandatory exposition.
- Original lightweight GBC music gains instruments as neighborhoods return. Keep sound effects and existing mute/control support.
- Preserve previous completion saves; reset old in-room flags on migration because room geometry changes. New saves include activated sockets and letters.
- Level contact sheets/metadata make authored routes reviewable.

Validation: golden movement traces, focused relay/save/map tests, solve and replay every room in the real cartridge, collect optional letters, verify progressive unlocks and original controller/touch paths, inspect location screenshots and audio output, publish exact tested build to the existing URL and GitHub repo.

## Implementation verified

All scope above is implemented. Golden movement fingerprint `ee887670` is unchanged. Native mechanics, city/save/migration and room-structure checks passed. All eighteen normal routes and eighteen optional-letter routes were solved; the letter routes were replayed in the actual GBC ROM through all deliveries, six restoration scenes, the homecoming and save/reload. All eighteen letters were collected. Standard controller, simulated 8BitDo axis/hat mappings, and multitouch browser checks passed. Distinct location scenery, radio/tutorial text, restored map and final letter count were inspected in ROM screenshots; all six music arrangements produced audio.

The first delivery contains no lethal hazards and teaches movement, jumping, the dash's charge flip, matching signal/power sockets and upward dashing through the room itself. Runtime cadence was checked in the real cartridge; room loading avoids expensive per-tile division. The release artifact remains a 32 KiB GBC ROM. Publication uses the established public site and `csells/polarity` main branch.
