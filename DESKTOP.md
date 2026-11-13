# Native desktop skirmish

The broader product target and verification limits are recorded in [EXPERIENCE.md](EXPERIENCE.md). This skirmish is an engine milestone; the first original-art launcher/main-menu flow is implemented; the desktop now uses the original bottom HUD artwork with a persistent command grid and minimap. Fidelity work remains.

This branch wraps Galactic Genie's existing C++ simulation and software renderer in an SDL desktop frontend. It runs natively on Apple Silicon using your legally installed Clone Campaigns assets. The earlier static recompilation is a separate reference project.

## Play on this Mac

Run `Play.command` in this checkout, or supply another asset location:

```sh
SWGB_GAME_DIR=/path/to/Game ./script/build_and_run.sh
```

`Play.command` uses a local copy of the skirmish assets (Data, music, terrain audio and language DLLs) in `~/Library/Application Support/Galactic Genie/Game-CloneCampaigns`. This profile uses the original Clone Campaigns data verified against the Windows reference. The previous `Game` profile is preserved and can still be selected with `SWGB_GAME_DIR`. The checkout, builds and runtime assets are all outside Documents. Keep custom asset paths outside protected folders if you want to avoid macOS folder-access prompts.

Choose **Start Game → Single Player → Standard Game**. The **Reveal Map** dropdown offers **Standard** (unexplored terrain, the desktop default) and **Explored** (known terrain with enemy activity still hidden outside sight). Choose **Start Game**. This opens a 64×64 land match. Both players can use any of the eight Clone Campaigns civilizations (Empire versus Rebels by default). The default Conquest objective requires eliminating enemy units and buildings; the shorter Command Center objective is also selectable. Population defaults to75 and can be set from25 to200 in steps of25; housing is still required. Both sides start with five workers, four troopers, buildings and 1200 of each resource. The computer gathers, constructs processing centers and a Troop Center, trains workers/troops and sends attack groups. It spends real resources and uses normal movement/combat; it receives no recurring free resources or units. This is a small custom AI, not the original Windows AI personality.

Shift-click adds your mobile units to the current selection; Shift-drag adds units inside a box. Ordinary click/drag still replaces the selection. This also works while paused, without issuing orders. Select workers and right-click resources to gather. Select a building to see training/research actions in the bottom-left grid. Select workers and press B to construct buildings. Recruit an army, then right-click enemy units/buildings to attack. Use the bottom-right minimap to move the camera (left click) or issue orders (right click). The result appears as an instruction banner; Enter returns to the main menu.

| Control | Action |
| --- | --- |
| Left click / drag | Select / box select |
| Right click | Contextual command; cancel building placement |
| B or Tab | Worker building submenu |
| Arrow in worker build grid | Cycle Economy / Military / Defense |
| Arrows / wheel | Pan / zoom |
| Minimap left / right click | Move camera / issue order |
| Ctrl+0–9 / 0–9 | Store / recall an owned-unit or building group |
| Space | Pause/resume; while paused, camera/minimap and selection work but orders and queue changes are blocked. Losing window focus pauses automatically |
| F8 | Mute/unmute effects and music (some Mac keyboards require Fn) |
| F10 | Game menu (Cancel resumes; Options; confirmed quit/restart) |
| Enter after result | Return to main menu |
| Escape | Close action menu, otherwise open game menu |
| Cmd-Q / close window | Quit |

Original WAV effects play through the existing decoder and a 16-voice mixer. `MUSIC/Track02.ogg` streams on a background thread into a bounded buffer and loops. No system volume setting changes. Outputs/logs live in `~/.local/share/swgb/galactic-genie-build`; the original assets are read-only. Set `SWGB_DESKTOP_BUILD` to another absolute output directory if needed.

## Build and checks

The macOS fallback requires Apple Command Line Tools, Python 3, pkg-config, SDL2 and libvorbis (installed through Homebrew on this Mac). `--build` only builds; `--smoke` runs with dummy video/audio, verifies 60 rendered frames, mixed WAV effects and streamed music including an EOF loop, then writes `desktop-smoke.png`.

```sh
SWGB_GAME_DIR=/path/to/Game ./script/build_and_run.sh --smoke
SWGB_GAME_DIR=/path/to/Game python3 script/test_skirmish.py
```

The second command builds with AddressSanitizer and UndefinedBehaviorSanitizer. It checks that the AI defeats an idle player through real combat within 15 simulated minutes, and tests match freeze, result handling, restart, resignation and isolation from the sandbox. A second match scripts the player: place housing through the action menu, gather, pay for recruits, defend with garrisoned troops, then attack the active AI and win. It uses no resource grants, unit spawning or direct damage; `player-match-result.png` records the result. Separate victory/draw edge-case fixtures use explicit objective damage.

The built `skirmish-test` also accepts `/path/to/Game/Data --civilizations` for eight complete Command Center matches, `--conquest` for eight complete Conquest matches, and `--player-conquest` for the scripted player's Conquest path. These longer checks run separately from the default suite. Run them from the external build directory so their captures stay outside the checkout.

Run `python3 script/test_algorithms.py` for asset-independent RMS and military-group checks with strict warnings and sanitizers. It writes binaries outside the checkout using the same build-directory override. These tests validate recovered algorithms; they do not imply the complete original map generator or AI is active in the playable app. CMake users can enable `-DSWGB_BUILD_ALGORITHM_TESTS=ON`, build the eleven algorithm test targets and run `ctest --test-dir <build-dir>`.

With CMake, configure `-DSWGB_BUILD_DESKTOP=ON -DSWGB_BUILD_SKIRMISH_TEST=ON` and build `swgb-desktop`, `swgb-skirmish-test` and `swgbtool`. Run `swgb-skirmish-test /path/to/Game/Data` and `swgbtool test-combat /path/to/Game/Data`. Desktop options are off by default; Vita and headless tool entry points remain available.

## Limits

This is a testable small skirmish, not a finished Clone Campaigns replacement. There is one fixed land layout, selectable civilizations, population, Standard/Explored visibility and Conquest/Command Center victory, with the remaining match settings fixed and no save/load flow, and no original Windows multiplayer or Expanding Fronts compatibility claim. The renderer is CPU-based at 960×544, with no new Metal renderer or shaders. Desktop action and placement hints use mouse/keyboard controls; the Vita frontend retains its original hints. Full campaigns, long matches, technology balance and every unit behavior have not been validated.

Local verification (2026-11-06): sanitizer matches completed with AI victory at 648.6 simulated seconds and scripted player victory at 760.8 seconds; upstream combat regression and dummy audio/render smoke passed. Native resignation/restart, worker recruitment and mute control were inspected in the window. The user confirmed hearing both music and unit sounds.

A tested opening: build a habitation to raise the population cap, send workers to food/carbon, recruit continuously from the Troop Center, and garrison troopers in the Command Center while defending. Once around twelve troops have assembled, eject them and attack together, bringing reinforcements. Select troops, click the GARRISON button, then click the Command Center. To eject a unit, select the Command Center and click its garrison portrait; these controls were also verified in the native window.

The restart test also guards against stale mobile-collision cells from a previous match. AI construction reacquires its worker after adding a foundation, avoiding invalid pointers if the object vector grows. The paused desktop redraws at a reduced rate.

## Original-art menu flow (2026-11-06)

The app now starts at the disk-style launcher. Start Game opens the Clone Campaigns main menu, Single Player opens its campaign/standard/saved-game panel; Standard Game opens the fixed-skirmish setup, and Start Game begins the match. Help documents the controls; Options adjusts music and effects independently, with a separate mute toggle. Tab/Up/Down navigate enabled buttons, Enter/Space activate, and Escape goes back. Grey original-menu destinations are unavailable (tutorial, databank, multiplayer, map editor, Zone).

In a match, F10 opens the game menu. Escape closes an open action menu first, otherwise opens the game menu. Cancel returns to the match; Options adjusts music/effects volume; Quit Current Game confirms resignation, and Restart confirms resetting the current match. Enter after a match result returns to the main menu. Space still pauses/resumes directly. Save/load and full result statistics are not implemented.

The app also accepts a bare double-click: assets default to `~/Library/Application Support/Galactic Genie/Game-CloneCampaigns`, with `SWGB_GAME_DIR` or a command-line path taking precedence. Missing assets produce a visible error. This is a local development app using Homebrew libraries, not a self-contained distribution.

Menu background and logo are decoded at runtime from original `interfac_x1.drs` SLP 53233 (frames 0 and 49), using palette 53237. Optional launcher images live under `Game/Launcher`: `Product.bmp` copied from the original installation's `Install/Product.bmp`, and `unselect.bmp` / `focused.bmp` converted locally from its same-named TGA files with macOS `sips -s format bmp`. No copyrighted artwork is committed or bundled. Without those optional images the launcher uses plain controls. The supplied screenshots are references only.

`--smoke` additionally checks launcher/main/setup/back/options/pause/resign-cancel navigation, disabled destinations, and writes `launcher-smoke.png`, `menu-smoke.png`, `setup-smoke.png` alongside the gameplay frame. The menus use original installed-language captions where mapped (French in this installation); prototype messages and the gameplay HUD remain English. Full localization, hover timing and UI fidelity remain unfinished.

Native window verification: clicked disk launcher → main menu → Single Player → Start Game; checked F10 pause menu, audio toggle, confirmed resignation and Enter return to main menu. The app was left at the main menu with sound enabled. Full-match sanitizer coverage predates this UI-only change; the new navigation/render/audio smoke passes.

## In-game overlays and audio (2026-11-09)

Pause, Options and resignation confirmation now render over the match at the unchanged 960×544 window size. Simulation updates stop while these screens are open. Escape backs out of Options to pause, then resumes. Overlay coordinates remain in screen pixels regardless of world zoom.

Music and effects have independent 0–100% controls in 10% increments, available from the launcher/main menu and in-game Options. They multiply the previous mix levels, so 100% preserves the previous loudness. F8 still mutes both without discarding their levels. Volume changes preview immediately. OK saves them locally; Cancel, Escape or leaving through the main-menu navigation restores the values from when Options opened. F8/mute remains an independent immediate toggle. Confirmed audio settings and the setup used to start a match persist under the SDL application preferences directory (on this Mac, `~/Library/Application Support/Galactic Genie/Clone Campaigns/settings.txt`). Cancelled volume previews are not saved. Malformed files fall back to defaults; a failed save leaves the previous file in place and displays a warning. F8 mute remains session-only. Pause and Options now use original installed dialog artwork. The volume controls and their layout remain approximations.

Verification: navigation/volume-boundary checks, deterministic mixer checks for independent gains and zero volume, streamed-music EOF test and 60-frame render smoke. The smoke mouse no longer triggers edge scrolling. Native window checked for overlay rendering, music adjustment while effects stayed unchanged, return/resume, and overlay positioning after scrolling to change zoom. No full match was replayed for this desktop-only change.

### Latest verification (2026-11-09)

Verified the new Single Player → Standard Game flow, shared left navigation, original holographic highlights, full artwork click targets, pause-menu ordering and restart confirmation/cancel paths. Native-window checks reached a match and restarted it. Desktop smoke covers navigation, frozen overlays, gain bounds, audio streaming and alpha-mask rendering. A focused renderer check also passed ASan/UBSan. Original setup choices, HUD, save/load and full UI parity remain incomplete; no new full human playthrough was completed.

Menus now read Arial family/size/style definitions and original captions from the installed language DLLs. CoreText handles Unicode and measured centering; no fonts are bundled. The original SIN files supply focus colors. Mouse activation requires pressing and releasing the same enabled button; dragging out cancels. Original font-role mapping and Windows rasterization parity still need verification.

Gameplay text also uses the installed font family on macOS, including UTF-8 unit names. The original bottom HUD layout remains unfinished. `--smoke` additionally captures selection/production panels and checks that font pixels and measurements remain unchanged by world zoom.

Control groups use Ctrl+digit to replace a group and the digit alone to recall it. Empty groups preserve the current selection, dead or no-longer-owned members are removed, and restart clears all groups before object IDs are reused. Hidden/garrisoned members remain stored but are not selected until inspectable again. Recall cancels active placement/targeting modes. These are initial desktop bindings; exact original defaults, group-number indicators and double-tap camera behavior are not yet matched.

## Original bottom HUD — 2026-11-09

The desktop loads the installed 51101–51106 HUD sheets, command/unit/building/research icons and language names. The original 800px panel is scaled to the current window width. Training and research use the existing cost/queue handlers; clicking a queued icon cancels it. Workers have building categories; grouped selections show clickable portraits. The minimap supports camera movement and orders; its Menu button opens the existing pause dialog.

This is the first integrated layout, not pixel-identical Clone Campaigns. Mobile command/stance/formation slots and worker category controls now follow executable evidence. Garrison portraits support individual ejection. Hover/pressed/disabled states, garrison overflow layout, font roles and full localization still need work. The simulation remains the existing custom skirmish; this change does not establish original gameplay parity.

Formation controls occupy the third command row and use the original active icons. Garrisoned units appear as portraits with health bars; click one to eject it, or use Eject All in the bottom-left command cell. When portraits overflow, the wheel over the selection panel changes the visible group without zooming the world. Overflow scrolling is a native accommodation, not yet verified retail behavior.

### Targeted unloading and paused inspection

Loaded transports show the original Unload icon in command slot 13. Click it, then choose a world location: the transport travels to a reachable endpoint and unloads there. Stop cancels the pending unload. Blocked passengers stay aboard and retry; the original path planner and unloading cursor have not been reproduced. Building garrison portraits still eject individually, and slot 12 ejects all. Portraits now include health and shield bars.

While setting a building rally point, slot 0 removes the existing rally and slot 14 cancels targeting without removing it. These positions/icons follow original mode 0x6f. Space-pause freezes simulation, movement, animation, AI and production; inspection, arrow/edge scrolling, minimap camera movement, zoom and selection remain available. Training, refunds, formations, deletion, ejection and world/minimap orders are blocked. Returning from the F10 menu preserves a manual pause.

ASan/UBSan checks cover paused selection/camera with unchanged simulation and queue/resource state, paused ejection rejection, rally remove/cancel, transport boarding/travel/unload and Stop cancellation. Desktop audio/render/navigation smoke passes. Native-window paused arrow scrolling was observed. Command Center alarm/return-to-work, Commander Locator (regicide), exact overflow presentation and full original-game parity remain unfinished.

### Command Center alarm and return-to-work

The Command Center's slot 14 uses the original inactive/active alarm icons (49/61). Ringing it searches the original ±25-tile area for friendly workers and completed Command Centers, fortresses, turrets and missile turrets. Workers take the nearest compatible shelter with space within 25 tiles, in increasing distance bands. Incoming workers reserve capacity, so excess workers keep their jobs instead of overfilling a shelter.

Ringing again recalls alarm-sheltered workers, including those still walking in. Their previous gathering, construction, repair or manual delivery target is revalidated before resuming; an unavailable job falls back to the remembered destination. Carried resources remain on the same worker. Repeated shelter orders preserve the first job. Explicit replacement orders and manual ejection discard that saved job. Blocked exits retain the worker and job, ready for another recall attempt once clear.

Slot 11's original Return-to-work button (icon 64) appears in Command Centers and fortresses containing workers. It returns workers without ejecting soldiers or clearing the alarm state. This works for manually garrisoned workers too. Both new commands are blocked during Space-pause. The installed original alarm/recall samples (50357/50358) play through the existing audio mixer. Exact original per-shelter panning/volume is not reproduced.

Garrison hover now shows the occupant's name, health and shields, with an ejection hint. Portraits use each occupant's civilization icons. Overflow still uses the native scroll adaptation; the original dynamic row/column layout and pixel origins remain unresolved. `hud-alarm-smoke.png` in the external build directory records the seven-occupant layout and both active controls.

### Recovered garrison geometry and positional audio

The desktop's 800px HUD profile now uses the original garrison origin (panel-relative100,42), 38px column spacing and41px rows. It compresses columns to fit three rows, or one row while production/research is active, instead of scrolling. Retained occupants fill bottom-right toward top-left; clicking overlaps selects the last-painted portrait. The subject name, portrait, health and shield rows now use the recovered left-column coordinates. Original caps40/19 replace native pagination; this does not increase any unit's actual garrison capacity.

Alarm sounds now emit per audible shelter with recovered pan/attenuation and the original -30dB alarm cutoff. Repeated calls for the same alarm resource rewind/restart one voice instead of adding overlapping copies. World unit effects use positional mixing with the original -50dB cutoff; interface clicks and acknowledgements remain centered. Per-voice pan and volume combine as decibel attenuation. Existing user volume settings and the app's overall effects/music gains are retained.

Limits: camera center, window width and zoom are native adaptations of the original listener/configuration fields. The fractional-position offset is reproduced for the flat skirmish; exact slope flags and original fog-driven audio visibility are not implemented. Overall hardware/output loudness and original-versus-native listening comparison remain unverified. The garrison layout uses the800px profile scaled to the desktop window; other original resolution profiles and exact fonts are not yet matched.

Garrison buttons also use the original btngame normal/pressed frames with clipped corners and the original pressed icon offset. Releasing outside the pressed portrait cancels ejection; an isolated release cannot eject. The pointer keeps the occupant ID so a changed roster cannot silently retarget the press. Hidden garrison controls do not accept clicks during multi-selection.


### RMS diagnostic renderer

`swgbtool render-rms <DataDir> tools/fixtures/explicit-lands.rms 32 1 <out.png>` executes a self-contained RMS script and renders the generated native map with installed game assets. The last numeric argument is shared CRT state at script entry, not a claim of original lobby seed parity.

The integration supports explicit nonplayer lands OR one `create_player_lands` block for 1..8 players (default 2), validated flat base/land terrain metadata, land separation 0..1024, border fuzziness 0..255, nonplayer border margins 0..100 percent, clumping 0..100, and one optional default ungrouped Gaia resource row (348 carbon,66 nova or102 ore). Default radius is 3, clumping 8, fuzziness 20 and tile target the map area; `land_percent` divides area across the selected land rows. Nonplayer zone commands -10..53 map to stored bytes 0..63; missing zones use shared0. Player zones follow owner IDs1..playerCount and cannot be overridden yet. Other stages, object types, mixed player/nonplayer blocks, object grouping and custom player borders return an error. Player starts are returned as positions; starting units are not yet spawned.

Terrain rows support validated explicit base/target terrain, tile counts or land percentages, clumps, spacing, clumping, map-size/group scaling and player-start protection. Original defaults are target equal to map width, one clump, zero spacing and clumping20. Protection uses fixed radius13/weight20 around each owned start. Zero clumps still consume the original shuffle draws. Resolved clumps above999, arithmetic overflow, identical base/target terrain, nonzero-clump terrain rows on1x1 maps, other terrain metadata and nonflat/connectivity-based behavior remain rejected. Work limits fail without committing partial terrain or RNG.

Player placement is explicitly the original game’s randomized-position mode for 1..8 players with equal radii and default borders. The original `<PLAYER_SETUP> random_placement` directive is accepted as a no-op; it does not select the game mode. Fixed-position mode remains unsupported, including an externally supplied `FIXED_POSITIONS` context. The original lobby default has not been established.

`swgbtool render-rms-resource <DataDir> <archive.drs> <id> <size> <rng-state> <out.png> [--define SYMBOL] [--players 1..8]` loads packaged DRS resources and their includes into the same runner. This does not imply the stock maps are fully supported: the current corpus still needs additional terrain metadata, automatic scenery, elevation/cliffs and resource/start-object generation. `tools/fixtures/two-player-lands.rms` exercises player lands; `tools/fixtures/player-terrain.rms` also exercises separated lands, scaled terrain clumps and start protection. Both render commands accept `--players 1..8`; this configures land generation, not playable multiplayer or starting units.

This is a diagnostic engine path, not a new playable random-map mode. The desktop launcher still starts the existing skirmish layout and fallback AI. `script/test_algorithms.py` covers the strict pipeline; the installed-data skirmish suite also checks original-stage terrain/object/RNG fixtures through native map initialization.

Run `SWGB_GAME_DIR=/path/to/Game python3 script/test_rms_cli.py` after building `swgbtool` to verify authored DRS includes, startup defines, file/archive rendering parity for two/eight players and error handling. Its temporary archives contain only authored test scripts and are removed after the check.

Automatic scenery supports original forest terrain 10/13/17/18/19/20/21 using the source terrain's slots, including 11-facet trees and 125-carbon trees. Global base terrain may have no active scenery or use the verified dry static-forest path (10/13/17–21); active forest land/terrain rows are supported. Water and ice-shore cleanup preserve original terrain IDs. Authored `dry-terrain.rms`, `water-terrain.rms` and `forest-terrain.rms` exercise these paths. Unsupported active scenery, elevations, cliffs and other objects still fail explicitly. Stored terrain/object/RNG parity does not imply texture-rendering parity or complete stock-map support.

Terrain49/50 additionally creates the original stationary carbon deposits (units1341/217). Their constructor consumes three random draws, including a frame draw for a one-frame sprite. `carbon-terrain.rms` exercises this path; original heading is retained in generated placements, while native import converts the final facet into its existing rendering-facing representation.

`early-forest.rms` exercises base terrain10 with dry land/terrain edits. Smoothing creates early trees in original setter order; final generation retains or replaces them according to current terrain slots. Removed creation records reserve their ordinal, and native import skips them. Native object IDs use an explicit +1 offset because zero is reserved. Carbon-deposit bases49/50, mixed active scenery after early forests, and forest-base/water combinations remain rejected.

`nova-resource.rms` exercises original ungrouped nova placement after early forest changes. The supported object row can instead request102 for ore or348 for timber. Grouping, multiple rows and owned starting objects still reject. Native zero-HP stationary Gaia resources remain inspectable/gatherable; zero health alone does not mark them dead.
