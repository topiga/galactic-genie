# Original-experience target and verification

Updated 2026-11-06. The product target is the original disk-launcher and Clone Campaigns experience on native macOS, followed by optional EF-inspired graphics improvements. A completed custom skirmish is an engine milestone, not completion of that target.

## Visual references supplied by the user

Keep the two layers distinct:

1. Disk launcher: compact dark-blue window, central Galactic Battlegrounds logo, troopers at both sides, four stacked controls: Start Game, Help, Options, Exit. Match composition, imagery, button treatment and hover/pressed behavior; use native macOS window management rather than reproducing Windows chrome.
2. Clone Campaigns main menu: original illustrated scene, logo at upper left, artwork-based navigation on the left and blue framed panel on the right. The supplied French reference labels Didacticiel, Un joueur, Banque de données, Options, Multijoueur, Éditeur de cartes, Quitter and Zone. Preserve this visual hierarchy and localization. Reference imagery establishes appearance, not the behavior of each click.

The EF launcher is not the original disk launcher. The Sikarugir wrapper launches the EF route, as clarified by the user. The launcher route does not identify the game selected within it. The user's 96.59-second recording is explicitly Clone Campaigns, not base Galactic Battlegrounds or Expanding Fronts. Direct attachment to the Sikarugir app timed out again; the observations below come from the recording, not a new hands-on reference session.

Reference images are retained locally under `~/Library/Application Support/Galactic Genie/Research/` as `disk-launcher-reference.png` and `clone-campaigns-menu-reference.png`. Do not bundle these screenshots as interface assets or publish them. Obtain presentation assets from the user's installed game and keep those outside the source repository.

## Verification performed

Fresh runs of the existing built executables on 2026-11-06 returned exit code 0 for all three checks:

| Check | Evidence established | Not established |
| --- | --- | --- |
| ASan/UBSan skirmish test | AI defeats idle player at 648.6 simulated seconds; scripted player wins at 760.8 seconds through paid production, construction, gathering and combat; restart, results and freeze assertions pass | A full human mouse/keyboard playthrough; Windows simulation equivalence; other maps/civilizations |
| Upstream combat regression | Existing combat, economy, construction, research, garrison and targeting assertions pass | Every unit, technology or original-game rule |
| Desktop smoke | 60 update/render/present frames; WAV mixer and streamed Vorbis/EOF loop checks pass | All audio cues, long-session reliability or performance under large armies |

The scripted player's selection, gathering, garrison and attack orders partly use engine test helpers. Building placement and recruitment exercise action-menu input. The test does not grant resources, spawn extra units or directly damage objectives in its full player match. Separate result edge-case fixtures do use forced damage. This is meaningful simulation coverage, not a substitute for end-to-end UI coverage.

Previously observed native UI: selection/movement, action menus, recruitment, building placement, garrison/eject, resign/restart and pause/mute. The user confirmed music and unit effects audible. These observations remain bounded to the checked interactions.

Fresh logs are `~/.local/share/swgb/galactic-genie-build/verification-{skirmish,combat,smoke}.log`. Binary/log SHA-256 values and exit codes are retained in the local research folder's `verification-2026-11-06.json`.

## Current gaps and acceptance order

| Priority | Area | Current implementation | Acceptance target |
| --- | --- | --- | --- |
| 1 | Launch | Original-art launcher with Start/Help/Options/Exit; bare app uses local asset default; no asset picker | Double-clickable native launcher matching the disk reference; persistent asset location, working Start/Help/Options/Exit; no terminal required |
| 1 | Main menu | Original CC background/logo with functional Single Player, Options and Quit; unsupported destinations disabled | Clone Campaigns reference layout, original assets and interaction feedback; functional navigation and return paths |
| 1 | Single-player setup | Fixed seed, map, factions, resources and custom Command Center objective | Original-style setup and only options actually supported by simulation; verify each setting reaches the match |
| 2 | Match HUD | Installed original bottom-panel artwork, persistent commands, selection/queues and interactive minimap; first desktop integration | Original-style persistent command panel/minimap, familiar selection and feedback, legible scaling; compare against original in-game reference |
| 2 | Keyboard/mouse | Small custom mapping, no desktop control-group bindings; F10 opens a game menu with confirmed resignation | Reference-verified hotkeys and selection modifiers, control groups, cancellation and confirmation flows; user-configurable shortcuts later |
| 2 | Match lifecycle | Pause/resume/options, confirmed resign, result banner, Enter returns to main menu | In-game menu, resume, options, confirmed resign, results and return to main menu; save/load requires actual serialization |
| 3 | Simulation fidelity | Custom opening/AI/objective and deterministic fixture | Compare economy, visibility/fog, pathing, targeting, technology progression and victory rules with original reference matches; measure discrepancies |
| 3 | Display/audio settings | Fixed 960×544 software surface and mute toggle | Window/fullscreen and scale choices, volume controls, persistent settings, correct pointer mapping and audio feedback |
| 4 | Enhanced graphics | Original Clone Campaigns assets; no EF graphics mode | Optional enhanced asset profile, validated palettes/animation/footprints; preserve classic mode and gameplay readability |

Do not treat screen resemblance as feature completion. Every enabled control must work; unsupported legacy destinations must be visibly unavailable. The Zone reference should not be wired to an unverified modern destination. Multiplayer remains deferred.

## Implementation constraints

Use original assets/layout conventions first. Avoid a generic dashboard or a replacement visual identity. Keep classic presentation available when enhanced assets arrive. Keep asset profile and gameplay-data profile separate: an EF art choice must not silently switch rules or claim EF data compatibility. Metal is a renderer option, not a prerequisite for familiar UX or better artwork.

The native runtime/builds and copied skirmish assets stay outside Documents. Do not modify original installations. Keep the prior static recompilation as a reference and all work local unless publishing is separately requested. Use feat/, fix/, docs/ branch prefixes; no co-author metadata.

## Clone Campaigns recording audit — 2026-11-06

Source: user's Desktop recording `Enregistrement de l’écran 2026-11-06 à 23.00.04.mov`. Duration 96.593 seconds; captured at 3024×1964, 120 fps; video only, no audio track. Contact sheet and timestamped reference frames are kept locally under `~/Library/Application Support/Galactic Genie/Research/clone-campaigns-recording/`. Neither recording nor extracted images are shipped game assets. Time ranges below are approximate, based on sampled frames; they do not measure animation timing.

### Observed behavior and appearance

| Recording position | Observation | Consequence for the native frontend |
| --- | --- | --- |
| 00:01–00:09 and 01:10 | Options occupies the existing right-hand main-menu panel. The illustrated left navigation remains labelled and usable-looking. Header contains page title and player name; contextual help occupies the separate bottom-right frame. | Preserve the shell and navigation when switching panel pages. Our current Options view removes the left labels and substitutes a large mute button, which is a prototype, not matching behavior. |
| 00:01 and 01:10 | Options has a player list with New/Change/Delete/Hotkeys, separate vertical Music Volume, Sound Volume and Scroll Speed controls, Graphics Detail choices, Screen Size choices, and four preference toggles. | Implement actual separate audio gains, scrolling sensitivity and persistence before declaring options complete. A mute toggle does not cover this screen. Do not add decorative settings that have no engine effect. |
| 00:01 | Visible toggles: Allow Audio Taunts, One-Click Garrisoning, Two-Button Mouse Interface, Friend or Enemy Colors. Visible screen choices: 800×600, 1024×768, 1280×1024. | Each preference needs a testable effect. Capture dimensions are not the game's logical rendering resolution; do not infer scaling policy solely from the recording. |
| 00:13–01:06 | Hotkeys uses a distinct full-screen background with two independently scrollable lists: groups on the left and commands/bindings on the right. There are Change, Reset, Reset All Keys, Allow Duplicates and Main Menu controls. Selection, hover and helper text have distinct treatments. | A real command/binding model must drive both gameplay dispatch and the editor. Observe rebinding, conflicts, reset confirmation and persistence before implementing their semantics. |
| 00:13 and 00:29 | Visible bindings include H for Command Center, F2 technology tree, F4 statistics, F11 game time, and Ctrl+letter building navigation. Some commands display multiple bindings; one appears as `???`. | Treat these as the recorded player's bindings, not proven factory defaults. Keep unsupported keys/commands distinguishable from known defaults; do not guess the `???` entry. |
| Approximately 01:14–01:18 | Databank replaces the main-menu composition with its own framed layout: topic list, heading, illustration, scrollable text, lower-left image, Return to Main Menu. Changing civilization changes content. | It requires its own screen and original content/assets, not a generic right-panel text page. It remains deferred, not implemented. |
| 01:26 | Basic Training has another distinct full-screen presentation: green holographic artwork, seven connected mission hotspots, Main Menu at upper left and helper text at the bottom. | Do not wire this to generic Help or pretend a training campaign exists. Keep unavailable until scenario loading and objectives work. |
| 01:35 | Multiplayer uses the same main shell and right panel: connection type list, Show Games, session list, Join/Create. The multiplayer artwork has a blue holographic highlight. | Preserve this presentation as reference, but leave networking deferred. Visible provider labels establish UI only, not working connectivity. |

Across the recording, thin blue inset controls and scanline surfaces are integrated into the artwork. Left-side destinations highlight entire illustrated regions with holographic overlays; our simple underline does not reproduce that feedback. The dedicated bottom-right panel changes contextual help according to the hovered control. Font metrics, alignment, active/disabled/hover colors and scrollbar treatment need comparison against the original resources rather than approximation with generic buttons. The current fixed-size window changes between launcher, main menu and gameplay were not validated as reference behavior by this recording.

### Revised next acceptance gates

1. Keep left navigation, header and bottom help panel consistent across main-menu panel pages; use original hotspot/control frames for hover and selection. Check mouse hitboxes and keyboard focus independently.
2. Replace the audio-only Options prototype with functional separate music/effects volume and scroll speed, then persistence. Match observed control layout; explicitly defer the player-profile manager and preferences whose simulation behavior is unverified.
3. Inspect Single Player → setup → match, in-game options/pause and result flow in the reference. This recording does not show those paths; our fixed match is not evidence of original setup behavior.
4. Audit real in-game selection, orders, building/production/research queues, minimap, hotkeys and dialogs. Compare reference/native screenshots and actions at the same logical size. Do not mark HUD fidelity complete from main-menu artwork alone.

Unverified from this recording: setting application timing, persistence after restart, reset/change dialogs, factory keybindings, original music/click sounds, game HUD, match setup, save/load, or network behavior. The native skirmish's previous audio confirmation remains separate evidence.

## Live Wine reference — 2026-11-07

Direct macOS scripting now works after the user granted Accessibility access. The computer-use connector still cannot attach to this Wine app; System Events can target the actual Wine PID, CoreGraphics can send mouse input, and `screencapture -l` captures its window. Resolve PID/window ID afresh each session; the wrapper's bundle ID does not identify the Wine UI process. Screenshots are retained locally in `~/Library/Application Support/Galactic Genie/Research/live-reference/`.

Verified sequence: version selector showed Clone Campaigns (x1) selected → Launch → window titled Star Wars Galactic Battlegrounds: Clone Campaigns → Single Player → New Player dialog → created local profile ReferenceTest → Standard Game → Start Game → match → F10 game menu → Options. No wrapper configuration, existing player profile or saved game was overwritten. The test match was left with its Options dialog open.

Reference provenance: this wrapper launches battlegrounds_x2.exe and logs EFPATCH 1.5.5, with an explicit Clone Campaigns/EF version selector. These are observations of Clone Campaigns mode in that patched runtime, not proof of an untouched retail executable. In particular the setup displayed population 375; do not assume that is a retail default.

New evidence and native gaps:

- First use requests a local player name. Single Player stays within the main-menu shell and offers original campaigns, Episode II campaigns, Standard Game, Custom Campaign and Saved Game (disabled in this new profile). Our direct Single Player → fixed setup skips this layer.
- Standard Game is a separate full-screen layout, with player/civilization/color/team rows at left and settings at right. Captured settings include game/map type, location, map size, difficulty, number of opponents, resources, population, reveal map, starting tech and victory; Team Together, Lock Teams, All Techs and Record Game toggles. Inspect each dropdown before implementing its choices.
- The observed random match started as Gungans with three workers and a mounted unit, normal unexplored terrain, a Command Center, and 4/5 population. It is not our custom opening.
- In this configuration the interface occupies the bottom: command grid left, selection/status area center, minimap and resource/population indicators right. The native frontend's top resources, debug minimap and large action overlay do not match this presentation. Do not generalize this configured interface layout to every preference without checking alternatives.
- F10 opens a centered overlay over the visible match, with Quit Current Game, Save, Load, Restart, Save Chapter, Options, About and Cancel. Our frontend instead switches to main-menu artwork; this needs correction.
- In-game Options is another centered overlay with Speed, independent Music/Sound volumes, Scroll Speed, Graphics Detail, Screen Size, interaction toggles, Hotkeys and OK/Cancel. This differs from the main-menu Options panel. Application/cancellation/persistence semantics still need testing; merely opening the dialog does not establish them.

No full reference match, save/load, changed settings, input rebinding or multiplayer was tested in this pass. These captures add real navigation/setup/HUD evidence; they do not imply simulation parity.

### Native implementation follow-up — 2026-11-09

Implemented centered pause/options/resign overlays over gameplay without resizing the match window, and independent music/effects gain controls plus mute. Native visual checks and mixer/navigation/render smoke passed. Original dialog assets/layout, persistent settings, OK/Cancel transactions, and the original bottom HUD remain outstanding. The reference's full settings screen is not yet reproduced.

### Original menu assets and navigation — 2026-11-09

Added the Single Player panel before Standard Game, with the main-menu navigation retained on Single Player and main-menu Options. Standard Game uses installed SLP 50104 and palette 50533. Its displayed settings describe the actual fixed test fixture; they are not configurable original-game settings. Campaign and saved-game entries remain disabled.

The pause menu now uses original SLP 50222 and the reference's eight-entry ordering. Quit Current Game requests resignation confirmation; Restart requests confirmation and uses the same deterministic initializer as Start Game; Cancel resumes. Save, Load, Save Chapter and About remain disabled. In-game Options uses SLP 50225 with functional separate music/effects controls, but its layout and semantics still differ from the original settings screen. Button scanlines are drawn procedurally, not claimed pixel-identical.

Desktop smoke verifies the extra navigation level, shared-panel return paths, disabled destinations, restart confirmation/cancellation, volume bounds and frozen simulation under overlays. Captures are kept in the external build directory. Original bottom HUD, configurable standard matches, save/load, settings persistence, campaigns and a full human UI playthrough are still outstanding.

The menu now draws the installed Zone logo and original holographic frames for Single Player/Options, with image-sized hit areas and keyboard focus feedback. Positions were matched against the installed background (frames 18, 36 and 12); exact retail hover/pressed timing is not yet established. Native-window navigation, match startup, confirmed restart and the corrected Options frame were visually checked. A shared renderer fix makes plain/tinted drawing honor alpha-only textures; the desktop smoke checks exact opacity/color samples, and the same check passed ASan/UBSan. The font atlas now uses that supported mask path.

### Original typography and language resources — 2026-11-09

REA inspected the copied original language.dll resource tree (Evidence `ev_896d79fc8ab6029705d40d24071da3f9b31f5fc88353f9b78a6cc720e1f9b769`, SHA-256 `7b7805d314f43a86ac6176b48bd521e2d8fbf5b32788eadabc5c46cad0e38dc4`). Its string table explicitly names Arial and supplies size/style triples. Menu typography now uses CoreText with the requested family, rejects silent family substitution, and caches UTF-8 label masks. It uses the Mac's installed font, not redistributed Windows font files. Captions/buttons/dialogs currently select definitions 119/146/110 and convert point sizes at 96 DPI; those role assignments and rasterization equivalence remain provisional until original GDI calls are verified. The gameplay HUD still uses the upstream font.

Original captions are loaded through the existing LanguageStrings reader, base then Clone Campaigns overlay. Main navigation uses 9500–9509, single-player campaign captions use 9222/9224/9226/9229, and game-menu captions use 9272–9279/9289/9508. This installation supplies French text. Prototype-only labels and instructions are still English; this is not complete localization. Setup/pause/options/main focus colors now come directly from the respective SIN definitions (50053/50015/50018/53232). Inset button text is centered using measured glyph metrics rather than estimated character widths.

Menu clicks now require a matching enabled press/release; releasing outside or without a press does not activate. Original alternate hologram frames provide pressed feedback, although the exact retail mapping of hover/pressed frames still needs runtime corroboration. Smoke checks cover UTF-8 accents, invalid UTF-8 rejection, glyph orientation, release-only/drag-out cancellation, navigation and streamed audio. Screenshot inspection caught and corrected an initial inverted CoreText bitmap. No claim of exact whole-game UI parity is made.

Native-window verification reached the localized Single Player panel, setup, gameplay and pause menu by keyboard. Live automation's mouse actions repeatedly delivered the same SDL coordinates (547,167), outside the launcher, despite different requested positions; this does not establish physical mouse behavior. Synthetic tests verify matching down/up activation, release-only rejection, drag-out and focus-loss cancellation. Temporary event tracing was removed after diagnosis. Physical mouse verification and exact GDI font-role/rasterization parity remain open.

### Gameplay typography follow-up — 2026-11-09

Gameplay selection/status/action text now uses the installed original font family through a desktop renderer text hook. The core's measured text widths use that same hook; renderers without platform typography retain the existing baked font. CoreText is confined to the macOS frontend. Gameplay sizes still follow the current HUD's requested line sizes, so this does not establish original per-control font roles or pixel-identical Windows rasterization.

The desktop smoke compares complete text pixels at world scales 1 and 2, including an accented label and explicit tint. The pixels and measured width must match. It also captures a selected Command Center and its production panel as `hud-selection-smoke.png` and `hud-actions-smoke.png`; fixture selection uses a test helper, while opening the action panel goes through InputState. Screenshot inspection confirms readable original-language names and measured stat rows. The original bottom command grid/minimap layout remains unfinished; merely placing its artwork underneath the current controls would misalign hit-testing and queue controls.

Added numbered control-group input (Ctrl+0–9 store; 0–9 recall). Storage includes only currently owned inspectable selections; recall prunes dead/transferred members, skips temporarily hidden members and clears placement/targeting state. Both world-reset paths clear groups to avoid accidentally selecting reused spawn IDs. Initial tests cover ordered recall, empty groups, building groups, enemy exclusion, dead units and restart. Original default bindings, group indicators, double-tap centering and complete UI parity remain unverified.

### Original bottom HUD integration — 2026-11-09

Desktop opts into a HUD using installed 51101–51106 sheets and original icon atlases. Rendering and hit testing share an 800×170 logical panel. Production/research/build commands reuse the existing handlers; queue cancellation refunds through the existing routine. HUD clicks are consumed before world selection/placement. Minimap camera and contextual-order coordinates share an isometric transform. Group portraits can narrow selection. The legacy frontend remains available to other platforms.

The 960×544 desktop scales this panel by 1.2. Command slots prefer DAT button IDs but use fallback slots and pagination when necessary; worker category icons currently use a representative building. These are known approximations, not verified retail mappings. Full formation/garrison presentation, map visibility rules, control states, localized labels, original font roles, setup choices and gameplay parity remain outstanding. Original assets stay in the local runtime directory and are not added to the repository.

Verification: ASan/UBSan passed HUD training/payment, cancellation/refund, empty-panel click isolation, menu consumption, worker submenu/back, group-portrait selection and construction placement with a paid worker order. The existing full AI and scripted-player match tests passed, as did desktop navigation/audio/render smoke. Native keyboard navigation reached the new HUD, restart and pause. A CUA mouse click again arrived at an unusable position and edge-scrolled the world; the match was restarted and left paused. Physical mouse interaction, minimap orders and a full human playthrough are not yet verified.

### Command positions, formations and garrison — 2026-11-09

Recovered the original layout resource 53290 (base 50006), command constructors 0x502d20/0x504f50, worker category constructors 0x504490/0x504900/0x504b80, event dispatch 0x4fb4f0 and garrison rendering 0x5dd410. Layout now uses the original (6,3) command origin and 41px cells, original minimap bounds and Menu rectangle in the scaled 800px profile.

The generic slot packing is removed. Ordinary mobile orders use slots 0–3, stances 4–7, formations 8–11, garrison 12 and delete 14. Formation names were checked against the original localized help strings; Box and Staggered must not be assigned from icon appearance alone. Worker categories use command frames 30/31/29; their building pages use DAT positions, cycle at 13 and cancel at 14. Building eject-all uses slot 12 only while occupied, gather-point slot 13. Special unit/building abilities are not all implemented, so this is not complete command parity.

Formation changes call the existing shared movement implementation and retain original normal/active assets. Garrison occupants use their original portraits and click-to-eject through the existing unload logic; eject-all preserves the original command icon. The garrison renderer establishes 40px portraits, 41px row stride and health/shield bars, but the native portrait origin and overflow scrolling remain adaptations pending a runtime comparison. No fog-of-war, remastered art, original AI or full gameplay-parity claim is made.

Evidence is retained locally in Research/hud-command-evidence-2026-11-09.json, target SHA256 30fac6f443391e1e3a8f29887f85f4c5061ca9e12397a4b50db1ffda0d633761. Key records: command construction ev_8a683f6b86e30204ad5c1a87e99a72b86365c049b662ada0537e74153faebbde; slot helper ev_ad6802d71ab4bd5b62448a6f77bdf487364d0cbd5e7fb5fc41bef743e87dff38; layout ev_1b3da89c7d4b85e5b50b88c00f6f2244fef5d3e8ca2a1df628b2f1c1ec6abf56; garrison ev_5fb5e12199718cdce0e7c3b9c6b7fa65b620cff1d70c512c7bdddfd1ab7c8e15. These are static observations, not proof of identical runtime behavior.

Validation: command-slot and original-icon assertions, all four formation selections/active states, physical garrison entry, individual ejection, eject-all disappearance, worker next/cancel positions and the existing paid construction/production checks pass under ASan/UBSan. Full scripted-player victory and idle-player defeat still pass. Desktop smoke covers navigation, audio streaming and captures hud-formations-smoke.png / hud-garrison-smoke.png; both captures were inspected. Full human playthrough and pixel-identical garrison placement remain unverified.

### Targeted transport unload, rally controls and pause — 2026-11-09

Original command constructor 0x502d20 assigns loaded-transport Unload to slot 13/frame 17/action 7; dispatcher 0x4fb4f0 enters target mode 0x69 with prompt 3053. Native Unload now targets a destination, uses the existing path planner, stops at its reachable endpoint and safely ejects. Blocked passengers remain aboard and retry at 0.5-second intervals. Stop and replacement UI orders clear the pending unload. A failed ejection restores hidden passenger coordinates. Exact original route planning, cursor and retry timing are not established; scenario-trigger/direct group-move callers still bypass the UI's shared order-cancellation path.

Original mode constructor 0x506990 establishes rally remove slot 0/frame 46/action 0x98 and cancel slot 14/frame 0/action 0x89. Both are wired to the existing rally state. Cancel preserves the rally; remove clears it and exits targeting. Garrison portraits now render health and shield bars. Original portrait origin/overflow remain unresolved.

Space-pause uses a separate inspection update, never a zero-timestep simulation call: some existing AI/completion routines can mutate even at dt=0. Camera, minimap, zoom, world/box selection and control-group recall remain usable. Command handlers and queue refunds are gated off. F10/options return preserves manual pause. ASan/UBSan asserts frozen match time, worker positions, object count, resources and production timers alongside working inspection. Tests also cover paused garrison ejection rejection, original rally icons/cancel/remove, loaded transport icon, pause/Stop during unloading, travel and passenger survival. Full AI defeat and scripted paid-player victory still pass. Desktop smoke verifies rendering/navigation/audio; native-window observation confirms arrow-camera movement while the paused title remains visible.

Remaining special behavior: Command Center slot 14/action 0xa3 is the alarm (Town Bell), requiring a shelter/return-to-work lifecycle, not a shield toggle. Self-shields are passive in the current simulation. Unit 82's Commander Locator requires regicide/fog behavior absent from the fixture. These remain unavailable rather than exposing misleading functional buttons. No new live Wine comparison was made in this pass; mappings come from the locally retained original executable evidence.

### Original alarm and return-to-work reconstruction — 2026-11-09

REA traced selected-CC dispatch 0x5d5720 through packet command 0x7f into alarm-on 0x5d50a0 and alarm-off 0x5d5420. The implementation now follows the ±25 integer-tile search area, class58 workers, shelter types CC109 / class10 turrets / class9 missile turrets / Fortress82, and squared-distance bands 5,10,...625. Shelter tie precedence follows CC, turret, missile turret, Fortress. Active shelter flags are local to buildings. Multiple selected CCs snapshot desired states before execution so overlapping search areas do not spuriously reverse the next selected CC.

Worker boarding saves a small job record, preserved across repeated boarding orders (0x55f830's existing-record behavior). Return validates gather/build/repair/drop-off targets through existing handlers and otherwise routes to the remembered coordinates. Explicit replacement orders and successful manual ejection clear the record. Failed exits preserve it. A garrison route is planned before discarding current work so an unreachable shelter does not erase the job. Slot11/frame64 Return-to-work restores CC/Fortress worker occupants without turning off the alarm or ejecting military occupants; slot14 frames49/61 toggles the alarm. Localized titles come from installed help41111/41015.

Static evidence: Research/alarm-command-evidence-2026-11-09.json (13 records), readable alarm-findings-2026-11-09.txt. Alarm-on assembly ev_fb22b136f1d1d7a205a585d979eff9b8fe61e6d48d1b960d7dbb666683710f4e; off assembly ev_d4fdbbaee1d58c9ac2787e0634bdf967950e166be7786701d14aa4c12c4a6f51. Job save/restore ev_f789b7b9eabf16780293d362d9d34dcd6e7ea37781ecfe5edda78edb397f9eb2; return-work/manual-eject ev_a9de8c62f3ba1968b3b80d8e71d0a4b95f4385208b81449517628fbabf1b8fb5. Same reference executable SHA256 as previous HUD work.

Original 0x5e5a10 maps sound events39/40 to townbell.wav50357 / townrcal.wav50358; 0x42cd70 applies spatial mixing. The native app uses the actual installed samples, one local cue per selected CC. Evidence ev_5550702337775a2b95472fe466c35a6bf7116bcd61021324f22f1ae013643070, Research/alarm-sound-evidence-2026-11-09.json. This establishes sample identity, not identical spatial mixing.

Garrison portrait hints now expose occupant stats and each portrait uses its own civilization's icon set. Retained 0x5dd410 shows dynamic columns, bottom-up41px rows and caps40/19 depending on mode, skipping excess older entries; it does not establish the current scroll UI. Exact original origins and overflow behavior remain unfinished. Native capacity reservation includes incoming workers whereas original alarm allocation initially counts occupants; this intentionally avoids lost/overbooked orders. Saved action restoration covers the supported worker jobs, not every original action-stack variant. No fresh live Wine comparison or pixel-identical/full gameplay parity is claimed.

Validation for this pass: ASan/UBSan passed original alarm icon/state and return-work placement, incoming cancellation, repeated shelter orders preserving the first job, gather/build/repair resumption, paused return rejection, blocked exits preserving occupants/jobs, exhausted-resource fallback, full-capacity preservation and out-of-range exclusion. Exact50357/50358 callback dispatch is asserted. Both full match outcomes pass. Desktop smoke loads the two original WAV resources, mixes alarm/recall, captures hud-alarm-smoke.png and checks that Return-to-work leaves soldiers and bell state unchanged. The capture was visually inspected. The refreshed native app reaches a paused match by keyboard; two automated building clicks failed to select it, consistent with the earlier native mouse automation limitation. Physical mouse interaction and a listening comparison of the bell remain unverified.

### Garrison geometry and original sound curve — 2026-11-09

Recovered 0x5dd410's floating-point layout calculations from assembly and initializer data:800-profile panel-relative origin100,42; base spacing38; row stride41; default maximum3rows/40entries, production mode1row/19entries. When necessary, columns=ceil(count/maxRows), spacing=(baseColumns-1)*38/(columns-1). Retained occupants are drawn in forward order from bottom-right toward top-left;0x5deda0 scans registration indices descending, so last-painted rectangles win overlaps. Native input, rendering and tooltip hit-testing now share this geometry. Removed the unsupported scroll-page affordance and invented count-label placement. Subject header/name/portrait/health/shield coordinates now follow the recovered800-profile left column. Roster iteration still uses native object order; matching original container insertion ordering is not independently established.

Original552a50 computes clamped pan and volume from listener-relative position, with a fractional-position vertical offset. Volume=3*min(0,1000-abs(vertical))-6*max(0,abs(rawPan)-1000); pan clamps to±1000. Alarm callers reject volume<=-3000; ordinary world sounds reject<=-5000. Native implements the recovered curve using native camera/width/zoom and terrain height. The camera/slope/sight follow-up below supersedes the initial geometry and fog limitations of this pass.

Original545040(1) stops, rewinds and plays the same sound buffer; keyed alarm voices now restart rather than stack. The SDL mixer applies independent per-channel decibel gains before existing effects/music master gains. Microsoft documentation confirms that DirectSound pan attenuates one side while leaving the other at full level, cumulatively with volume: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee418148(v=vs.85) and https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee418150(v=vs.85). This restores relative mixing behavior, not identical global loudness or device output.

Local reference evidence: Research/garrison-layout-audio-evidence-2026-11-09.json (414 records), readable garrison-layout-audio-findings-2026-11-09.txt and companion layout-audio-ADDRESS.c/.asm. Same reference executable SHA25630fac6f443391e1e3a8f29887f85f4c5061ca9e12397a4b50db1ffda0d633761. Mixing evidence ev_8f7a835c42e75e8a297957d1b9f0a7f69ae331beea1a4e0468fd321d27ea1487; scale assembly ev_c907fb06bcfb57ce150a1b920aace758aee4c1ee084de0218b22516283fac5cd; map geometry ev_0eaa0bfc3ffc55e0fb0a470414acc861028370d75d49cf304318daa719c1e942. These are static executable observations; no fresh live Wine comparison was performed.

The spatial tests exposed an important distinction: original map initializer defaults32/16 are not runtime SWGB geometry. Both original scale and fractional offset use runtime map half-width/half-height; native SWGB uses48/24. The offset now uses kTileHalfH and numeric tests use actual native geometry. The local research ledger records this correction.

Additional5deba0 assembly establishes btngame.shp50751 frames36/37 for garrison buttons. Two clipped draws construct40px corners from54px artwork; the icon is inset and shifts one pixel while pressed. Native now uses that original artwork and tracks the pressed occupant ID through release, rejecting release-only and drag-out activation. Multi-selection hides and disables garrison hit targets; crowded-slot tests cover reverse-order placement and overlap priority. Evidence is retained in Research/garrison-bevel-evidence-2026-11-09.json. No original artwork was copied into the repository.

Final validation: gameplay ASan/UBSan suite passes, including full player victory and idle-player defeat. Garrison tests cover15occupants, compressed bottom-right placement, topmost overlap activation, busy single-row layout without queue cancellation, release-only rejection, drag-out cancellation, paused ejection rejection and invisible multi-selection hitboxes. The owned-selection fixture helper now permits owned inspectable buildings, matching the building/group test requirement. Spatial tests check center/left/right/vertical numeric results at actual48/24geometry, per-shelter alarm events and distant-alarm suppression. SDL smoke passes decibel pan/volume combination, buffer restart, streaming music and rendered normal/pressed garrison captures. Captures were inspected; the refreshed native app reaches a paused skirmish. Fresh original-runtime listening and physical mouse playthrough remain unverified.


### Camera, slopes and sight-filtered sound — 2026-11-09

Static Clone Campaigns comparison recovered the full-client-width audio denominator, gameplay-viewport listener center, all 17 slope interpolation/offset cases, and the original sight row spans for radii 0–20. Evidence: local Research/camera-slopes-fog-evidence-2026-11-09.json and camera-slopes-fog-findings-2026-11-09.txt; original-los-stencils-2026-11-09.json supplies the compact numeric table in sight_stencils.h. Reference SHA256 remains 30fac6f443391e1e3a8f29887f85f4c5061ca9e12397a4b50db1ffda0d633761.

Native camera targeting now centers above the classic HUD; minimap targeting no longer adds that adjustment twice. Zoom retains this gameplay center, and camera bounds account for the HUD offset. Ambience samples that gameplay center. Normal terrain height uses the original triangular planes rather than bilinear interpolation; malformed steep/opposing-corner native tiles retain their prior fallback. Spatial sound includes the recovered slope flag and viewport anchor. Projectile impact audio now follows the same spatial/sight path as other world events.

For the current Explored skirmish, world audio requires current own-player sight even though terrain is known. Sight is queried from live sources at dispatch using original row spans; this avoids a stale per-frame cache and needs no exploration history for this mode. Foundations reveal their footprint; completed buildings use the original alternate/normal stencil rule. Centered selection and order acknowledgements remain interface feedback.

Scope limits: visual fog/remembered buildings, shared allied sight, observer/reveal modes, exceptional terrain visibility rules and AI knowledge are not implemented by this change. The source scan is appropriate for small matches; profile before adopting it for large-army audio loads. This comparison is executable evidence, not a new live Wine side-by-side recording; exact sprite-origin and audible loudness calibration remain open.

Verification: final ASan/UBSan skirmish suite passes, including current-sight boundaries, foreign ownership, movement, overlapping sources/death, foundation completion, slope height, above-HUD camera center and paused zoom/listener consistency. Existing alarm/garrison/pause checks and complete AI defeat/scripted-player victory matches pass. Desktop menu/render/audio smoke passes. Native keyboard navigation reached the rebuilt skirmish and it was left paused; no fresh physical listening or live Wine comparison was performed. Logs: external build directory camera-fog-tests.log and camera-fog-smoke.log.

### Optional higher-resolution art direction

Preserve original artwork as the default and fallback. A future optional pack can replace raster frames while preserving their logical size, hotspots, animation timing/directions, shadows and player-color masks. Vector text, borders and simple geometric controls can scale cleanly; painted metal panels and holograms should retain raster artwork or be carefully reconstructed, not blindly traced into SVG. SVG would be rasterized/cached at the requested display scale, not interpreted every frame.

A separate community [HD mod](https://www.moddb.com/downloads/star-wars-galactic-battlegrounds-hd-mod) advertises upscaled sprites/terrain and a 4K field-of-view option; its completeness, quality and EF compatibility are unverified. [EF 1.5.5 notes](https://www.moddb.com/mods/star-wars-galactic-battlegrounds-expanding-fronts/news/expanding-fronts-155-now-available1) document selected graphic replacements and display improvements, not a verified complete 4K art pack. EF artist [Hao Dinh's portfolio](https://www.haodinh.com/expanding-fronts) describes models rendered into sprites: source-model rerenders, if available with permission, could produce stronger results than upscale-only frames. No third-party pack has been installed or bundled.

Increasing each sprite dimension 4× costs approximately 16× its pixel storage. The current software renderer and 64 MiB texture budget need profiling and a revised upload/cache strategy before a whole-game high-resolution pack. GPU rendering and better artwork are separate tasks; neither implies rewriting the simulation.


### Visual fog and researched allied sight — 2026-11-09

This pass supersedes the prior visual-fog/shared-sight limitation for the current small skirmish. Per-player sight/exploration masks now use the recovered row spans and foundation rules; current sight is recomputed from living sources, and exploration persists as units move. The desktop fixture remains Explored. Normal/unexplored behavior is exercised by tests, not a new selectable setup option. Existing explore/sight cheats affect the corresponding presentation states.

Terrain and minimap distinguish current sight, explored fog and unknown black. Moving units, projectiles, remains, target picking, selection indicators and health information disappear outside sight. Static buildings/resources retain snapshots of their last observed state; unseen damage/destruction does not update those snapshots, and revisiting clears stale objects. Remembered terrain prevents unseen farm/terrain-ID changes from appearing. Explored starts seed stationary Gaia resources from the initial map; this initialization choice still needs live-reference corroboration. Memory resets between matches/viewers.

Original mask builder 0x624330 requires the viewer's ally-reveal flag and mutual diplomacy stance 0; the ally does not need its own research. DAT tech 61 REVEAL-ALLY (localized Emetteur Holonet / Holonet Transceiver), effect 46, sets resource 50 to 1. The native mask reuses playerAttribute(50), so it follows installed-data research effects rather than hardcoding tech 61 in gameplay. Resource 183's separate all-player reveal effect also contributes source masks. Masks apply to current visual sight, explored terrain, minimap and spatial world sounds. Breaking mutual alliance removes that source's shared view; exploration obtained from one's own units remains.

Original resource-50 handlers latch the ally-reveal flag whenever the resource is written, even if the value is zero. Native playerAttribute(50)>0 covers normal research; unusual scenario writes of zero and observer/global overrides remain outside this pass. The native AI retains its existing information model; this work does not claim original AI fog fairness.

Presentation limits: uniform RGB 144/255 dimming uses original textures and slope geometry, but original palette dithering/fringe transitions are not reproduced yet. Hidden building memories are drawn but not selectable; no current hidden stats are exposed through them. Terrain-ID history is preserved, but mutable elevation history is not separately recorded. These limits prevent a claim of pixel-exact or complete original fog parity.

Tests include pixel equality for hidden movement (including minimap), unseen building damage/destruction and unseen terrain changes; exploration persistence/revisit/reset; viewer-only Holonet research, mutual versus one-way diplomacy, shared exploration and sound gating. Local comparison captures: external build fog-visible.png, fog-remembered.png and fog-unexplored.png. ASan/UBSan suite passed, including both full-match outcomes; desktop menu/render/audio smoke passed. Local logs: visual-fog-tests.log and visual-fog-smoke.log. Static evidence: Research/shared-sight-evidence-2026-11-09.json (78 canonical records) and shared-sight-findings-2026-11-09.txt. No fresh live Wine comparison was performed.


### Original terrain fog edges, remembered inspection and AI awareness — 2026-11-09

This supersedes the uniform native terrain-fog edge limitation. The game now reads the installed TileEdge.Dat / BlkEdge.Dat, validates all 17 slope blocks and 47 canonical patterns, and uses the original neighbor normalization and span regions. Current visibility chooses normal/fog clipping; explored visibility chooses the black-edge cutout. The native software renderer applies the recovered palette-zero checkerboard at destination-pixel parity, including zoom and clipping. Original patterns remain local runtime assets, not repository data. Missing edge files retain the previous uniform fallback; malformed present files fail validation. Non-software renderer backends still use average-opacity fog pending a screen-parity shader.

Evidence: Research/fog-edge-findings-2026-11-09.txt and fog-edge-evidence-2026-11-09.json. Reference SHA256 30fac6f443391e1e3a8f29887f85f4c5061ca9e12397a4b50db1ffda0d633761. Key functions: loader/47-pattern mapping 0x609720, neighbor mask 0x60fcc0, clipping passes 0x60dbe0 / 0x610bb0, pixel copier 0x651bc0 / 0x648483. This is static ordinary 8-bit-path evidence; the alternate renderer and exact live screenshot phase remain unverified. Minimap and remembered sprites retain the prior dimming treatment; this pass does not claim their palette mapping is exact.

**Historical implementation, superseded by the 2026-11-10 Windows comparison below:** Native classic HUD memory inspection was isolated from live simulation selection. Clicking a remembered building (also while paused) displays its snapshot portrait/name/health and selection footprint, with no commands, queues or live garrison disclosure. Losing sight of a selected building switches to memory; revisiting restores current selection or clears a stale destroyed-building memory. Local evidence memory-doppelganger-evidence-2026-11-09.json confirms original stored sprite/prototype/owner references, but no HP snapshot. The original picker links to the real object's ID, so our safe last-seen health presentation is an explicit behavior choice pending runtime corroboration. Legacy compact-HUD inspection remains unchanged.

Combat acquisition, guarding, retaliation, target retention, pending volleys and hidden rally targets now use player-specific sight, including the researched mutual-ally rules. Lost targets become movement toward the last observed position, without reading hidden movement or death. Resource searches require sight. The fixture's attack waves search deterministic waypoints from their own base/map geometry instead of reading the enemy Command Center's current coordinates. This is custom scouting and economy behavior, not a recovered original AI policy or full strategic exploration model.

Verification covers original edge-file parsing (including truncation/invalid offsets/out-of-bounds spans), all 256 neighbor masks, slope dimensions, checker parity at different zoom/clipping, remembered mouse inspection and unchanged pixels under hidden damage/death, AI last-seen pursuit and hidden-target changes, and the prior fog/alliance/UI/audio regressions. Both complete-match outcomes remain covered. Local captures: fog-visible.png, fog-remembered-inspection.png and fog-unexplored-edge.png in the external build directory; logs fog-fidelity-tests.log and fog-fidelity-smoke.log. Final ASan/UBSan suite and desktop/menu/audio smoke passed. The final native app was opened through launcher → single-player → setup → skirmish and left visibly paused. Automated player victory completed at 675.1 simulated seconds; the idle-player defeat path also completed. No new live Wine side-by-side run was performed.

### Remembered orders and observation-driven scouting — 2026-11-09

Right-clicking a remembered enemy building now orders movement to its last-seen location using normal formation/collision placement. Attack intent is resolved only when that location becomes visible and the target is still visible and hostile. Stop, replacement movement and a new attack clear the intention; unseen destruction alone does not. Paused inspection remains available but issuing orders remains blocked.

Scouting now tracks actual observation separately from the initially explored terrain setting. Attack waves choose a common destination by expected new sight versus travel distance, using their own observations and researched shared allied sight. They advance after observing a destination and retry failed/stale destinations after a cooldown. Hidden enemy position/death does not influence destination choice. This remains custom small-land-map AI, not the recovered Clone Campaigns AI; naval and general-map reachability planning are not implemented.

Additional original evidence: Research/memory-interaction-findings-2026-11-09.txt. Function 0x5d3d00 is a player virtual contextual cursor/help query, called at 0x60edfe; its hidden-object HP check supports repair/heal cursor eligibility, not proven live HUD health. Original destroyed-memory picking can return -1 while its sprite persists. Native frozen-health inspection and deferred orders therefore remain deliberate behavior choices pending a live runtime comparison; this static evidence does not establish exact interaction parity.

Verification: final ASan/UBSan full suite passed, including real paused/unpaused mouse orders on memory, Stop/replacement cancellation, passive-stance reacquisition, hidden destruction/revisit cleanup, scouting history and hidden-enemy independence. Both complete-match outcomes passed; scripted player victory took 677.7 simulated seconds without resource/unit/damage cheats. Desktop/menu/audio smoke passed. Logs: external build memory-scouting-tests.log and memory-scouting-smoke.log. The refreshed native app was opened through its menus and visually verified paused in a skirmish. No new live Wine comparison was performed.

### Scouting revisit coherence and live-reference attempt — 2026-11-09

Fixed a repeat-search bug after terrain had already been observed: reinforcements now retain the wave's revisit destination until it is currently visible again or the existing timeout expires. A completed destination uses the existing 30-second cooldown before reconsideration. Historical exploration still completes first-discovery goals; current sight completes revisits. This avoids splitting idle reinforcements away merely because a travelling wave's destination exists in observation history.

Added fully observed-map, displaced-reinforcement cohesion and completed-revisit checks. Full ASan/UBSan suite and desktop/menu/audio smoke passed; both full-match outcomes still pass and scripted player victory remains 677.7 simulated seconds. Logs: scouting-revisit-tests.log and scouting-revisit-smoke.log in the external build directory.

A fresh live comparison launched /Applications/SWGB.app with Clone Campaigns (x1) selected, created a disposable local MemoryReference profile, and reached a normal Shoreline match. The Wine game process exited during reveal-map setup before remembered-building interactions could be observed; the user also reported a crash/error. The patch log stops after map startup and does not identify a cause. No hidden-building behavior is inferred from this failed run. Captures and control helper are local under Research/live-memory. This reference remains the EFPATCH runtime in x1 mode, not an untouched retail executable.

Crash diagnosis follow-up: the user-supplied backtrace identifies an illegal `FFREEP ST(2)` (`DF C2`) instruction at efpatch.dll RVA 0xa3f4. The installed DLL contains that instruction there and at 0xa3f9 in its normal epilogue. An isolated x86_64 probe on this Mac also traps on DF C2, while ordinary x87 free/increment instructions execute. This strongly supports an x86 translation compatibility issue, not a demonstrated remembered-building bug. Evidence, original backtrace and probe are local in Research/efpatch-crash. No game DLL or Wine configuration was changed; a complete replacement still requires semantic and runtime validation. The user confirmed controlling the reference to reproduce the bug.

### Unpatched windowed reference and match-start crash — 2026-11-10

The user's revised SWGB.app default launches battlegrounds_x1.exe matching our original reference SHA256. Verified main → Single Player → Standard Game with no EF patch in the new crash's module list. Enabled 1280×800 windowed rendering through the existing cnc-ddraw configuration's game-specific battlegrounds_x1 section; original ini retained in Research/live-retail-memory. Mac foreground activation required Carbon SetFrontProcessWithOptions; AppKit/System Events activation alone remained ineffective.

Match startup crashes separately from the prior EF x87 failure: read fault at0x64dc8d, EDI0x0f9f0002, four-byte palette-remap read starting0x0f9effff. This is a buffer-boundary hypothesis supported by the original assembly, not a proven allocation trace. Current upstream cnc-ddraw adds prefix padding absent from7.1 source; testing its official experimental DLL in the bundled slot failed DirectDraw initialization. Wine builtin DirectDraw also failed initialization. Both experiments were reverted, restoring the original renderer bytes and Info.plist. The verified window setting remains. No game executable changed. Local evidence: Research/live-retail-memory/findings-2026-11-10.txt, screenshots, backtrace and assembly. Live remembered-building comparisons remain incomplete.

### Native audio Options confirmation — 2026-11-10

Returned development focus to Galactic Genie; no further Wine changes in this pass. Options now previews music/effects volumes with separate OK and Cancel controls, using installed language strings4001 and9275. Cancel/Escape and leaving via the Single Player navigation restore entry values and notify the audio mixer. OK retains the preview for the session. Re-selecting the Options navigation does not reset the rollback baseline. Both front-end and paused-game contexts use the same logic. F8/mute remains an independent immediate toggle; persistence and exact original slider geometry remain unfinished. This is a native interaction improvement, not newly verified retail runtime parity.

Desktop smoke coverage includes volume bounds, both channels on rollback, confirmation, repeated Options selection, main-menu navigation cancellation and in-game confirmation/cancellation. Desktop/menu/audio smoke passed, including 60 rendered frames, effects, streamed music and EOF looping. The log is in the external build options-transaction-smoke.log; options-smoke.png captures the new dialog. Simulation code was unchanged, so the full gameplay sanitizer suite was not repeated for this UI-only pass.

### Playable Standard/Explored setup — 2026-11-10

Native desktop setup now defaults to Standard visibility and offers Standard/Explored through a keyboard/mouse dropdown. Captions come from installed language strings9724,9755,9756, over the existing original setup artwork. Escape closes the dropdown before leaving setup; selecting a value closes it. The choice survives back navigation and is passed to both match start and restart. Other setup values remain fixed. Full original combo-box styling and the other reveal modes remain unfinished.

The skirmish initializer accepts the initial exploration choice after its usual state reset. Existing engine callers keep their prior Explored default; desktop callers pass their selected setting explicitly. No cheat is used: Standard uses normal per-player discovery, memories, shared sight and fog edges; Explored does not grant current sight or AI observation history. Restart tests alternate modes to catch stale explored-state leakage. No fresh Windows comparison is claimed.

Verification: full ASan/UBSan suite passed. The AI defeats an idle player at734.3 simulated seconds in both Standard and Explored modes; the existing scripted player victory remains677.7 seconds. Tests cover initialization mode switches, no unseen-enemy disclosure and unchanged actual-observation history. Desktop menu/audio/render smoke passes with keyboard/mouse selection, back-navigation persistence, dropdown cancellation and Standard-mode startup after restart navigation. Logs: external build reveal-setup-tests.log and reveal-setup-smoke.log. The setup-reveal-smoke.png capture was visually inspected. No live human playthrough or original Windows comparison was performed for this pass.

### Native additive selection — 2026-11-10

Installed Manual.pdf page7 documents Shift-click for adding units to a group. The scanned page was rendered and visually inspected locally; evidence is retained in Research/selection-controls. Native desktop now passes the Shift modifier to the shared click/box-selection paths in both live and paused play. Existing group order is retained and duplicate adds do not reorder members. Foreign/resource/building inspection is not merged into a controllable mobile group; Shift-click on non-groupable targets or empty ground preserves the group. Ordinary clicks/boxes retain replacement behavior. Shift clicks do not seed a subsequent normal double-click.

Additive drag is a native extension consistent with the grouping operation, pending live original verification; the manual page only establishes additive clicking. Re-click toggle/deselection semantics and every original grouping edge case are not claimed. No runtime original comparison was performed, and Wine was untouched.

Validation: full ASan/UBSan suite passed, including both map-mode AI victories and scripted player victory. A focused sanitizer rerun also passed the strengthened visible-enemy inspection fixture. Desktop smoke passed effects/music/EOF and a new synthetic SDL button-event test where event position differs from the polled cursor. Input now retains event coordinates for clicks, commands and committed box endpoints instead of overwriting them with a later poll. This fixes that event-handling defect but does not resolve the live computer-use selection mismatch: two coordinate clicks still selected nothing in the rebuilt native window. Native launcher → main → setup → Standard skirmish was visibly verified and left paused. Physical Shift-selection and original runtime parity remain unverified. Logs: additive-selection-tests.log and additive-selection-smoke.log in the external build directory.

### Eight civilizations, population and confirmed preferences — 2026-11-10

Setup now offers all eight Clone Campaigns factions for both sides, with installed language captions10231–10238. Initial troop suffixes come from the DAT civilization-tagged UNIT-LASER2 records, including Republic's A suffix. Invalid choices fail before resetting the running match. The classic HUD now uses installed51107/51108 for Republic/Confederacy instead of clamping them to51106. Dropdowns use the installed caption font role and reverse paint-order hit testing so a popup choice wins over the AI row beneath it. Exact original widget bevel/spacing remains unverified.

Population is selectable25–200 in steps of25, default75 in desktop setup. The engine's existing housing/production rules enforce the chosen limit for both sides; queued units wait for space. Custom AI no longer builds extra housing once its housing reaches the configured limit. These offered values are native setup choices; the full original allowed-choice list has not been independently established.

Preferences persist only on Options OK or successful match start/restart, using SDL's application-support path. The file validates version, integer ranges and trailing data before applying anything, and writes through a sibling temporary file followed by rename. Cancel/Escape keeps saved audio unchanged; preview values and F8 mute are not auto-saved. Read errors use defaults; failed writes warn and preserve the previous file. Smoke tests use a disposable temporary directory and never read/write the user's preferences.

Eight faction opening tests passed under ASan/UBSan, including actual HUD recruitment costs/completion and original HUD assets. Population cap/block/resume and invalid-setup preservation tests passed. Complete Command Center matches also passed for every AI civilization against an idle opposing faction; all-civilization-matches.log records each outcome. The original default scripted player victory remained677.7 simulated seconds. Setup/population and preferences desktop audio/render smoke checks passed. Fresh Windows comparison remains pending.

### Conquest mode — 2026-11-10

Installed language strings4321 and9823 identify Conquest and its elimination objective. Native Conquest retains a player while an active, living worker/military/boat/ship or building remains; gatherable livestock and building annex parts do not independently keep a player alive. A surviving AI continues operations after its original Command Center is lost, and launched assault survivors can keep scouting below the initial six-unit wave threshold. Command Center victory remains selectable. Desktop defaults to Conquest; preferences version2 stores the objective, while version1 retains its previous Command Center behavior. Exact original elimination exceptions still need runtime comparison.

Validation: the full ASan/UBSan suite passed, including last-worker survival, victory/defeat/draw and continued AI scouting after Command Center loss. All eight AI civilizations completed Conquest against an idle opposing faction, between846.3 and1075.2 simulated seconds. These are deterministic fixed-map matches, not a claim of general-map or strategic AI parity. Logs: conquest-final-tests.log and all-conquest-matches.log in the external build directory. The desktop render/navigation/audio smoke passed with preferences version2 and the localized captions.

Native keyboard verification changed setup to Republic versus Confederacy, population25, Standard visibility, Conquest; started the Republic match and inspected its original HUD; restarted the app and visibly confirmed restored setup values. Defaults were then restored to Empire versus Rebels, population75, and a new Conquest match was left paused. The saved file was verified as `2 100 100 1 3 75 0 1`. Computer-use mouse events still arrive in SDL at0,0; synthetic SDL-event checks pass, but physical mouse interaction needs independent verification.

The next original-runtime comparison should establish remembered-building inspection after hidden damage/destruction; Shift selection and repeat-click behavior; original popup geometry, focus and font metrics; camera speed and edge scrolling; and sound balance at matching camera positions. Native tests establish internal behavior, not exact retail parity. Save/load, general random maps, campaigns and broader technology/unit validation remain separate implementation work and are not blocked by Windows availability.

### Construction damage and player Conquest completion — 2026-11-10

Construction previously reassigned health from progress every update and healed to maximum on completion. It now adds only newly earned construction health, preserving damage while unattended and when finishing. Stack-unit completion carries the health deficit into the completed unit's maximum. The test-only force-completion helper follows the same damage-preserving rule. Regression fixtures cover damaged abandoned construction and completion; no claim is made about exact retail construction-healing rates without a runtime comparison.

The scripted player Conquest path now continues beyond the enemy Command Center, targets living visible enemies, and searches the known base perimeter without reading hidden survivor positions. Destroyed ruins must not remain its command target. Its initial 30-minute simulation limit proved too short: projectile traces showed continued damage against armored processing centers, rather than an immortal target. With a45-minute limit, the player won at2319.6 simulated seconds using ordinary gathering, paid recruitment, housing, garrison defense and combat. No resources, units or damage were granted in this complete-match test. The construction regression was separately established and fixed; it was not the explanation for the late-match time-limit failure.

Final validation: construction-damage-tests.log (full ASan/UBSan suite), construction-damage-smoke.log (native frontend/render/effects/streamed music/EOF), construction-all-conquest.log (all eight AI victories) and player-conquest-match.log (player victory) passed. player-conquest-result.png records the result. The refreshed app was opened through its native menus into a default Conquest match and left visibly paused. Physical mouse input, original Windows UI/behavior parity and long-session human play remain unverified.

### Live Windows reference: selection and menu geometry — 2026-11-10

QuickDesk MCP now captures the fullscreen 800×600 Clone Campaigns reference and controls its menus and match. A disposable local profile named `x` accepted a typed character and Enter. Standard Game with one computer started successfully; worker selection, right-click travel, paid recruitment (50 food, population4→5) and two-worker grouping were observed. Keyboard checks used the limited Mac scan-code workaround for character, Enter and Shift input; arbitrary hotkeys and remote audio are not verified.

Live Shift-click removes an already-selected worker, including the final member, and adds an unselected worker. Native click selection now toggles membership in the shared live/paused path, preserving the remaining group's order. Shift-drag remains additive. Regression coverage checks removal, empty selection, re-addition, ownership filtering and paused input. This supersedes the earlier assumption that repeated Shift-click should leave membership unchanged. The full ASan/UBSan suite passed, including existing AI and scripted-player match tests (retail-selection-tests.log in the external build directory).

The five Single Player mode buttons now use observed original 800×600 bounds: x442, width322, height38, rows94/152/210/268/326. This corrects geometry only; native Back navigation, disabled unimplemented modes and font/rendering differences remain. Reference screenshots and connection notes remain local under Application Support/Galactic Genie/Research/windows-reference. Remembered-building behavior, standalone pause semantics, camera speed and original audio balance still require further comparison. No new full-match retail completion or native physical-mouse verification is claimed.

### Original menu fonts, transparent buttons and fog picking — 2026-11-10

REA confirms the Single Player buttons use font index0 (installed language110: Arial13pt normal), not the11pt role previously assigned. Their outer rectangles are441,92+58*i,324,40; the earlier screenshot measurements captured inset border pixels. Native mode buttons and the Single Player heading now use the original13pt role. Mode buttons preserve the background and use bevel colors from original design53232/palette53237, with the normal and pressed edge runs recovered by the earlier controlled recompilation. Disabled text uses the original muted color without a shadow; enabled text has the original left/down shadow and right/up pressed offset. This applies to the Single Player mode buttons, not every dialog. CoreText rasterization still differs from GDI. The extra Back control and disabled unimplemented modes remain native accommodations.

Live Windows fog test: a worker discovered red enemy buildings before dying to wildlife. Repeated left clicks on their fogged remembered images produced no selection HUD. Positive control selected the owned Command Center normally; panning to the remembered enemy retained that selection, then clicking the remembered building cleared it. Native left click no longer opens a snapshot HUD for a remembered building. Rendering and remembered right-click orders remain unchanged. Retaining an already-selected building's snapshot after losing sight is a separate, still-unverified compatibility behavior; hidden damage/destruction and right-click parity are not established by this test.

Local evidence: Research/windows-reference/menu-fog-findings-2026-11-10.txt, menu-font-evidence-2026-11-10.json and screenshots/2026-11-10-fogged-building-click.png. The desktop smoke checks transparent interiors, pressed rendering and release-outside cancellation, alongside existing navigation/audio checks; retail-font-buttons-smoke.log passed. The full sanitizer run retail-memory-tests.log passed with the new fogged-click regression; the final test revision also checks a living hidden target explicitly.

Final validation: retail-memory-final-tests.log passed the full ASan/UBSan suite including the living-hidden-target check, AI completion and scripted-player victory. retail-font-buttons-smoke.log passed navigation, transparency, press/cancel, rendered frames, effects and streamed music/EOF. The refreshed native app was reopened and its restyled Single Player page visually verified; left open there.

### Original reference: lost selection and hidden destruction (2026-11-10)

Controlled Windows Clone Campaigns 1.1 editor test through QuickDesk: Player 1
Advanced Bomber observes Player 2 Gungan Airbase. Selecting the Airbase shows its
health and footprint. Ordering the bomber away, then selecting the still-visible
Airbase, clears the HUD and footprint once sight is lost. The remembered image
remains. This supersedes the earlier frozen remembered-HUD approximation.

Added an Object Not Visible condition targeting that Airbase and a Kill Object
effect targeting the same object. With a second distant enemy Airbase preventing
victory, the first remains intact under fog after the trigger fires; returning
the bomber reveals rubble. An initial single-building trial also ended in victory
while the intact fog image remained, providing an independent kill indication.
Screenshots are retained locally under Research/windows-reference/screenshots.

Native changes remove remembered inspection state, HUD substitution and selection
footprints. Map memory and deferred right-click orders remain separate. Regression
coverage checks selection loss, no automatic reselection on reveal, unchanged fog
rendering after unseen damage/death, inability to reselect hidden buildings, and
stale memory removal on revisit. Exact rubble sprites/decay and every building
type have not been compared in this pass.

Reference scenario saved as `x` in the Windows scenario editor (new file; only `default0` existed). Local validation logs: `retail-lost-selection-tests.log` and `retail-lost-selection-smoke.log` in the external desktop build directory.

### Rubble assets and exhausted farm cleanup (2026-11-10; superseded by clocked comparison below)

Live reference checks extended the saved scene with an Imperial farm and food
processing center. The farm's killed appearance is an exhausted field; the food
processor leaves smaller debris than the Gungan Airbase. A killed food processor's
debris was no longer drawn after its location entered fog. The farm field was
gone at a later observation. These are qualitative observations, not calibrated
simulation-time measurements. The reference game closed during this pass; a
subsequent screenshot showed the Windows disc in Explorer. The current executable
and data hashes were not bound to these captures. Its editor also showed an
extended unit list; do not label this run a verified untouched-retail comparison.

Local DAT inspection maps house/food processor to dead unit 144 (SLP 569), wall
to 143 (SLP 568), and Airbase to 146 (SLP 572). Farms use terrain instead of a
standing sprite. The dead-farm resource record contains 180, but the live check
does not establish that as a lifetime, so the existing decay timing is unchanged.
Exact timing remains open; the native fallback is currently 60 simulation seconds
after the death animation.

Fixed native exhausted-farm terrain persisting forever: when its remains expire,
restore the recorded ground. New fields and terrain edits are preserved. Reseeding
inherits the original ground beneath old exhausted terrain; later dead fields are
not erased by an older decay event. Existing fog terrain memory continues to hide
unseen ground changes. No replacement artwork or dependency was introduced.

Regression fixtures cover Empire and Gungan farms, food processors, houses, walls
and Airbases: original asset references, remains creation, pause, eventual decay,
original-ground restoration and rebuilding/repeated destruction. Farm terrain
requires a different assertion from building SLPs; a missing farm SLP is expected.
Local captures: rubble-civ-1.png and rubble-civ-2.png in the external build folder.
Research table: windows-reference/rubble-dat-2026-11-10.txt.

Validation: ASan/UBSan full suite passed, including overlapping dead-field timers, both complete-match outcomes, and the new rubble fixtures. Desktop menu/render/effects/music smoke passed. Logs: rubble-final-tests.log and rubble-smoke.log in the external build directory. Exact retail lifetime and all building/civilization combinations remain unverified.

### Clocked rubble comparison and correction (2026-11-10)

The earlier field-disappearance observation was insufficient: repeated captures
showed a field disappear and reappear. With the observer moved closer and the
original game clock enabled, the Imperial exhausted field remained through 02:57
and disappeared by 03:24, leaving dirt rather than restoring grass. The earlier
60-second farm cleanup and ground-restoration claim are superseded. Airbase
rubble was visible at 00:39 and gone by 01:07. These brackets support the matching
DAT dead-unit decay records (resource 12 / decay rate): 180/1 for farms, 60/1
for ordinary debris. Frame-exact transition timing is not established.

The restarted running process was identified by SSH: battlegrounds_x1.exe,
FileVersion/ProductVersion 1.1, SHA256
c01d78a039514211a845b41338c6f342e0d9c9b3396c8ff558f05d00ae67420d.
Windows Data/genie_x1.dat SHA256
9d2917c6c67e9df7af656459486f8e102406ad4a7299e7d0a85daac75c9c4b7d;
local native data SHA256
7446eaea7ca33340eafbed687f162604a92c3f083020721fb6effb1e2be6717b.
A local research copy of Windows data was retained without replacing either game
installation. The tested rubble records match. An extended editor unit list alone
does not establish that the running reference is modified.

Native building remains now use their dead-unit resource-12 lifetime when valid.
Expired farm terrain becomes Dirt1 (terrain 6), with replacement-field protection.
The dirt selection is a visual correspondence; a static executable lookup at
previously noted 0x554710 did not resolve in the historical local binary and is
not evidence for that terrain ID. REA was closed without changing the binary.
The disposable x profile's temporary clock hotkey was restored to defaults.

Follow-up Gungan runtime comparison: exhausted field still present at 02:29 and
replaced by swamp-water ground by 03:10, with the observer beside it. Thus the
cleanup is civilization-specific: Gungans terrain 4 (WATERSWAMP), others tested
(Empire) terrain 6 (DIRT1). The IDs are matched to installed terrain artwork;
static transition code remains unresolved. Native code and tests now retain the
180-second field, clear it to the appropriate terrain, preserve replacement live
farms and protect overlapping newer exhausted fields. This supersedes earlier
original-ground restoration for both civilizations. Screenshots retained locally
include gungan-farm-clock-before/after; Windows scenario xx retains the Gungan
fixture separately from original saved scenario x.

Final validation after clocked corrections: ASan/UBSan rubble/civilization and full-match suite passed (rubble-civilization-tests.log); desktop menu/render/effects/music smoke passed (rubble-civilization-smoke.log). No frame-exact transition or complete all-civilization claim.

## 2026-11-10 — Medic healing implementation

User report: medics accepted movement but never healed. The engine had no executor for DAT task action 105. Medics now accept right-click healing orders, approach to their effective range, restore HP without resource costs, and acquire nearby wounded friendly targets while idle. Other orders clear the healing target; invalid, hidden, hostile or fully healed targets stop the order. Pause freezes healing. Rendering uses the original attack/healing graphic and the original action sound on starting work.

Native DAT medic 939 has action 105 workValue1=2, workRate=1, maxRange=1, searchRadius=6, attackGraphic=6653 and attackSound=213. These are asset facts; the continuous rate formula and idle acquisition policy are native implementation choices pending a timed Windows comparison. Current eligibility covers worker, Jedi and organic infantry classes 58/50/51/52; class-52 Federation/Confederacy droids require repair. Unusual heroes and civilization-specific special units still require an eligibility audit. No new command icon or original cursor-parity claim is made.

Regression coverage in tools/healing_checks.h includes healing, health cap, pause, automatic acquisition, allied/enemy eligibility, mechanical exclusions and the actual right-click path with approach from outside healing/search range. Desktop smoke also exercises menu, rendering and audio pipelines; it does not establish that the medic sound was heard on physical output.

## 2026-11-10 — Original medic timing, controls and eligibility matrix

Windows captures `Research/windows-reference/medic/timing-{03,06,09}.png` show a wounded Gungan heavy trooper at 22/40 HP at 00:05, 32/40 at 00:10 and 40/40 at 00:15. This supports the native base rate of 2 HP per game second; startup delay and healing technologies remain unmeasured. The original medic automatically acquired the wounded unit.

The native classic HUD now uses the original Heal icon (frame 15) in slot 0, localized title/help, Y hotkey, a cancel-only targeting grid (slot 14), Escape/right-click cancellation, and Stop while working/moving. The medic no longer shows Patrol/Guard/Follow or attack stances. The shared combat predicate rejects healers, and medic class54 contributes no garrison fire. This addresses the user's noncombat requirement at the order/automatic-combat boundary rather than only hiding buttons.

The Windows matrix supports broader eligibility than the initial worker/Jedi/infantry-only implementation: bounty hunters, anti-air troopers, Royal Crusaders, Fambaa, scouts and Geonosians recovered with medics present. Anakin also recovered, but innate hero regeneration was not isolated. Droid infantry and power droids retained damage; an explicit droid healing order did not restore more HP. Eligibility now includes these additional organic/special classes. The retained findings distinguish observations from remaining inferences: worker/grenadier/second-medic/clone captures were interrupted by a return to the editor, and class49/55 droid exceptions and unusual heroes still need dedicated comparison. No claim of exhaustive unit/technology parity.

Validation after this pass: `medic-reference-tests.log` passed the full ASan/UBSan suite, including Heal button and Y-hotkey dispatch, live/paused Escape cancellation, base 2HP/s rate, expanded target classes, explicit attack/attack-ground rejection, AI completion and scripted player victory. `medic-reference-smoke.log` passed menu/render/effects/music checks. The native medic HUD render shows the original Heal icon with empty combat slots. Windows controls remain untouched pending the shared-input clarification; special-unit edge cases and healing technologies are still open.


## 2026-11-11 — Republic healing, controls and Bacta Tanks

Matched Windows scenarios distinguish target maximum health from civilization effects: the same Republic medic restores approximately 4 HP/game-second to both a 40-HP heavy trooper and a 175-HP Royal Crusader. Upgraded Med Droids increases the latter to approximately 4.8 HP/second. Clock/HP display rounding and acquisition delay remain; these are sustained rates, not claims about exact tick scheduling. Original DAT civ7 team effect528 sets resource89 to2; technology578/effect596 multiplies it by1.2. Other civilizations' raw zero is the neutral multiplier. Native healing now reads this resource, applies unique active mutual-allied civilization team effects before personal research, and retains the task's base2 HP/s. Production code uses the DAT effects, not hardcoded technology IDs. Allied/duplicate-team handling has native regression coverage but no dedicated Windows diplomacy test yet.

A no-medic control confirms retained damage in the Royal Crusader, Elite Geonosian, Jedi Knight, Fambaa, anti-air trooper, bounty hunter and scout. This strengthens their prior with-medic observations. Anakin instead recovers without medics, so his earlier recovery cannot establish medic eligibility. The unexplained extra2 HP on several shielded fixtures also occurs without medics; it is not evidence of a medic tick. Rare variants and hero healing remain open.

Original Command Center garrison healing measures approximately0.3 HP/second: a Royal Crusader damaged to75/175 was ejected around00:29 at83/175. With Bacta Tanks, the matched patient was ejected around00:25 at103/175. These agree with DAT garrisonHealRate0.3 and technology501/effect525 multiplying building attribute108 by4. Native garrisoned units now recover using their host's modified attribute108, capped at maximum health; pause and ejection stop recovery. Republic's medic bonus is not applied to buildings. Tests exercise actual entry, base and upgraded rates, pause, and ejection. Turret/transport and foreign-owner cases are not independently compared yet.

Jedi Meditation is not a health upgrade: the original tooltip28366 describes only one Jedi needing recovery after group conversion, and a wounded Knight with that technology remained100/200 through00:34 without medics. No speculative health-regeneration effect was added.

Fixtures, generators, screenshots and detailed findings remain local in `~/Library/Application Support/Galactic Genie/Research/windows-reference/medic/`. Installed Windows game files were not changed. Native test evidence: `medic-upgrade-tests.log` and `medic-upgrade-smoke.log` passed before the garrison addition; final validation is recorded below.

Final validation: `medic-bacta-tests.log` passed the ASan/UBSan suite, including the medic/team/upgrade and garrison tests, AI match completion and scripted player victory. `medic-bacta-smoke.log` passed original menu navigation, rendering, effects, streamed music and looping checks. `git diff --check` passed. The app bundle was rebuilt outside Documents; physical sound output was not re-auditioned. Windows reference remains at its in-game menu.


## 2026-11-11 — Packed combat upgrade decoding

The exact Windows1.1 executable (SHA c01d78a039514211a845b41338c6f342e0d9c9b3396c8ff558f05d00ae67420d) resolves the type5 attack/armour multiplier at0x489d20. It matches the packed high-byte class, uses the unsigned low-byte percentage with original0.01f, adds0.5 and truncates/stores the integer result after each command. The x87 intermediate matters:7×150×0.01f rounds to10, not11. Native previously multiplied every class by each complete packed value; that could produce enormous attack values. Attack and armour now filter the class and apply the decoded percentage with per-command rounding.

Windows confirms Light Turret79 displayed attack5→11 after Ion Accu-Accelerator, despite a tooltip saying+25%. Both installed DATs encode225 for general attacks and150 for naval attacks. A controlled turret-versus-cruiser815 fixture recorded144/160 HP at00:05 and136/160 at00:08 (one8-damage hit). Earlier ship fixtures were invalidated by other friendly combatants or immediate conquest defeat; only ion-ship-damage captures are used. Native regressions cover troop/house/naval targets, unaffected infantry, duplicate research and repeated modifiers. Complete REA evidence is retained locally in windows-reference/packed-combat-evidence-2026-11-11.json; pseudocode was not copied into engine source.


## 2026-11-11 — Original Clone Campaigns asset profile

The previous native `Game/Data/genie_x1.dat` (SHA7446eaea…) differed from Windows in7 effect vectors and56 selected gameplay-unit definitions per civilization. Inventory counts were identical; no claim is made that this was EF. The installed SWGB wrapper's original DAT matches Windows byte-for-byte (SHA9d2917c6…). A separate local `Game-CloneCampaigns` profile now preserves the old profile and copies the verified original Data files, including the changed game-data/sound archives and CRX lists. Shared graphics, interface and terrain archives already matched. The launcher and bare app now default to this original profile; `SWGB_GAME_DIR` still selects another explicit installation. A hash manifest and detailed differences are retained under Research/windows-reference.

`original-profile-recovery-tests.log` passed ASan/UBSan, original-data modifier/healing checks, AI completion and scripted player victory. `original-profile-recovery-smoke.log` passed menus, renderer, effects and music. This fixes the data baseline; it does not make the custom opening, map generator or AI identical to retail.


## 2026-11-11 — Original discrete hero recovery

Windows executable function0x55a760, branch0x55a8b0, checks any nonzero master heroMode,1<=HP<max, then accumulates simulation time and adds1HP for every2seconds. It caps HP and retains the remaining timer while fully healed. Constructor0x55a147 initializes the timer to zero. Constants and complete REA/Capstone evidence are retained in windows-reference/hero-recovery, tied to the exact Windows executable SHA. Native now implements this discrete timer rather than continuous0.5HP/s. Regression tests cover the2second boundary, pause, a nonhero special unit, HP cap, timer retention after full health and death. The full original-profile sanitizer suite passed.

The prior no-medic Anakin runtime control corroborates recovery, but the new timed capture was interrupted by QuickDesk timeouts. Exact cadence here is established by executable analysis. Garrison scheduling and the following resource49/96 regeneration branches are not yet claimed as complete.

## 2026-11-11 — Jedi Master and Self Regeneration branches

The remainder of Windows function 0x55a760 uses owner resource49 for Jedi Masters, or resource96 for other units with DAT trait bit0x20. Predicate0x54b1d0 identifies eight Master copy IDs and sixteen named Jedi hero IDs; DAT loading confirms the copyId and trait offsets. Original resource49 is2 seconds; Wookiee technology133 sets resource96 to3 seconds. Each elapsed interval restores one whole HP, independent of maximum health. The hero branch runs first and falls through, sharing its retained timer with the second eligible branch. Native code preserves this order, including the second time increment for named Jedi heroes. Masters never also enter Self Regeneration.

Evidence is retained locally under `Research/windows-reference/hero-recovery/additional-recovery-findings.txt` with the assembly, DAT inventory and exact executable hash. Tests cover ordinary Masters, named Jedi heroes, research gating, the three-second biological-unit interval, foreign biological units owned by the researching player, mechanical exclusion and Master precedence. The test measures from post-research health because the original effect also adds30 HP and armour to certain classes. These rates are executable-derived; dedicated Windows timing for the added branches and outer garrison update scheduling remain unverified.

Validation: `additional-recovery-tests.log` passed the full ASan/UBSan suite, including AI match completion and scripted player victory at677.7 simulated seconds. `additional-recovery-smoke.log` passed menus, rendering, effects, streamed music and EOF looping.

## 2026-11-11 — Civilization/team startup trace (implementation pending)

Exact Windows executable block0x601f90–0x6020f5 applies each non-Gaia player's civilization tree, then team effects in ascending civilization ID order. A civilization flag array deduplicates the recipient's own civilization and those of players they regard as allied. Predicate0x4c0450 checks recipient diplomacy only, not reciprocity. Thus the native healing-specific mutual-alliance calculation is not an exact model of original startup. Dynamic diplomacy changes, save/load behavior and the remaining setup gates are not resolved; no speculative generalization was shipped.

The full-technology-tree flag at global game+0x1439 gates this block: getter0x5ef150 feeds UI string9708, "Arbre tech. complet : %s". Another team gate at+0x1437 and a scenario setting are still unidentified. Derived constructor0x5cdcf0 separately invokes0x5c5dc0 for technology-cost/disable/time commands101–103; that call alone does not establish application of unit statistics. Assembly, exact source hash, original DAT effect inventory and REA evidence remain local under `Research/windows-reference/civilization-effects/` for the next implementation pass.

## 2026-11-11 — Civilization effects and original AI time

Native initialization now snapshots each active player's own tree effect and the distinct civilizations they regard as allied, including their own civilization. Team effects are ordered by civilization ID and precede researched effects. Unit-stat, attack, armour, resource, upgrade-lineage and availability consumers share that ordered effect list instead of independently walking research alone. The Republic healing special case is removed; its data effect now also reaches Jedi movement speed. Research rebuilds the list without recomputing startup diplomacy. Tests exercise all eight trees, enemy exclusion, one-way startup alliances, duplicate Republic teams, research ordering and restart isolation. Runtime diplomacy transitions beyond this startup snapshot remain a separate reference-comparison task.

Population capacity now includes resource4, so Trade Federation's original250 population provision is capped by the selected match limit rather than requiring extra housing. Training and construction read attribute101 through the same unit modifier path; a foundation test checks the Empire's original construction-time modifier reaches the actual timer. Other consumers still need work: costs and some worker/trading work rates read raw fields, and integer rounding for general attribute effects is not yet exhaustively matched. This is not a claim that every civilization advantage is complete.

The AI interpreter now implements `game-time` using a separate simulation clock, including scenarios. It counts whole simulation seconds, freezes on pause, survives AI reloads and resets on new games/scenarios. The original packaged age-advancement script (resource60001) uses this fact. Tests cover fractional thresholds, pause, late AI loading and both reset paths.

The exact Windows executable's filename/resource extraction calls map56 packaged AI scripts. They were extracted unchanged to local `Research/windows-reference/ai-packaged/original-ai`, with source and per-file hashes. The existing parser successfully loaded `randomgame.per` at Moderate difficulty:37 included files,158 constants,714 rules,2923 conditions and1348 actions. Parsing does not establish execution of all those facts/actions, and the desktop still uses the custom skirmish controller.

Original map evidence is also retained there: resource54000 (`random_map.def`),54103 (`GBG_land_resources.inc`) and25 explicit `#include_drs` mappings. The original Shoreline script uses seven generation stages plus conditional/random branches. The native FBM/custom map has not been replaced. A DRS-aware RMS preprocessor and the corresponding placement/generation algorithms are still required; unsupported directives must not silently produce a misleading "original" map.

The user-supplied SWGB wiki is a secondary lookup lead (https://swgb.fandom.com/wiki/Star_Wars_Galactic_Battlegrounds_Wikia); direct retrieval was blocked. Implementation evidence above comes from local original assets and executable traces, not EF wiki balance values. The local x1 archive matches the native original profile but has not yet been independently hashed on the Windows host.

Validation: `bonuses-final-tests.log` passed the full ASan/UBSan suite, including all eight civilization openings, actual foundation timing, AI game-time, AI victory and scripted player victory at677.7 seconds. `bonuses-final-smoke.log` passed launcher/menu, renderer, effects, streamed music and EOF looping. `original-ai-parse.log` records the packaged-script parser check. Dedicated new Windows runtime comparisons were not performed in this pass.

## 2026-11-11 — Original unit costs, integer timing and AI timers

Exact Windows modifier functions0x5c8cb0/0x5c8dc0/0x5c8ee0 establish cost attribute100 (all enabled slots),103–106 (food/carbon/nova/ore), and training time101. Set truncates toward zero; add truncates its delta first; multiply truncates `old * float-factor + 0.5` using extended intermediates. Each command writes a signed16-bit result. Only the low byte of a cost slot's flag enables it. Local assembly, source hash and loader mapping are retained in `Research/windows-reference/unit-cost-modifiers/`.

Native unit costs now flow through one shared helper used by purchases, building/wall placement, farm reseeding, AI affordability/payment, repair costs, current-cost refunds, tooltips and both command-menu cost displays. Training/construction durations now use the same verified per-command integer arithmetic. Queue refund helper0x5d0d80 reads current unit master costs, so native cancellation deliberately uses current modified costs rather than a historical payment snapshot. Technology research costs remain a separate unfinished path; malformed nonfinite modifiers are ignored safely rather than emulating undefined original conversion behavior.

Original timer functions0x4181d0/0x4181f0/0x418230 confirm20 per-player timers, deadline overwrite on enable, disable by sentinel, and a non-consuming expiry predicate. Native enable/disable/timer-triggered support these rules with the simulation clock. Original packaged scripts use this pattern for age advancement and periodic economy decisions. Invalid IDs and negative durations are rejected. The native double-second clock does not emulate32-bit millisecond wrap, and timer persistence across an AI script reload remains unverified.

## 2026-11-11 — RMS archive loading boundary

`RmsScript` expands original `#include_drs` resources through the existing archive reader, tokenizes comments/braces/stage markers, preserves resource/line provenance and validates structural nesting. It bounds depth, total bytes and expanded tokens, rejects recursive/missing/invalid includes and unknown preprocessor directives, and retains constants/conditions/random choices as unresolved tokens. `swgbtool rms-script <archive.drs> <resourceId>` explicitly reports loading only. The original Shoreline54205 fixture expands to5,923 tokens across seven stages using54000/54103 includes. This is not a generation plan: constant evaluation, conditional/random evaluation and all terrain/object placement algorithms are still pending.

Validation for this pass: `cost-timer-tests.log` passed full ASan/UBSan, actual discounted AI payment and current-cost refund checks, timer lifecycle checks, eight civilization openings, AI match completion and scripted player victory. `cost-timer-smoke.log` passed menus, rendering, effects, streamed music and EOF looping. The RMS standalone suite passed ASan/UBSan with strict compiler warnings against original Shoreline and malformed inputs. No Windows runtime interaction was needed for these executable-derived arithmetic changes.

## 2026-11-11 — Effect acquisition order and building work rate

Researched effects now retain acquisition order, including automatically granted technologies, while the existing set still handles prerequisites and duplicate research. Rebuilding modifiers no longer silently sorts by numeric technology ID. A regression acquires an additive effect before a lower-ID multiplier and verifies120 rather than110, then verifies repeated research does not apply twice.

The original normal research/production action paths consume `dt * building.master.workRate`. Native queue progress now reads modified attribute13 instead of ignoring that field. A paid production fixture verifies the modified rate reaches the real queue clock. Original difficulty-specific modifiers, changing a queued item's duration after it was queued, and all research cost/time effects remain separate pending work.

Further exact evidence is retained under `Research/windows-reference/research-modifiers/`: effects101/103 overwrite per-player cost/time deltas; research resources85/86 scale cost/time; Bothan Network62 has a separate price using capped active-opponent resource37, classes12/14/45/58, mutual-alliance exclusion and resource197. Its refund omits those special factors. Those research calculations have been traced but are not yet implemented; dying-object lifetime and nonzero player-status cases still need native representation work.

Validation: `research-order-workrate-tests.log` passed the full ASan/UBSan suite and both completed-match outcomes; `research-order-workrate-smoke.log` passed the desktop menu/render/audio checks. The rebuilt native app includes these changes. The RMS inspection command also passed against54205 and rejected a malformed resource ID (`rms-tool-check.log`).

## 2026-11-12 — Research cost and duration modifiers

Exact Windows functions0x5c0790/0x5c085f overwrite signed-short per-player deltas for effect types101/103. The native helpers now reproduce first-enabled matching cost slots, low-byte mode selection, truncation, original-base-relative replacement, resource85 cost scaling (except technologies0–3), and resource86 duration scaling before the time delta. Unscaled base-plus-cost-delta remains wider than16 bits; positive scaling narrows after truncation. Evidence and executable identity remain under local `Research/windows-reference/research-modifiers/`.

Human and AI affordability/payment, cancellation refunds, original help text and both cost displays use these helpers. The original negative-cost transaction behavior is not reproduced: existing payment/refund consumers still ignore nonpositive amounts. Bothan Network62's population-dependent purchase multiplier remains pending player-status/lifetime accounting; this pass does not claim its price matches. Queue durations remain snapshots at enqueue; dynamic thresholds were removed after the accelerated test-queue fixture exposed an integration conflict. The original progress-percent denominator's resource86 discrepancy is also not emulated.

QuickDesk was observed disconnected, with no remote connections. It remained disconnected during this pass to avoid remote audio; no persistent mute preference was changed and system audio was untouched.


Validation: `research-modifiers-tests.log` passed full ASan/UBSan, research arithmetic/current-cost refund checks, eight civilization openings, completed AI matches and scripted player victory at677.7 seconds. `research-modifiers-smoke.log` passed launcher/menu/rendering and silent effects/music-backend checks. Native app rebuilt.


## 2026-11-12 — RMS constant and conditional preprocessing

The RMS loader now offers a separate, immutable preprocessing pass for signed 32-bit integer `#const` definitions, `#define`/`#undefine`, and nested defined-symbol `if`/`elseif`/`else`/`endif`. Callers supply match-defined symbols. Selected directives retain argument/braces order and original use-site resource/line locations. Inactive definitions do not leak into later branches, and failed preprocessing returns no partial output. Processing is bounded by the loader's byte/token limits and does not expand tokens recursively.

Evidence comes from the original packaged `random_map.def` (resource54000, 808 integer constants) and `GBG_land_resources.inc` (54103). Tests load those original constants and exercise the include's `MIGRATION` placement-radius branch (14 versus18), `BLDG-MAIN1`109, `UNIT-WORKER`83 and `GRASS1`0. These tests establish preprocessing, not original map generation or placement correctness. Original assets remain outside the repository.

Active random blocks explicitly fail; unreachable random blocks are skipped without applying definitions. Constant-valued conditions, conflicting definitions and noninteger constant expressions are rejected rather than assigned unverified semantics. Include expansion still occurs during loading, before branch evaluation. Shoreline54205 remains fully inspectable (5,923 tokens, seven stages), but preprocessing refuses its active random choices. No RNG, terrain algorithm, object placement or desktop map/controller integration was added.

Validation: the standalone `tools/rms_test.cpp` suite passed strict compiler warnings plus ASan/UBSan against the original x1 DRS, including malformed/overflow constants, missing operands, nested inactive branches, repeated preprocessing, source provenance and existing include/path/depth/token-limit checks. Exact command and output are retained locally in `Research/windows-reference/rms-preprocess-tests-2026-11-12.log`. No Windows runtime comparison was performed for this pass.

## 2026-11-12 — Original AI condition coverage

The Moderate original `randomgame.per` parser audit enumerated714 rules from37 files:48 distinct unsupported fact names and19 action names before this pass. Counts describe parsed presence, not execution or original-game parity. The full inventory and source locations remain local in `Research/windows-reference/ai-packaged/interpreter-coverage-2026-11-12.txt`.

Added `nand` using the existing three-valued boolean evaluator. The original `spaceport.per` uses it11 times, including personality/age-time conjunctions. Regression coverage exercises its truth table, nested conditions and unknown propagation: an unsupported operand alone cannot become true merely through negation, while a known false operand determines a NAND result. Other missing predicates still prevent those original rules from running fully.

Highest remaining controller blockers are research/escrow gates (139 `can-research` and38 `can-research-with-escrow` occurrences), civilization/population queries, military groups and strategic-number consumers. `release-escrow` remains a no-op; unsupported actions can still be followed by `disable-self`, so loading the original controller into normal matches is not yet appropriate. The desktop retains its custom AI.

Exact predicate traces0x57b680/0x57b6e0 require research status1, effective-cost affordability0x5c052f and an idle completed exact-ID researcher via0x587550→0x596680. Training/research actions and queued production exclude a building. Normal affordability subtracts reservations and escrow; the with-escrow form subtracts reservations only. Two original building fields remain semantically unnamed. Raw assembly and findings are local in `Research/windows-reference/ai-can-research/`; these predicates have not yet been implemented.

Validation: `ai-nand-tests.log` passed the full ASan/UBSan suite, including the new boolean checks and completed AI/player matches. `ai-nand-smoke.log` passed the rebuilt native app's menu/render/audio-backend smoke checks using silent drivers. No live Windows interaction or physical sound check was performed.

## 2026-11-12 — Original AI research gates and escrow accounting

The native interpreter now implements `can-research` and `can-research-with-escrow`, sharing availability, affordability and an idle exact-location researcher with the `research` action. Completed/disabled/civilization-incompatible/in-progress technologies fail availability; prerequisite checks remain shared with the technology engine. A completed building with queued training or research cannot qualify. Research symbols now support script-defined numeric aliases while preserving the different age-fact and research-ID numbering. The original building action-controller pointer has no separate native representation; original pending-command reservations are unnecessary while native orders debit synchronously.

Escrow amount/rate now lives per player outside reloadable script state. `set-escrow-percentage` clamps0–100 and affects future income only; `release-escrow` clears savings without changing the rate or stock. Normal AI unit/research affordability subtracts escrow; with-escrow predicates include it, but the actual research action still needs a preceding release. Worker deposits and scenario tributes pass through resource-add accounting; paid unit/research refunds remain direct stock changes and do not count as income. The shared add path caps escrow against remaining stock after additions/removals. Nursery food also uses this native shared accounting path, although its original dedicated call site has not been traced independently. New game/scenario initialization resets amount and rate, while loading an AI script preserves both.

Static sources: exact Windows executable c01d78…; predicate/action traces0x57b680/0x57b6e0/0x577010, researcher0x587550→0x596680, escrow setters0x4c44d0/0x4c4530 and resource addition0x4c1830. Local assembly/findings: `Research/windows-reference/ai-can-research/` and `ai-escrow/`. Original resource-amount facts compare total stock, so those facts deliberately do not subtract escrow. No deferred order queue, original military controller or multiplayer lockstep was introduced.

Validation: `ai-research-escrow-tests.log` passed full ASan/UBSan, exact-age-token and numeric-alias resolution, busy/unfinished/in-progress/completed/civilization/prerequisite gates, unaffordable/escrow affordability, original age-up release/payment sequence, refund accounting, percentage bounds, script reload, player isolation and new-game reset. Both AI and scripted player matches completed. `ai-research-escrow-smoke.log` passed the rebuilt native app's menu/render/audio-backend checks.


## 2026-11-12 — Original RMS random-choice evaluation

REA tracing of the exact Windows executable found the original shared CRT generator at `0x632bdd`: 32-bit state advances by `state * 214013 + 2531011`, and the return value is `(state >> 16) & 32767`. RMS dispatcher `0x4e1fa0` scales that to an inclusive0–100 roll using integer `rand * 100 / 32767`; `percent_chance` selects at equality and subtracts preceding weights. Consequently a zero-weight branch can win at roll0, and an underweight block may select nothing. Registration `0x4e0ab0` and original language strings8507–8509 establish the command names. The same dispatch proves zero-valued constants count as existing labels in conditions and `#undefine` is a no-op in this executable; preprocessing now follows those observations.

An optional explicit RNG state enables this nonnested random evaluation. State commits only on successful preprocessing; inactive conditional blocks consume no draws. Nested random blocks remain rejected because their interactions need separate fixtures. Inputs are shared CRT state at script entry, not a claim that a map seed alone reproduces a map: other generation consumers use that global, and the complete generator call path is not recovered. Include expansion remains eager and generation directives remain uninterpreted. No terrain or object placement was implemented.

Inspection command: `swgbtool rms-script <archive.drs> 54205 --rng-state 1 --define TINY_MAP --dump`. Without options it preserves raw inspection. Original Shoreline yields767 directive tokens and final RNG3673273548 from initial state1. Standalone ASan/UBSan tests cover endpoint/equality/zero-weight behavior, state consumption, rollback and repeatability. A six-state corpus sweep passes44 of46 packaged RMS resources (264 successful evaluations). Resource55012 has a bare `elseif` at191;55014 retains an unclosed `if TINY_MAP` from158 through EOF. Both stay rejected; no unsupported semantics were relaxed to make the sweep pass.

Local provenance, full REA evidence and corpus results are `Research/windows-reference/rms-random-findings-2026-11-12.txt`, `rms-random-evidence-2026-11-12.json` and `rms-corpus-2026-11-12.tsv`. This is executable-derived preprocessing with native tests, not a Windows runtime map-generation comparison.

Final integration: `rms-random-cli-check.log` confirms raw inspection, deterministic Shoreline evaluation and rejection of invalid/overflow RNG states and incomplete option combinations. `rms-random-smoke.log` passes the rebuilt desktop app's menu/render/audio-backend checks. Original game assets and reverse-engineering evidence remain local and uncommitted.

## 2026-11-12 — Original AI population and strategic-number queries

`civilian-population` and `military-population` now use the class predicates traced from original resources37/40. Civilian classes are12/14/45/58; military classes follow the distinct0x54b4e0 switch. The creatable lifecycle (types70/80) updates those original counters without visibility, garrison or HP filtering. Native counts include garrisoned objects and exclude pending production and noncreatable decorations. Native inactive/dead objects stop counting earlier than the original destruction callback; that lifetime difference remains. `soldier-count` is a different AI list and is not aliased to military population.

Extracted138 strategic-number name/index registrations from the exact executable. Setters, numeric IDs, script aliases, queries and native gatherer allocation now share indexed storage, range0..241. Queries for uninitialized values remain Unknown because original defaults are not yet established; accepting a setting still does not implement every downstream group/strategy behavior. Invalid or unresolved indices remain rejected rather than reproducing the original invalid-index query result−1. `civ-selected` compares the player's own civilization ID for numeric/script-defined values; the built-in civilization-name registrations remain unproven, so they have not been guessed. Resource-amount facts now truncate toward zero, matching original total-stock queries even for fractional negative balances.

Evidence: exact Windows SHA c01d78…; facts0x57bea0/0x57caf0, classifiers0x54b550/0x54b4e0, civilization0x57be80, SN lookup0x57e2c0→0x5a5710 and registration table starting0x57fc8c. Local files are `Research/windows-reference/ai-controller-facts/{findings.txt,assembly.txt,sn-registrations.json}`. No live Windows comparison or activation of the complete original AI controller occurred in this pass.

## 2026-11-12 — First RMS terrain stage and object-placement trace

`generateRmsBaseTerrain` now produces the actual row-major base-terrain bytes from preprocessed LAND_GENERATION/base_terrain directives and DAT terrain metadata. Original handler0x4e2c40 retains the low byte and the last assignment; constructor0x4da320 passes it to runner0x4da380, which paints the full inclusive rectangle via0x495f80. Disabled terrain may use an enabled drawing alias for validation while retaining the original terrain ID in the map. Validation completes before output is produced; unsupported directives clear output and fail. The1..1024 dimension bound is a native safety limit, not an original-engine limit.

This is a strict base-fill stage, not a complete map generator. Player lands, land growth, elevations, objects and renderer edge updates remain unsupported; full Shoreline explicitly rejects rather than silently flattening. The playable desktop retains its existing custom map generation. Standalone strict-warning ASan/UBSan tests passed rectangular edges,1x1/max bounds, narrowing/overwrite behavior, terrain aliases, invalid metadata and commands, and the original WATER_SHALLOW constant.

Object handler0x4e3850 and consumers0x4ddd40→0x4dddf0 were traced separately. The60-byte row defaults and18 command mappings, including order-sensitive group counts, are retained locally. Unrestricted placement0x4de570 constructs the map-wide candidate list via0x4dded0 before prototype checks: row-major insertion then repeated two-draw move-to-front operations. Candidate popping consumes no RNG. Prototype virtual+0x38 must return zero before creation via player virtual+0xb4; its dynamic implementation remains unresolved. No object-placement code was added from an assumed collision rule.

Local provenance and machine-byte evidence: `rms-base-terrain-findings-2026-11-12.txt`, `rms-base-terrain-byte-provenance.json`, `rms-object-findings-2026-11-12.txt` and `rms-object-static-evidence-2026-11-12.json` under `Research/windows-reference/`. REA sessions were closed; no system configuration or original game files were changed.

Validation for this pass: `ai-controller-facts-tests.log` passed the complete ASan/UBSan suite, including fishing/trader/medic classifications, noncreatable and building exclusions, garrison/queue counters, own-civilization comparisons, strategic-number name/index/alias sharing, per-player isolation, unknown-default propagation, and negative fractional resource facts. Existing eight-civilization openings and completed AI/player matches passed. `ai-terrain-smoke.log` passed the rebuilt app's menu/render/audio-backend checks. No full original map or original military controller was claimed or activated.

## 2026-11-12 — Original PER startup and controller ownership

Fresh original AI controller construction sets its context to−1; the first tick applies profile events before evaluating PER rules. The verified live values for queried strategic numbers are0=34,2=66,4=2,42=0,74=20,118=0,120=0,143=1,226=0. These now initialize native AI state. Other242-slot defaults remain unverified rather than copied wholesale from the extracted profile template. Exact difficulty registrations add hardest0,hard1,moderate2,easy3,easiest4 to both parser entry points; scripts previously could not resolve those named operands.

The original counted `or-list` evaluator0x40fb40 reads its operand count, recursively evaluates operands and short-circuits on true. Native evaluation now supports that syntax, including empty OR=false, while preserving Unknown for unimplemented facts. Native rejects negative/oversized/mismatched counts rather than copying malformed original bytecode behavior. Explicitly loading an AI script (even an empty one) suppresses fallback skirmish strategy orders, while match timing/outcomes still run; a new skirmish restores the fallback controller. This avoids two competing controllers when testing the original script.

Local evidence: `ai-controller-facts/sn-defaults-{findings,assembly}.txt` and `counted-boolean-{findings,assembly}.txt`. The original714-rule Moderate PER was also executed for120 simulated seconds in a native sandbox diagnostic, before and after startup fixes. It assigned90%food/10%carbon and spent resources on buildings; workers remained5 and the sampled trajectories were unchanged. This is execution evidence, not an original map/opening comparison or proof of complete AI. Missing dynamic operations included generate-random-number, resource-found, opponent/diplomacy facts and commodity queries. Source and before/after logs remain local under `ai-controller-facts/native-original-ai-*`.

Validation: `ai-startup-tests.log` passed full ASan/UBSan, verified default initialization/difficulty comparisons, counted-OR truth/arity/unknown checks, script/fallback isolation, normal fallback restoration and completed AI/player matches.

## 2026-11-12 — Original AI random-number operation

Implemented `generate-random-number` and `random-number` from action0x576f90 and fact0x57ddd0. Generation advances the original32-bit LCG and stores `draw % limit + 1` per player; querying never advances the stream. Stored values start at−1, survive script reload and reset with a new game/scenario. Signed negative divisors retain the original remainder behavior; zero is safely rejected before consuming a draw instead of triggering the original divide exception. Limits above32768 preserve the original15-bit draw ceiling/modulo bias.

Important boundary: native AI currently has a legacy RNG stream shared between AI players, initialized from the native match seed (scenario fallback1). The original shared CRT stream also includes map, object creation and world consumers that are not fully ported. Therefore this does not establish original seed-to-match parity or multiplayer lockstep. Native save/load persistence of this additional state is not implemented. Exact findings/assembly are in `ai-controller-facts/random-number-{findings,assembly}.txt`.

`ai-random-tests.log` passed full ASan/UBSan and completed matches. Fixtures verify seed1 draws41/18467/6334, modulo results, initial−1, shared draw order/per-player stored values, reload/query behavior, zero rejection and reset. A second120-second original-PER sandbox probe no longer reports the generation action as unsupported; sampled worker/resource totals remain unchanged. The local probe is not an original-opening or live Windows comparison.

## 2026-11-12 — Explicit RMS land seeds and concrete tree placement boundary

`generateRmsLandSeedTerrain` extends the tested base fill with explicitly positioned, nonplayer `create_land` seed rectangles. Original handler0x4e2c40 converts percentage positions by truncating width*x/100 and height*y/100. Stage0x4da6f0 paints inclusive center±base_size rectangles clipped to the map before checking growth budgets; later seeds overwrite earlier overlap. The native subset requires explicit terrain, position0..99, radius0..1024 and number_of_tiles0, with at most64 lands. These are implementation bounds, not original input-limit claims. Unsupported growth, implicit/random positions, player lands and other directives fail with no output.

The result is explicitly intermediate: original frontier growth and final smoother0x492ab0 have not been implemented, so even zero-budget seeds are not presented as finished original maps. Standalone strict-warning ASan/UBSan tests pass percentage truncation, edge clipping, overlap order, malformed/unsupported inputs and base-fill regressions. Evidence and exact machine-byte ranges are retained in `rms-land-seed-findings-2026-11-12.txt` and `rms-land-seed-byte-provenance.json`.

Further object evidence resolved actual Gaia timber348 (type10) prototype placement to0x48e180 and creation via0x5cfad0. On an empty map, its tile-centered candidate has a one-tile clearance footprint and requires restriction11>float32(0.05), plus bounds. The placement check consumes no RNG on this bounded path. Creation includes a proven random facet draw (graphic mask0x04,12 angles), but all transitive creation RNG effects remain unproven. Those findings are local in `rms-object348-gate-findings-2026-11-12.txt` and its evidence JSON; object spawning remains disabled rather than pretending the whole creation path is known.

Final integration: `ai-land-seeds-smoke.log` passed the rebuilt desktop app's launcher/menu, rendering, effects/music-backend and looping checks. The playable app still uses the custom map and fallback AI unless a script is explicitly loaded through the engine API. Original assets and all raw reverse-engineering/probe files remain local and uncommitted.


## 2026-11-12 — Original terrain smoother, object order and military clustering

Implemented the terrain-byte portion of original full-map smoother0x492ab0: two row-major in-place passes repeated to convergence. Verification used isolated original machine instructions with the tile setter reduced to a terrain-byte write:512 neighborhood/phase decisions,256 complete3x3 maps (FNV corpus2477551712), and a7x5 fixture. Native tests cover those map results, idempotence, map boundaries, preserved terrain IDs, aliasing and bounded-work failure. This helper remains separate from pre-growth seeds: it does not implement land growth or tile-renderer side effects.

`generateRmsObjectCandidates` implements the initial all-eligible full-map list from0x4dded0/0x4d6ab0. Candidates begin row-major; each random pair moves an existing node to the front, without duplication. Fixtures independently derived from original instructions cover repeated nodes, the32767 endpoint, exact RNG state, narrow maps with no draws and a maximum-size permutation. This produces candidate order only; collision checks, object spawning and subsequent eligibility changes remain separate. Original CRT arithmetic is now shared by RMS, candidate ordering and AI without merging their currently separate stream ownership.

`selectAiMilitaryGroups` implements the bounded selection portion of0x4506f0 and caller truncation at0x5ae726. It examines at most the first40 eligible, ungrouped candidates, scores centers using each center's movement-region map, picks the first maximum, then recruits using each neighbor's own region. Those region tests deliberately differ. It retains the original termination comparison against the full input count, even above40. Inputs require real region samples and original candidate order; no fabricated connectivity is supplied. Tests cover density ties, inclusive3-4-5 radius, asymmetric regions, truncation, the40-candidate quirk and invalid inputs. Apple Silicon long-double arithmetic may differ from x87 at exceptional distance boundaries.

The helper is not wired to `attack-now`: the original caller initially stops/cancels group tasks, while subsequent target selection and group scheduling remain unimplemented. No attack-all substitute was introduced. The desktop still uses its custom map and fallback AI; these tested components do not establish complete original match behavior.

Local evidence: `rms-smoothing-*2026-11-12*`, `rms-object348-creation-{findings,evidence}-2026-11-12.*`, and `ai-controller-facts/attack-group-{findings,assembly}.txt` under Research/windows-reference. The concrete first-timber348 creation trace now supports exactly one facet draw under fresh-empty-world/ordinary-Gaia/known-callback conditions. This is static evidence, not Windows runtime validation or a generic object-spawning implementation.

Validation: `rms-military-algorithms.log` passes strict-warning ASan/UBSan builds and all three standalone suites. CMake exposes optional asset-independent algorithm tests; CMake itself was unavailable on this host, so its new test targets were not executed here.

Integration: `rms-military-tests.log` passes the full ASan/UBSan skirmish suite, including completed AI/player matches. `rms-military-smoke.log` passes the rebuilt native menu/render/effects/music-backend checks. These regression runs exercise the existing playable path, not the newly recovered original map/military pipeline.


## 2026-11-12 — Executable-backed single-land growth

`growRmsSingleLand` now executes a complete bounded subset of original function 0x4da6f0, before final smoothing: one explicitly positioned land, zero separation and zero edge rejection. It retains seed-perimeter order, per-tile queue membership, newest-first priority ties, queued-node reprioritization, comparison draws even with a zero rejection gate, left/right/up/down neighbor draws and final frontier gap filling. Zone ownership stays separate from terrain values. Final filling can paint extra terrain without incrementing the target budget, as in the original.

The resolved-parameter API deliberately does not extend the existing RMS seed parser into an incomplete full generator. Native safety bounds are dimensions 1..1024, radius 0..1024, target at most map area, and priority coefficient 0..100. The original parser maps `clumping_factor` directly to land-row +0x1c, with default 8; the API uses that name and default. Original input is signed and unclamped; the supported native subset deliberately rejects values outside 0..100. A 64M work budget includes queue insertion traversal; failures clear output and leave caller RNG unchanged.

Validation uses 62 distinct fixtures from complete original-function execution under isolated Unicorn, intercepting only the rectangle terrain painter. Native results match terrain hashes and final RNG across 7x7, rectangular, one-cell-wide and singleton maps; interior and corner seeds; radii 0/1; zero, partial and full-map targets; coefficients 0/10/100; two initial RNG states; and identical base/land terrain. Local corpus also retains original zone bytes. `script/test_algorithms.py` passes all three strict-warning ASan/UBSan suites; original packaged Shoreline loading checks pass. The runner makes these asset-independent checks reproducible without CMake or game files.

Evidence: `rms-land-clumping-mapping-{findings,evidence}-2026-11-12.*`, `rms-growth-implementation-2026-11-12.txt`, `rms-growth-small-corpus-2026-11-12.json`, and the corresponding local probe/model. Multi-land competition, nonzero separation/edge rejection, player starts, full RMS plan execution and desktop integration remain unfinished. This adds actual terrain growth at the engine API boundary, not a new playable original random-map mode.

Further military tracing is retained in `ai-controller-facts/attack-scheduler-{findings,assembly}.txt` and `attack-scheduler-byte-ranges.json`. Original readiness and processing are separate resumable passes. Target planning reads remembered-object records and uses additional eligibility/scoring rules; exact scoring, cadence and state transitions remain unresolved, so native `attack-now` still does not use an approximate nearest-enemy replacement.

Final checks: `rms-growth-final-algorithms.log` records the complete algorithm suite after naming the verified clumping field and checking its original default 8. `rms-growth-smoke.log` passes the native app menu/render/audio-backend regression. No new live Windows gameplay comparison was performed; the growth comparisons execute isolated original instructions.


## 2026-11-13 — Competing original land growth

`growRmsLands` extends the resolved-parameter stage to 1–64 distinct land zones. It preserves source-order seed overwrites, shared per-tile frontier ownership, queue transfers between lands, newest-first priority ties, one-pop-per-land growth rounds and source-order final gap filling. The single-land API delegates to the same implementation. Work limits now include initial seed painting as well as queue traversal, avoiding unbounded repeated painting of large overlapping seeds.

Thirty-one cases executed the original 0x4da6f0 instructions and matched terrain, zone bytes and final RNG against an independent model. Native fixtures check terrain and final RNG for reversed ordering, overlapping/touching/identical seeds, unequal and exhausted budgets, identical terrain IDs across distinct zones, three-way competition, narrow maps and the 64-land boundary. All previous single-land fixtures and sanitizer algorithm suites pass.

Still unsupported: multiple rows sharing one land zone, nonzero separation or edge rejection, automatic/player coordinates, complete RMS interpretation and desktop random-map selection. The API takes already-resolved land rows; this is not yet an original random-map setup mode. Evidence remains local in `rms-multiland-{findings,corpus,probe,model}-2026-11-13.*` under Research/windows-reference.


## 2026-11-13 — Original timber placement and native spawning

`generateRmsTimberPlacements` executes one default ungrouped unit-348 object row on an empty flat map. It validates the supported Gaia prototype/graphic/resource fields, uses the original candidate order, accepts restriction-11 values strictly above float32(0.05), and consumes one facet draw per successful placement. Rejected terrain consumes no facet draw. A zero object count still builds/shuffles the candidate list. Distinct tile centers may touch: original strict footprint overlap permits all adjacent timber objects. Native object-count safety bound is map area; other units, existing objects, grouping, spacing modifiers and terrain-to-place-on overrides remain unsupported.

`Game::populateRmsTimber` materializes these placements as real native Gaia objects on an empty, zero-elevation initialized map. It uses the ordinary object setup without the unrelated sandbox spawn RNG, preserves carbon/HP, converts the original facet to native renderer facing and rebuilds collision/adjacency data. Unsupported world state fails before modifying objects or caller RNG. This is an engine map-building API; the desktop skirmish generator has not switched to this partial RMS pipeline.

Original instructions verified nine placement-gate boundary fixtures, all 30 sequential centers on a 5x6 grid and 30 facet draws. Seed 1 produces first centers (1.5,1.5), (0.5,3.5), (0.5,2.5), facets 4/10/9 and final RNG 1766988168. Zero count or entirely rejected terrain ends at 1030492215. Full transitive world creation is statically audited under the stated prototype/world constraints; it was not emulated or compared in a live Windows session.

Focused sanitizer tests cover ordering, facets including the 32767 clamp, terrain threshold, count zero, exhausted candidates, invalid metadata and RNG rollback. The installed-DAT test creates 36 native objects, checks exact positions, resources, HP and projected facets, and rejects nonempty/elevated worlds. `multiland-spawning-targets-tests.log` passes the complete ASan/UBSan suite and completed AI/player matches. Local provenance is `rms-object348-repeat-*2026-11-13` under Research/windows-reference.


## 2026-11-13 — Original military target scoring and scheduler boundary

`rankAiTargets` reproduces score/order stages 0x592980 and 0x592c30 from already-resolved candidate metrics and classification/region flags. It retains original component normalization, the second component's surprising reuse of metric A, class-6 score capping, priority overrides, reachability/class buckets, original-order score ties and omission of nonpreferred walls below score 100. Invalid/nonfinite inputs and the original zero-divide/NaN case fail explicitly. The 1024-record bound follows scratch capacity; native ordering does not reproduce the original partially cleared visited-buffer hazard, so large/reused original scratch-state parity is unclaimed. Apple Silicon near-boundary x87 score precision also remains unverified.

Five isolated original-instruction fixtures corroborate the normal scorer and complete bucket ordering; special priority overrides are statically traced and covered by native fixtures, not the world-predicate oracle. All four algorithm suites pass with strict warnings and ASan/UBSan. CMake and the direct macOS test script include the new suite; CMake is still unavailable on this host, so the direct compiler path was used.

This is not target discovery or a running military controller. Candidate eligibility, damage/vulnerability calculation, priority-condition resolution and group commands remain upstream/downstream requirements. Further original-byte tracing shows tactical 0x59f030 is a resumable 24-phase machine. Readiness/planning use separate phases; shared work budgets come from WINMM host milliseconds and incomplete passes keep their phase and group cursor. A guessed fixed attack interval would not match this mechanism. World invocation/pause gates, gathering, group maintenance, opponent/target-memory preparation and command completion/replanning remain to integrate.

Local evidence: `ai-controller-facts/target-score-*` and `scheduler-cadence-*`, including exact bytes, executable digest and the original-code oracle. No live Windows comparison, system configuration changes or remote publishing occurred in this pass.


## 2026-11-13 — Preserve original land-zone budget accounting

Explicit zone bytes are now supported by `RmsLandGrowth.zone` in the native range 0..63; -1 preserves the previous ordinal-zone API behavior. Original seed setup resets the area counter by land ordinal, then writes seed area by zone byte, while growth reads the ordinal counter again. Shared or shifted zones therefore change growth budgets. Normalizing `zone 0` (stored as byte10) into native zone0 was experimentally incorrect and is no longer assumed.

Thirty-one additional cases executed the original growth instructions for shifted zones10/11, shared0/63, reversed zones and three-way cycles. Native terrain and RNG results match. Existing ordinal/single-land fixtures still pass. Seed/work-budget failures remain transactional. Local evidence is in `rms-pipeline-findings-2026-11-13.txt` and the expanded original-stage probes/corpus; all algorithm sanitizer suites pass.


## 2026-11-13 — Integrated strict RMS pipeline and native diagnostic renderer

`generateRmsMap` now connects preprocessing, original setup RNG, land growth/smoothing, the automatically enabled empty terrain stage and default timber placement on one shared CRT stream. The original finalizer generates eight unique player-order slots by rejection sampling and then consumes an offset draw even without player lands. The module scheduler consumes a draw before its first stage and after each of its three successful stages. These non-obvious consumers are retained; omitting them changed generated maps.

The supported script boundary requires explicit base/land terrain 0 or 3 with no aliases/automatic terrain units, explicit nonplayer positions, radii and tile targets, explicit border_fuzziness0, zero separation, clumping0..100 and zone values mapping into bytes0..63. Omitted zones use original shared0. One optional default ungrouped timber348 object row is supported. Unknown stages/directives, player starts, other terrain metadata, grouping and spacing fail with source location, empty output and unchanged caller RNG. It is a strict end-to-end subset, not full support for the packaged map scripts or lobby seed initialization.

The otherwise mandatory terrain stage was checked separately: terrain-ID1 smoothing cannot change0/3 rasters, and all512 binary3x3 patterns pass the original cleanup unchanged. Empty-map terrain-object callbacks exit before RNG when numTerrainUnitsUsed is0. Stage connectivity calls were statically traced as allocation/floodfill only. Full world creation and OS callbacks are not emulated; the oracle composes actual setup, scheduler draws, growth, smoothing, cleanup, object candidates, placement gate and facet instructions under these established boundaries.

Two composed original-instruction fixtures now pass through the installed DAT and native map initializer. Seed1 yields terrainFNV1509571036, centers(3.5,1.5)/(2.5,4.5)/(0.5,1.5), facets2/7/5 and finalRNG2283988067. Seed12345 yields FNV2339149697, centers(0.5,3.5)/(4.5,1.5)/(3.5,0.5), facets5/9/9 and finalRNG4028089366. Tests also verify preprocessing shares the generation stream, original default handling, unsupported-stage rejection, invalid metadata, late-failure rollback and orientation/facets through the native scenario bridge.

`Game::initRms` initializes the generated native world, and `swgbtool render-rms` accepts a bounded self-contained file and renders it using installed original assets. The authored `tools/fixtures/explicit-lands.rms` produces a32x32 diagnostic with80 native timber objects, finalRNG3347987320, captured locally as `rms-integrated-map.png`. This is not a playable skirmish map: player starts and the full original controller are absent. The normal launcher retains the existing skirmish path.

Validation: all five strict-warning ASan/UBSan algorithm suites pass (`rms-pipeline-algorithms.log`); the full installed-data sanitizer suite and completed AI/player matches pass (`rms-pipeline-tests.log`); app menu/render/effects/music-backend smoke passes (`rms-pipeline-smoke.log`). CLI checks cover successful rendering, invalid dimensions/RNG and unsupported stages without creating output. CMake targets are wired but CMake is unavailable locally; direct compiler scripts were used. Original-byte evidence, DAT probes and oracle remain local in `rms-land-parser-*`, `rms-pipeline-*` and `rms-terrain-autoobjects-*` under Research/windows-reference.

## 2026-11-13 — Military scheduler invocation constraints

Actual-byte fixtures confirm that tactical updates require world state byte+29 equal0 and selected computer-player cursor equal the player ID; information updates occur earlier. Constructor evidence identifies player type3 as computer-controlled. Rotation cycles among type3 players; inactive/missing-controller repair happens upstream rather than inside rotation. The world simulation-time multiplier also scales the shared host-time budget (initial multiplier1.5 and base budget2).

These findings are saved in `ai-controller-facts/scheduler-upstream-*`. No military scheduler was enabled: manager predicate meanings, group maintenance/reconciliation, readiness destinations, opponent/remembered-target preparation and command completion/replanning remain requirements. The four-state world byte is not mislabeled as a simple pause flag. Existing native pause behavior and fallback AI remain unchanged.


## 2026-11-13 — Original border fuzziness and land rejection

Land growth now implements original edge-rejection function0x4da5e0 with resolved rectangular bounds and fuzziness0..255. It preserves the asymmetric corner calculation, a score of101 for values at least100, one comparison RNG draw for every popped candidate, and rejection to permanent zone255 before checking prior occupancy. Default bounds are0,0,width,height, not the last tile indices. Native work/input limits and transactional RNG/output behavior remain intact.

The formula was checked against2,268 original calls;13 full-growth fixtures include inset/degenerate bounds, several fuzziness values and maximum random roll100 rejecting score101. All six sanitizer algorithm suites pass, including previous zero-fuzziness results. Local findings/probes are `rms-edge-*2026-11-13` under Research/windows-reference.

The packaged-map audit found no map unlockable by this field alone: all46 inspected scripts use player lands and terrain rows,34 specify separation and37 explicitly specify nonzero fuzziness. Flats54220 is the smallest starting candidate, but also needs resource/object rows. The audit remains local in `rms-packaged-*`; no unsupported map is silently flattened.


## 2026-11-13 — Two-player starts and packaged RMS resource entry

`generateRmsPlayerStarts` reproduces exactly two owner-ordered player-land centers from original setup0x4e0740, including the full eight-slot rejection permutation, offset endpoint32767, rectangular-perimeter integer arithmetic and the width-derived margin reused on both axes. Thirty-six actual-instruction fixtures cover square/rectangular maps, several radii and seeds. Invalid dimensions/radii/out-of-map results fail without changing RNG; randomization work is bounded. Unequal radii, more players and custom borders remain unsupported.

The strict runner now accepts one `create_player_lands` block, returns its two start coordinates and grows zones1/2. It does not mix that block with nonplayer lands or permit explicit player coordinates/zone overrides. Proven defaults are radius3, clumping8, fuzziness20 and tile target equal to map area; land_percent divides area across selected rows. Terrain types still require explicit values. Starting units are not created; this remains diagnostic map initialization.

Important mode qualification: the actual constructor installs derived vtable65ef20. Its setup0x4e76a0 delegates to0x4e0740 only when the game randomizePositions flag is enabled; full derived-function fixtures confirm no additional RNG on that branch. The disabled branch is team-aware and has additional draws, so it remains unsupported. Existing explicit nonplayer fixtures retained their RNG in both modes. `<PLAYER_SETUP> random_placement` is verified as a compatibility no-op, not a setter for that game flag. The original option default is unknown. The runner explicitly uses randomized-position mode and rejects an externally supplied FIXED_POSITIONS context, which the original registers only in the other mode.

Two complete32x32 composed original-stage fixtures now pass through the installed DAT and native initializer. Seed1: starts(18,5)/(13,20), terrainFNV3728055951, timber(1.5,5.5,7)/(3.5,18.5,3)/(1.5,29.5,3), finalRNG1403560318. Seed12345: starts(5,20)/(20,11), terrainFNV1771775183, timber(18.5,15.5,1)/(26.5,22.5,10)/(4.5,15.5,2), finalRNG3972322661. These compare actual-instruction stages under documented callback boundaries, not a live full Windows match.

`render-rms-resource` adds DRS resource/include loading and explicit startup defines to the same native renderer. Automated CLI tests package authored scripts in a temporary archive and require byte-identical PNG output to file loading, then check missing resources/arguments, unsupported terrain and conflicting modes create no output. Original Flats54220 now loads and accepts its setup/land instructions but still rejects TERRAIN_GENERATION at resource54220:21; no complete stock-map claim is made. The authored two-player example renders80 trees with finalRNG2346674949 in local `rms-player-lands.png`.

Validation: six strict-warning sanitizer algorithm suites pass (`rms-players-algorithms.log`), full installed-DAT ASan/UBSan and completed AI/player matches pass (`rms-players-tests.log`), and file/DRS CLI tests pass. Native menu/render/effects/streamed-music smoke also passes (`rms-players-smoke.log`). Source fixtures contain no original game assets. Local evidence is `rms-player-starts-*`, `rms-player-setup-binding-*`, `rms-player-derived-*` and `rms-player-pipeline-oracle-*` under Research/windows-reference.

## 2026-11-13 — Military assembly and command acceptance boundaries

Further original-byte tracing corrected two labels:5ad4a0 counts remembered records owned by the current opponent, while5ab350 rebuilds the ungrouped soldier pool; neither chooses an opponent or reconciles existing group membership. Fresh type100/101 groups gather around member index0's current position. Readiness measures distance from a unit's rectangular footprint, subtracts1 and compares inclusively against spacing; state7 uses a strict stopping boundary and respects existing controller orders.

Command724 passes through a bounded200-packet buffer. Ordinary unit acceptance creates movement action1 and records controller action610, before pathfinding happens. State7 permits action614, so existing movement is not restarted each pass. Certain class34/36 transition states reject or defer movement. Movement completion compares footprint distance directly to spacing, without readiness's subtract1. Priority is controller metadata; general preemption is still unresolved.

Native issueMove instead reports synchronous path success and lacks equivalent spacing completion/transition/controller-order semantics. Directly wiring the original scheduler to it would change behavior, so the scheduler remains disabled. No disconnected controller shell was added. Exact bytes and executable fixtures are local under `ai-controller-facts/readiness-*`, `military-order-*` and `military-move-acceptance-*`. Next integration needs queued action acceptance and those controller states, plus opponent/target preparation; the existing fallback AI remains active.

## 2026-11-13 — Nonplayer RMS border margins

The script runner now applies explicit nonplayer left/right/top/bottom border percentages to the existing original edge-rejection path. Original handler4e2c40 probes cover24 opcode/value combinations on a7x9 map without RNG. Left/top truncate dimension*percent/100; right/bottom subtract that truncated margin from the full dimension, rather than truncating the remaining percentage. Values0..100 are accepted; reversed resolved bounds fail transactionally. Custom player borders remain unsupported because they also affect start placement.

Two composed original-stage7x7 fixtures validate parser-to-growth-to-smoothing integration with all margins25 and fuzziness100: seed1 terrainFNV2387610166/finalRNG483449721; seed12345 terrainFNV204622211/finalRNG3426167425. All six strict-warning ASan/UBSan algorithm suites pass. Local evidence is rms-border-parser-probe-* and rms-border-pipeline-oracle-* under Research/windows-reference. This closes one script-to-engine integration gap, not complete packaged-map support.

## 2026-11-13 — One-to-eight-player randomized RMS starts

The native player-start helper now supports configured counts1..8. Full original create_player_lands handler/initializer and setup4e0740 match288 fixtures across square/rectangular maps, radii and seeds, including offset32767. The full eight-slot permutation is still consumed independently of active count. Perimeter step is integer-truncated perimeter/playerCount with no remainder redistribution. Prior two-player fixtures remain unchanged. Fixed-position/team-aware placement and unequal radii/borders remain unsupported.

Count is wired through generateRmsMap, Game::initRms and both diagnostic render commands (`--players 1..8`, default2). Row expansion and land_percent division use the configured count. Composed original32x32 fixtures for1/3/8players (seed1,target60) match terrainFNV2429689585/1986512803/2929584340 and finalRNG940038285/2511621164/1668072822. These verify the integrated terrain pipeline, not starting units or a playable eight-player match.

All six sanitizer algorithm suites pass; CLI tests verify eight-player file/DRS image parity, difference from two-player output, and invalid-count rejection. Evidence is rms-player-counts-* and rms-player-count-pipeline-oracle-* under Research/windows-reference. The existing normal skirmish still uses its prior setup until full RMS terrain/object/world initialization is supported.

## 2026-11-13 — Ordinary point orders inside the unit footprint

Original ordinary move packet3 follows44b070 →55f220 →409160 and creates point action408220 with spacing0; independent control records command710/action610. Action4084b0 first checks rectangular-footprint distance and completes without pathfinding when the destination lies inside it. Waypoint/group commands711/733 deliberately bypass this initial check. Twenty-five original-instruction fixtures verify boundary corners, just-outside points, spacing and command distinctions.

The native independent point-command branch now cancels its previous order without moving when both coordinate differences lie within the DAT collision footprint. Generic issueMove remains a synchronous path probe for callers that require it; formation destinations and later movement completion are unchanged. Regression tests cover an asymmetric rectangular corner, each just-outside axis, prior-path cancellation and formation isolation. Full installed-DAT ASan/UBSan suite passes, including both AI matches and scripted player victory at677.7s (`point-move-tests.log`). Evidence remains local under ai-controller-facts/player-point-move-*. This is initial completion parity, not a claim of complete movement/controller parity.

## 2026-11-13 — Original separated lands and integrated terrain generation

Land growth now implements nonzero other_zone_avoidance_distance. Original4da400 uses an asymmetric horizontal span initialized from trunc(distance*0.666666), expanding after rows outside that span; it is not a square or radial gap. Own-zone adjacency still counts within5x5; foreign nonreserved zones inside the separation window reject. Shared zones remain exempt. Original-instruction probes cover300 randomized predicates,5808 neighborhood sweeps and44 full-growth fixtures. Integrated parser fixtures (distance2) match terrainFNV2659705154/2623728704 and finalRNG483449721/1874836596 for seeds1/12345. Scan visits now count against the existing work budget; unusually small explicit budgets may reject earlier.

The terrain stage reproduces original4e6bc0 partial raster shuffle, clump seeding/exclusion, transferable shared frontier nodes, round-robin growth, priorities, spacing and final horizontal gap fill. The original vertical cleanup branch is unreachable and is not silently repaired. Native helper fixtures cover313 original-worker cases including rectangular maps, competing terrain, zero clumps, player protection and RNG boundaries. Zero clumps consume shuffle draws without changing terrain. Identical target/base and nonzero-clump1x1 cases remain rejected because the original can loop indefinitely. Inputs and exhausted work fail transactionally.

Original parser4e3480/finalizer4e02f0 probes establish target default mapWIDTH, clumps1, spacing0 and clumping20; base terrain is uninitialized and must be explicit in this strict runner. land_percent stores a negative count then resolves area*percent/100. set_scale_by_size scales positive counts by area/10000; set_scale_by_groups also scales clumps. Original signed-arithmetic overflow is rejected. Resolved counts above999 are conservatively rejected even though the original worker caps them. No script may silently request unsupported terrain/world effects.

Player-start protection is wired through the full runner. Original finalizer appends only owned land centers, with literal radius13/weight20 independent of land radius/fuzziness. Original4e69e0 mask sums weighted integer-distance contributions and caps scores above100 to101. Protected rows require zero-mask seeds, consume one rejection draw for every popped growth candidate, and add the mask to priority. The mask is immutable across rows. The connectivity getter stub used by isolated worker probes is read-only and its result unused for the supported avoidance modes0/1; mode2 remains unsupported.

Two-row original-stage fixtures through the installed DAT/native initializer match both terrain and later timber placement/facets. Unprotected seed1/12345 terrainFNV1549600828/693012607 and finalRNG3883528439/2130667826; protected fixtures FNV3452956004/353970588 and RNG942928812/3736035543. These compose original instructions under documented callback boundaries, not a full live Windows world.

Evidence stays local as rms-separation-*, rms-terrain-worker-*, rms-terrain-parser-*, rms-terrain-protection-* and rms-terrain-*-pipeline-oracle-* under Research/windows-reference. Authored player-terrain.rms exercises the native renderer; four-player64x64 image rms-engine-terrain.png contains80 native timber objects, finalRNG536614793. Complete stock RMS maps, non0/3 terrain metadata/scenery, starting units and original military controller integration remain unfinished; the normal launcher retains its existing skirmish setup/fallback AI.

Validation: all seven strict-warning sanitizer algorithm suites pass (`rms-terrain-integration-tests.log`); full installed-DAT ASan/UBSan tests and completed AI/player matches pass (`rms-terrain-skirmish-tests.log`); authored file/DRS two/eight-player rendering parity passes; native menu/render/effects/music smoke passes (`rms-terrain-smoke.log`). Original-like map generation remains diagnostic, with unsupported stages rejected explicitly.
