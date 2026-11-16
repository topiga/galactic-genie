# Clone Campaigns random-map parity

## Current comparison coverage: 2026-11-15

Packaged **Forest54204, Reytha54220 and Arid54201** now have complete original-runner
comparisons in the fixed, Tiny, two-Empire profile: three seeds each for Forest
and Reytha, four for Arid. The standard production audit passes9/138 attempts
(three of46 map resources). This is bounded startup coverage; it is not general
engine or gameplay parity.

Forest resource 54204 now runs directly from `gamedata_x1.drs`, including all 23
object rows. For seed 16160, 120×120, Tiny, FIXED_POSITIONS, DEFAULT_RESOURCES and two
Empire players, native generation matches the original executable replay:

| Check | Result |
|---|---|
| Terrain and elevation | All 14,400 bytes of each equal the saved Windows grid |
| Automatic scenery | All 7,554 trees match creation order, position and facet |
| Owned startup | All 10 objects match type, owner, creation order, position and facet |
| Explicit Gaia objects | All 238 match type, creation order, position and facet |
| Final RNG | 3740942338 in both implementations |
| Native import | 7,802 objects; all IDs/types/owners/positions checked; animal/fish animation checked |
| Fresh live Windows read | All 201 static resources/explicit trees match; none missing |

`tools/rms_packaged_forest_checks.h` loads the user's actual archive and verifies
the complete map plus native import. It does not embed the game's RMS source.
The image `clone-campaigns-forest-full.png` in the external build directory is a
native diagnostic render, not a claim of a fully configured playable match.
Starting resources/UI map selection and later simulation remain separate work.

Civilization coverage includes all 64 ordered pairs in the controlled Tiny Forest
profile. Live Windows coverage includes all eight same-civilization pairs and four
mixed orders. These startup comparisons do not establish later simulation parity;
the detailed coverage and limitations are recorded below.

Fresh live Windows restarts independently verify fifteen more packaged Forest
seeds with the same size, defines and civilizations:

| Seed | Terrain / elevation / slopes | Static resource identities, owners, positions and order |
|---|---|---|
| 1 | 14,400 / 14,400 / 14,400 cells equal | 188 / 188 equal |
| 2 | 14,400 / 14,400 / 14,400 cells equal | 202 / 202 equal |
| 3 | 14,400 / 14,400 / 14,400 cells equal | 180 / 180 equal |
| 13 | 14,400 / 14,400 / 14,400 cells equal | 178 / 178 equal |
| 19 | 14,400 / 14,400 / 14,400 cells equal | 186 / 186 equal |
| 42 | 14,400 / 14,400 / 14,400 cells equal | 172 / 172 equal |
| 12345 | 14,400 / 14,400 / 14,400 cells equal | 193 / 193 equal |
| 16161 | 14,400 / 14,400 / 14,400 cells equal | 178 / 178 equal |
| 32767 | 14,400 / 14,400 / 14,400 cells equal | 174 / 174 equal |
| 65535 | 14,400 / 14,400 / 14,400 cells equal | 190 / 190 equal |
| 1409540336 | 14,400 / 14,400 / 14,400 cells equal | 172 / 172 equal |
| 2912922832 | 14,400 / 14,400 / 14,400 cells equal | 172 / 172 equal |
| 2666028048 | 14,400 / 14,400 / 14,400 cells equal | 192 / 192 equal |
| 2766341764 | 14,400 / 14,400 / 14,400 cells equal | 187 / 187 equal |
| 3878309407 | 14,400 / 14,400 / 14,400 cells equal | 193 / 193 equal |

These checks do not certify all startup animation/facets or mobile positions for
the new seeds. Forage facets change on the original's first simulation update;
constructor comparisons and later live appearance must remain separate. Terrain22
startup and nonblocking waves now have controlled original object-stage coverage.

This is **not general 1:1 terrain generation**. Other seeds, sizes, civilizations,
teams and map families still need complete comparisons. Historical checkpoints
below describe earlier coverage, not the current result.

## Owned dispatch and selected lands: 2026-11-16

The shared selector now dispatches the existing Command Center, worker and scout
startup callbacks in selected-land order. Gaia rows select matching land records,
preserve duplicate matches, and skip absent IDs without constructing a queue or
consuming RNG. Explicit `land_id` now stores argument+10, matching the original
parser and `place_on_specific_land_id`; default neutral0/player1 IDs stay special.
Selected Gaia rows retain all land centers for inner-distance exclusion.

127 executions of original `4dddf0` and its `4de8fb` owner-resolution prefix verify
selection, ordering, zero counts, scaling, empty/missing IDs and owner overrides.
The fixtures are in `tools/rms_object_dispatch_checks.h`. Those probes intercepted
placement calls, so they establish dispatch, not constructor parity. A complete
runner comparison with an unmatched specific-land row additionally matches all
6,202 objects, grids, startup poses and final RNG664476888.

The original `4dad80` civilization mapper was also executed for every unit ID
0..32767 and civilization0..8 (294,912 calls); sparse results remain local in
`rms-object-civilization-map-oracle-2026-11-16.json`. This is research evidence,
not a newly implemented general native constructor.

Five further complete runner comparisons now cover matched neutral ID7→17,
duplicate IDs, a neutral center next to starting workers, and clipping at the
map edge. The native importer also checks every resulting object's ID, owner,
type and position. Workers and scouts now exclude every land center when
applying their inner placement distance, as the original does.

| Selected neutral lands | Objects | Final RNG |
|---|---:|---:|
| Center50%,50% | 6,144 | 472243236 |
| Center25%,25% | 6,144 | 101811930 |
| Both centers, duplicate ID7 | 6,157 | 2212104137 |
| Center75%,28%, near worker area | 6,145 | 778134838 |
| Center0%,0%, clipped queue | 6,147 | 1759471658 |

This enables the bounded fixed120/two-Empire profile with one or two fixed neutral
seed lands (zero radius/growth, base terrain6), without cliffs, elevation or
connections. These five cases also match all original startup poses and grids.
Additional worker83/count1 rows now execute through the shared worker placement
callback and remain interleaved with Gaia rows in native import. Three complete
original runner comparisons cover a row just after scouts, a row after all Gaia,
and two rows interleaved after Nova rows. They match6,209/6,208/6,215 ordered
objects and RNG1555457656/1935763599/3226277433 respectively. The native importer
checks all ownership/positions and54 original owned animation/health/foundation
states across these cases. As in the original, each count1 row creates the owner's
resource84 quantity (three in this verified profile), not one worker.

The original resource arrays were also captured after a repeat replay that
revalidated all records, poses, grids and RNG. Both owners have the following
counters; native import compares resource4 to capacity minus usage and resource84
to its civilization attribute, alongside the stored counters:

| Additional worker rows | Population used/capacity | Resources4/11/21/37/40/84 | Cost counters98/164/165 |
|---|---|---|---|
| One | 7/5 | -2/7/2/6/1/3 | 725/350/375 |
| Two | 10/5 | -5/10/2/9/1/3 | 875/500/375 |

Additional scout550 rows with `match_player_civ` now reuse the verified scout
callback and preserve their positions among Gaia rows. Three more original
comparisons (early/late/interleaved) match6,204/6,204/6,206 ordered objects and
RNG3734544698/1377182600/3096183411. Native import checks38 original owned poses
and the corresponding population/cost counters: one added scout gives5/5
population and costs625/250/375; two give6/5 and675/300/375. These new full
constructor comparisons use two Empire owners; the exhaustive mapper research
alone is not full constructor coverage for every civilization.

Other arbitrary owned rows, broader mixed-land startup and new unit constructors
still reject explicitly. Additional workers require the verified ungrouped83/
radius3/min6/max6 profile; scouts require550/radius3/min7/max9 with civilization
mapping, both without spacing/terrain modifiers. This does not establish full
terrain or gameplay parity.

## Owned worker/scout quantities: 2026-11-16

The shared startup callbacks now accept nonnegative ungrouped quantities in both
initial and later worker/scout rows. Initial rows use the common RMS parser and
scaling logic. A finalized worker quantity of1 still uses resource84 (three in
this verified profile); scaled quantity2 creates two workers. Zero still builds
and shuffles the placement queue: later zero-worker/scout rows preserve all6,202
objects but consume120/288 additional CRT draws respectively.

Twelve complete original-executable replays cover later counts0/2/4, player-scaled
count1, exhausted count99, and initial worker/scout counts0/2. All match ordered
objects, terrain/elevation/slopes, owned/Gaia startup poses and final RNG. Native
import also verifies original population, worker/scout and cost counters. Requests
for99 stop at exhausted candidates: the tested worker row adds17/11 workers to
the two owners; the scout row adds60/17 scouts. Initial zero rows leave no workers
or scouts; initial count2 gives each owner two of each.

These extend `rms_general_object_checks.h` to35 complete-map cases. Private
provenance is in `rms-owned-quantities-findings-2026-11-16.json` and the matching
`rms-general-rows-quantity-*-comparison-2026-11-15.json` files (the latter retain
the helper's filename date). This is execution of the original PE with captured
assets and modeled host boundaries, not a new live Windows session.

At the research checkpoint, aircraft1521 remained unsupported. Complete original replays now cover early and late1521 rows. The late row gives
6,204 objects, RNG3529783542,
and two Empire transports1036 at95.5/37.5 and41.5/95.5, both with original z1.
The early row gives6,206 objects, RNG4112436110 and transports at99.5/35.5 and
31.5/87.5, also z1. All four transports have150HP and original startup poses
recorded; independent resource replays preserve the complete outputs.
These runs use the actual air-path manager constructor/bind calls and captured DAT,
task-header, graphics and sound records. Replaying the previous zero-worker case
with that manager added preserves its objects, poses, grids and RNG exactly.
The transport constructor consumes seven CRT draws, unlike the scout path.
Each transport adds one population and military-unit count, with160 recorded
unit cost (35food+125carbon). Both owners end at counters4/11/21/37/40/84 =
0/5/2/3/2/3 and98/164/165 =735/360/375.

This is original-side evidence, not native aircraft parity. The actual placement
gate checks nearby static ground-object collision boxes; it cannot be replaced
by an assumption that aircraft ignore ground blockers. Exact collision handling,
startup height/import and native comparison remain open. Local evidence includes
`rms-airtransport-placement-static-2026-11-16.json`, successful gate traces,
`rms-general-rows-airtransport-late-1-1-1-oracle-2026-11-15.json` and the matching
resource supplement; `rms-airtransport-findings-2026-11-16.json` indexes both
complete replays and their provenance. The last packaged production audit remains9/138 accepted
configurations, and RMS menu integration remains separate work.

## Native transport placement: 2026-11-16

The native generator now reproduces the two complete original transport replays
above, including every ordered object, terrain/elevation/slope grid, owned/Gaia
startup pose and final RNG. Support is bounded to one ungrouped1521/count1 row,
civilization matching, distances3..6, flat120/base6 and two Empire owners. Other
civilizations, quantities and elevated transport starts still reject explicitly.

Transport placement reuses the owned mobile queue/zone/animation path, with the
original collision radii and seven constructor draws rather than the scout's
eight. The static gate tests other objects' collision boxes and build-on flags;
flying-object overlap is checked before the ground mobile-unit exemption.
Native import retains original height above ground and the population/military/
cost counters. Regression cases include both row positions and unsupported
quantity rejection without altering the running world or caller RNG.

The37 complete-map regression cases pass. The render check additionally verifies
actual native quads: transport5460/layer19 stays at the ground anchor and
5461/layer20 moves up one `tileHalfHeight` (24pixels in the loaded DAT). Screen
selection/health-bar projection uses the same altitude. Original54a643/54a6d8
select ground coordinates for layers0..10 and19 independently for each graphic;
this was confirmed with six original selector calls. Terrain elevation still
uses its separate `elevHeight` scale. These changes apply to retained RMS startup
height; flight transitions and all other scenario-created aircraft remain
separate work. No fresh live Windows or pixel-for-pixel UI comparison is claimed.

## Transport civilization mapping: 2026-11-16

Four mixed-pair complete original replays now cover all eight civilizations:
early1/2, early3/4, late5/6 and late7/8. Native generation matches every ordered
object, grid, owned/Gaia pose and final RNG. The native mapper uses the actual
1036/1040/1046/1042/1038/1044/1521/1522 transport prototypes and their graphics.
The Rebel transport has eight frames at0.5seconds instead of five at0.85seconds.

Early3/4 produces6,205 objects and RNG1007836572, unlike early1/1 and1/2
(6,206/RNG4112436110): an earlier nonanimated scout omits a constructor draw.
Both late mixed pairs match the late1/1 count/RNG. Import additionally checks
all original owned poses, height and counters, including resource21=3 for Rebels
and1 for Trade Federation (2 for the other compared civilizations).

The original harness executes civilization restrictions and automatic research,
not full ordinary civilization/team bonus application. Its resource4 therefore
represents RMS building grants minus used population. The native test compares
that same component separately from final capacity; Trade Federation retains
its existing native200 scenario-capped capacity, and Confederacy workers retain
the existing native55HP rather than the harness's unbonused40HP. These captures do not prove
full civilization-bonus parity. The boundary is documented in
`rms-research-initialization-findings-2026-11-13.txt` and the transport ledger.

`rms_general_object_checks.h` now contains41 full-map cases. These four pairs
cover each civilization, not all64 pairings or every seed. The one-row/count1,
flat120/base6 and distances3..6 limits remain. Full-map evidence and resource
replay hashes are indexed in `rms-airtransport-findings-2026-11-16.json`.

## Transport quantities and exhaustion: 2026-11-16

The single transport row now accepts nonnegative ungrouped quantities. Three
more complete original comparisons cover zero, two and99 per owner:

| Requested per owner | Total map objects | Transports placed | Final RNG |
|---:|---:|---|---:|
| 0 | 6,202 | 0 / 0 | 1530472592 |
| 2 | 6,206 | 2 / 2 | 4238084908 |
| 99 | 6,316 | 63 / 51 | 2174457350 |

All ordered records, grids, owned/Gaia poses and RNG match. Zero creates no
transport but still consumes120 queue draws. The99 case exhausts candidate
space; strict flying-object overlap prevents stacking transports. Independent
original resource replays verify population/military counts and unit-cost
accounting; native import compares these alongside height and sampled poses.

This brings the general full-map suite to44 cases. Unsupported grouping variance
still rejects without changing the running world or caller RNG. Evidence is
indexed in `rms-airtransport-quantity-findings-2026-11-16.json`. Multiple transport
rows, other owned-unit constructors and broader geometry remain unverified.

The fresh production acceptance audit still passes9/138 packaged combinations
(Forest, Reytha and Arid over three standard seeds each); this transport work
does not yet unlock another complete packaged map. All44 general cases, the
packaged-map regressions, transport render checks, RMS validation/CLI tests and
desktop menu/audio smoke checks pass. The new audit is retained separately as
`rms-packaged-generation-audit-transport-production-2026-11-16.json`.

## Explicit player zones with owned startup: 2026-11-16

Owned startup now accepts explicit shared, team-based and random player zones
on the compared fixed120/base6 Forest profile, with or without cliffs. Six
complete original replays match all ordered objects, terrain/elevation/slopes,
owned/Gaia startup poses and final RNG:

| Zones | No cliffs: objects / RNG | Cliffs: objects / RNG |
|---|---|---|
| Shared `zone 0` | 6,336 / 3578249443 | 6,202 / 664476888 |
| `set_zone_by_team` | 6,336 / 3578249443 | 6,202 / 664476888 |
| `set_zone_randomly` | 6,322 / 1025370822 | 6,330 / 2120920959 |

The original shared zone is stored as10/10; team zones are2/3 for these enemy
players. Random zones consume their draws during parsing and change the later
starts to87/89 and43/30. The original team selector runs with actual enemy
diplomacy arrays, not an assumed team result. Each case retains ten owned objects;
independent original replays verify their population and cost counters.

Support remains limited to two Empire owners, radius13/7200-tile base6 lands,
one6000-tile terrain21 row (60clumps, spacing2, clumping20 and start avoidance),
fixed positions, and no elevations/connections. Allied or other-civilization
startup remains rejected; a native import regression checks that rejection
preserves the running world and caller RNG. Existing unowned cliff restrictions
remain in place. The general suite now contains50 full-map cases.

Local provenance is in `rms-owned-zone-findings-2026-11-16.json` and
`rms-owned-zone-cliff-findings-2026-11-16.json`. These are configured original
executable replays, not fresh live Windows comparisons or general map parity.

All50 general cases, packaged-map regressions, transport render checks, RMS map/
CLI validation and desktop menu/audio smoke checks pass. A fresh packaged audit
still accepts9/138 configurations; the other zone-bearing packaged maps need
additional geometry and object-profile work. Its separate record is
`rms-packaged-generation-audit-owned-zone-production-2026-11-16.json`.

## General object-row controller: 2026-11-15

Gaia resources after owned startup now execute in source order from parsed rows,
rather than matching an expected Forest resource sequence. The original
`4e3850..4e3cfb` handlers establish the 60-byte row semantics; `4dddf0` scales
its group count. Grouping directives are order-sensitive. Temporary spacing
removes candidates from the current row; permanent spacing survives later rows.
Terrain selection filters group centers, not every child object. Global clearance
uses each candidate's terrain-connectivity zone; per-player rows additionally
require the player's start zone. Both use the original eight clamped samples from
`4df2f0`, with diagonal distance truncated from `radius*10/14`.

Eleven authored variations on the same cliff/forest geometry were executed through
Clone Campaigns' original `4df9b0` runner, seed 1, fixed Tiny map, two Empire owners.
Every case matched all ordered object records, terrain/elevation/slope bytes,
owned/Gaia startup animation states and final RNG:

| Variation | Objects | Final RNG |
|---|---:|---:|
| Reverse Gaia resource rows | 6,203 | 4194478548 |
| Change Nova/Ore group counts | 6,206 | 4166862475 |
| Default mode and order-sensitive grouping | 6,210 | 885130658 |
| Temporary/permanent spacing and zone radius | 6,203 | 502114030 |
| Terrain-filtered group centers | 6,219 | 170179477 |
| Grouped trees with variance | 6,217 | 2876734422 |
| Player-count scaling | 6,206 | 1396042352 |
| Holocrons after all other resources | 6,203 | 327591995 |
| Global zone clearance | 6,205 | 1003759820 |
| Avoid-player groups with zone clearance | 6,213 | 1186593183 |
| Large global zone clearance near edges | 6,221 | 707435866 |

`tools/rms_general_object_checks.h` preserves complete record/RNG/grid regression
fingerprints against these original captures. The parser has a separate standalone
suite. Full animation comparisons and original runner captures remain in local
research (`rms-general-rows-*-oracle-2026-11-15.json` and matching comparison files).
These are executable replays with captured original assets and modeled host I/O;
no fresh live Windows playthrough was performed for these eleven cases.

The production audit remains **9/138 accepted**. General Gaia row execution does
not yet generalize owned startup, specific-land selection,
unknown unit prototypes or the terrain pipeline. Holocron modifiers retain their
verified profile. Unsupported paths reject explicitly. The normal skirmish menu
still uses procedural startup; these results concern RMS generation and import,
not complete gameplay parity.

## Historical packaged-script baseline: 2026-11-14

At commit `039f3e5`, the local audit ran 46 packaged RMS resources with three
seeds each: 138 attempts, **zero complete native maps generated**. Passing
stage fixtures must not be reported as complete packaged-map support.

The audit used 120×120 maps, two civilization-1 players and explicit
`TINY_MAP`, `RANDOM_MAP`, `2_PLAYERS` defines. These are test inputs, not proof
of the original game's injected symbols or initial RNG state. Source scripts
stay in the user's game archive and private research directory.

| Boundary found | Status |
|---|---|
| Omitted object quantity in shared startup resource 54103 | Fixed in `8a58316`; original default is one |
| Shared worker placement at distances 6..6 | Original-executable comparison; see `ORIGINAL_SYSTEMS.md` for current coverage |
| Scout construction and civilization substitution | 54 composed startup cases; bounded one/two-player support in `df02da7` |
| Near-player Nova resources | Four composed grass/forest cases in `eb40b44`; randomized 32×32/two Empire players only |
| Remaining per-player resources, spacing and zone filters | Full stock-row composition still required |
| Fixed-position player starts | 29 original cases, live Windows corroboration and settings integration |
| Mixed player/nonplayer lands, borders and random land placement | Still blocked in the integrated importer |
| Team/random zones and flat-terrain flags | Zone geometry now compared with the original; owned startup remains gated. See current evidence below |
| Packaged elevation and one conditional syntax branch | Still blocked in the loader |

Ten resources shared the initial omitted-quantity failure. Fixing that default
advances them to subsequent validation; it does not make ten maps playable.
Resource 54220 is a useful next candidate because its active terrain profile
is closer to the existing forest tests. Terrain 44 and early forest behavior
still need verification alongside the remaining object rows.

## Live Windows startup observation

QuickDesk was used to start a Standard Random Map game: Forest, Tiny (2 player),
two Galactic Empire players, Easy, Standard resources, population 75, Normal
visibility, Standard initial Tech Level and victory. No unit orders were issued.

The visible start had a Command Center, three workers and a scout; population
was 4/5, resources were carbon 200, food 200, nova 100 and ore 250. The selected
worker had 40/40 HP. These observations corroborate visible startup behavior;
they do not establish tile coordinates, constructor ordering or RNG parity.

The previous editor scenario was saved separately. The new match was saved as
`forest_tiny_empire_startup_20261114.ga1` about 21 game seconds after startup;
the AI could already have acted. Its 227375-byte file was confirmed in the
Windows user's VirtualStore SaveGame directory. Settings and worker screenshots
and `live-forest-startup-2026-11-14.json` are retained in local research.
The save was copied locally through OpenSSH, preserving the Windows file.
Bounded raw-DEFLATE decompression yielded 2280369 bytes beginning `VER 9.4`;
the stream ended cleanly without trailing bytes. This establishes the container
format only; startup seed and terrain fields have not been decoded.

### Decoded grid and live seed recovery

`python3 script/inspect_cc_save.py SAVE.ga1 --output-directory NEW_DIRECTORY`
extracts row-major terrain/elevation bytes through the verified 1.1 save prefix.
It validates the format, bounds decompression and counts, and rejects malformed
input. It does not load objects or treat saved RNG checkpoints as map seeds.
Authored fixtures run with `python3 script/test_cc_save.py`.

Original serialization evidence: `5fe970`, `40f230`, `5817f0`, `418190`,
`61ffa0`, `494560` and `625c70`. The private REA bundle is
`save-prefix-rea-evidence-2026-11-14.json`; the map-serializer record is
`ev_3b59c2169184a10b7635490bd2a4f9e4eeb52a846e414560314667bd8f043e0c`.
The live save decodes to 120×120 tiles at inflated offset 439134, four zone
records, local player 1 and three players including Gaia. Terrain SHA-256:
`14ea7ec5f7aefd0290f93134bdccbe914776acc6219bbaaba786a3b7b08ab5cf`.

Read-only Windows process inspection verified the reference executable hash
and recovered **new-game seed 28724** at application offset `0x4c` and
**map seed 16160** at offset `0x50`. The configured overrides at `0x44/0x48`
were both -1. `61df00` preserves the former; `5c9df0` preserves and reseeds
from the latter. These are separate from save-time checkpoints 28186/31754
and the current full CRT state. Raw observations and the read-only probe are
retained privately as `live-forest-startup-seeds-2026-11-14.json` and
`read-live-startup-seeds-2026-11-14.ps1`.

Original wrapper replay now establishes that Forest selects resource **54204**
and reaches the RMS generator with **16160 unchanged**. It executes the filename,
map-type and root-constructor path after the second reseed; no RNG calls occur
before generator entry. Evidence: `rms-live-mapseed-boundary-oracle-2026-11-14.json`.

Original dispatcher `4e7470`, executed with the observed live settings, injects
**`TINY_MAP`, `FIXED_POSITIONS`, `DEFAULT_RESOURCES`**, without consuming RNG.
The live size/random-placement/resource/game-mode fields are all zero.
`RANDOM_MAP` and `2_PLAYERS` are not injected. The script's `random_placement`
handler does not change the game setting.

Fixed-position mode is now connected to the native pipeline for mutually enemy
players with equal-radius player lands and default borders. The corrected native
attempt (120×120, two civilization-1 players, seed 16160 and the three confirmed
defines) initially stopped at `set_flat_terrain_only`, resource 54204:154.
The parser now accepts that flag; remaining object, elevation and connection
stages still prevent producing the complete grid. The earlier attempt with assumed randomized-position defines
was not the live setup mode. Fixed-position near-Nova composition remains
explicitly rejected until its complete startup behavior is checked.
There is **no whole-map equality result yet**. The Windows grid also includes
elevations 0..7 and local foundation painting, which must be accounted for.

The fixed-position integer model does reproduce both live Command Center
coordinates: owner 1 at **(19,36)** and owner 2 at **(42,96)**. Read-only process
inspection found original object IDs 7554/7555 at those positions, with map seed
16160 unchanged. Independently, the saved terrain has exactly 16 foundation-27
cells at x17..20/y34..37, the first center's 4×4 footprint. This is a matched
starting-position result, not a whole-map result. Private evidence:
`live-forest-starting-centers-2026-11-14.json` and its read-only PowerShell probe.
The local audit and executable-oracle JSON files
are under `Research/windows-reference` in Galactic Genie's application-support
directory; no original game scripts or saves are committed to this repository.

### Flat-terrain filtering and elevation boundary

The original terrain worker now has 72 additional exact fixtures covering three
seeds, four supplied height/slope profiles, spacing 0/1/2 and the flat-only flag
on/off. All final terrain bytes and RNG states match. Candidate height limits
are inclusive. Flat-only checks the existing slope bytes in the clipped spacing
square only when spacing is nonzero; horizontal gap cleanup bypasses both
filters. The helper accepts original height/slope bytes, while the integrated
map path still supports only zero-height terrain. It rejects elevation stages.

Forest 54204 contains explicit elevation blocks near the end of the script:
height 4 with 700 tiles/10 clumps, then height 7 with 4300 tiles/7 clumps; both
scale by groups. Their textual position does not determine execution order.
Original constructor priorities and scheduler dispatch place land at 1.0,
elevation at 1.5, cliffs at 1.75, terrain at 2.0 and connections at 2.1.
Elevation generation and slope construction must precede terrain painting
before the saved Forest grid can be compared meaningfully.

Private evidence: `rms-flat-terrain-oracle-2026-11-14.json` and
`rms-flat-terrain-findings-2026-11-14.txt`. These controlled-worker fixtures
prove filtering, not complete hill generation or whole-map parity.

The native height smoother now matches 106 original whole-map fixtures,
including rectangular maps and sparse hills on 32×32/64×64 grids, using original ceiling 8 and heights 0..8. The
comparison executes `4925d0`, `4922c0`, `491fc0`, `495bb0` and `496170`;
only final visual refresh callbacks are skipped, with no objects/local player.
The original slope classifier additionally matches 19,683 neighboring-height
cases and edge fixtures. This replaces corner-gradient guessing for RMS
geometry, but is not yet connected to full map startup. Evidence:
`rms-height-smoothing-oracle-2026-11-14.json` and
`rms-slope-oracle-2026-11-14.json` and
`rms-sparse-smoothing-oracle-2026-11-14.json`.

The complete native elevation helper now matches 180 original growth-worker
fixtures and 83 composed stage fixtures, including height reset, persistent
protection, smoothing, slope classification and exact final RNG. One fixture
executes both Forest elevation blocks on a **120×120 uniform terrain-6 grid with
stage-entry seed 1**, using the two live protection centers. It matches every
height, slope and protection-mask byte (via full-grid fingerprints) and final
RNG 1103906832. This controlled stage is **not the live Forest terrain or entry
RNG**, and does not establish whole-map equality.

Original parsing expands those blocks into four then seven successive level
rows. At 120×120, `set_scale_by_groups` produces 1008 tiles/10 clumps for each
first-block row and 6192 tiles/7 clumps for each second-block row. Elevation's
scaling differs from terrain: that keyword scales tile counts only, while
`set_scale_by_size` scales clump counts only. Original parser/finalizer execution
also confirms protection radius 13 and weight 20 at each positive-owner center.

The full-stage fixture exposed a lifecycle dependency: candidate-node links
survive between elevation rows even though the list sentinel resets. Preserving
those links is necessary to match the third and later Forest rows. The original
stage clears all node links at entry (`4d6b40`), so incoming land-stage links do
not carry into this elevation stage. Zero-row execution preserves initial
height/slope/RNG but rebuilds protection. The helper remains bounded to square
maps and already finalized rows; no map parser activation or complete
land/elevation/terrain/object pipeline parity is claimed. Private evidence:
`rms-elevation-worker-oracle-2026-11-14.json`,
`rms-elevation-stage-oracle-2026-11-14.json`,
`rms-elevation-forest-groups-oracle-2026-11-14.json` and
`rms-elevation-forest-parser-oracle-2026-11-14.json`.

### Integrated elevation and native hill import

The RMS parser now executes elevation between land and terrain, accepts late,
repeated and empty elevation sections, and preserves the original separate
scaling rules. Nine script-seed comparisons match original land/elevation/terrain
rasters and RNG; the empty object stage adds its independently verified final
scheduler draw. Reordered sections produce the same result. The integrated map
exports original tile heights and slope bytes. Post-elevation automatic trees
411/413 are now verified; nonflat owned-object startup remains rejected while
its callbacks are being composed.

`Game::initRms` imports those tile heights directly and rotates slope IDs to the
native renderer's coordinate convention. It does not reconstruct the slope from
four corners: original shapes can share those corners but differ internally.
All 17 triangulations match original `54f820` samples on the generated test map,
and scenario restart clears the RMS height mode. Original asset template
outlines confirm the rotation. `tools/fixtures/elevated-land.rms`, size16,
seed4294967295 renders with final RNG2180953274; the image was visually inspected.
This is a controlled hills fixture, not a playable stock Forest map or a claim
of complete lighting/filter-pixel equivalence.

Private evidence: `rms-land-elevation-terrain-scriptseeds-oracle-2026-11-14.json`,
`rms-original-height-sampling-oracle-2026-11-14.json`,
`rms-slope-rotation-model-2026-11-14.json` and
`rms-slope-asset-orientation-2026-11-14.json`.


### Connection integration and live Forest geometry (2026-11-14)

The parser now resolves player-land and team-land connection commands and runs
connections after terrain cleanup, before automatic scenery. Six composed
original scheduler cases (three seeds, with/without elevation) match terrain,
height, slope, starts and final RNG. Twelve source-order variants are covered by
`rms_connections_map_checks.h`. Early forest bases with connection setters remain
unsupported; team membership in the integrated map is currently all-enemy.

For the saved Windows Tiny Forest match (map seed16160, fixed starts, two Empire
players), the fully preprocessed stock resource54204 geometry now matches every
one of the **14,400 elevation bytes**. Terrain differs only on the16 cells of the
local Command Center foundation: the diagnostic deliberately omitted explicit
object rows. Both starts, (19,36)/(42,96), and7554 automatic trees also match.
The diagnostic consumes all original preprocessing choices before filtering
object rows; its entry RNG is1993207714 and final RNG2078673907. That final state
includes an empty object-stage scheduler draw, not the full game's object draws.

Private evidence: `live-forest-geometry-comparison-2026-11-14.json`,
`live-forest-geometry-native-2026-11-14.json` and
`native-live-forest-grid-2026-11-14/`. The original saved map and extracted grids
remain local. This establishes geometry equality for one seed/setup, not all
Forest seeds, all packaged maps, or complete startup equality.

### Ordered stock resource prefix

After Command Centers, workers and scouts, the integrated parser accepts the
verified holocron row followed by forage, near/medium/far Nova and near/far ore.
The sequence is deliberately bounded to120-square, two Empire owners, randomized
starts on the verified flat grass/dry-forest contexts. Earlier64-square holocron
and32-square near-Nova cases remain supported separately.

One eligibility raster must survive across resource rows. Resetting it can move
ore while leaving the final RNG unchanged. Native integration retains it and
adds every prior resource to collision occupancy. Four original ten-row startup
compositions provide240 resource positions/appearances, per-row RNG states and
final eligibility masks. The full parser/pipeline tests check all six successive
resource prefixes in those four setups. Later animal rows and elevated stock
resource startup remain unsupported. Holocron placement alone does not establish
pickup or victory gameplay parity.


### Forest starting units, connectivity and allied connections

The native Forest21 profile now composes base6/player-land6 geometry, hills,
automatic413 trees, Command Centers, packaged workers and scouts for two Empire
players on120-square maps. It rejects additional unverified terrain/scenery and
later resource rows on this elevated profile. Existing flat grass/forest paths
retain their previous supported civilizations and owner counts.

Original starting-unit placement first clones the unit's terrain restriction,
forces the start-center terrain type passable for zone lookup, and compares
terrain-only component IDs. Different components are skipped without RNG draws.
The final gate still uses the unmodified restriction threshold greater than.05,
plus occupancy; the last rejected candidate can still be forced. The cache is
shared across owner/row lookups. Workers/scouts have hillMode0, so slopes alone
do not reject them. Original tests cover1802 terrain/slope gate calls and108
component queries in12 water/hill/forest contexts; explicit cliff objects remain
unsupported.

The full native live-Forest starting prefix now matches the original replay:
terrain/elevation/slope fingerprints,7554 trees,10 owned objects with initial
poses/animation holds, and final RNG302719008. A fresh read-only Windows process
inspection independently confirms both CCs and all four local mobile units' IDs,
types and positions. Enemy mobiles had moved or changed work variants under AI,
so their current positions are not used as startup evidence. Two smaller authored
map fixtures compare original scenery/object-stage execution on separately
validated native geometry, including every tree and owned pose.

Private evidence: `rms-live-forest-owned-fullstage-oracle-2026-11-14.json`,
`live-forest-starting-objects-comparison-2026-11-14.json`,
`rms-owned-connectivity-findings-2026-11-14.txt` and authored Forest21 oracles.

Connection teams follow the original directional rule: inherit the first earlier
player regarded as allied, otherwise create a new ordinal. It is not a union of
mutually allied groups. The helper matches4096 directed four-player matrices and
100 additional mixed-state cases. Ten full parser/scheduler comparisons now
cover mutual allies and directed four-player groups. Optional diplomacy is
accepted for randomized geometry; allied owned startup remains unverified.
Fixed starts reject any directed playable-player alliance, including alliances
that do not merge connection ordinals, because original fixed placement changes
in those cases. The packaged corpus uses only the implemented63/64 commands.

### Nerf startup rows

The ordered flat120-square resource prefix now extends through two original
Nerf594 rows: four near each start, then two groups of two farther away. Each
constructor consumes seven CRT draws, including initial animation and heading.
Original collision queries show that trees, deposits and Command Centers block
Nerfs, whereas workers, scouts, holocrons and existing Nerfs do not. Row-specific
blockers preserve that distinction while retaining earlier resource-spacing
exclusions. Native import restores the initial animation frame and hold.

Four original twelve-row compositions verify64 animals and each row's RNG.
These establish startup placement/appearance, not later animal wandering,
capture, nursery or complete gameplay parity. Other herd types, hunting animals,
predators and the remaining packaged object rows are still open.


Reference-harness audit: earlier low-ID master loaders resolved null task-header
pointers. The current bounded twelve-row startup was therefore replayed with all
ten type70/80 master task pointers deliberately unmapped and instrumented. Both
forest seeds made zero accesses to those fields and reproduced all91 creation
records, states/RNG and eligibility exactly. The original controller's max-X/Y
fields were also corrected to119; both flat resource fixtures remained identical.
This establishes task-header independence for the tested startup path, not for
later AI ticks. Live Windows header reads identify the real task-list vtable as
667414 and provide counts for workers, CCs, holocrons, scouts and animals. Full
header loading is being added before extending behavior beyond that boundary.


### Mixed lands, wet resources and direct Windows object comparison

The parser now preserves source-order land centers, owners and land IDs, accepts
mixed player/neutral blocks, and runs the original implicit neutral-land sampler
before scheduler execution. It retains separate growth zones, protection and
connection ownership.96 sampler fixtures and20 complete geometry cases match
original results, including fixed enemy starts and random placement. Mixed owned
startup and allied fixed starts remain guarded.

Grouped resource placement now checks the start-connected terrain component and
the original eight-point zone7 clearance (cardinal7/diagonal5, clipped to map
edges). Persistent spacing excludes only terrain with positive restriction
values; impassable water cells retain eligibility. Authored hill/water maps
compare all resource poses, per-row RNG and the full persistent mask. Holocrons
use their actual passability gate on Forest21 terrain. These expanded profiles
remain bounded to the validated120-square two-Empire context.

A fresh read-only Windows comparison exposed a source-profile mistake: the
selected Forest script requests12 forage objects per player, whereas the earlier
controlled resource profile requested6. That fixture remains a valid diagnostic,
but is not the live map's resource sequence. The native parser/helper now support
the verified12-object variant, and the live comparison uses the **exact confirmed
preprocessed token stream**, removing only object rows beyond the tested prefix.

The resulting10-row prefix matches all77 existing static objects read from
Windows (original IDs7564..7640):5 holocrons,24 forage,30 Nova and18 ore. Types,
Gaia ownership and coordinates match. Every terrain/elevation byte also matches
the saved Windows grid, including the local Command Center foundation:

- Terrain SHA256: `14ea7ec5f7aefd0290f93134bdccbe914776acc6219bbaaba786a3b7b08ab5cf`
- Elevation SHA256: `f87e22490633baee990897fbdf14f1720b27f6af35f6afc78787f3703e771690`

Native prefix final RNG1692931439 matches the original replay boundary; it is not
a read of the current live CRT state. Evidence:
`live-forest-actual-resource-comparison-2026-11-14.json`,
`live-resource-prefix-read-2026-11-14.json`, `live-resource-tail-read-2026-11-14.json`
and `native-live-forest-actual-10-rows-2026-11-14/`. All remain local.

The separate FOODC3 hunting variant now supports Kaadu1363/Eopie1365 with original
quantity variance, eight constructor RNG draws and initial animation hold.
Eight original cases cover60 animals, and native imports check IDs, position,
phase and food amounts. This row is **not** the next selected live-Forest row:
that map instead continues with Nerfs, predators and other object rows.

The task-header audit has since been repeated with actual original-loaded task
headers and corrected controller bounds on all four flat reference contexts.
All91 creations, RNG states and eligibility remain identical; the bounded
startup-context limitation is closed. Later animal AI behavior is separate.

The exact selected Forest prefix now includes both Nerf rows:103 explicit
objects plus7554 automatic trees,7657 total. Its terminal RNG154380403 matches
the original replay. This extends startup parity through row12 of23; the
remaining predator, global resource, tree and decorative rows are not implied.


### Selected Forest predators and global deposits

The verified selected-script prefix now reaches row15:107 Gaia resources/animals,
10 owned starting objects and7554 automatic trees. Every Gaia unit ID, coordinate
and facet matches the original replay; terminal RNG is2463465860. Four Rancors
use original ungrouped mode0, including its final-candidate gate fallback. Global
Nova/ore use a full-map candidate queue and exclude the strict axis-aligned square
around every land center; they do not use per-player zone7 clearance. Spacing
preserves the shared queue and persistent passable-tile mask.

A fresh read-only comparison with Windows seed16160 independently matches all ten
global deposits (IDs7661..7670), bringing directly compared static resources to87.
Animals are checked against original startup replay because live animals can move.
Evidence remains local in `native-live-forest-actual-15-rows-2026-11-14/`,
`live-global-resources-comparison-2026-11-14.json` and the selected full-stage
oracle. Thirty small original global-placement cases cover spacing, variance,
partial exhaustion, bounds and rollback. Later explicit trees/decorations and
other maps/seeds remain separate verification work.


The selected prefix now reaches row17:117 Gaia objects,10 owned starting objects
and7554 automatic trees (7681 total), terminal RNG1616468694. All117 Gaia
unit IDs/coordinates/facets match original replay. Fresh Windows reads also match
all ten explicit trees (IDs7671..7680). Tree413 placement uses mode0 with spacing2,
restriction11, a single facet draw, and collisions against every retained object
class, including mobiles and holocrons. Four authored-water row cases compare
positions, facets, row RNG and complete persistent eligibility, including work-limit
rollback. Rows18..23 remain unsupported; startup parity is not animal-AI parity.


Row18 is now verified: eight flying816 birds (six scaled by120*120/10000),
125 Gaia objects and7689 total native objects, terminal RNG2235827064. Original
4dddf0 scales group count by map area and clamps to one; this profile remains
bounded to120-square maps. Each bird consumes one heading draw, starts at frame0
with a0.1265-second hold, ignores ground occupancy, and preserves eligibility.
227 original gate probes cover all53 terrain IDs and retained object centers.
All125 Gaia positions/types/facets and the terminal RNG match original replay.
Private evidence: `rms-live-selected-forest-bird18-fullstage-oracle-2026-11-14.json`,
`rms-bird-gate-checks-2026-11-14.json`, `rms-object-row-scale-4dddf0-2026-11-14.asm`.
The next unsupported row is the global grouped predator row19.


### Complete Forest tail and evidence

The final rows include scaled global Rancor groups, variable Eopie herds, the
empty wave row,100 globally scattered413 trees and four area-scaled69 fish.
Fish require cardinal contact with shore2/35 (diagonal contact fails), pass their
terrain restriction and consume three constructor draws. Their native idle delay
is the original animation hold minus the normal frame duration. Birds likewise
start with no extra idle delay; the normal frame timer already supplies that hold.

Private evidence: `rms-live-selected-forest-full23-fullstage-oracle-2026-11-14.json`,
`native-live-forest-actual-23-rows-2026-11-14/comparison.json`, and
`live-complete-forest-static-comparison-2026-11-14.json`. The full replay uses the
original object controller, constructors, real-loaded masters/headers/graphics,
retained world objects, zone/occupancy gates and persistent spacing state.
Terrain/elevation for that replay are independently matched to Windows. The
original executable and all extracted game records remain local.

### Controlled positive wave placement (2026-11-14)

A separate original-executable probe changes water terrain 1 to 22 immediately
before the selected Forest wave row. This is a controlled fixture, not an
unmodified packaged seed. The native aquatic helper matches all eight original
wave positions, animation frames, facet 7 and zero extra idle delay. Entry RNG
3803376597 becomes 4166964575, and the persistent eligibility mask is unchanged.
`tools/rms_wave_checks.h` retains these expected outputs and input rasters.
The full map terrain whitelist remains unchanged because earlier startup and
resource rows on terrain 22 have not yet been compared. Unsupported wave
quantities still fail atomically.

### Controlled smaller forage groups (2026-11-14)

The same original full-stage replay also ran with the forage59 quantity changed
from twelve to three per owner, keeping all other 23-row Forest inputs unchanged.
The native parser and shared placement helper now accept this verified quantity.
The packaged Forest test preprocesses the user's script, changes that single
quantity and compares all 220 resulting Gaia object poses in creation order,
unchanged terrain/elevation grids and final RNG 278519141. This checks the random
sequence's downstream effects, not just the six forage plants. Quantity two
remains an atomic rejection in the shared helper test. This controlled variation
does not establish another complete packaged seed or arbitrary group quantities.

The eight additional packaged seeds audited (1, 2, 3, 42, 12345, 16161, 32767,
65535) still reject at explicit profile boundaries. Seeds 16161 and 32767 now
pass the three-forage row and reach later unsupported rows. The current complete
packaged comparison remains seed 16160 only. The additional data inventory
identifies hunt units 822/1249, predators 812/1197 and explicit trees 348/349 as
next comparison targets; their original placement/constructor behavior is not
established by reading their metadata alone.

### Controlled Nerf quantities (2026-11-14)

Original full-stage replays on the verified Forest geometry, with three forage
plants per owner, now cover the packaged smaller and larger Nerf variants:

| Near herd per player | Far herd per group (two/player) | Total Gaia objects | Final RNG |
|---|---|---|---|
| 2 | 1 | 210 | 4161792159 |
| 4 | 2 | 220 | 278519141 |
| 8 | 4 | 235 | 2125502712 |

The packaged-script regression compares every resulting Gaia pose in creation
order, all Nerf animation frames, zero extra idle delay, terrain/elevation grids
and final random state. These are controlled object-stage variants, not three
new seed matches. The shared helper still rejects unverified quantities such as
three Nerfs, preserving RNG and the eligibility mask on failure.

### Controlled predator species (2026-11-14)

Original DAT master, graphics, sound and task loaders now provide runtime records
for 812, 822, 1197 and 1249. Controlled full-stage probes substitute predator 812
or 1197 for both Rancor rows in the three-forage Forest variation. Both original
runs produce 220 Gaia objects and final RNG 278519141. Their positions and random
consumption match the Rancor variation, while their animation frame duration
(0.1 seconds) and replay delay (1 second) change startup animation state.

The native parser/helper and packaged regression cover these two predator
substitutions, all resulting object poses and the eight predators' original
frames/extra idle delays through native import. The import regression caught and
fixed the old species whitelist discarding these new predators' animation state.
This does not establish arbitrary
predator quantities or alternative terrain profiles. Hunt species 822/1249 have
loaded runtime records but still need placement/constructor comparisons.

The paired global 1197 row requested by packaged seed 32767 has a separate
controlled comparison: four scaled groups of two, after the same owned/resource
prefix. The original produces 224 Gaia objects and final RNG 44224755. The
regression checks all poses plus twelve predator startup animations (four near
players and eight global), including native import. Other predator species still
accept only the verified single-object global groups. This adds an individual
row profile; seed 32767 still needs its hunt animals and explicit tree variant.

### Large hunt placement and clearance (2026-11-14)

Controlled original thirteen-row prefixes retain the selected Forest geometry,
forage3 and both stock Nerf rows, then substitute a large hunt row at distances
16..22. The local player remains1. All later elevation/connection sections remain
in the native RMS input even though they occur after OBJECTS in the source.

| Unit | Placement per player | Gaia objects | Final RNG |
|---|---|---|---|
| 822 | loose group of2 | 79 | 1777230500 |
| 822 | loose group of4 | 83 | 1427887360 |
| 1249 | one ungrouped object | 77 | 3550983994 |
| 1249 | two ungrouped objects | 79 | 1585104208 |

The original rejects blocking objects in the surrounding3x3 tiles and rejects
outermost map tiles. These differ from the previously supported small animals.
Both large species consume seven constructor draws, versus eight for the small
hunting species. `tools/rms_hunt_profile_checks.h` checks terrain/elevation, every
Gaia pose, terminal RNG, animation frames/extra idle delays and native import.
The prefix still cannot continue with arbitrary additional hunt rows; the distant
35-tile profiles and the full alternate packaged sequences remain unverified.

### Distant large hunt rows (2026-11-14)

The original command handler decodes the packaged distant large-hunt request as
loose count3, variance1, one group/player, minimum35, maximum-1 and spacing5.
Controlled original thirteen-row prefixes for 822 and1249 each produce five
animals, 80 Gaia objects total and final RNG3597322543. Their differing terrain
restrictions change the positions in the second group. The original checks
passability across the full3x3 clearance footprint: checking just the centre tile
incorrectly admits animals next to impassable water. The distant fixtures catch
that error while the near-start fixtures retain neighbour and edge coverage.

The helper uses a full-map outer candidate queue for this no-maximum profile,
then applies player exclusions, connected zones, spacing and large-animal
clearance. The parser validates the complete distant profile, including explicit
variance and spacing. Both original results join the packaged-data regression
with every pose, startup animation and native import checked. This supersedes
the preceding note about the35-tile profile; arbitrary row sequences remain
unsupported.

### Alternate large hunt grouping (2026-11-14)

Two additional original prefixes verify the other grouping forms present in the
audited scripts: one ungrouped822 per player (77 Gaia objects, RNG3550983994),
and a loose pair of1249 per player (79 Gaia objects, RNG1777230500). The parser
selects the profile from the grouping command and distance; absence of the loose
command keeps the ungrouped path, which consumes no group-variance draw. The
regression now contains eight large-hunt profiles with original pose/animation
expectations and native import. Quantities and row sequences outside these
verified profiles remain rejected.

The final eight-seed audit still rejects every additional complete script. Seeds
12345 and32767 now pass their nonstandard Nerf quantities but stop at the extra
hunt rows after the existing prefix; 65535 reaches its unverified small-hunt
quantity. Seeds1/16161 also need the branch that omits Nerfs. Next integration
work must preserve those script sequences, verify small-hunt count8 and explicit
trees348/349, then repeat complete original-executable and live Windows seed
comparisons. Individual row parity is not complete-map parity.

### Small hunt quantities on selected Forest geometry (2026-11-14)

Four original thirteen-row prefixes now cover both1363 and1365 after forage3 and
the stock Nerf rows on the complete selected Forest geometry. Count4/variance1
produces seven animals (82 Gaia objects, RNG2967105290); count8/variance1 produces
sixteen (91 Gaia objects, RNG3099826770). The native comparisons check every
pose, original startup frames/idle delays, grids and native import. The combined
hunt-profile checks are now in `tools/rms_hunt_profile_checks.h`; the separate
historical row20 fixture in `tools/rms_forest_hunt_checks.h` remains intact.
These prove the terminal row profiles, not sequences that omit or reorder Nerfs.

### Explicit tree species348/349 (2026-11-14)

Original full23-row replays preserve all7554 automatic413 trees and substitute
348 or349 only in the two near-start and one global explicit-tree rows. With
forage3, both replays create110 explicit trees,220 Gaia objects total and end at
RNG278519141. The original loader confirms12 facets for348 and11 for349, each
with100 carbon instead of413's125. The native regression compares every object
pose/facet/order, grids, RNG and imported explicit-tree positions/carbon.
Existing grouped348 profiles remain separate from these mode0 rows. This does
not yet establish owned startup on automatic348/349 forest terrain18/20.

### Nerf-free small-hunt prefixes (2026-11-14)

Four original eleven-row replays omit both Nerf rows before the terminal1363 or
1365 hunt. On the same selected geometry with forage3, count4 creates eight
animals (67 Gaia, RNG2951609664); count8 creates sixteen (75 Gaia,
RNG4160012160). The existing hunt-profile regression checks all object poses,
grids, original animation startup and native import. The parser accepts these
verified prefixes and still rejects their unverified later object rows.
This is controlled sequence evidence, not a second complete packaged seed.

The rebuilt CLI still rejects all eight additional audited seeds
(1,2,3,42,12345,16161,32767,65535). Seeds1/16161 now get past the omitted-Nerf
prefix, and65535 gets past its count8 hunt; they stop at later, unverified rows.
Seeds3/42 still fail the earlier expected resource profile. No audit rejection
was converted into an approximate map.

### Automatic forest18/20 startup (2026-11-15)

Controlled original-executable replays relabel the independently verified
selected Forest21 grid to18 or20, load the actual348/349 prototypes, and run
original automatic scenery plus all23 object rows. Each creates7554 automatic
trees,10 owned starts and220 Gaia objects with forage3, ending at RNG278519141.
The native fixture swaps terrain IDs in the source and connection rules to
preserve the controlled geometry, then compares grids, all tree facets/order,
Gaia poses and imported tree positions/carbon. Forest20 uses11 tree facets;
Forest18 uses12. Owned startup and resource validation now admit these species.
This proves controlled startup on those terrains, not the terrain-generation
stages of another packaged seed or mixed-owner/civilization configurations.

### Hunt-to-predator continuation (2026-11-15)

Two original controller replays insert a1365 loose count4/variance1 hunt before
the near predator and retain all later rows. On controlled Forest18 with both
Nerf rows, the24-row sequence produces226 Gaia objects and RNG2914264210.
On controlled Forest20 without either Nerf row, the22-row sequence produces211
Gaia objects and RNG2709380116. Both retain7554 automatic trees and10 owned
starts. The regression checks all Gaia poses/order, grids, tree facets and
native tree import. The parser tracks the resource profile independently of
physical row position, allowing one hunt then predators and the stock suffix.
Additional consecutive hunt rows remain rejected pending sequence evidence.
These remain controlled sequences on relabeled selected geometry, not complete
seed1/2/16161 parity or a fresh live Windows comparison.

### Forest branch without forage (2026-11-15)

An original22-row replay omits the forage row on controlled automatic Forest18
and retains the two Nerf rows and all later resources. It creates213 Gaia
objects and ends at RNG3794078108. The native regression checks complete object
poses/order, Nerf animation frames, automatic tree facets, grids and tree import.
The parser can begin the resource profiles at the near-Nova row when forage is
absent. This targets the initial rejection seen in packaged seeds3/42; their
longer hunting sequences and complete terrain generation remain unverified.

The final eight-seed audit still rejects every additional complete map. Seed42
now reaches the longer hunt-chain guard; seed3 reaches an unverified1365
count2/variance1 row. The other six stop at additional consecutive hunts.
Rejected scripts produce no map image. Full packaged parity remains limited to
the previously verified seed16160 configuration.

### Two-Eopie hunting groups (2026-11-15)

Original terminal1365 count2/variance1 comparisons cover the selected geometry
with and without the preceding Nerf rows. With Nerfs they create four Eopies
(79 Gaia total, RNG891822066); without Nerfs they create three (62 Gaia,
RNG3061237336). Regression coverage includes all resource poses/order, original
animation frames, delays and heading, unchanged grids and native import.
This verifies the quantity that blocked seed3, not that seed's complete layout.

### Three-row hunting prefixes (2026-11-15)

Full original controlled sequences now cover1365 count4 followed by822 loose4
and1249 two singles on Forest18, and1365 count4 followed by1249 loose2 and822
one single on Forest20 without Nerfs. The original26/24-row runs retain all
later rows and produce238/219 Gaia objects, ending at RNG1393824384/2530534559.
The native fixture compares all object poses/order, grids, tree facets and
native tree import. The parser permits an initial hunt followed by at most two
large-hunt rows before predators; quantities and grouping remain validated.
This is sequence evidence on controlled geometry, not complete packaged seeds.

### Late hunting rows and distant groups (2026-11-15)

Three original complete controlled sequences extend the three-row hunt prefix:
late two-Nerf group plus distant1249 on Forest18 (242 Gaia, RNG1364577121),
late single822 plus distant822 on Forest20 without Nerfs (220 Gaia,
RNG734846818), and no late near hunt plus distant822 (219 Gaia, RNG3194867101).
Each distant group uses count3/variance1, distance35 and spacing5. The native
fixture compares all resulting Gaia poses/order, grids, automatic tree facets,
Nerf animation frames and imported tree state. The parser retains the stock
suffix after these optional rows and rejects repeated distant groups.

The eight-seed audit advances seeds2/12345 past object parsing to the startup
terrain guard. The other six still reject specific quantities or species:
1249 loose count1 near/far; late1363 count3; or global126 pairs. These are not
new complete packaged parity results; seed16160 remains the only such result.
The reported startup terrain is9 for both seeds2/12345. Explicit CLI checks
also reject a fourth prefix hunt, a second distant hunt and a near-only group
in the distant slot without writing an output image.

### Mixed Forest terrain9 (2026-11-15)

A full original23-row controlled Forest18 run replaces the selected terrain5
patches with9. It retains220 Gaia objects and RNG278519141; all Gaia poses and
Nerf frames match the control, while the terrain hash changes to3296036497.
The native source fixture swaps the same terrain IDs in generation/connection
rules and checks the resulting grid, scenery, resources and import. Startup
now permits terrain9. Seeds2/12345 advance to their next guard, terrain22;
they still do not constitute complete generated or verified packaged maps.

### Water22 startup and wave/fish occupancy (2026-11-15)

The first water22 test incorrectly assumed that relabelling a completed water1
raster commuted with the fixed water1 smoothing pass. Four shoreline tiles
contradicted that assumption. The corrected oracle executes the original
object controller on an explicitly native-supplied pre-object grid; its scope
is object startup/placement, not independent terrain-generation parity.
It produces228 Gaia objects and RNG3038787869, including eight waves with
frames1,21,46,47,34,22,1,1. The regression checks all poses/order, frames and
native animation import on that supplied geometry.

This exposed an actual occupancy bug: native waves blocked a later fish,
placing it at83.5,86.5 instead of the original86.5,86.5. Original fish can share
a wave's tile. Waves now stay out of the shared blocker list, while validation
admits water22 through the mixed Forest startup/resource path.

Packaged seeds2/12345 complete native generation, with7860/7982 objects and
RNG2364297847/3553717540 in the diagnostic audit. These outputs still require
independent full original generation comparisons. The other six audited seeds
remain rejected for the specific quantities/species described above.

A fresh read-only Windows check confirms the expected executable SHA256 and
map_seed16160. All197 static resource records (unit/owner/position) in the
original ID range match the retained original reference; mobile animals and
later-created objects were excluded. This reconfirms the old live baseline,
not either newly generated seed. The Windows game was not restarted or edited.

### Remaining Forest hunt quantities (2026-11-15)

Three original controlled thirteen-row prefixes verify loose1249 count1 near
and far, and Kaadu1363 count3. They produce respectively77/77/80 Gaia objects,
with final RNG1885517358/1126655240/2341451770. Native regressions check all
poses/order, animal frames, extra idle delay and heading against those original
records. The parser and resource validation admit these quantities, including
the late Kaadu row. The complete ASan/UBSan skirmish suite passes. These are
controlled object comparisons, not additional complete packaged-map proofs.

### Paired global Rancors (2026-11-15)

The original controlled Forest18 sequence with global126 groups of two yields
224 Gaia objects, pose hash636226797 and final RNG44224755. Native parsing,
placement and import now accept that verified group size; the full-stage
fixture checks the terrain, automatic trees, object order/poses and Nerf frames.
The eight-seed native audit now generates seeds2/3/12345; remaining rejections
include near822 count1, distant822 count6 and base12 startup. Seed3 is not yet
an independently verified complete original map.

### Live Windows seed2 raster and static placement (2026-11-15)

The unmodified reference executable restarted with map seed2, set through its
configured-map-seed field at app+0x48; that field was restored to UINT_MAX after
generation. All14,400 terrain, elevation and slope cells match the native map
exactly. FNV hashes are4035986315/3707759835/3939991058. Command Centers are at
47,17 and17,81. All202 checked static resource IDs/types/owners/positions match
the native sequence; the corresponding position hash is582877992. The new
regression passes the full ASan/UBSan skirmish suite.

This does not certify the full seed: animals had already started moving.
A separate appearance comparison found19 facet differences (18 forage59, one
tree348); startup versus later runtime changes remain under investigation.
The old seed16160 save remains available; no executable on disk was patched.

### Single near and six-animal distant Bantha groups (2026-11-15)

Original controlled prefixes verify near822 loose count1 and distant822 count6
with variance1, minimum distance35 and group spacing5. They yield77/87 Gaia
objects and final RNG1885517358/4044577126, with pose hashes880263998/660531056.
Native admission is limited to those verified near/far quantities. Regression
fixtures compare all poses, startup frames/delays/headings and native import;
the full sanitizer suite passes.

### Live Windows seeds3/12345 and simulation appearance boundary (2026-11-15)

Both additional live Windows restarts match all14,400 terrain/elevation/slope
cells. Seed3 hashes are1730168867/94274854/3441657481; seed12345 hashes are
519134464/169086623/2102275482. Their180/193 static resource identities, owners
and positions match in creation order (hashes195710362/4181115470). The native
regression now covers seeds2/3/12345 and passes ASan/UBSan. Each temporary
configured map seed was restored to UINT_MAX after generation.

The forage appearance discrepancy is a simulation boundary, not evidence of
a mismatched generation draw: original0054ef00 changes zero-HP forage59 from
state2 to3 on its first update and draws a fresh random facet, even when the
graphic stays unchanged. An original-code replay verifies this on six bushes;
a fresh Windows seed2 capture confirms state3/HP0. The controlled constructor
facets remain unchanged through the end of object placement. Do not replace
those generation fixtures with later live facets or consume the simulation
draws inside map generation. Matching later appearance still requires the
original simulation RNG initialization and update ordering; it is not claimed.

After the Bantha quantity fix, six of the eight audited seeds generate natively
(1,2,3,42,12345,16161). Seed32767 still rejects base12 startup;65535 rejects
distant1249 count6. Newly accepted seeds1/42/16161 need live comparisons.

### Base12 starts and distant1249 count6 (2026-11-15)

An original full23-row controlled Forest18 run swaps terrain6 and12. Startup,
resources and cleanup retain220 Gaia objects, pose hash3933298748 and final
RNG278519141; the final raster hash is2620373331. Native mixed Forest startup
now admits base12 with matching player-land terrain. A separate original
prefix verifies distant1249 count6 with variance1 and spacing5:87 Gaia objects,
pose hash2226817864 and RNG4044577126. Both retain strict existing prototype
and other startup validation.

Live seed32767 independently matches every terrain/elevation/slope cell after
this startup change (hashes1119165674/3881025133/4054086905). Seed65535 also
generates natively. All eight audit seeds now generate, although live proof
remains scoped to the captured grids and static objects.

## Additional Forest quantities and live seeds: 2026-11-15

Controlled original 13-row prefixes independently verify ungrouped Bantha822
count2, loose hunt1249 count4, and Kaadu1363 count2. Each uses the original
16160 geometry, forage3, both Nerf rows, and actual executable factories.
The fixtures compare all preceding Gaia positions/facets/order, final RNG,
and terminal animal frames, idle delays and headings through native import.

| Terminal row | Final RNG | Gaia pose/facet FNV | Terminal animals |
|---|---:|---:|---:|
| Ungrouped822 count2 | 1585104208 | 3911607922 | 4 |
| Loose1249 count4 | 1427887360 | 99082249 | 8 |
| Kaadu1363 count2 | 891822066 | 1187128642 | 4 |

Private evidence is `rms-controlled-{bantha-ungrouped-two,near1249-four,kaadu-two}-fullstage-oracle-2026-11-15.json`.
Live Windows evidence additionally covers seeds1,32767,65535, using the same
read-only raster/object readers and seed guards as the earlier comparisons.
No later live facet is treated as a constructor reference. The broader seeds0–31
audit found 18 supported seeds before these quantity fixes; that audit is coverage
of native generation, not independent parity evidence. Remaining rejections include
predator quantities, terrain4 and other hunting-row combinations.

## Predator pairs and distant single Bantha: 2026-11-15

An independent full23-row original replay verifies global812 pairs:224 Gaia
objects, final RNG44224755, pose/facet FNV3286888937. All12 predator frames and
idle delays match the existing1197 pair fixture, now checked for812 as well.
A controlled13-row distant822 count1 replay yields77 Gaia objects, final
RNG1126655240 and pose/facet FNV3564246435. It retains group variance and
constructor RNG, including the original's two resulting animals.

Private evidence: `rms-controlled-predator812-pairs-fullstage-oracle-2026-11-14.json`
(the inherited date in this filename is historical; this replay ran November15),
and `rms-controlled-far822-one-fullstage-oracle-2026-11-15.json`.
Live seeds42 and16161 also match all terrain/elevation/slope bytes and respectively
172 and178 static resources. The temporary in-memory map-seed override was
restored to UINT_MAX after the eight-seed comparison; no executable was patched.

## Terrain4 and omitted early herds: 2026-11-15

The original controller replay accepts the Forest grid with terrain1 replaced
by4, retaining forest18 and its348 trees. The216 Gaia objects, final
RNG2887979497 and pose/facet FNV2551677122 differ from water1 because fish
are no longer placed there. The fixture checks native terrain FNV2438347840,
all object poses/order and unchanged Nerf animation FNV513271022. This
controlled object replay alone does not establish terrain4 generation parity.

A separate original21-row replay omits both early Nerf rows and goes directly
to near812 predators. All later rows execute:203 Gaia objects, final
RNG1423244924, pose/facet FNV1233270215. The native fixture additionally
checks all eight predator frames/delays and their import into the game.
The parser enables precisely the observed812 transition; other unverified
no-herd species sequences remain rejected.

Private evidence: `rms-controlled-forest18-terrain4-native-input-fullstage-oracle-2026-11-15.json`
and `rms-controlled-predator812-no-herds-fullstage-oracle-2026-11-14.json`
(the latter retains its template's historical date; replay performed November15).

The initial synthetic terrain4 experiment relabelled an already generated water1
grid. Four shoreline cells differ from generating the modified RMS itself; it is
retained separately with terrain FNV85196936. The committed fixture uses a second
original object-stage replay on the native pre-object grid (restoring the local
Command Center's16 foundation cells to their saved original terrain). It reproduces
the same object poses/RNG and terrain FNV2438347840. Independently, live packaged
seed13 validates terrain4 generation against Windows, and seed19 validates the
no-early-herd branch. Both match all three grids and their static resource records.

The expanded native audit now accepts all256 seeds0–255 under the stated Tiny
Forest/two-Empire configuration. This is a generation coverage result; only the
independently compared seeds and controlled branches constitute parity evidence.

## Full live automatic scenery and distributed seed audit: 2026-11-15

The read-only Windows object reader now batches its memory reads in C# rather
than making thousands of PowerShell calls. It verifies the executable hash, image
base, table bound and each object ID before serializing records. It made no game
or system changes. Every automatic tree matches native identity, owner, creation
order, position and sprite facet in three fresh live maps:

| Seed | Tree type | Trees | Live pose/facet FNV |
|---|---:|---:|---:|
| 19 | 348 | 7622 | 3968719120 |
| 32767 | 349 | 7687 | 1594315501 |
| 65535 | 413 | 7639 | 1630944130 |

The six stationary starting objects also match in each capture. Three AI workers
and the AI scout have already moved or changed jobs, so these captures do not
certify their original startup pose. The committed fixture checks the full tree
hashes independently of the static-resource hashes. Private evidence is
`live-seed{19,32767,65535}-full-scenery-comparison-2026-11-15.json` and the complete
original object snapshots beside it. The seed override was restored after capture.

Consecutive seeds0–255 all generate, but a reproducible Python Random(0x53475742)
sample of256 unsigned32-bit seeds initially generated189. The remaining67 expose
domesticated Bantha833, Ronto1364, explicit411 trees, and1197 predators without early herds.
These are unsupported branches, not certified mismatches. The distributed audit
is retained as `forest-spread256seed-audit-2026-11-15.json`.

A further original21-row replay verifies1197 with both early herds omitted:
203 Gaia objects, final RNG1423244924, pose/facet FNV639490319. Its eight predator
frames/delays equal the independently checked812 no-herd sequence. Evidence is
`rms-controlled-predator1197-no-herds-fullstage-oracle-2026-11-14.json` (template
filename date retained; execution on November15).

## Explicit411 trees and forest10: 2026-11-15

Actual original411 masters/graphics verify near, distant and global explicit
tree rows on forest18 and forest10. Each complete23-row replay creates220 Gaia
objects, ending at RNG278519141 with Gaia pose/facet FNV2615402066. The forest10
raster hash is1196292149; a separate original automatic-scenery capture retains
all7554 records before starting objects. The native fixture checks its full hash
as well as complete object placement and game import. Tree411 uses restriction4,
graphic5292,12 angles and100 carbon, distinct from413's restriction11/125 carbon.

Private evidence: `rms-controlled-forest{18,10}-explicit411-fullstage-oracle-2026-11-15.json`
and `rms-controlled-forest10-scenery-records-2026-11-15.json`. The forest18 replay's
extra tree-gate diagnostics use348 as a comparison prototype; only its actual411
controller/factory output is used for the411 fixture.

Distributed seed1409540336 now matches live Windows on all terrain/elevation/slope
cells, every static resource identity/position, and all7689 automatic trees including
facets (FNV3285574391). Its script omits both forage and early herds and uses1197
near predators with812 global predators. This is independent live coverage of the
combined optional-row path, beyond the controlled isolated1197 replay.

## Ground41 startup and a live forest10 map: 2026-11-15

The original controlled23-row replay with ground41/player lands41 and forest10
retains220 Gaia objects, RNG278519141 and pose/facet FNV2615402066; its terrain
FNV is415015038. Native admission is limited to the same verified120-square,
two-Empire worker/scout profile. Other startup configurations remain guarded.
Evidence: `rms-controlled-forest10-base41-fullstage-oracle-2026-11-15.json`.

Fresh live packaged seed2912922832 independently matches all14400 terrain,
elevation and slope cells,172 static resources including explicit411 trees,
and all7691 automatic411 trees including facets (FNV3170254571). The helper
and fixture static-resource filters now include411. The live script omits
forage and early herds; later optional hunts are absent too. This validates
its full generated geometry/static placement, not the AI's later unit movements.

## Domesticated Bantha833 herds: 2026-11-15

Original full23-row controller replays replace both early Nerf rows with833,
using its actual original master, unit header, graphics and sounds. This species
has radius/clearance0.35, restriction19,11 animation frames and training sound615;
the native validator retains those distinctions. All following rows execute.

| Near/far quantity | Gaia objects | Final RNG | Pose/facet FNV | Herd-frame FNV |
|---|---:|---:|---:|---:|
| 2/1 | 210 | 4161792159 | 2122489821 | 1095685733 |
| 4/2 | 220 | 278519141 | 2149940932 | 1248656011 |
| 8/4 | 235 | 2125502712 | 2865837869 | 3128446147 |

All8/16/32 Banthas respectively retain zero extra idle delay. The native fixtures
check full object counts/order/poses, RNG, every herd frame, and imported Bantha
identity, position and animation. Evidence is `rms-controlled-forest18-bantha833{,-half,-double}-fullstage-oracle-2026-11-15.json`.
Later optional833 rows and mixed Ronto chains require additional comparisons.

## Ronto1364 placement and constructor: 2026-11-15

Eight original13-row prefix replays use actual1364 master/header/graphics/sound
records, with forage3 and both Nerf rows before the terminal Ronto row. Unlike
822/1249, its collision radius and clearance are0.3; it does not require the
entire surrounding3×3 tile footprint. Native placement now consults validated
clearance for that rejection test. The prototype retains restriction19,8 frames,
0.12-second duration,0.5-second replay delay,400 food and training sound624.

| Row | Requested per group | Rontos created | Final RNG | Full Gaia pose/facet FNV |
|---|---:|---:|---:|---:|
| Loose near | 2 | 4 | 3957987938 | 3952837900 |
| Loose near | 1 | 2 | 261303900 | 3832532001 |
| Loose near | 4 | 8 | 426697310 | 2164337889 |
| Loose far | 1 | 2 | 1126655240 | 98383283 |
| Loose far | 3 | 5 | 3597322543 | 2006571828 |
| Loose far | 6 | 12 | 4044577126 | 2025509104 |
| Ungrouped near | 1 | 2 | 3550983994 | 492535568 |
| Ungrouped near | 2 | 4 | 1585104208 | 863600152 |

The fixture also verifies each Ronto's frame, extra idle delay and heading, then
checks native game import. These are isolated terminal-row comparisons; later
Ronto chains still need full-script/live coverage. Evidence files are
`rms-controlled-ronto1364-{two,near-quantities,far-quantities,ungrouped}-fullstage-oracle-2026-11-15.json`.

## Late domesticated Bantha row: 2026-11-15

The original23-row controller replay also replaces the late Eopie row with
833 count2 near each player, after global predators and before waves/trees/fish.
With both early833 rows retained it creates219 Gaia objects, including20 Banthas,
ending at RNG3202634963. Full Gaia pose/facet FNV is3606605597 and all20 herd
frames hash to210905748. The fixture preserves subsequent placement and verifies
native import. Evidence: `rms-controlled-forest18-bantha833-late-fullstage-oracle-2026-11-15.json`.

Before enabling this row, the same distributed256-seed audit improved from189
to211 after Bantha support and239 after Ronto support. Its remaining17 failures
were late833 rows, retained in `forest-spread256seed-after-ronto1364-2026-11-15.json`.
These counts measure accepted generation; independent comparisons are listed above.

## Combined512-seed audit and mixed live branches: 2026-11-15

All512 inputs (consecutive0–255 plus the fixed distributed256-seed sample) now
generate successfully using the current native exporter. This is acceptance
coverage, not512 independent original comparisons. The retained audit is
`forest-combined512seed-after-late-bantha833-2026-11-15.json`.

Three additional live Windows maps independently match all terrain/elevation/slope
bytes, all static resources, and every automatic tree including facets:

| Seed | Combination | Automatic trees | Tree pose/facet FNV |
|---|---|---:|---:|
| 2666028048 | Early833 herds, Kaadu/Ronto chain, far822 | 7353 | 520122316 |
| 2766341764 | No early herds, late833 | 7551 | 1939473913 |
| 3878309407 | Kaadu/Ronto chain and distant Ronto | 7690 | 608848445 |

These full-script comparisons include forest10/ground41 and retain the same
mobile-AI limitation as earlier live captures. Evidence is the matching
`live-seed*-comparison-2026-11-15.json`, `*-full-scenery-comparison-*` and raw
Windows object/raster snapshots in the private reference directory.

### Two Gungan Forest owners (2026-11-15)

A controlled original-executable replay now covers both owners using civilization2,
including Gungan scout552 and foundation terrain4. On the existing seed16160
Forest geometry with forage3 and predator 812 substitutions, all10 owned objects
and220 Gaia objects match creation order, coordinates and facets. Final RNG is
278519141; terrain FNV is1056424125, owned-pose FNV613198960 and Gaia-pose
FNV1670957952. The existing generator needed only its verified-profile guard
extended to the two-Gungan case; shared placement logic already matched.
Native import also checks all10 starting identities, owners and positions.

Private evidence: `rms-controlled-gungan2-forest-fullstage-oracle-2026-11-14.json`
(replayed November15; filename inherited from its harness). This first replay is controlled object-stage evidence.
At that checkpoint, civilization pairs beyond Empire/Gungans and other map sizes
remained outside this Forest profile; later same-civilization coverage is recorded below. The512-seed acceptance audit and15 live comparisons above
used two Empire owners.

A subsequent live Windows two-Gungan startup at seed16160 also matches all14400
cells of terrain, elevation and slopes (FNV1056424125/908132044/458332910),
all197 static object identities/positions/order (FNV163032056), and all7554
automatic trees including facets (FNV2138192714). Moving AI workers/scout and
first-tick forage facets are excluded from static startup claims. Evidence is
`gungan-live-seed16160-comparison-2026-11-15.json` and its full-scenery companion.
The temporary seed override was restored to the original unset value.

Both mixed Empire/Gungan orders also match controlled original23-row replays:
terrain FNV3348257725 for1/2 and1056424125 for2/1; owned-pose FNV3019737774
and888513054 respectively. Both retain220 Gaia objects, Gaia-pose FNV1670957952
and final RNG278519141. Fixtures also verify Gungan scout animation frame7 and
normalized initial hold times, including native import. Private evidence:
`rms-controlled-mixed-gungan-forest-fullstage-oracle-2026-11-14.json` (replayed15th).

The same512 native-generation seed cases also complete for two Gungan owners
(`forest-combined512seed-gungan-2026-11-15.json`). This is an acceptance audit,
not512 independent original-engine comparisons.

### Remaining same-civilization Forest pairs

Original23-row object-controller replays also match each same-civilization pair
for civilizations3 through8. Every case matches the complete terrain raster,
all10 owned poses and all220 Gaia poses, including creation order and facets.
Civilization4 changes the scout RNG consumption: final state1059939153 and Gaia
pose hash2499735454; the others retain278519141 and1670957952. Civilization6
uses foundation terrain6 (terrain hash1487786509). The remaining foundations
retain terrain hash3348257725. Regression fixtures verify scout animation and
hold normalization plus native import for each pair.

Private evidence: `rms-controlled-six-civs-forest-fullstage-oracle-2026-11-14.json`
(replayed November15). These six cases are controlled original-executable
comparisons, not live Windows startup captures. All eight same-civilization pairs
are now accepted in the verified Tiny Forest profile; mixed pairs remain limited
to Empire/Gungans. Other sizes, settings, maps and simulation parity are not proven.

The subsequent live Windows two-Wookiee seed16160 map matches all terrain,
elevation and slope cells (1487786509/908132044/458332910), all197 static placements
(hash163032056), and all7554 automatic tree positions/facets (hash2138192714).
Evidence: `wookiee-live-seed16160-comparison-2026-11-15.json` and full-scenery
companion. The seed override was restored; four moved AI owned objects and
first-tick forage facets are treated separately from startup comparisons.

### Mixed foundation/scout coverage and live Naboo check

Controlled original replays now cover every two-owner order among Empire(1),
Gungans(2), Royal Naboo(4) and Wookiees(6), plus the other four same-civilization
pairs:20 pairings total. All match the full terrain raster, starting-object and
Gaia creation order/poses, final RNG, and scout initial frames/hold times.
The regression table retains per-case animation values because changing the
first scout's sequence changes the second scout's random state. Other mixed
civilization pairs remain outside this verified profile.

Evidence adds `rms-controlled-mixed-scouts-forest-fullstage-oracle-2026-11-14.json`
and `rms-controlled-mixed-foundations-forest-fullstage-oracle-2026-11-14.json`,
both replayed November15. The original Empire/Empire fixture supplies the20th
case;19 additional cases exercise native import.

Live Windows two-Royal-Naboo seed16160 independently matches terrain/elevation/
slopes (3348257725/908132044/458332910), all197 static placements (hash3153695046),
and7554 trees including orientation (hash2138192714). This correctly differs
from the Empire/Gungan/Wookiee static placement hash. Evidence:
`naboo-live-seed16160-comparison-2026-11-15.json` and its full-scenery companion.
The temporary seed override was restored after capture.

### Player-land borders

The integrated importer now accepts bounded left/right/top/bottom borders on
`create_player_lands`. Original player-placement results match translating the
existing routine into the border rectangle:190 original fixed/random fixtures
match centers and final RNG. Another42 degenerate fixtures have insufficient
room for the base radius and remain rejected by the native bounded helper.

Sixteen original full geometry runs combine asymmetric20/5/30/10-percent player
borders with PL/LP/PLL/LPL land ordering, both placement modes, two seeds, terrain
painting and connections. Native terrain bytes, final RNG, all land centers and
ownership metadata match. Invalid/empty rectangles fail without changing RNG or
leaving partial output. Bordered owned-object startup remains explicitly gated;
this change does not make another packaged map fully supported.

Private evidence: `rms-bordered-fixed-player-starts-oracle-2026-11-15.json`,
`rms-bordered-random-player-starts-oracle-2026-11-15.json` and
`rms-bordered-mixed-geometry-oracle-2026-11-15.json`.

A fresh46-resource/three-seed audit with Tiny/FIXED/DEFAULT inputs accepts only
Forest (3/138 attempts). Remaining failures include player zones, additional
resource rows, mixed owned-land startup and parser branches. This is native
acceptance evidence, not original-map parity; the input defines are independently
confirmed for Forest only. Results remain in
`rms-packaged-fixed-default-audit-2026-11-15.json` (before this border change).

### Explicit player-land zones

The importer now preserves explicit `zone` values on player lands instead of
rejecting them or overwriting them with owner IDs. Eighteen original full geometry
comparisons cover zone2 and zone-10 (internal zero), player/neutral land order,
fixed/random starts, terrain painting and connections. Terrain bytes, final RNG,
centers and ownership metadata match. Out-of-range zones fail atomically.
Owned-object startup with explicit player zones remains gated pending its own
comparison. Evidence: `rms-zoned-mixed-geometry-oracle-2026-11-15.json` and
`rms-zerozone-mixed-geometry-oracle-2026-11-15.json`.

### Team-based player-land zones

`set_zone_by_team` now uses the original directional rule: for each player, find
the first earlier player whose relation to that player is allied; use that earlier
player's index plus one, or the current player's index plus one if none qualifies.
It neither compacts the zone numbers nor inherits an earlier player's assigned
zone. The existing connection-team numbering therefore cannot be reused.

Evidence combines 32 original parser cases over 1..8 players with 32 original full
geometry cases, including four-player allied pairs and both directions of
asymmetric diplomacy. Native terrain, final RNG, centers and land ownership match.
The directive takes no argument and consumes no RNG. Explicit numeric zones and
team zones obey their source order. Owned startup with either override remains
gated. Random-zone timing is covered in the following section.

Private evidence: `rms-teamzone-parser-oracle-2026-11-15.json`,
`rms-teamzone-mixed-geometry-oracle-2026-11-15.json`,
`rms-teamzone-four-geometry-oracle-2026-11-15.json`. Ghidra handler0x4e2c40
case0x1d corroborates the rule, retained as
`rms-player-zone-handler-ghidra-2026-11-15.json`, evidence
`ev_830e5a531f99f1c3a87bb24ae20dfbb38ab2742af75b931276cce6f6a139a2e1`.


### Random land zones and interleaved script RNG

`set_zone_randomly` consumes one original CRT draw per selected land, assigning
`floor(draw * playerCount / 32767) + 2`. Neutral lands use the same player-count
multiplier. These draws occur during command dispatch, before subsequent script
random choices, and still occur if a later zone directive replaces their result.

The preprocessor now lets the map importer consume command RNG as active tokens
are emitted. The importer retains those zone values for land parsing. Inactive
branches consume nothing; failed map generation preserves the caller's RNG.
Existing preprocessing callers retain their previous behavior without a visitor.

Evidence includes 112 executions of the original dispatcher and land handler
(one to eight players, player/neutral lands, seven seeds), checking zone bytes,
RNG state and the following `start_random` roll. Twenty-four geometry cases cover
player-only, neutral-only and both kinds of random zones. Eight additional
composed cases execute the original subsequent random roll, select the radius
using the previously verified inclusive 50-percent threshold, then execute the
original finalizer and geometry scheduler. These test command/branch interleaving;
they do not execute the original file tokenizer. Native terrain bytes, final RNG,
land centers and ownership match all 32 geometry cases.

Private evidence: `rms-randomzone-dispatch-oracle-2026-11-15.json`,
`rms-randomzone-mixed-geometry-oracle-2026-11-15.json`, the corresponding
`randomzone2` / `randomzone3` files, and
`rms-randomzone-branch-geometry-oracle-2026-11-15.json`.
Owned player starts with zone overrides remain gated until object-stage comparison.


### Live Forest comparison for every original civilization

The seed-16160 Tiny/FIXED/DEFAULT Forest case now has live Windows comparisons
for all eight same-civilization pairs. The additional Rebel Alliance, Trade
Federation, Republic and Confederacy runs match every terrain, elevation and
slope byte, all 7,554 automatic tree positions/facets, and all 197 stationary
resource positions/order. Their terrain/elevation/slope hashes are
3348257725 / 908132044 / 458332910; automatic tree pose hash is 2138192714.
Earlier Gungan and Wookiee comparisons retain their different local foundation
terrain hashes; Naboo retains its different later resource placement.

Captures occur after simulation begins. The AI's three workers and scout have
already changed position/activity, and forage facets change on the first update.
These live runs therefore corroborate raster/static placement, not moving-unit
startup poses or post-startup simulation RNG. Controlled original startup replays
cover initial worker/scout placement and animation separately. The temporary
Windows seed override was restored to 4294967295 after capture.

Private evidence uses the `rebel`, `trade`, `republic` and `confederacy` prefixes:
`<prefix>-live-seed16160-comparison-2026-11-15.json` and
`<prefix>-live-seed16160-full-scenery-comparison-2026-11-15.json`.

A fresh production-CLI audit evaluates raw packaged RMS, including zone/branch
RNG interleaving, over 46 resources and three seeds. Only Forest passes (3/138).
Zone and border parsing now advance affected resources to later unsupported
resource-row/startup cases; this does not establish additional playable maps.
Input settings remain independently confirmed only for Forest. Results:
`rms-packaged-zone-integration-audit-2026-11-15.json`.


Four live mixed-civilization runs also match at seed 16160: Empire/Naboo,
Naboo/Empire, Republic/Confederacy and Confederacy/Republic. All three grids,
7,554 tree poses/facets and 197 stationary resource records match. The two Naboo
mixtures have stationary-resource pose hash 2934879652; the Clone Campaigns
Republic/Confederacy mixtures have 163032056. This checks actual poses rather
than inferring equivalence from a converged final RNG state. The same moving-unit
and first-update forage-facet limitations apply. Private capture filenames use
`empire-naboo`, `naboo-empire`, `republic-confederacy` and
`confederacy-republic` with the comparison suffixes above. The reference game was
left paused and its default seed setting restored; remote sessions were closed.


### All 64 ordered civilization pairs on controlled Forest startup

The remaining 44 ordered pairs pass original-executable replay, extending the
previous 20-pair coverage to the complete 8×8 matrix. The importer no longer
rejects an otherwise supported two-player Forest start because its civilizations
are mixed. The original 1..8 civilization bounds and other map/startup gates remain.

The controlled input is seed-16160 Forest geometry with three forage objects per
player and predator 812 substitutions. Each comparison checks all terrain bytes,
7,554 automatic tree poses/facets, ten owned objects, all 220 later Gaia objects,
final RNG, and both scouts' initial frames/holds. Static Naboo scout sprites have
no dynamic hold field; their native extra delay is zero. These are object-stage
replays on previously verified geometry, not 64 executions of the original RMS
file loader or 64 full gameplay comparisons. Stock live-map comparisons above
provide separate evidence for all eight same-civilization pairs and four mixtures.

The new regression fixtures retain independent original terrain/pose hashes,
RNG states and scout animation values, and exercise native game import. Private
per-pair originals are `rms-pair-A-B-forest-fullstage-oracle-2026-11-15.json`;
`rms-remaining-pairs-native-comparison-2026-11-15.json` records all 44 successful
full-array/object comparisons. The replay's inherited method string describes
an older 17-row boundary; these runs execute all 23 controller rows, as recorded
in their row snapshots and creation logs.

The trace harness now hooks specific entries and observed return addresses
instead of every controller instruction. Complete evidence for baseline pair 1/3
is identical before and after that change; subsequent runs take about 19–24 seconds
instead of roughly 90 seconds. Original instructions and emulated behavior are unchanged.
Evidence: `rms-pair-tracing-optimization-2026-11-15.json`.


### Cliff path stage: original-executable comparison

`planRmsCliffs` matches 90 controlled original-generator cases for ordered map
segment calls and final RNG. Coverage includes flat ground, all seven excluded
terrain IDs, uneven heights, protected starts, curl and spacing, zero-count and
short-length stages, map dimensions not divisible by three, and seeds forcing
32767 at the bucket, initial-direction and curl draws. Sanitized checks also
verify invalid inputs and excessive work leave caller output/RNG unchanged.

The implementation preserves the original two-pass 100-bucket shuffle, reversed
path emission, initial zero-length segment, and the direction helper's unusual
period-three wrapping outside 0..3. On square maps, the original bottom-bound
clearing bug only touches unused coarse-grid rows; native clearing stays within
the logical grid. Rectangular maps are not accepted by this helper.

This is a **path-stage boundary**, not newly playable cliff maps. The original
map virtual+0x20 was replaced by an inert segment recorder in these fixtures.
The real map vtable is 0x66759c: line callback 0x5cbbc0 delegates to 0x493300,
which calls point callback 0x5cba10 and topology worker 0x5cb4b0. Object creator
0x5cb1b0 consumes RNG when an alternate graphic exists, before calling Gaia's
object factory. Those effects must be interleaved with path generation before
integrating the stage into `generateRmsMap`; replaying all paths afterward would
not preserve the original random sequence. Production cliff maps remain gated.

The private oracle is `rms-cliff-path-oracle-2026-11-15.json`, produced by
`rms-cliff-path-probe-2026-11-15.py` plus the build-directory extension probe.
The executable SHA-256 is
`c01d78a039514211a845b41338c6f342e0d9c9b3396c8ff558f05d00ae67420d`.
Generator addresses: 0x4d76b0, initialization 0x4d7700, walk 0x4d79c0,
direction 0x4d7c70 and spacing 0x4d7d50. No binary modification is involved.

A separate private diagnostic skips explicit object blocks while retaining RNG
preprocessing to locate earlier blockers across 46 packaged resources and three
seeds. Its successful geometry-only runs are **not original comparisons or full
map support**. Many other resources stop at cliff generation; elevation/scenery
and connection/forest combinations remain additional blockers. Evidence:
`rms-geometry-only-audit-2026-11-15.json`.

Cliff graphic selection now has a separate verified helper. The original map
constructor registers 20 supported E/S/W/N edge patterns with unit IDs 264–272,
facets and half-tile offsets. Four straight patterns have alternate facets;
only those patterns consume a selection draw, choosing the alternate above
16383. `selectRmsCliffArt` matches 486 original selection cases (all 81 ternary
edge patterns × six seeds, including draws 16383, 16384 and 32767). Unsupported
patterns leave output and RNG unchanged. The original game's Gaia construction
and game-event calls were recorded without effects; their behavior, topology
replacement/destruction and actual object construction still need comparison.
Private evidence: `rms-cliff-art-oracle-2026-11-15.json`.

### Cliff objects and interleaved RNG: complete stage boundary

`generateRmsCliffs` now matches 72 executions of the original cliff stage,
including real Gaia factories, type-10 constructors, destructors, map object
registration and game events. Checks cover surviving unit IDs, facets, positions,
creation ordinals (including gaps from replaced pieces), order and final RNG.
Inputs span flat/water/split-height/protected maps, eight seeds and three
count/length/curl/spacing profiles on 120×120 empty worlds. Zero spacing is
rejected: intersecting paths and pre-existing objects are outside this helper's
verified domain. It is not yet called by the packaged RMS importer.

The earlier inert-callback and graphic helpers remain independent tests. Thirty
real constructor cases for units 264–273 show no constructor draws, a static
sprite pointer, and original Gaia registration. Twelve simple connected-path
cases then match synthetic topology replays with real constructors/destructors
and events. The complete-stage comparison exposed additional behavior that those
simple paths missed: when extending an endpoint, 0x5cb340 runs for all three
non-incoming directions, even absent edges. A path that bends back near itself
can destroy/recreate neighboring straight pieces and consume extra selection
draws. The native implementation preserves that order. Seed 1 diverged at the
38th segment call before this correction; the corrected full-stage fixtures pass.

No source executable was patched. The replay configures world state directly and
models external allocation/time boundaries; it does not run the RMS file parser,
scheduler, later terrain/connection stages, or a complete live match. Native-world
import, DAT compatibility validation at integration, zero-spacing topology, and
later stages interacting with existing cliffs remain work before playable support.

Private evidence: `rms-cliff-constructor-oracle-2026-11-15.json`,
`rms-cliff-real-topology-comparison-2026-11-15.json`, the twelve
`rms-cliff-real-stage-{profile}-{seed}-oracle-2026-11-15.json` baselines, and
`rms-cliff-real-stage-expanded-oracle-2026-11-15.json` (60 additional cases).
The expanded replay also asserts all 14,400 terrain, height and slope bytes are
unchanged by cliff construction; object occupancy is recorded separately.

The settings defaults now match the original reset routine 0x4dfdf0:
count 3..9, length 5..9, curl 36, water distance 2 and cliff distance 2.
Twenty-eight executed handler cases verify direct parameter writes and no RNG:
opcodes 35..39 set count/length/curl, 40 (`min_distance_cliffs`) sets cliff
spacing, and 41 (`min_terrain_distance`) sets water exclusion. Three additional
real-stage default-setting fixtures bring the full-stage total to 75.
Evidence: `rms-cliff-parameters-oracle-2026-11-15.json` and
`rms-cliff-default-stage-oracle-2026-11-15.json`. The final Ghidra evidence bundle
and reusable snapshot are saved locally; the analysis process is closed.

### First composed cliff RMS profile and native import

The initial RMS integration accepted one explicitly bounded cliff-only profile:
120×120, base/land terrain 6 (`DIRT1`), one neutral land at 50%/50%, zero base size and
zero requested land tiles; no elevation, connections or explicit objects.
The later grass-painting extension below permits one bounded terrain-0 (`GRASS1`) row. Original cliff masters/graphics are checked. Other combinations remain
rejected, including normal packaged cliff maps. This supersedes the earlier
statement that the stage helper had no importer caller.

Nine original composed runs include the actual no-player-land setup routine
0x4e0740, scheduler 0x4d6660, land, cliff, empty terrain and empty object stages.
They cover three seeds with original defaults, explicit settings and zero cliff
counts. The native parser also tests equivalent late-section placement. It must
retain setup's permutation/offset draws even without player lands; the original
scheduler-only replay starts at a different RNG boundary. The committed fixtures
compare from script-entry RNG after accounting for actual original setup.
Source tokenization remains native; the original comparison uses resolved module
inputs rather than the original file loader.

`RmsMap::cliffs` preserves surviving creation ordinals. Native import puts these
objects before later scenery, offsets IDs by one for native ID0 reservation, and
retains replacement gaps. Asset-backed checks cover IDs, unit/player identity,
positions and the facet-facing projection. With the authored
`tools/fixtures/cliff-diagnostic.rms`, seed1 produces 29 surviving objects and
final RNG 1150416911. `swgbtool render-rms` renders those original game assets;
the private image is `cliff-diagnostic-native.png` in the external build directory.
This is a generated diagnostic scene, not a playable skirmish or visual Windows
comparison.

The 19 algorithm suites, asset-backed skirmish lifecycle/AI/player-victory tests,
and RMS CLI tests pass. The new integration evidence is
`rms-cliff-scheduler-cases-oracle-2026-11-15.json`. Next integration boundaries
are land/elevation variations, cliff interactions with later terrain/scenery and
connections, starting objects, and native collision/UI comparisons. Empty-world
stage success must not be counted as packaged-map or full-game parity.


### Cliff boundary settings and later grass painting

The composed cliff fixtures now include 24 additional original replays: path
lengths 0/1/2/3, dense cliffs with curl 0/100, spacing 39, and 999 short attempts.
Each runs under three seeds, with both ordinary and late native section order.
This gives 33 original composed inputs and 66 native parser-order checks.
Even attempts producing no objects retain the original random draws.

The importer additionally permits one terrain-0 row painted over base 6 after
cliffs, without player-start avoidance or custom height bounds. Nine composed
original replays cover 30/2000/8000 requested tiles, 1/3/20 clumps, spacing 0/1/2,
clumping 0/10/20 and flat-only on/off. Checks compare every terrain tile via an
ordered hash, surviving cliff unit/facet/coordinates/creation IDs, final RNG and
absence of later scenery. Other terrain types, multiple rows, elevation,
connections and owned starts remain gated for cliff maps.

The first exploratory reference row accidentally selected terrain 3: resolved
row offset +8 is the terrain ID, not offset +4. Those expectations were discarded;
the corrected oracle asserts that only terrain 0/6 is present. This illustrates
why matching RNG and object counts alone does not establish map equivalence.

Private evidence: `rms-cliff-scheduler-boundaries-oracle-2026-11-15.json` and
`rms-cliff-terrain-cases-oracle-2026-11-15.json`, from the same original executable
and configured-world/scheduler boundary as above. This is no additional packaged
map claim and does not establish runtime cliff collision or live Windows visuals.


### Elevation before cliff placement

Cliff import now also accepts one level-1 elevation row on the same neutral
120×120 DIRT1 profile, with no terrain-painting row. The cliff stage receives
the generated height bytes rather than an assumed zero-height raster. Other
levels, empty elevation sections and combined elevation/terrain painting remain
gated pending composed comparisons.

Nine original configured-world replays run setup, land, elevation 0x4d9950,
cliffs, empty terrain and empty objects through the actual scheduler. They cover
400/1000/8000 requested elevation tiles and 8/2/30 clumps under three seeds.
Native tests compare every height and slope byte via ordered hashes, all surviving
cliff records and final RNG. Both section orders are tested, since writing the
cliff section first must not move its execution before elevation.

Private evidence: `rms-cliff-elevation-cases-oracle-2026-11-15.json`. The boundary
remains configured original modules with original constructors and modeled CRT
allocation/time, not original RMS file loading or a live Windows screenshot.

The 8000-tile/30-clump seed-1 import checks the original object-Z values against
native rendering heights in surviving-ID order: 24 cliffs at height 1 and 23 at
height 0. The headless render contains 47 objects with final RNG 2269132341;
`cliff-elevated-native.png` is retained in the external build directory. This
checks height placement, not visual equality to an original Windows frame.


### Combined elevation, cliffs and grass painting

The one-level elevation and single grass-0 terrain row can now be combined on
the same bounded neutral-land profile. This supersedes the combination rejection
above. Twenty-seven original replays cross all three elevation profiles with all
three grass-painting profiles and three seeds. Both native section orders match
the original scheduler's fixed elevation → cliff → terrain order.

The checks cover terrain, height and slope arrays, surviving cliff records and
final RNG. Original terrain painting is also asserted to preserve the preceding
height/slope arrays. The reference row layout remains explicit: terrain ID at
+8, not +4. Local evidence is `rms-cliff-composed-cases-oracle-2026-11-15.json`.
Forests, connections, owned starts and multiple elevation levels still require
further composed evidence; no additional packaged map is claimed.


### Multiple elevation levels before cliffs

One terrain-6 elevation block with levels 1–7 is now accepted before cliffs on the
same neutral-land profile. Multiple elevation blocks remain rejected. Painting a
later grass row is currently verified only with level 1, so higher-level plus
terrain-painting combinations remain gated.

Twenty-seven additional original replays cover levels 2/3/7, all three elevation
size/clump profiles and three seeds. The native checks retain both section orders
and compare full height/slope hashes, surviving cliff records and final RNG.
The level-7, 8000-tile, 30-clump seed-1 case has 33 surviving cliffs with final RNG
2215531909: 16 at height 0, 9 at height 2 and 8 at height 7. Native import checks
original creation IDs/positions and rendering-height hashes at these objects.

Evidence: `rms-cliff-multilevel-cases-oracle-2026-11-15.json`. Original setup,
scheduler, elevation and cliff callbacks run under the same configured-world
boundary as the earlier fixtures. These are authored diagnostics, not packaged
maps or proof of full engine/rendering equivalence.


### Cliff collision during automatic forest placement

A flat terrain-21 (`TIMBERG`, unit 413) row is now accepted after cliffs on the
bounded neutral DIRT1 profile. Elevation plus this forest row remains rejected.
Six original runs cover 2000/8000 requested forest tiles and three seeds. The
native pipeline matches terrain, every surviving cliff/tree record, creation
IDs and final RNG under ordinary and late section ordering.

The original rejects 16/67/50 centered trees in the smaller cases and 104/145/115
in the larger cases. Those rejected placements consume no appearance draw and
allocate no object ID. The native pipeline previously ignored existing cliffs
when generating automatic scenery. It now supplies an occupied-tile mask to the
shared scenery pass, rasterized from verified original cliff collision sizes.
Touching edges do not overlap. Clearance sizes are not interchangeable: that
model rejects up to 57 extra trees in these fixtures.

REA/Ghidra analysis of original 0x48e180 confirms strict X/Y collision-box overlap
and return code 6. The cliff masters are class 22, collision height 0.1, with
positive X/Y sizes and no can-be-built-on flag. Native metadata guards now check
those properties and the original obstruction fields before using the mask.
The shared scenery pass validates mask dimensions and skips occupied cells
before drawing a facet. Calls without a mask retain their previous behavior.

The configured object module must have auto-scenery byte +0x1ddf set to 1;
without it, original 0x4ddd40 intentionally skips the forest scan. The first
exploratory forest run exposed that missing harness setting. Earlier dirt-only
cliff fixtures had this scan disabled; three new original runs with it enabled
confirm unchanged objects and RNG on DIRT1. This distinction remains part of the
reference boundary, rather than treating configured modules as original file loading.

Private evidence: `rms-cliff-forest-cases-oracle-2026-11-15.json`,
`rms-cliff-forest-collision-comparison-2026-11-15.json`,
`rms-cliff-auto-empty-cases-oracle-2026-11-15.json` and
`rms-cliff-collision-ghidra-evidence-2026-11-15.json`. The seed-1 smaller forest
imports 29 cliffs and 2036 trees, final RNG 1567879473, with original replacement
ID gaps preserved. No packaged-map, live Windows visual or runtime pathfinding
parity claim follows from this startup comparison.


App integration exposed an existing coordinate normalization: `Assets::init`
swaps footprint X/Y for native world axes, whereas RMS retains the original
raster axes. `DatFile::footprintAxesSwapped` now records that conversion and resets
on a successful raw reload. RMS cliff validation and the occupancy mask read the
original axes explicitly. The algorithm fixtures exercise both raw and normalized
DAT representations; accepting either shape without recording its orientation
would silently transpose narrow cliff footprints. Runtime movement/collision
orientation remains a separate comparison boundary from this startup mask.


### Cliff collisions with automatic forest types 18 and 20

The same flat cliff/forest profile now accepts terrain 18 (unit 348) and terrain
20 (unit 349). Twelve additional original configured-world runs cover both forest
sizes and three seeds for each type, with real loaded masters and graphics.
Their collision exclusions, creation IDs and RNG agree with terrain 21, while
unit 349 uses 11 appearance angles rather than 12. Fixtures compare the original
unit IDs and facet values independently for each terrain type, under both native
section orders and both raw/normalized footprint representations.

Native import tests cover all three tree types, including the original 11/12-angle
facet-to-facing projection and IDs after rejected placements. Evidence is retained
in `rms-cliff-forest-types-cases-oracle-2026-11-15.json`. This extends the bounded
cliff diagnostic profile; it does not claim new packaged-map or live pathfinding
parity. Forests combined with elevation remain gated.


### Cliff protection follows explicit neutral land positions

The bounded cliff profile now permits any explicit neutral land position, still
with one terrain-6 land, zero base size/target tiles and the existing stage gates.
The cliff exclusion follows that resolved position with radius 15 rather than
using a fixed map center. Randomly placed neutral lands remain rejected.

Forty-eight original composed runs cover all four corners, off-center positions,
plain DIRT1, grass painting, forest types 18/20/21 and level-3 elevation. They
compare all objects, terrain, heights, slopes and final RNG under both native
section orders and both raw/normalized DAT representations.

Six additional runs execute original finalizer 0x4dfe90, including its virtual
0x4e0740 setup call, before the scheduler. They confirm the resolved protection
triples and identical outputs to the configured-module cases. The finalizer's
team grouping requires valid player diplomacy arrays (+0xe0); the harness now
supplies two mutually hostile players rather than permitting null-array reads.
The declaration remains neutral and creates no player-owned objects.

Evidence: `rms-cliff-center-cases-oracle-2026-11-15.json`,
`rms-cliff-center-composed-oracle-2026-11-15.json` and
`rms-cliff-finalizer-center-cases-oracle-2026-11-15.json`. The seed-1 origin-corner
case imports 19 cliffs with final RNG 958887679. Multiple lands, player lands,
nonzero land growth and packaged-map startup remain separate boundaries.


### Cliffs around two randomized player lands

The bounded DIRT1 cliff profile now accepts two randomized player lands with
zero base size and target tiles. Protection follows both finalized positions
with radius 15. This creates land ownership metadata, not starting units.

Nine original runs use the create-player-lands handler 0x4e2c40 and land
initializer 0x4e33c0, finalizer 0x4dfe90 (including player setup 0x4e0740), then
the real land/cliff/empty-terrain/automatic-scenery scheduler. Three seeds cover
default cliffs, custom settings and zero cliffs. Native fixtures compare both
player positions, surviving unit/facet/position/creation IDs, terrain and final
RNG, with early/late section order and raw/normalized DAT footprints. Seed 1
with default settings produces 44 cliffs and final RNG 1152740336.

Evidence: `rms-cliff-player-lands-cases-oracle-2026-11-15.json`. The harness runs
original executable code in a synthetic world with resolved module inputs;
allocation/time/log boundaries are modeled and original RMS file loading is
not exercised. Fixed positions, extra lands, nonzero land growth, terrain or
elevation composition with these player lands, and owned startup remain gated.
This is diagnostic coverage, not a newly verified packaged map.


### Player-land growth before cliffs

Nine more original scheduler runs extend the two-player DIRT1 profile to nonzero
land size and growth: radius/target pairs (3,1000), (10,2000), and (0,8000), each
at seeds 1, 12345 and UINT32_MAX. Both lands retain their original separate zones.
Although these lands paint the same terrain as the base, the land module consumes
random numbers during growth and changes subsequent cliff placement.

All player starts, surviving cliff objects and final RNG match. The native bound
is radius <=10 and target <=8000 per player, still without subsequent painting or
owned objects. Seed 1 at radius 3/target 1000 creates 63 cliffs and finishes at
RNG 1003445079. Evidence: `rms-cliff-player-growth-cases-oracle-2026-11-15.json`,
using the same original handler/finalizer/scheduler harness and limitations as
above. This does not establish terrain transitions, full packaged maps or live
startup callback parity.


### Player-land cliffs followed by grass and forest placement

Thirty-six additional original composed cases cover grass0/forest21 painting
with and without `set_avoid_player_start_areas`, three seeds, and zero-growth,
radius3/target1000, and radius10/target2000 player lands. The finalizer's two
terrain-protection rows (radius13/weight20) are copied into the original terrain
module, alongside the separate radius15 cliff protection. Static inspection of
0x4e69e0 confirms terrain rows at module+0x1140 and count at +0x1770.

Native fixtures match every terrain tile, player position, cliff/tree unit,
facet, coordinate, creation ID and final RNG. Both RMS section orders and both
DAT footprint representations are exercised. All eighteen non-avoiding cases
also match a control run without terrain protection, confirming that the flag
selects the protection behavior rather than changing unrelated placement.

The seed1, radius3/target1000 forest case creates 2053 objects without avoidance
(final RNG405893282) and 2055 with avoidance (RNG2281347096). Native import and
CLI rendering fixtures cover both. Counts are outcomes of the original placement
algorithm, not a guarantee that protection always reduces total tree count.

Evidence: `rms-cliff-player-protected-terrain-cases-oracle-2026-11-15.json`,
`rms-cliff-player-avoiding-terrain-cases-oracle-2026-11-15.json`, and
`rms-cliff-player-terrain-protection-disassembly-2026-11-15.json`.
The configured-world original-code harness limitations still apply. Player-land
cliffs with elevation, other forest types, extra lands, fixed positions or owned
startup remain gated; no new packaged map is claimed.


### Player-land cliffs with forest types 18 and 20

The same two-player land/cliff composition now accepts forest18 (tree348) and
forest20 (tree349). Thirty-six additional original runs cover both start-area
avoidance modes, three land-size/growth profiles and three seeds. The real tree
masters and graphics are loaded, including unit349's 11 appearance angles.
Terrain, protection, surviving cliffs, tree placement/IDs/facets and final RNG
are compared under both section orders and raw/normalized DAT footprints.

Evidence: `rms-cliff-player-forest-types-cases-oracle-2026-11-15.json`. Native
import and CLI rendering cover all three forest types with both avoidance modes.
This extends the existing flat profile; player-land elevation and owned startup
are still outside the verified cliff composition.


### Fixed-position player lands with cliffs

Forty-five original cases now exercise finalizer 0x4dfe90 with its virtual setup
bound to the original fixed-position implementation 0x4e76a0. They cover bare
land growth and grass/forest21 painting, with both start-area avoidance modes,
three land profiles and three seeds. Owners remain two mutually hostile players.
The native fixed-position path matches the original positions, complete cliff
and tree records, terrain raster and final RNG. The seed1 radius3/target1000
forest case places starts at (104,28) and (32,99), creates 2069 objects without
avoidance (RNG685587785), and 2048 with avoidance (RNG3019377153).

Evidence: `rms-cliff-fixed-player-terrain-cases-oracle-2026-11-15.json` and
`rms-cliff-fixed-player-growth-cases-oracle-2026-11-15.json`. Native import and
CLI checks cover both avoidance modes. Fixed-position forest18/20, allied fixed
positions, player-land elevation and owned cliff startup remain gated. The
configured-world harness still does not execute the original RMS file loader.


### Owned startup after cliffs and forests

A bounded two-Empire profile now runs land growth, cliffs, forest21 scattering,
and Command Center/worker/scout startup together. Each land has radius13 and
7200 target tiles; the forest row uses 6000 tiles, 60 clumps, spacing2, clumping20
and player-start avoidance. Owned rows are the stock109,83,550 sequence with
0/0,6/6,7/9 placement distances. Additional resource rows, fixed positions and
other civilization pairs remain gated for this composition.

At seeds 1,12345,UINT32_MAX, the actual original handler/finalizer/scheduler and
owned-object callbacks match every cliff/tree/starting-unit record, replacement
ID gap, terrain tile and final RNG. The native import additionally checks health,
initial animation frame/delay, deferred foundations, population and cost counters.
The seed1 case creates6045 surviving objects, with the final original allocation
ID6065 and RNG2896867181. Source fixture: `tools/fixtures/cliff-owned-start.rms`.
Evidence: `rms-cliff-owned-startup-cases-oracle-2026-11-15.json`.

These remain configured-world original-code comparisons, without the original
RMS file loader or a new live Windows screenshot comparison. The authored map
has forest and starting units, but lacks the stock food/ore/Nova resource rows;
it is not a claim of a complete balanced skirmish or a new packaged map.

### Research-completion counter discovered during startup comparison

Original resource21 increases only when the completed technology has location
>=0 and button>0. Runtime writes at0x4c1860 originate from the completion branch
ending at0x5bffc5; the definition fields are location+0x34 and button+0x32, not the
icon field. Hidden automatic technologies and repeated completion do not add to
this counter. Native explicit and automatic completion now maintain resource21.

The startup graph normally yields1 counted completion before a Command Center
and2 after its unlock cascade. Civilization2 starts at2 and reaches3; Trade
Federation remains1. Tests exercise all
civilizations, both missing-field cases and duplicate explicit completion. Free
population is compared through native capacity minus usage, since the native
engine derives that value rather than maintaining original resource4 verbatim.
Evidence: `rms-cliff-owned-accounting-1-oracle-2026-11-15.json` and
`rms-research-counter21-disassembly-2026-11-15.json`. Counter values are verified;
this does not establish parity of intermediate UI/event callback ordering.


### Completed-research cost statistics

The same original completion branch accumulates resource99 from the three cost
slots returned by0x5c0285. Controlled original-code probes on technology486
confirm totals175 for three enabled slots,125 when the50-cost slot is disabled,
and20 when an invalid resource slot is ignored but a signed negative cost remains.
An unscaled100+32767 modifier is narrowed to a signed16-bit value for this
statistic: with the other50/25 slots, the original total is-32594. This narrowing
is specific to the statistic; the wider unscaled cost-query result is unchanged.

Native explicit and automatic completions now share this accounting. Costs are
summed as signed integers and added once to the float counter. Automatic cascade
statistics are recorded in reverse completion order, after the dependent effects,
matching the original recursive refresh structure. Repeated completion adds
neither the research count nor its cost again. These controlled-input tests do
not represent normal shipped technology prices.

Evidence: `rms-research-counter99-cases-oracle-2026-11-15.json` and
`rms-research-counter99-overflow-1-oracle-2026-11-15.json`. Original resource writes
reach0x4c1860 through the completion return at0x5c001c. Unit cost and research
payment/refund behavior is covered by the existing independent tests.

### Hills and cliffs around player starts

The two-player120 DIRT1 cliff profile now accepts one elevation block before the
cliff stage, with randomized or fixed enemy-player positions. Existing land
radius/target limits remain10/8000. Terrain painting, owned startup, connections
and explicit zone overrides remain excluded from this elevated composition.

Original finalizer0x4dfe90 supplies separate protection arrays: elevation uses
radius13/weight20, while cliffs use radius15. Native generation already carried
both arrays through the corresponding stages; this change opens the composed
profile after comparison, without replacing either terrain algorithm.

The original handler, finalizer and scheduler provide54 randomized cases across
three land-growth profiles, sparse/dense elevation rows, levels1/3/7 and seeds
1/12345/UINT32_MAX. Another12 cases use original fixed-position setup0x4e76a0,
two growth profiles and levels1/7. Regression fixtures compare every height and
slope through hashes, surviving cliff records including replacement ID gaps,
player starts, terrain and final RNG. Representative native imports also compare
cliff rendering heights against original object z values. Both RMS stage orders
are exercised; redundant player-case order permutations are omitted.

Evidence: `rms-cliff-player-elevation-cases-oracle-2026-11-15.json` and
`rms-cliff-fixed-player-elevation-cases-oracle-2026-11-15.json`. These run actual
original code in the configured-world harness; they do not execute the original
RMS file loader or establish full packaged-map/skirmish parity.

The algorithm test runner now usesO1 with ASan/UBSan retained, keeping the expanded
map comparisons within the existing timeout. Command-line rendering and native
Game imports are checked independently against installed Clone Campaigns assets.

### Civilization startup with cliffs

The bounded cliff/forest109/83/550 startup now accepts all64 ordered Clone
Campaigns civilization pairs. All192 original comparisons at three seeds match
every object record, terrain tile, final RNG, deferred foundation and animated
sprite phase. Raw static-sprite offsets are not interpreted as animation timers.
Evidence: `rms-cliff-owned-civ-matrix-2026-11-15.json`, with losslessly compressed
per-case originals and source hashes. The configured original
world runs research restrictions/automatic unlocks before RMS, but not the later
full civilization-bonus pass. Function0x5c5dc0 handles research-effect types
101/102/103; it does not apply ordinary resource or unit-stat bonuses.
Consequently, its raw free-population value1 is not the final native Trade
Federation capacity, and its raw Confederacy worker HP40 precedes the class58
+15HP bonus. Native import tests retain the independently verified200 population
cap and55 worker HP. Those differences are capture boundaries, not engine fixes.


### Cliff blockers during resource startup

Holocron and grouped-resource placement now receive the same cliff collision
footprints that already excluded automatic trees. Previously, later rows only
received tree blockers. This changed holocron candidates at seedUINT32_MAX and
Nova placement/RNG at seed12345, despite identical earlier terrain and objects.
Reusing the existing occupied-cell mask fixes those differences without changing
candidate ordering or random draws in the shared placement helpers.

The authored `tools/fixtures/cliff-resource-start.rms` profile extends the verified
two-Empire startup with five holocrons,12 food bushes per player, near/medium/far
Nova groups and near/far ore groups. Three seeds with default cliffs and the same
three with dense20-cliff settings match the complete original object lists,
terrain and final RNG. The default seed12345 case has6100 objects and final RNG
146046165; dense seed1 has5556 objects and RNG2424374048. Native import checks also
cover IDs after cliff replacements, resource positions and subsequent allocation.

Evidence: `rms-cliff-resource-prefix-cases-oracle-2026-11-15.json`; six original
configured-world runs through object row10. Resource compositions remain restricted
to this two-Empire prefix. Other civilizations with resources, animals, later
resource rows, elevations and connections remain unverified in this cliff
composition. This is not a full packaged-map or complete gameplay parity claim.

### Full23-row cliff startup and Gaia animation state

The same two-Empire cliff/forest profile also accepts the authored23-row sequence
in `tools/fixtures/cliff-full-start.rms`: the earlier resources plus herd/hunt
animals, predators, distant Nova/ore, explicit trees, birds, waves and fish rows.
This controlled sequence uses three food bushes per player; the earlier10-row
fixture uses12. Its unit/count sequence is validated separately. It is not a
claim that these two authored inputs are interchangeable or a packaged map.

All six default/dense-cliff cases match the complete original object records,
terrain and final RNG. The default seed1 case has6260 objects and RNG3558270440;
dense seed1 has5694 objects and RNG2453120652. Separate original recaptures leave
the baseline unchanged and verify222 animated Gaia sprite frames/holds across
those six cases. Native import fixtures additionally check their health and
animation timers after insertion into Game. Terrain here is dry: the authored
water-object rows execute, but this does not verify water/cliff placement.

Evidence: `rms-cliff-resource-full-cases-oracle-2026-11-15.json` and
`rms-cliff-resource-poses-cases-oracle-2026-11-15.json`, with compressed per-case
pose captures. Extra herd-count variants, other civilizations with resources,
fixed owned starts, hills, connections and other map sizes remain gated for this
composition. The original RMS text/file-loader boundary remains outside these
configured-world comparisons.

### RMS import uses the scenario world axes

`RmsMap` retains original coordinates and unchanged oracle hashes. `Game::initRms`
now imports that raster and those object coordinates through the normal scenario
transform: original `(x,y)` becomes native `(y,mapSize-x)`. Previously the import
undid that transform while asset loading still swapped collision footprint axes;
this misaligned asymmetric objects such as narrow cliffs. Heights rotate with the
raster and keep their original slope IDs for the existing slope sampler.

A regression compares equivalent RMS and scenario imports, including the terrain,
object positions and worker passability around asymmetric cliffs. The original
17 slope-shape samples and existing object/startup fixtures use native coordinate
queries without changing their original reference values. This checks coordinate
consistency, not exact original pathfinding. The desktop short skirmish still uses
its procedural `initSkirmish` layout; these RMS imports are currently exercised by
the diagnostic renderer and tests, not the default menu game.

### Full cliff startup across all eight civilizations

The authored23-row cliff/resource profile now accepts all64 ordered pairs of the
original eight civilizations. Each pair matches the configured original engine
at seeds1,12345 and4294967295:192 comparisons of complete object records, terrain,
final RNG, owned animation/foundation state and animated Gaia state. Original
captures are retained compressed with SHA256 identities in
`rms-cliff-full-civ-matrix-2026-11-15.json`.

Native regression fixtures add26 representative full startups, covering each
civilization in both owner slots and all seven distinct seed/wildlife animation
outcomes. They check imported positions, population, health and animation after
native initialization. The original captures still precede the full civilization
bonus pass; native Trade Federation population and Confederacy worker HP are
checked separately, as in the earlier owned-only matrix. The shorter10-row
resource prefix remains restricted to Empire/Empire. The new192 captures use
default cliffs; arbitrary packaged maps, fixed owned starts, mixed hills and
water placement remain outside this evidence.

### Fixed player starts with forest18 and20 on cliff maps

The flat two-player cliff profile now accepts forest18 and20 with
`FIXED_POSITIONS`, alongside forest21. Thirty-six original configured-world runs
cover three land sizes (radius/tile target0/0,3/1000,10/2000), both forest types,
start-area avoidance on/off and seeds1,12345,4294967295. The original fixed-start
finalizer supplies both cliff and terrain protection areas before the real
land/cliff/terrain/object scheduler runs.

The regression checks ordered object hashes, terrain, starts and final RNG for
all36 cases, in both textual stage orders and both raw/normalized DAT layouts.
Native imports and CLI rendering also exercise both new forests with and without
start protection. Evidence is retained in
`rms-cliff-fixed-player-forest-types-cases-oracle-2026-11-15.json`. These cases have
no owned startup objects or hills; those compositions remain gated.

### Conditional stage headers and unfinished conditions

The original command interpreter (`0x4e1fa0`) permits stage headers in active
conditional/random branches. Its file-reader EOF path (`0x4e16f0`) retains the
condition stack instead of rejecting an unfinished `if`. Native loading now
accepts those cases while preserving validation of braces, random blocks and
unknown stage names. Inactive branches still contribute no stage directives.

The packaged Mos Espa script (55014) has `else if SMALL_MAP` and only one matching
`endif`, leaving the outer Tiny condition open. It now loads and preprocesses;
Tiny excludes the trailing elevation/cliff stages, while Small retains them.
This preserves the shipped control flow rather than adding an inferred `endif`.
It does not yet make Mos Espa a supported complete generated map.

Evidence: `rms-conditional-parser-ghidra-evidence-2026-11-15.json` and
`rms-conditional-stage-oracle-2026-11-15.json`: four original nested-condition
cases, two random-stage cases and two EOF cases. Command cases supply decoded
condition arguments; EOF cases model CRT file operations with immediate EOF and
run the actual reader cleanup. These are not complete original text-loader
captures. Native tests additionally use the installed Mos Espa archive source.
The malformed `elseif` in packaged55012 remains a separate unsupported case.
EOF compatibility does not relax resource/include limits or unknown-stage checks.

### Missing `elseif` argument in Dagobah

The original text reader consumes whitespace-separated words. When an `elseif`
argument resolves to another registered command, the argument decoder returns
that command to the reader and discards the incomplete `elseif`. Native
preprocessing now preserves this behavior using the94 command names registered
by the original engine. EOF during that argument also leaves the branch unchanged.

The shipped Dagobah script (55012) now preprocesses past its empty `elseif`.
In a controlled equivalent, an active first branch emits land IDs20 then25;
an inactive first branch emits26. Inferring the missing symbol or treating the
line as `else` would produce different results. Complete Dagobah generation is
still unsupported at later map stages.

`rms-text-parser-oracle-2026-11-15.json` records four runs of the actual original
command registration, text reader, argument decoder and conditional interpreter,
plus the recovered command registry. CRT whitespace scanning/formatting, memory
allocation and language callbacks are modeled; stage handlers are intercepted to
record their decoded arguments. This verifies text/control behavior, not world
generation. Native regressions cover both branch states, EOF recovery and loading
the installed Dagobah script. Other parser restrictions remain in place.

### Packaged text/control corpus comparison

A separate parser audit covers46 packaged scripts × Tiny/Small × three seeds
(1,12345,4294967295):276 cases. Both sides receive the same explicit
`RANDOM_MAP`, `2_PLAYERS`, `FIXED_POSITIONS`, `DEFAULT_RESOURCES` and size labels.
The original executes command registration, text reading, include recursion,
argument decoding and branch/RNG handling. Active tokens are captured immediately
before dispatch at `0x4e26d0`. CRT I/O, allocation, resource extraction and language
lookups are modeled; generation handlers are intercepted. This is a preprocessing
comparison, not a complete map or live startup-settings comparison.

261 cases match the full active token/argument stream and final RNG. Their
original token hashes/counts and RNG states are checked against installed archives
by `tools/rms_packaged_parser_checks.h`. The remaining15 are recorded separately:

| Resource | Cases | Difference |
|---|---:|---|
| 55014 (Small) | 3 | Undefined `ANIMAL_SAARLAC`; the original discards that `create_object` command |
| 55016 (Tiny/Small) | 6 | Original ignores unrecognized `set_avoid_player_start_area` |
| 55019 (Tiny/Small) | 6 | Undefined `ANIMAL_SAARLAC`; the original discards that `create_object` command |

The defined label is `ANIMAL_SARLAAC`. Native preprocessing retains those unknown
words; interpreting the resulting generation rows still needs separate work.
Do not silently replace the typo with a valid unit: that would add an object that
the original does not create. Discarded object commands may leave subsequent
parameters acting on the previous row, so full row interpretation matters.

Evidence: `rms-packaged-token-parser-matrix-2026-11-15.json` and per-case captures;
`rms-parser-corpus-2026-11-15/provenance.json` records archive/resource hashes.
The earlier decoded-command matrix matches the same261 cases; its15 adapter
errors are superseded by the complete token captures, not treated as passing.

### Full resource startup at fixed cliff starts

The authored23-row cliff/resource profile now accepts `FIXED_POSITIONS` for
Empire/Empire. Six original comparisons cover default and dense cliff settings
with seeds1,12345,4294967295. Complete ordered object records, terrain rasters,
final RNG, pending foundations and animated owned/Gaia poses match. Native
regressions also check import coordinates and resource/population state.

Evidence: `rms-cliff-fixed-full-cases-oracle-2026-11-15.json` and its compressed
per-case captures. The original fixed-start finalizer and generation callbacks
execute in the configured-world harness; the RMS file loader and full civilization
bonus pass are outside that capture. Other fixed-start civilizations remain gated
pending comparison. This does not establish complete packaged-map parity.

The original object handler also confirms the undefined `ANIMAL_SAARLAC` behavior
in55014/55019: the following `number_of_objects 1` modifies the preceding Womprat
row, retaining its variance2. Object-row captures are saved as
`rms-packaged-object-rows-*-2026-11-15.json`. Native generic row interpretation
still needs to reproduce that behavior before these maps can be enabled.

### Whitespace-separated directive arguments

Directive arguments and DRS include arguments now accept line breaks, matching
the original whitespace scanner. Required preprocessor arguments still reject a
following registered command rather than consuming it as a symbol; the separately
verified incomplete-`elseif` recovery remains unchanged. Include filename, ID,
recursion and resource limits are retained, as is the unverified cross-resource
argument restriction.

Four original text-reader cases and four random-wrapped variants verify split
`#define`, `#const`, `if` and numeric arguments. A separate original Dagobah include
with its directive/name/ID on separate lines retains all641 active tokens and
final RNG196285776. Evidence: `rms-multiline-parser-oracle-2026-11-15.json`,
`rms-multiline-random-parser-oracle-2026-11-15.json` and
`rms-multiline-include-parser-55012-TINY_MAP-1-2026-11-15.json`.
Native regression checks cover expansion, branch output, RNG and source locations.

### Fixed full startup across civilizations and composed hills

The fixed-position23-row cliff/resource profile now accepts all64 ordered original
civilization pairs. All192 original comparisons (three seeds per pair) match the
complete ordered object list, terrain raster, final RNG, owned foundations/poses
and animated Gaia poses. Native import regressions retain25 representative cases,
covering each civilization in both owner slots and all six distinct seed/Gaia
animation outcomes. Original research/civilization-bonus capture boundaries remain
as documented above. Evidence: `rms-cliff-fixed-full-civ-matrix-2026-11-15.json`.

Fixed Empire/Empire startup additionally supports hills composed with the same
cliffs, forest21 and full resources. Thirty comparisons pass: requested levels1..7
with400 tiles/eight clumps, and levels1/3/7 with8000 tiles/30 clumps, each over
seeds1,12345,4294967295. Comparisons include complete height and slope rasters as
well as terrain, object order/placement, RNG and animation. The small-hill seed1
case reaches only level3 even when requesting7; the large level7 case actually
contains3769 level7 tiles, so high-level coverage is not inferred from the request.
Native regressions check every case, Gaia poses and imported flat-tile heights.

Evidence: `rms-cliff-fixed-elevated-full-cases-oracle-2026-11-15.json` and
`rms-cliff-fixed-elevated-full-large-cases-oracle-2026-11-15.json`, plus compressed
per-case originals. These execute original fixed-start finalization and the real
land/elevation/cliff/terrain/object scheduler in a configured world. The RMS file
loader and complete gameplay are outside this comparison. Elevated startup with
other civilizations, random positions, other hill density profiles, connections,
water placement and arbitrary packaged maps remain unverified and gated.

### Packaged parser coverage across all six map-size labels

The text/control audit now covers46 packaged scripts, six explicit size labels
(Tiny through Gigantic) and three seeds:828 cases. The additional552 runs contain
516 exact token-stream/RNG matches and36 differences. Together with the earlier
Tiny/Small audit,777 cases match and51 remain different. All777 matching originals
are retained in the native installed-archive regression test.

The remaining differences are unchanged in kind:55014 discards the undefined
Sarlacc command on every size except Tiny (15 cases);55019 does so on all sizes
(18 cases);55016 ignores the singular unknown terrain command on all sizes
(18 cases). There were no harness errors. These are parser comparisons with
explicit identical labels and intercepted generation handlers, not828 complete
map generations. Evidence: `rms-packaged-token-parser-large-matrix-2026-11-15.json`,
per-case captures and the prior Tiny/Small matrix. The native baseline is frozen
at4ca5212 for the additional sizes; source and binary hashes are recorded.

### Original command lookup and argument recovery

The shared preprocessor now decodes the original94-command registry and its
argument kinds. Unknown words are ignored. Commands with missing required labels
are discarded; a following recognized command remains available to the reader.
Only complete decoded commands reach the generation/RNG visitor. Defined labels
resolve to zero when used as required value labels. Numeric parameters use the
original `atoi` behavior: numeric prefixes are accepted and nonnumeric words become
zero, without substituting constant labels into numeric-only fields.

All828 installed packaged-script cases now match their captured original active
token streams and final RNG, including the51 previously different cases. This is
a preprocessing milestone, not complete generation of828 maps. The unknown
Sarlacc command is discarded without replacing it with another unit; following
braces/parameters remain. The generation layer still needs original row-update
semantics and unsupported unit/resource profiles before those complete maps work.

`rms-command-recovery-oracle-2026-11-15.json` contains11 authored original reader
comparisons, including unknown words/labels, pending commands, EOF, define values,
numeric prefixes and invalid signs. Native tests use ten matching cases. The
remaining case explicitly demonstrates an existing native extension: authored
numeric IDs are accepted where original scripts require named labels. Signed32
overflow, include/resource bounds, unknown stages and other unverified parser
paths remain guarded. Generation tests separately verify that unknown terrain
words are harmless while registered but unsupported commands still reject the map.

### Complete original RMS runner: packaged Forest

The local original-executable harness now calls the actual `4df810` constructor
and `4df9b0` runner. This executes command registration, nested resource reading,
all stage handlers, row finalization, module construction and stage scheduling.
It no longer relies on manually constructing the module rows for this comparison.
The first authored cliff case reproduced all 6,202 ordered records, HP, terrain,
startup poses and final RNG from the earlier controlled original replay.

Packaged Forest54204 then passed at 120×120, two Empire players, with explicit
TINY_MAP/RANDOM_MAP/2_PLAYERS/FIXED_POSITIONS/DEFAULT_RESOURCES labels:

| Seed | Live objects | Final RNG |
|---|---:|---:|
| 1 | 7,802 | 2591164073 |
| 12345 | 7,982 | 3553717540 |
| 16160 | 7,802 | 3740942338 |

Every ordered ID/owner/unit/facet/position, terrain/elevation/slope byte, owned
foundation state and animated owned/Gaia startup pose matched the production
native generator. `rms_packaged_forest_checks.h` retains the original ordered
record and three-grid hashes for all three seeds. The private full-array and
animation comparisons are saved as `rms-text-full-forest-*-comparison-2026-11-15.json`.
Original executable, helper, asset capture and native exporter hashes accompany
the local evidence. Game scripts and executable bytes remain outside the repo.

This remains a configured-world harness: language, CRT I/O/allocation/time and
diagnostic logging boundaries are modeled; original gameplay callbacks execute.
The fixed-placement virtual method and five settings labels are explicit, not
supplied through the original game UI. This does not certify full game startup,
later simulation, all maps or every seed. The updated 46-script/three-seed native
audit still accepts only Forest (3/138); the other 135 attempts reject unsupported
object/resource sequences or attributes. Parser coverage remains a separate
828-case result, not 828 complete generated maps.


### Packaged Reytha: full generation and native import

Reytha54220 now runs directly from the installed archive. The original full RMS
runner and production native implementation agree for these fixed Tiny120 cases,
with two Empire players and DEFAULT_RESOURCES/RANDOM_MAP/2_PLAYERS labels:

| Seed | Live objects | Final RNG |
|---|---:|---:|
| 1 | 1,981 | 3962920155 |
| 12345 | 2,013 | 1129739132 |
| 16160 | 1,940 | 4000893853 |

Comparison covers every ordered object ID/type/owner/facet/position, all14,400
terrain, elevation and slope values, owned pending foundations, and animated
startup frames/holds. Native-import regressions additionally verify every live
object's ID/type/owner/position, flat-tile elevation and202 captured owned/Gaia
health/animation records across the three cases. CLI regressions render all three
maps from the original archive.

The added behavior is a late Kaadu group at minimum distance19 with no outer
limit,43 scaled global tree411 placements with exclusion8/no spacing, and two
global ore groups at exclusion40/spacing9. Existing original queue and placement
algorithms handle these rows. The cliff composition is gated to the compared
Reytha geometry: base41, two1800-tile/radius9/fuzz15 lands, five terrain rows,
seven elevation levels and the original30–60 cliff settings. Terrain44 is now
accepted by the shared dry-ground resource/holocron checks. The new Kaadu range
accepts verified counts4/8; count2, other ranges and unmatched cliff/civilization
profiles remain rejected. Invalid inputs and work-limit failures preserve RNG
and eligibility.

Local evidence is `rms-text-full-reytha-complete-1-1-*-oracle-2026-11-15.json`
and the corresponding comparison files. The harness validates that every Gaia
object row has its actual captured prototype before running the modules. Early
Reytha outputs without Bantha833 were explicitly invalidated in
`rms-text-full-reytha-incomplete-assets-audit-2026-11-15.json`; the original silently
skips unavailable prototypes, so those outputs are not complete-map evidence.
All final runs include Bantha833/Ronto1364 and the other required original assets.

The repeated production audit accepts6/138 combinations: Forest and Reytha for
all three seeds. The remaining132 attempts still reject unsupported placement
rules. These are local original-executable comparisons, not a fresh live Windows
UI/settings comparison. The desktop's normal skirmish launcher still uses its
procedural setup; RMS-backed menu integration and later simulation remain open.

### Packaged Arid: Tatooine and Zaloriis

Arid54201 now has complete original-runner comparisons for fixed Tiny120,
DEFAULT_RESOURCES, two Empire players. Three cases cover the Tatooine branch;
seed16160 covers Zaloriis, including its automatic roots and scaled explicit trees.

| Seed | Branch | Live objects | Final RNG |
|---|---|---:|---:|
| 1 | Tatooine | 2,002 | 1324588119 |
| 12345 | Tatooine | 2,040 | 3205565334 |
| 16160 | Zaloriis | 2,061 | 1931636252 |
| 4294967295 | Tatooine | 2,091 | 3111280523 |

The original full runner and native generator agree on all ordered object
ID/type/owner/facet/position records, terrain/elevation/slopes, owned foundation
state, animated startup poses and final RNG. The fourth case specifically covers
the unbounded late Eopie group of8; the first covers4. Count2 remains gated.
New captured assets include automatic carbon217, explicit roots350, Jawa1250 and
Krayt1129. Exact original DAT loader spans, master/graphic/sound/task-header data
and executable/source hashes are retained in the private Arid provenance record.

`rms_packaged_startup_checks.h` shares the Reytha and Arid regression machinery:
seven complete-map cases and404 original owned/animated Gaia records. A native
import failure exposed a fixed species list that discarded Jawa/Krayt animation
state. Import now applies the pose carried by every generated resource instead
of maintaining a second species list. Original HP, animation time and hold checks
cover the fix, alongside object IDs/positions and flat-tile elevation.

Arid geometry is gated to its compared base14/26 profiles, five terrain rows,
seven6000-tile elevation levels, and10–15 cliffs of length3–10. Existing scenery
validation already covered carbon217 and roots350; the resource and owned-startup
paths now accept the verified compositions. Additional tests keep global Jawa/
Krayt pairs and the unverified100-root profile rejected without advancing RNG or
changing eligibility. Only43 global roots at minimum distance8 are added.

Evidence: `rms-text-full-arid-1-1-*-oracle-2026-11-15.json`, matching comparison
files, and `rms-arid-assets-provenance-2026-11-15.json`. These are configured-world
original-executable runs, not new live Windows UI or later gameplay comparisons.


The final Arid production audit accepts9/138 standard combinations and rejects
129 unsupported cases. Forest/Reytha/Arid native import,19 algorithm suites,
archive rendering and desktop smoke checks pass. The smoke check also verifies
effects and streamed music; it does not establish original sound mixing parity.

### Mixed-land cliffs and selected resources

Two additional complete original-runner cases combine fixed Tiny120 two-Empire
startup, cliffs and one zero-growth neutral land. A center neutral land produces
6,067 objects and RNG112705475; a worker-edge neutral land produces6,077 objects
and RNG2361844694. Ordered objects, all three terrain/elevation/slope grids,
owned and animated Gaia poses match. Independent resource replays also agree.
The general regression suite now covers52 complete-map cases.

The gate permits exactly one extra neutral seed land with cliffs; growing neutral
lands, elevation/connections and additional cliff neutral lands remain excluded.
Evidence is retained locally in `rms-selected-cliff-findings-2026-11-16.json`
and `rms-selected-cliff-source-provenance-2026-11-16.json`. This is bounded
original-executable evidence, not universal RMS or live Windows parity.

### Packaged Hoth: snow, carbon and predators

The fixed Tiny120, two-Empire, default-resource Hoth55004 seed1 case matches
the complete original runner:2,334 ordered objects, all terrain/elevation/slope
cells, owned and animated Gaia poses, and final RNG1985962519. This includes
explicit carbon1160, rocks1170, Wampa1252 and automatic carbon1341. Their
validators use captured original DAT loader metadata;1341 remains distinct from
explicit1160. The packaged startup regression imports the result into Game.

Hoth reuses the existing cliff controller with its exact four terrain rows and
seven elevation levels. Original554953 classifies snow by the building's center
tile in{32,33,34,36,47};557d8c replaces raw foundation27 with36 for the whole
footprint before civilization overrides. The native RMS startup now follows that
rule. This does not yet replace the later gameplay construction heuristic.

Original full runs and independent resource replays also exist for seeds12345
and16160, including optional mountains1166/1165. Both mountain rows now match in native generation:seed12345 has2,374 objects
and RNG2432524620;seed16160 has2,402 objects and RNG1898746486. All ordered
objects, grids and startup poses match, including native import. Original gate
traces confirm integer centers,6×6 terrain footprints and zero constructor RNG
draws. Support remains limited to one mountain as the final Hoth object row;
multiple mountains and subsequent rows are explicitly rejected until their
collision propagation is compared. The packaged suite now has10 complete cases.
Private evidence: `rms-packaged-hoth-full-findings-2026-11-16.json`,
`rms-hoth-assets-provenance-2026-11-16.json`, and the matching Hoth comparison
and snow-flag disassembly files. This is configured-world original executable
evidence, not a new live Windows UI/settings comparison.

The Hoth production audit accepts12/138 standard combinations, up from9:
Forest, Reytha, Arid and Hoth each at seeds1,12345,16160. The remaining126
cases reject unsupported rules. Full startup/import regressions, the algorithm
suites, production CLI include/define/rejection checks and desktop smoke pass.
The smoke uses dummy video/audio; it verifies the audio pipeline, not listening
or original sound-mixing parity. Audit and PNGs remain local.

### Packaged Zaloriis and consecutive mountains

Zaloriis55011 reuses Arid's verified base26 geometry. Three complete original
runs and independent resource replays now match native ordered objects, terrain,
elevation, slopes, owned/Gaia startup poses and RNG:

| Seed | Objects | Final RNG |
|---|---:|---:|
| 1 | 2,405 | 3103567552 |
| 12345 | 2,485 | 786711914 |
| 16160 | 2,514 | 2011336731 |

Actual DAT loader captures supply plants1123, rocks1171 and mountains1019/1020.
The original48e180 collision gate excludes zero-height1123 both as a candidate
and as an existing blocker. Its terrain checks and random facet draw remain.
This matters: treating the plant like rocks preserved counts and RNG while
placing the plants at incorrect coordinates. Complete ordered comparisons
expose that error; final-count checks alone would miss it.

Zaloriis16160 and Hoth4 exercise two consecutive mountain rows. Hoth4 matches
2,415 objects and RNG2003801244. Native collision occupancy includes the first
mountain's6×6 footprint for the second row. The supported bound is now one or
two trailing mountain rows, one object per row, within the compared Hoth or
Zaloriis geometry. Three rows, multi-object rows and mixed later objects remain
rejected. This supersedes the earlier final-row-only Hoth restriction.

The shared packaged regression has14 complete cases, plus the separate Forest
suite and52 general object-row cases. Evidence stays local in
`rms-packaged-zaloriis-full-findings-2026-11-16.json`, the Hoth ledger,
`rms-zaloriis-assets-provenance-2026-11-16.json` and
`rms-zaloriis-plant-collision-findings-2026-11-16.json`. These bounded cases do
not establish all-seed, all-size, all-civilization or live Windows parity.

A separate original48e180 matrix isolates one real mountain: candidate offsets
0/±5 collide and±6/±7 pass on both axes, without RNG changes. Native regression
checks preserve all14 outcomes and integer centers. This supports the footprint
propagation independently of the two complete-map cases, whose successful
mountains happen to be farther apart. Ledger:
`rms-consecutive-mountain-findings-2026-11-16.json`.

The subsequent production audit accepts15/138 standard combinations: Forest,
Reytha, Arid, Hoth and Zaloriis across the three audit seeds. The remaining123
cases still reject unsupported rules. Full import regressions, the focused
mountain-boundary checks and production CLI tests pass. This is acceptance
coverage for fixed Tiny/two-Empire/default-resource settings, not a percentage
of overall engine parity.

### Packaged Kessel: Gaia buildings and craters

Kessel55008 now has complete original-runner comparisons for fixed Tiny120,
two Empire players and default resources:

| Seed | Objects | Final RNG |
|---|---:|---:|
| 1 | 2,058 | 2472476762 |
| 12345 | 2,062 | 1548149991 |
| 16160 | 2,119 | 158063007 |

Every ordered object, terrain/elevation/slope cell and startup pose matches.
The native import also checks mine heights on slopes and pending-foundation
state. Original mine1307 startup consumes nine draws, then reseeds with the
last15-bit result. It completes at frame0 without terrain painting, research or
annex effects. The original reference needed its real air-manager initialization
and previously missing heap-owned damage graphics; failures are retained, and
no unknown engine callbacks were replaced with successful stubs.

Crater1329/1330/1331 placement uses its actual radius1/1.5/2 and terrain
restriction29. Small/large centers are integer-aligned; medium centers are
half-tile aligned. Their zero-height, build-over behavior ignores object
collisions, while bounds and terrain footprint checks remain. Their static
constructors consume no random draws.

Gaia accounting exposed an import omission. RMS startup now records Gaia costs
and original storage grants, including five holocrons granting resource11.
For seeds1/12345 the Gaia unit/building/total costs are2600/475/3075; for16160
they are2850/425/3275. Building completion grants flagged population storage;
owned civilian/military counters remain separate. Original instructions
54c395,5544c8,55a2c3 and554aad support these rules.

Evidence: `rms-packaged-kessel-full-findings-2026-11-16.json`,
`rms-kessel-assets-provenance-2026-11-16.json`,
`rms-kessel-gaia-stats-findings-2026-11-16.json`, and matching native comparison
files in local research. The shared packaged fixture set now has17 complete
cases. This does not establish untested building/diplomacy overlap branches,
all settings or later-game simulation parity.

The Kessel production audit accepts18/138 standard cases;120 remain rejected.
Complete import/pose/height/resource regressions, all52 general object-row cases
and production CLI checks pass. The reference comparison remains local and
configured-world; these figures are not whole-engine completion percentages.

### Packaged Tatooine: settlements and Sandcrawlers

Tatooine55009 now matches three complete original runs at the same fixed
Tiny120/two-Empire/default-resource settings:

| Seed | Objects | Final RNG |
|---|---:|---:|
| 1 | 2,175 | 778802560 |
| 12345 | 2,199 | 524660488 |
| 16160 | 2,206 | 890442689 |

The comparison includes all ordered objects, terrain/elevation/slopes and
startup poses. All five Gaia building types1184–1188 are exercised, including
towers in12345. Their completed constructors consume seven draws and reseed;
they retain frame0 and no pending foundation. Houses use half-tile centers,
while integer-radius buildings retain integer centers. The vaporator's zero
clearance remains distinct from its half-tile collision radius. Shared footprint
handling now carries these buildings into later placement rows.

Sandcrawler938 uses six constructor draws and a nonanimated standing sequence;
its resource grants contribute population−1, unit count+1 and military count+1.
Predator1251 keeps its distinct speed, restriction and animation timing. Gaia
unit/building/total cost counters match3150/450/3600,3050/575/3625 and
3350/475/3825 respectively; each case has two Sandcrawlers and five holocrons.

The packaged fixture suite now contains20 complete cases, with Gaia building
height, animation, health, pending-foundation and resource checks at import.
Evidence: `rms-packaged-tatooine-full-findings-2026-11-16.json`,
`rms-tatooine-assets-provenance-2026-11-16.json` and matching comparisons.
These remain bounded original-executable tests, not all-settings or subsequent
AI/gameplay parity.

The Tatooine production audit accepts21/138 standard cases, with117 unsupported
cases still rejected. All20 shared packaged startup cases,52 general object-row
cases, production CLI checks and desktop smoke pass. Desktop smoke uses dummy
video/audio and verifies effects/music streaming rather than audible mixing.

### Growing neutral lands and aquatic boundaries (2026-11-16)

Raw packaged54209 now matches the complete original4df9b0 runner for fixed
Tiny120/two-Empire/default-resource starts. This composition adds two growing
neutral lands before the player lands, explicit player zones, elevation on two
base terrains, flat-only water and cliffs. The existing geometry stages match
without changing their algorithms; the acceptance gate remains limited to the
compared composition.

| Seed | Objects | Final RNG |
|---|---:|---:|
| 1 | 2846 | 2542443735 |
| 12345 | 2805 | 4168222063 |
| 16160 | 2793 | 3585658713 |

All ordered object records, terrain/elevation/slopes and startup animation poses
match. Three shared-controller corrections were necessary: global holocrons
exclude neutral land centers as well as player centers; loose-group members
individually check the requested terrain before placement (4dee03); and
zero-height aquatic prototypes bypass object overlap (48e81b), including nearby
cliff footprints. Moving objects also retain the4a1440 boundary clamp: the
seed12345 fish enters its factory at x0.5 and exits at float0.51. Stationary
waves retain their half-tile positions. Neither rejected loose-group terrain
nor the boundary adjustment consumes random draws.

The shared packaged suite now holds23 complete cases. New checks cover native
import, Gaia cost counters3850/3800/3350, five holocrons, exact imported fish
boundary position and atomic rejection of uncompared random-position starts.
The private full comparison uses full-precision coordinates; the historical
half-tile object hash alone would not detect the fish boundary difference.
Evidence: `rms-packaged-mixed54209-full-findings-2026-11-16.json`, the three
`rms-text-full-mixed54209-*-comparison-2026-11-16.json` reports and targeted
loose-group/aquatic trace supplements. These execute the original PE locally
with modeled host boundaries; they are not new live Windows captures or proof
of all map settings, civilizations or subsequent gameplay.

Validation passes all23 shared packaged fixtures,52 general object-row cases,
Forest/holocron regressions, strict integrated RMS tests and production CLI
checks. The production audit accepts24/138 sampled cases across eight map
families;114 remain rejected. Native import also preserves zero-HP Gaia fish
instead of raising them to1HP. The rebuilt desktop passes menu/preferences and
headless effects/music smoke; audible mixing was not rechecked.
