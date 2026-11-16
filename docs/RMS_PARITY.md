# Clone Campaigns random-map parity

## Current comparison coverage: 2026-11-16

Twenty packaged profiles have complete original-runner comparisons at fixed120,
two Empire players and default resources (54218 uses mutual allies): Forest54204, Reytha54220, Arid54201,
Hoth55004, Zaloriis55011, Kessel55008, Tatooine55009, mixed-land54209,
Tatooine New55019, Endor54207, four-biome connections54210, inland54203, flat Tiny Mos Espa55014, Migration54216, five-land islands54217, central basin54206, allied roads54218, central water54212, Motherlode54225 and Fortress54208 samples from all three biomes.
The shared packaged suite contains72 complete cases in addition to the separate Forest fixtures. It checks ordered objects,
terrain/elevation/slopes, startup animation and native import; added tests cover
Gaia resource accounting and deferred foundation rendering. Twenty-nine additional authored
fixtures cover owned Troop Center, Prefab, Farm and defensive-building placement, completion research order, population grants and rejected placement. Three separate authored wall/gate fixtures cover complete emission and native import.

The production audit accepts57/138 map/seed attempts (nineteen of46 resources,
three seeds each);81 are explicitly rejected. The allied54218 configuration is
verified separately and is not counted as accepted by this enemy-player audit. This measures supported
configurations, not an engine completion percentage. New comparisons run the
original executable locally with modeled host boundaries. Live Windows evidence
below remains a separate, earlier baseline. Later gameplay and other map settings
are not established as1:1.

## Reference-context correction: 2026-11-16

The Fortress integration exposed a reference-harness distinction: base reader
`4df810` does not install the game presets automatically. Production constructs
`4e7420`, whose derived registration installs the map-size, position and resource
settings. Earlier base-reader worlds remain valid controlled comparisons, but
120×120 dimensions alone do not establish the original Tiny preset. Historical
Tiny labels must be read with each capture's actual script-definition context.
The earlier live Windows Forest comparison is a separate source of evidence.

Fortress's two new shared cases use the actual derived reader, verified original
DLL language strings and the unchanged packaged script/includes. Its Tiny
preset adds two resource rows absent from the base-reader captures. The native
terrain parser also now applies the derived reader's implicit flat-only default
for terrain1. The retained68 packaged comparisons still pass after that change.
See `rms-fortress54208-parser-context-correction-2026-11-16.json` and
`rms-derived-terrain-default-findings-2026-11-16.json` in local research.

## Retained Forest and live Windows baseline

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

### Object parameters after unresolved labels (2026-11-16)

Original55019 contains `ANIMAL_SAARLAC`, distinct from the defined
`ANIMAL_SARLAAC`. The reader discards that unresolved create command but keeps
its parameter block. Original4e3850 applies the block to the previous Womprat1475
row: per-group count becomes1 while mode1, variance2, one group, Gaia ownership
and per-player land selection remain. The native reader already discarded the
command correctly; the object-row controller now continues the previous row
across the orphan block instead of rejecting it. A missing previous row or
invalid modifier still rejects atomically. Strict sanitizer object-row and
integrated map tests pass. This prepares55019; its remaining unit constructors
and Gaia foundation behavior are not yet enabled.

Evidence: `rms-packaged-object-rows-55019-TINY_MAP-1-2026-11-15.json` and
`rms-tatooine-new-inventory-2026-11-16.json` retain original parser output and
source hashes. The malformed shipped script is not rewritten.

The broader20-suite asset-independent run exposed one old synthetic assertion
that expected loose-group members on the wrong terrain. It is corrected to the
original4dee03 behavior already exercised by the complete54209 captures; the
single eligible tile now yields one object. All20 suites pass after rerunning
the corrected suite and the remaining candidate/AI suites.

### New Tatooine settlements and deferred Gaia foundations (2026-11-16)

Raw55019 (Tatooine New) now matches the full original runner with fixed Tiny120,
two Empire players and default resources, including the shipped misspelled label.

| Seed | Objects | Final RNG | Gaia unit/building costs |
|---|---:|---:|---:|
| 1 | 2197 | 65450833 | 3400 / 875 |
| 12345 | 2220 | 126653571 | 3250 / 1000 |
| 16160 | 2227 | 4224403267 | 3550 / 900 |

All ordered objects, terrain/elevation/slopes, startup poses, native import and
resource counters agree. Six additional Gaia building prototypes275/518/519/524/
536/537 use seven draws followed by reseeding. Sarlacc1465 uses six draws with
no heading or military-timer draw; Womprat1475 uses eight. New resource blockers
include rock1171 in animal placement. Original5c9572 skips type70 objects only
for the direct animal-gate argument; building overrides retain those blockers.
This distinction is independent of the existing unit's movement speed.

Jawa camp519 and Tusken tent537 retain pending foundation27 at startup. Separate
original496e80 executions prove4×4 and2×2 footprints respectively, raw Gaia
terrain27 and no RNG change. Their fog mode1 permits dispatch in explored128
and visible15; unseen0 retains the pending flag. Native rendering now uses the
prototype footprint and this visibility rule, while the existing enemy Command
Center regressions still retain foundations in remembered fog. New checks cover
both sizes, all three visibility states, untouched neighboring tiles and one-time
consumption. The native importer preserves the original pending flags and
building height on slopes.

The shared packaged suite contains26 complete cases. Full comparisons and
resource replays are in `rms-packaged-tatooine-new-full-findings-2026-11-16.json`;
asset closure and original loader provenance are in
`rms-tatooine-new-assets-provenance-2026-11-16.json`. The original harness needed
Gaia diplomacy storage for a newly reached building gate; its values were set
by executing the original player-constructor initialization loop (self0, others3),
not by bypassing that gate. These remain local original-executable comparisons
at bounded settings, not fresh Windows runs or universal gameplay parity.

Validation passes26 complete shared packaged cases,52 general object rows,
Forest/holocron regressions, Gaia and owned-foundation render checks, strict
integrated RMS tests and production CLI tests. The audit now accepts27/138
sampled cases across nine map families, with111 unsupported cases rejected.
The rebuilt desktop passes preferences, menus and headless effects/music smoke.

### Endor mixed water/forest startup (2026-11-16)

Raw54207's fixed Endor profile now matches the complete original runner at
Tiny120/two-Empire/default resources. It reuses the growing-neutral-land/cliff
pipeline with water1, neutral grass0, player terrain12, forest21, deep water23
and player zones1/2. All three terrain/elevation/slope grids match without new
geometry algorithms.

| Seed | Objects | Final RNG | Gaia unit/total cost |
|---|---:|---:|---:|
| 1 | 2414 | 3714391971 | 2450 |
| 12345 | 2393 | 2516654529 | 2900 |
| 16160 | 2360 | 3431027007 | 3850 |

The comparison also covers every ordered object and startup pose. Each map
creates36 deep-water fish456 at integer centers, using three constructor draws
and an original2×2 terrain footprint. Their zero-height collision ignores
objects. Two each of marine predators998/1033 consume eight draws and can share
positions with one another. Their exact placement terrain23 agrees with the
original master gate; restriction27 independently permits only23 in the supplied
DAT. Native placement checks both constraints rather than relying on that
coincidence if restrictions change.

The shared packaged suite now includes29 complete cases, native fish health and
animation, Gaia cost/storage checks and rejection of uncompared random starts.
Evidence: `rms-packaged-endor54207-full-findings-2026-11-16.json`,
`rms-endor-assets-provenance-2026-11-16.json` and
`rms-endor-marine-terrain-gate-findings-2026-11-16.json`. Original full runs and
independent resource replays agree for all three seeds. These bounded captures
do not establish other civilization/settings combinations or subsequent gameplay.

Validation passes all29 shared packaged cases,52 general object rows, Forest,
Gaia/owned-foundation rendering, strict integrated RMS and production CLI tests.
The production audit accepts30/138 cases across ten profiles;108 remain rejected.
Desktop preferences/menu/effects/music smoke passes using dummy video/audio.
The final explicit marine master-terrain check also passes all three full Endor
comparisons with full-precision coordinates.

The specialized mixed-land acceptance gates now retain the common cliff-master
validation. Previously they bypassed that validation along with the older geometry
gates. A malformed/missing cliff prototype now rejects before generation with
unchanged RNG and empty output. Both mixed-land profiles have negative fixtures;
all29 complete packaged startup cases still pass.

### Four-biome connection composition (2026-11-16)

Raw54210 now matches complete original runs in all four biome branches at fixed
Tiny120/two-Empire/default resources. It combines water-based land growth,
explicit or randomized zones, seven elevation levels, cliffs, eight terrain
rows and connections replacing water1/22/23 with terrain4. No connection or
cliff algorithm changes were necessary.

| Seed | Biome | Objects | Final RNG | Gaia unit/total cost |
|---|---|---:|---:|---:|
| 1 | Yavin | 2860 | 42721692 | 2250 |
| 12345 | Yavin | 2843 | 805080015 | 1000 |
| 16160 | Kashyyyk | 2900 | 3916911490 | 950 |
| 3000 | Naboo Forest | 2764 | 2227137235 | 2950 |
| 8000 | Reytha | 2807 | 1162255447 | 1250 |

Every ordered object, full terrain/elevation/slope grid, startup pose and native
import matches. The actual Kashyyyk random zones are3/2; native preprocessing
consumes their draws before subsequent script choices. Both player separation
values7/10 are exercised. Native acceptance validates the exact connection
settings and all four compared geometry compositions.

Kashyyyk and Naboo include rows that paint terrain9 onto9 or6 onto6. They must
still execute the queue/growth work. The earlier native validation rejected
these rows; removing that rejection lets the existing algorithm reproduce the
original RNG. Eight isolated9×9 original-worker cases confirm unchanged terrain
but71/68 draws for two clumps (seeds1/12345), or20 draws with zero clumps, for
both height limits7/255. An initial private probe supplied base0 in the wrong
argument slot; its invalid-input artifacts are retained separately. Corrected
worker-entry/gate traces and the five independent full-map captures agree.

The shared packaged suite now has34 complete cases, with negative tests for
uncompared random positions and missing cliff assets. Evidence:
`rms-packaged-composed54210-full-findings-2026-11-16.json` and
`rms-same-terrain-worker-oracle-2026-11-16.json`. Full original runs and separate
resource replays agree for all five seeds. This remains bounded startup evidence,
not proof of all map sizes, player counts, civilizations or later gameplay.

Validation passes all34 shared packaged cases,52 general object rows, Forest,
Gaia/owned-foundation rendering and the strict integrated RMS test. The production
audit accepts33/138 cases across eleven profiles;105 remain rejected. Desktop
preferences/menu/effects/music smoke passes using dummy video/audio.

### Connection-map boundary seeds (2026-11-16)

Seven additional fixed Tiny120/two-Empire runs of raw54210 match the complete
original runner and independent resource replay:

| Seed | Objects | Final RNG | Gaia unit/total cost |
|---:|---:|---:|---:|
| 0 | 2796 | 3947895317 | 1650 |
| 2 | 2860 | 1434243024 | 2200 |
| 3 | 2810 | 807623464 | 1250 |
| 4 | 2792 | 892659483 | 2800 |
| 42 | 2834 | 313647106 | 1400 |
| 99999 | 2777 | 2364668520 | 2550 |
| 4294967295 | 2772 | 1897108669 | 650 |

The original harness initially stopped on missing animal1197. Its previously
captured master/header referenced uncaptured task graphic39; that graphic was
loaded through the original loader before rerunning. Original probe sources
remain intact, with a separate expanded probe and asset-provenance ledger.
All ordered objects, full grids and captured poses agree; four1197 poses each
are additionally covered at seeds0/2/3/4294967295. Gaia building cost stays0 and
storage counter11 stays5. Seed4 retains its earlier full capture, independently
checked by the expanded resource replay. Evidence:
`rms-packaged-composed54210-expanded-findings-2026-11-16.json` and
`rms-composed54210-expanded-assets-provenance-2026-11-16.json`.

All41 shared packaged cases pass with ASan/UBSan, including native import and
startup resource checks, alongside the retained Forest/general-row/foundation
suites. Strict integrated RMS checks pass. No fresh live Windows session was
used for this expansion.

### Inland mixed-land and large fish (2026-11-16)

Raw54203 composes one growing neutral shallow-water land with two player lands,
40 border fuzziness on the neutral land, eight terrain rows, cliffs and two
successive elevation passes over the same ground. The first pass creates five
levels; the second creates seven. Original finalizer arrays independently confirm
the expanded terrain/elevation parameters, including the script's last-wins
scaling directives. Native generation uses the existing algorithms with a bounded
admission profile for fixed Tiny120/two-Empire/default resources.

| Seed | Biome | Objects | Final RNG | Gaia unit/total cost |
|---:|---|---:|---:|---:|
| 1 | Naboo Grass | 1717 | 3082523881 | 1900 |
| 12345 | Naboo Grass | 1724 | 3299116862 | 3750 |
| 16160 | Reytha | 1752 | 682418526 | 2300 |
| 8000 | Zaloriis | 1703 | 147225625 | 1800 |

Full ordered objects, terrain/elevation/slopes, captured startup poses and final
RNG match. Independent original resource replays also agree. Holocron exclusion
now handles the three land centers; large fish450/451/455 reuse456's integer
centers,2x2 terrain footprint, nonblocking collision and three-draw construction.
Their metadata and graphics were loaded through original loaders. Fish450/451
have35 frames at.1s and350 food;455 has30 frames at.13s and225 food.

Zaloriis explicitly creates43 type20 carbon rocks1161 after map-size scaling.
Unlike type10 resources or type30 moving resources, they consume two draws and
retain object facet0. The base constructor's random facet is cleared at54fe56
following sprite creation. The second draw selects the sole sprite frame and
still consumes RNG. Static sprite/render tracing confirms the temporary angle
is not retained;54a0f0 only dispatches graphic sounds, which are absent for5158.
Native placement now follows this distinct constructor path and preserves HP100.
The new resource remains a blocker for subsequent resource and herd placement.

Evidence: `rms-packaged-inland54203-full-findings-2026-11-16.json`,
`rms-inland54203-startup-ledger-findings-2026-11-16.json`,
`rms-inland54203-assets-provenance-2026-11-16.json`,
`rms-inland54203-carbon1161-factory-findings-2026-11-16.json`, and the individual
`rms-text-full-inland54203-1-1-SEED-oracle/resources-2026-11-16.json` captures.
This establishes bounded startup behavior; it does not certify later fishing,
resource depletion, all settings or live rendered-frame parity.

All45 shared packaged cases pass with ASan/UBSan. The retained full Forest,
52 general rows, foundation and transport checks also pass, as do strict RMS
integration, production CLI tests and the desktop dummy-video/audio smoke.
The production audit accepts36/138 attempts across twelve profiles;102 reject.

### Flat Tiny Mos Espa script behavior (2026-11-16)

Raw55014 Tiny retains the shipped malformed `else if SMALL_MAP`: after the Tiny
Sandcrawler block, the original interpreter suppresses the remaining objects,
elevation and cliffs. Native preprocessing already reproduced the conditional
behavior. The flat composition now admits the same Tatooine land/terrain pattern
and verified Gaia buildings without requiring a cliff stage.

| Seed | Objects | Final RNG | Gaia total/unit/building cost |
|---:|---:|---:|---|
| 1 | 2079 | 3909827769 | 3650/3150/500 |
| 12345 | 2097 | 3952924120 | 3600/3050/550 |
| 16160 | 2090 | 109627149 | 3850/3350/500 |

Every ordered object, grid and captured pose matches the full original runner;
independent resource replays confirm startup costs and Sandcrawler counters
(population4=-2, units11=7, military40=2). All elevation and slope cells remain0.
The shared test treats native omitted flat grids as zero-filled original grids.

`set_avoid_player_start_areas` appears inside the Sandcrawler object block.
Actual object-handler opcode77 warns and returns success without row mutation;
native parsing now preserves that specific no-op. Other unsupported directives
still reject. The resulting row has two loose groups of one object each and a
20-tile exclusion distance, rather than the previously compared10. The existing
placement algorithm reproduces it unchanged.

Evidence: `rms-packaged-mosespa55014-full-findings-2026-11-16.json` and
`rms-55014-object-avoid-starts-op77-2026-11-16.json`. These captures establish
Tiny/fixed/two-Empire startup only. Small/larger Mos Espa layouts and the suppressed
building/mountain rows remain unverified and are not enabled by this change.

All48 shared packaged cases pass with ASan/UBSan, alongside retained full Forest,
52 general rows, foundation and transport checks. Focused parser, strict RMS
integration and production CLI tests pass. The audit accepts39/138 attempts
across thirteen profiles;99 still reject explicitly.
The rebuilt desktop preferences/menu/effects/music smoke also passes with dummy
video/audio. No new live Windows or audible-mixing check was performed.

### Migration and selected-land holocrons (2026-11-16)

Raw54216 places the two player islands before a larger neutral island. Its object
rows select the neutral island using script land ID21 (internal31). Five complete
original runs cover all three biomes at fixed Tiny120/two-Empire/default resources:

| Seed | Biome | Objects | Final RNG | Gaia unit/total cost |
|---:|---|---:|---:|---:|
| 1 | Alaris | 1305 | 2998338829 | 2000 |
| 12345 | Alaris | 1273 | 729864312 | 800 |
| 16160 | Reytha | 1233 | 1961976023 | 1200 |
| 8000 | Reytha | 1305 | 2144920549 | 1050 |
| 5000 | Naboo Grass | 1278 | 2980740548 | 800 |

All ordered objects, full terrain/elevation/slopes and captured poses match.
Independent original resource replays agree (Gaia building cost0, storage11=5).
The Command Center row's group radius14 is accepted for its ungrouped single
placement. Existing selected-land resource dispatch handles the later rows.

Holocrons now support a selected neutral land: construct the global candidate
queue for unlimited maximum distance, exclude25 around every land center, force
the selected center's terrain passable for zone lookup, and retain only its
connected zone. The normal master placement gate still uses the original terrain
restriction. Temporary spacing20 removes candidates without changing persistent
eligibility. Static4de830 analysis and full captures verify these distinctions.

The selected handler also forces the last actual queued candidate after a master
gate rejection. It does not force candidates rejected by distance or zone
prefilters. A bounded original probe with nine eligible tiles(70..72,70..72),
seed1 and selected center(40,40) verifies the fallback and queue order. Eligibility
filters before queue insertion, and temporary spacing deletes future queue nodes;
native fallback therefore uses the last remaining eligible/unremoved candidate.

Real original-gate controls distinguish the paths: an entirely shallow-water map
returns gate failure for all nine candidates but creates one holocron at(71.5,72.5),
RNG2073138905 (6962 queue draws plus6 constructor draws). With only the nine
eligible tiles underwater, zone filtering creates none, RNG166830707. Empty
eligibility also creates none while retaining all6962 queue draws. Separate
forced-rejection controls prove controller flow rather than physical collision
geometry; normal-gate and minimum-distance controls are retained too.

Evidence: `rms-packaged-migration54216-full-findings-2026-11-16.json`,
`rms-migration54216-selected-holocron-findings-2026-11-16.json` and
`rms-selected-holocron-fallback-findings-2026-11-16.json`. The resource supplement
retains an inherited cosmetic case label; source/raw-script hashes and actual
rows identify54216. These remain startup comparisons, not whole-match parity.

All53 shared packaged startup cases pass with ASan/UBSan, alongside the retained
Forest/general-row/foundation/transport suites. Focused selected-holocron tests
include the real-water, zone-rejection and empty-eligibility controls. Strict RMS
integration and production CLI tests pass. The production audit accepts42/138
attempts across fourteen profiles;96 remain explicitly unsupported.
The rebuilt desktop smoke passes using dummy video/audio; no fresh live Windows
or pixel/audio-mixing comparison is claimed.

### Five-land islands and wider connections (2026-11-16)

Raw54217 combines two player lands with three neutral islands, seven elevation
levels and water connections (no cliff stage). Three full original runs cover
Kessel, Alaris and Endor at fixed Tiny120/two-Empire/default resources:

| Seed | Biome | Objects | Final RNG | Gaia unit/total cost |
|---:|---|---:|---:|---:|
| 1 | Kessel | 1726 | 3072405541 | 2050 |
| 12345 | Alaris | 1763 | 3707918282 | 2400 |
| 16160 | Endor | 1735 | 999382854 | 1350 |

All ordered objects, grids and captured poses match; independent original
resource replays agree (Gaia building cost0, storage11=5). Native startup now
admits the compared player-border combinations and five growing lands. The
existing fixed-start algorithm correctly places players inside the reduced
border rectangle. Global holocrons exclude all five centers.

The neutral islands retain internal IDs30,35,35 and owner0. Two tight groups of
four nova deposits select only ID30; the duplicate35 islands receive neither
group. Player-only connection kind63 excludes all three neutral owners and uses
water width3/variance1, replacing terrain1/22/23 with4. Exact original metadata
replays capture all99 terrain connection settings and the seven finalized terrain
rows, independently of native preprocessing.

Evidence: `rms-packaged-islands54217-full-findings-2026-11-16.json`, its three
metadata supplements and individual full/resource/comparison files. Metadata
supplement raw records begin four bytes into the16-byte land record and are
labeled[y,id,owner,nextRowX]; IDs and owners are read directly, while full land
records independently supply positions. No new game assets were required.
These captures do not establish larger sizes, other player settings or later
match behavior.

All56 shared packaged cases pass with ASan/UBSan, along with retained Forest,
52 general rows, selected-holocron edge cases, foundation and transport checks.
Strict RMS integration and production CLI tests pass. The audit accepts45/138
attempts across fifteen profiles;93 remain unsupported. The rebuilt desktop
preferences/menu/effects/music smoke passes with dummy video/audio.

### Central basin, random zones and repeated water passes (2026-11-16)

Raw54206 combines a neutral island created before the players, bordered player
lands, random player zones, twelve terrain passes, seven elevation levels,
cliffs and active player-only water connections. Three complete original runs
match at fixed Tiny120/two-Empire/default resources:

| Seed | Objects | Final RNG | Player zones | Gaia unit/total cost |
|---:|---:|---:|---|---:|
| 1 | 1278 | 1896101015 | 2,3 | 1800 |
| 12345 | 1289 | 2743095053 | 3,3 | 3150 |
| 16160 | 1284 | 1733658282 | 3,2 | 3050 |

All ordered objects, grids and captured startup poses agree. Independent resource
replays verify the same output plus owner counters, actual land metadata,
terrain/elevation arrays and all connection settings. The repeated water passes
repaint shallow/rough/deep terrain before the forest rows. The existing workers
and connection algorithms reproduce the composition without changes.

Random-zone draws execute during parsing, before later random object choices.
Native generation's existing runtime preprocessing callback preserves that
ordering. Standalone token dumps without the callback are not used as final
object-row evidence. Seed12345 verifies equal random player zones without
implicitly treating the enemy players as allies.

Evidence: `rms-packaged-basin54206-full-findings-2026-11-16.json` and individual
`rms-text-full-basin54206` full/resource/comparison captures. This remains bounded
startup parity, not evidence for other sizes, settings or subsequent simulation.

Validation: all59 shared packaged cases and retained Forest/general-object,
holocron, foundation and transport regressions pass. The strict RMS suite and
production CLI checks pass; the desktop smoke verifies60 render frames and
audio streaming without a live display or listening comparison. The production
audit accepts48/138 attempts, with90 unsupported attempts rejected.

### Two-player allied roads (2026-11-16)

Raw54218 is compared with mutual owner1/owner2 alliances, Gaia enemies, fixed
Tiny120, two Empire players and default resources. The original alliance check,
fixed-position routine, team finalizer and route worker execute unchanged.
This is an active road comparison: the two player lands form one kind64 record,
not two singleton enemy records that would produce no route.

The original fixed-position routine traverses the shuffled owner list twice,
placing each unassigned ally of the current owner. For exactly two mutual allies,
this produces the same placement order and RNG draws as the existing enemy
helper. Twelve bounded executions verify that equivalence across three seeds
and two border rectangles. Native regression fixtures retain the resulting six
coordinate/RNG cases. Larger allied groups remain rejected.

The profile preserves literal script asymmetries: Alaris paints forests50/20
but replaces terrain10 with road25; Naboo paints forests10/18 but replaces18
with road24. Road terrains24/25/38 now pass the bounded holocron validation;
placement, route search and random-number algorithms remain unchanged.

Evidence: `rms-two-player-allied-fixed-starts-proof-2026-11-16.json`,
`rms-two-player-allied-fixed-starts-oracle-2026-11-16.json` and
`rms-packaged-allied54218-full-findings-2026-11-16.json`. This establishes RMS
startup with captured prototypes and restriction initialization. Complete team
bonuses, shared sight and later allied gameplay are outside this comparison.

| Seed | Biome | Objects | Final RNG | Route draws | Changed route tiles |
|---:|---|---:|---:|---:|---:|
| 1 | Zaloriis26 | 1550 | 3127025553 | 208 | 105 |
| 5000 | Alaris12 | 1577 | 2171223389 | 264 | 243 |
| 16160 | Naboo0 | 1441 | 3058030090 | 244 | 327 |

All three native captures match ordered objects at full coordinate precision,
complete terrain/elevation/slope grids, startup poses and final RNG. The active
route counts and changed tiles above are trace observations from the original.

Independent original resource replays verify Gaia unit/total costs2450,1200,1400
for seeds1,5000,16160 respectively, zero building costs and five Gaia units.
Both player accounts retain575 total/200 unit/375 building costs. All62 shared
packaged fixtures pass with sanitizers, including native import and these counters.
Strict RMS integration, production CLI checks, fixed-start tests and the desktop
menu/render/audio-streaming smoke also pass. The20-suite algorithm run passed
before this profile addition; affected integration/startup paths were rerun after it.


### Central water and enemy starts sharing a zone (2026-11-16)

Raw54212 at fixed Tiny120/two-Empire/default resources grows a large neutral
water land before two smaller player lands. Both enemy players explicitly use
zone11; the water uses zone26. This is separate from allied diplomacy. Ten
terrain rows, seven elevation levels and cliffs compose without connections.

| Seed | Biome | Objects | Final RNG |
|---:|---|---:|---:|
| 1 | Zaloriis26 | 1734 | 3896178188 |
| 12345 | Zaloriis26 | 1771 | 696640726 |
| 16160 | Naboo0 | 1712 | 3193406899 |

All ordered records at full coordinate precision, terrain/elevation/slopes,
startup poses and final RNG match actual original execution. Existing growth,
resource grouping and three-center holocron exclusion need no algorithm changes.
The branches cover variable hunt groups, carbon rocks and the repeated Nova
quantity directive whose last value wins. Uncompared random-position settings
and missing cliff assets remain rejected transactionally.

Evidence: `rms-packaged-central54212-full-findings-2026-11-16.json` plus the
`rms-text-full-central54212` original/resource/native-comparison captures. This
remains bounded startup evidence, not later gameplay or other sizes/settings.

Independent resource replays verify Gaia total/unit costs1550,3950,2350,
zero building costs and five Gaia units. All65 shared packaged cases pass with
ASan/UBSan, retained Forest/general-row/holocron/foundation/transport regressions,
strict RMS integration and production CLI tests. The audit accepts51/138 enemy
map/seed attempts;87 remain explicitly rejected. The separate allied profile
adds three full comparisons without changing those audit figures. Desktop
preferences/menu/render/effects/music-streaming smoke passes with dummy output.

### Motherlode forest land with an inner lake (2026-11-16)

Raw54225 uses four lands: a large neutral forest, an inner neutral lake, then
two enemy player lands sharing zone11. Forest/water zones are26/25. Global
bases46/0/41 are nonforest; a forest land row does not itself select the early
forest creation path. The original phase trace distinguishes that behavior from
a forest global base.

For the compared branches, the original world registry counter is zero on entry
to the object controller. Five rectangle operations run during generation, with
no495020 scenery calls. The automatic phase runs one rectangle operation and
14400 actual495020 calls before manual RMS rows. These observations and the full
ordered object comparisons support the existing deferred scenery scan. The
registry counter is not used as a live-object count, and tracing5cfad0 alone
would miss the separate automatic factory path.

Terrain50 creates automatic carbon objects217; carbon1161 is a separate explicit
RMS row. The native profile keeps both identities, the original object order and
their different constructor draws. Holocron exclusion uses all four land centers.
No generation algorithm or callback stub was added for this profile.

Evidence: `rms-packaged-motherlode54225-full-findings-2026-11-16.json` and
`rms-text-full-motherlode54225` full/resource/native comparisons. Settings remain
fixed Tiny120, two Empire players and default resources; later simulation and
other settings are not established by these startup captures.


| Seed | Biome | Objects | Final RNG |
|---:|---|---:|---:|
| 1 | Kessel46 | 5271 | 980332046 |
| 16160 | Naboo0 | 5485 | 1304177538 |
| 8000 | Reytha41 | 5341 | 3198581591 |

All three native captures match complete ordered objects at full coordinate
precision, all three map grids, captured startup poses and final RNG. The phase
observations described above hold for all three original biome runs.

Independent resource replays verify Gaia total/unit costs1400,2100,1350 for
seeds1,16160,8000, zero building costs and five Gaia units. All68 shared packaged
cases pass with ASan/UBSan, together with the retained Forest/general-row,
holocron, foundation and transport regressions. Strict RMS and production CLI
checks pass. The audit accepts54/138 enemy configurations;84 remain rejected.
The desktop menu/render/effects/music-streaming smoke passes with dummy output.

### Owned Troop Center placement and completion order (2026-11-16)

A separate authored flat120 cliff/forest fixture appends the original54208
Troop Center87 row after Command Centers, workers and scouts. Three seeds and
a duplicate-row control run the actual owned controller and constructors with
captured Empire87 master/header/graphics/sound assets. This does not enable the
whole54208 map, its other buildings or its wall controller.

| Seed | Troop Center rows | Objects | Final RNG |
|---:|---:|---:|---:|
| 1 | 1 | 5989 | 1745024857 |
| 12345 | 1 | 5973 | 3433587427 |
| 16160 | 1 | 6065 | 3908054248 |
| 1 | 2 | 5991 | 2727130514 |

All ordered objects at full coordinate precision, map grids, captured poses and
RNG match. Four constructor draws precede completion reseeding. Local87 paints
its3×3 foundation immediately; enemy87 retains pending27 until visible rendering.
Placement uses strict footprint overlap, including mobiles and earlier buildings.
The duplicate fixture directly exercises rejection against the first87.

RMS import now completes each building's technology refresh in creation order.
Previously, importing all completed buildings together could grant marker31
before the Command Center's dependent451. The original per-owner sequence is
0,486,22,451,31,85,209; effect70 enables307. Ordinary scenario import retains its
existing initialization path. The first87 raises research counter21 from2 to3;
additional87 buildings add160carbon to recorded building cost without repeating
research or population grants. No startup resource charge is applied.

`tools/rms_owned_building_checks.h` verifies these four full fixtures, native
research order/unlock, costs, health/animation and deferred foundation rendering.
Wrong foundation metadata and randomized-position variants reject atomically.
Support remains limited to the compared flat two-Empire composition. Slopes,
mixed additional object rows and forced boundary placement remain unverified.
The subsequently verified last-candidate overlap path is described below.

Evidence: `rms-owned87-bounded-findings-2026-11-16.json`, actual-loader provenance
`rms-owned87-assets-provenance-2026-11-16.json`, and the original full/resource/
research-v2/native comparison captures under `rms-text-full-owned87`.
All68 packaged cases plus the four authored87 fixtures and retained regression
suite pass with ASan/UBSan; strict RMS and production CLI checks also pass.
The rebuilt desktop preferences/menu/render/effects/music-streaming smoke passes
with dummy output; no fresh live Windows visual/audio comparison was performed.

The owned87 guard also rejects quantity-scaling modifiers before generation.
A dedicated negative fixture prevents a scaled group count from being accepted
by the currently single-building-per-owner path; the four completion fixtures
pass again after this admission fix.


### Owned87 forced placement retains overlapping objects (2026-11-16)

Separate actual-controller probes force87 onto a forest tree, an existing scout,
and a local forest tile. All overlapping objects remain registered in state2.
The local foundation still paints nine tiles27, including four formerforest21
cells; the enemy foundation remains pending27. Original4de1b0 unlinks candidate
queue nodes, not world objects. A minimum-distance control exhausts its queue
without invoking the gate, creating a building, charging cost or granting tech.

An additional authored full map uses the87 row at min0/max0, forcing both
Troop Centers over their owners' Command Centers. Native output matches all5989
objects, complete grids/poses and RNG524140293. Import verifies that the existing
buildings survive and new buildings receive the original research/cost/foundation
state. The five owned-building regression fixtures pass with sanitizers.

Native placement now uses the original last-rejected candidate after distance
and zone filtering, preserving overlapping objects. The compared8..14 ring and
0..0 control are admitted; out-of-map forced footprints still reject atomically.
Evidence: `rms-owned87-forced-findings-2026-11-16.json` and the
`rms-text-full-owned87-on-cc` full/research-v2/native comparisons.


### Owned Prefab placement and completion (2026-11-16)

The bounded flat120, two-Empire cliff/forest startup now supports Prefab70:
one per owner, a repeated second row, and the raw54208 loose group of four.
Seven complete comparisons against the original executable match ordered object
records, terrain/elevation/slopes, final RNG, Gaia and owned animation, and
pending foundations. These are authored fixtures, not full54208 support.

| Row form | Seed | Objects | Final RNG |
| --- | --- | --- | --- |
| Single | 1 | 5989 | 1040066035 |
| Single | 12345 | 5973 | 3797195514 |
| Single | 16160 | 6065 | 1428071604 |
| Repeated single | 1 | 5991 | 2617341845 |
| Loose four | 1 | 5995 | 2723706306 |
| Loose four | 12345 | 5979 | 2206864911 |
| Loose four | 16160 | 6071 | 1753002939 |

Prefab centers snap to integer coordinates before collision checks. Each
constructor consumes seven random draws and reseeds on completion. Loose groups
use a separate radius-three candidate queue and consume the quantity-variation
draw even with zero variance. Members pass their own collision checks without
repeating the outer player-distance filter. The existing candidate queue helper
and owned-building placement path handle this; no separate map generator was added.

Each completed Prefab contributes five population capacity and25 carbon to
construction accounting. Tech16 unlocks only on the first completion, with no
research effect or repeated technology counter increment. Native import checks
these counters and chronology, HP800, reset animation, and local/deferred enemy
2x2 foundations. The same test runs retain the five Troop Center cases, including
forced overlap. Unsupported scaling, foundation metadata, population storage and
randomized start configurations reject before generation.

Evidence: `rms-owned70-mode0-findings-2026-11-16.json`,
`rms-owned70-loose-findings-2026-11-16.json`, original loader captures and
`rms-text-full-owned70-{single,double,loose}` full/research-v2/native comparisons.
All research stays local. Slopes, other civilizations, mixed building rows,
partial loose-group exhaustion and boundary fallback are not established by
these fixtures. Farm50, defense building79 and barrier117 still block full54208.


An additional loose-group seed8000 comparison matches all6088 objects, all grids,
poses and RNG1640586513. The shared68 packaged cases and twelve authored building
fixtures pass under ASan/UBSan; strict integrated RMS and standalone CLI checks
also pass. This extra seed is retained as a local full comparison, not counted
as an additional shared regression fixture.


### Wall controller investigation (2026-11-16)

Static original-executable evidence shows that class6 walls take a separate
placement path (`4df5b0`), rather than the ordinary single/group candidate loop.
It caches per-owner boundary segments produced by `499a60`, using player-start
ranges, path search, reachable-terrain filtering and a further pruning pass.
The segment walker can insert gate IDs667,659,64,88, limiting each orientation
to two insertions and skipping four wall positions for each inserted gate.
A ring drawn directly from the requested minimum/maximum distances would omit
this behavior. Gate constructors also require their own annex/head/transform
asset closure. This is investigation evidence, not enabled wall generation.

The six function records and snapshot are retained locally as
`rms-wall117-{4df5b0,499a60,49a030,49a660,49abe0,499d10}-rea-2026-11-16.json`
and `rms-wall117-rea-snapshot-2026-11-16.json`. They identify the original PE
hash and Ghidra12.1.4 provider. Full original execution, native implementation
and seed-matched wall/gate tests remain required before enabling that path.


### Owned Farm placement and completion (2026-11-16)

Five authored flat120/two-Empire cases now match the complete original runner:
three single-Farm seeds, the raw54208 five-per-owner row, and a blocked one-tile
candidate queue over the existing Command Centers. Every comparison covers
ordered objects, all grids, animation, pending foundations and final RNG.

| Case | Seed | Objects | Final RNG |
| --- | --- | --- | --- |
| Single | 1 | 5989 | 1626247642 |
| Single | 12345 | 5973 | 2524091778 |
| Single | 16160 | 6065 | 2983090062 |
| Five per owner | 1 | 5997 | 1477568208 |
| Blocked over Command Centers | 1 | 5987 | 2551954224 |

The ordinary owned placement loop now continues through the shared candidate
queue for repeated single objects. Farm50 skips forced last-candidate placement.
Actual gate controls over existing, unmodified world objects confirm that farms
ignore moving workers/scouts, but reject overlapping farms and Command Centers;
zero vertical collision extent does not mean all collisions are ignored.
The blocked full case produces no farms, no cost or research changes, and no
constructor random draws.

Completed farms have480HP and fifteen stored food, consume four constructor/
completion draws including reseeding, and record50 carbon cost each. Tech20
completes only for the first Farm per owner. Native import checks these values,
including the repeated five-Farm row, unchanged population, and local/deferred
enemy3x3 foundation terrain7. The Farm metadata guard rejects altered storage.

Evidence: `rms-owned50-mode0-findings-2026-11-16.json`,
`rms-owned50-real-gate-controls-2026-11-16.json`, original loader captures and
`rms-text-full-owned50-{single,five,on-cc}` full/research-v2/native comparisons.
Only the compared row quantities and geometry are enabled. Mixed building rows,
slopes, other civilizations, harvesting and depletion remain outside this proof.

The68 packaged cases and seventeen owned-building fixtures pass under
ASan/UBSan after this change. Strict integrated RMS checks also pass.


### Defensive-building startup and further Farm seeds (2026-11-16)

Defense79 now uses the shared owned-building candidate loop for one per player
and the raw54208 four-per-player row, at distances10..13 and spacing6. Four full
original comparisons match every ordered object, terrain/elevation/slope grid,
startup animation, pending foundation and final RNG.

| Row form | Seed | Objects | Final RNG |
| --- | --- | --- | --- |
| Single | 1 | 5989 | 3117704239 |
| Single | 12345 | 5973 | 2132019962 |
| Single | 16160 | 6065 | 510229448 |
| Four per owner | 1 | 5995 | 313705913 |

The original constructor consumes six draws including completion reseeding.
Its random initial heading is overwritten by adjacency handling: isolated
buildings finish at facet2. The verified spacing keeps these buildings apart;
touching-building configurations remain rejected. Spacing removes the inclusive
square from the current candidate queue and persists passable-cell exclusions
for the next owner's queue. Enemy1x1 foundation27 remains deferred until seen.

Native import tests verify1250HP, facet2,125 ore+25 carbon construction
accounting per building, no population grant, and tech11 completion only once
per owner. Tests also reject an unverified zero-spacing row before generation.
Evidence: `rms-owned79-findings-2026-11-16.json`, original loader provenance,
`rms-owned79-facing-static-2026-11-16.txt`, and the
`rms-text-full-owned79-{single,four}` full/research/native comparisons.

Two additional five-Farm cases now belong to the shared building regressions:
seed12345 produces5981 objects/RNG3311600017; seed16160 produces6073 objects/
RNG1320910692. Both place five Farms per owner, with food15 and one-time tech20
confirmed by independent original resource replays. No partial group was found
in these seeds; the separate blocked case still verifies zero-placement handling.

This brings the authored owned-building suite to23 cases, separate from the68
packaged cases. Full54208 still requires mixed rows, walls/gates and its actual
terrain composition; this section does not claim those are enabled.

All68 packaged and23 owned-building regressions pass under ASan/UBSan;
strict integrated RMS checks pass after the defensive-building change.


### Mixed owned-building startup (2026-11-16)

The same owned-building placement path now accepts the compared ordered mix:
Troop Center87 single, Prefab70 loose-four, Farm50 five. Each row validates its
own captured prototype before generation. The common geometry remains the
flat120, two-Empire cliff/forest fixture; other row combinations are rejected.

| Seed | Objects | Final RNG |
| --- | --- | --- |
| 1 | 6007 | 659122895 |
| 12345 | 5991 | 1730317561 |
| 16160 | 6083 | 4222011319 |

Complete native/original comparisons match all ordered objects, all terrain,
elevation and slope bytes, startup poses and pending foundations. Native import
also matches the interleaved completion order: base CC technologies,31/85/209,
then16 and20. Each owner has one Troop Center, four Prefabs and five Farms, with
resource4=21 (remaining population capacity),21=3,98=1085,164=200,165=885.
A malformed Farm prototype in the final row rejects the entire generation
atomically, checking that validation covers later rows as well as the first.

The three cases bring authored building regressions to26; packaged coverage
remains68 cases. Evidence: `rms-owned-mixed-87-70-50-findings-2026-11-16.json`,
the disjoint original-loader installer plan, and
`rms-text-full-owned-mixed-87-70-50` original/research/native captures.
This enables the proved building sequence, not the complete fortified map.

The original wall reference has separately advanced beyond static analysis:
full seed1 executes actual cached boundary generation, gates and annexes after
supplying captured Gaia83 and required original-constructor host context.
It produces6095 objects/RNG3334285395, with an independent replay matching
objects, grids, poses and boundary segments. This remains reference evidence;
no native wall generator is enabled by the mixed-building change.

All68 packaged and26 owned-building regressions pass under ASan/UBSan;
strict integrated RMS checks pass after per-row validation was introduced.


### Persistent building-row spacing and partial Prefab groups (2026-11-16)

An authored fixture now follows the packaged building order with the wall row
omitted: four Defense79, four loose Prefab70, one Troop Center87, five Farm50.
It exposed a real controller interaction: permanent tower spacing must survive
into later object rows, including the Prefab group's inner candidate queue.
The native building path now uses the existing shared row-eligibility grid
instead of resetting its own spacing mask for each building row.

| Seed | Objects | Final RNG | Prefabs, owners1/2 |
| --- | --- | --- | --- |
| 1 | 6014 | 332111031 | 4/3 |
| 12345 | 5999 | 2037640229 | 4/4 |
| 16160 | 6091 | 4185843122 | 4/4 |

For seed1, original tower placement excludes1314 cells; that exact eligibility
grid persists unchanged through the Prefab, Troop Center and Farm rows. The
second owner's inner Prefab queue exhausts after three placements. No fourth
Prefab cost or population grant occurs. Native import therefore matches
resource4=21/16,98=1685/1660,165=1485/1460 for owners1/2 respectively.
Completion order is11,16,31,85,209,20 after the base Command Center technologies.

All three full comparisons match every ordered object, all grids, animation,
deferred foundations and RNG; shared native import tests check the partial
counts and resources. Evidence:
`rms-owned-mixed-79-70-87-50-findings-2026-11-16.json` and its full/research/native
captures, including actual before/after eligibility grids for every owned row.
The29 authored cases remain distinct from the68 packaged cases. Full54208,
wall insertion and elevated building gates are still not enabled by this change.

All68 packaged and29 authored building regressions pass under ASan/UBSan,
and strict integrated RMS checks pass after sharing the eligibility grid.


### Wall boundary controller (2026-11-16)

The native controller now reproduces original499a60's obstruction grid,
consumed direction bytes, eight side routes, reachability, pruning and ordered
wall segments for three authored flat120 Empire fixtures (seeds1,12345,16160).
The shared tests construct its inputs from native-generated cliffs, surviving
scenery and starting objects; they do not replay captured object coordinates.
All16 stage hashes per seed match the original executable. Unsupported
obstructions and malformed grids reject without replacing the output.

Evidence: `rms-wall117-native-prototype-findings-2026-11-16.json` and the three
`rms-text-full-wall117-gatelist-1-1-SEED-stage-inputs-2026-11-16.json` captures.
Only the low direction byte consumed by this controller is reproduced; this is
not a claim of complete original hierarchical pathfinding. Gate creation,
wall-row integration and full packaged54208 remain separate work. These three
boundary comparisons do not increase the complete packaged-map case count.


### Wall and cardinal gate startup integration (2026-11-16)

The authored flat120 Empire wall row now runs through native RMS generation
and import. For seeds1,12345,16160, the complete original comparisons match
all ordered objects, terrain/height/slope grids, startup poses and final RNG:

| Seed | Total objects | Added walls/gates/posts | Final RNG |
| --- | --- | --- | --- |
| 1 | 6095 | 108 | 3334285395 |
| 12345 | 6076 | 105 | 222964401 |
| 16160 | 6161 | 98 | 3621490841 |

The original consumes each owner's ordinary candidate queue before special wall
dispatch, even though wall routes do not use those candidates. Native generation
preserves those draws and the constructor/completion reseeding order. Gate posts
are explicit ordered placements with parent references; RMS import suppresses
implicit annex creation, while regular scenario import retains it.

Shared tests verify all257 imported wall facets, every new object's identity,
position, HP and startup animation, exact wall costs, technology10 completion
without gate-head technology17, and absence of duplicate posts. A native gate
kill also verifies that explicit post links retain existing destruction behavior;
that is an integration regression, not a fresh original death comparison.

Evidence: the three `rms-text-full-wall117-gatelist-SEED-comparison-2026-11-16.json`
results, `rms-wall117-emission-api-findings-2026-11-16.json` and
`rms-wall117-lifecycle-review-2026-11-16.json`. Prototype guards account for
both raw DAT axes and the native asset loader's rotated footprints. This covers
one authored wall-row composition; full packaged54208 and diagonal gate dispatch
remain unadmitted. Separate controlled original diagonal-constructor evidence
is retained for the next integration step.


### Packaged Fortress54208, actual Tiny Naboo branch (2026-11-16)

The unchanged packaged map now executes through native land/elevation/terrain,
scenery, resource, tower/wall/gate, Prefab, Troop Center and Farm startup. The
original reference uses the production derived reader and actual Tiny/fixed/default
settings, with independent resource/research replays. Shared tests cover:

| Seed | Objects | Final RNG | Owner1/2 recorded total cost |
| --- | --- | --- | --- |
| 1 | 1852 | 472822087 | 2200 / 2160 |
| 12345 | 1808 | 3283216374 | 2085 / 2160 |

Every ordered object, all terrain/height/slope bytes and final RNG match. Native
import additionally checks every owned object's absolute ground height and HP,
all trailing identities/positions, wall facets, deferred foundations, population,
costs, and research order22→451→11→10→16→31→85→209→20 after native bootstrap.
Both players have remaining population capacity resource4=21. Gaia startup cost is450.

Two integration fixes were necessary: original ground-building collision uses
hard and late passes, so zero-height bird816 does not block Prefab70 while
ordinary positive-height workers still can; later scenery must include the
footprints of newly created owned buildings. Troop Center hill-mode3 checks the
original two footprint edges against the reference corner height, not the whole
footprint's height range. Modes0 used by other supported buildings do not add a
slope restriction.

Wall routing now includes the original class29/30 deposit query and retained
path-cache bytes. Actual observers verify every obstruction/direction byte in
three authored wall fixtures and both Tiny Fortress cases. Untouched shallow
water can retain the initial passable cache; recomputing the entire grid would
be wrong. The bounded reconstruction uses the verified registration update
rectangles. General terrain edits or unmatched object removals still require
ordered cache-update history; broader cache lifecycle parity is not claimed.

Evidence: `rms-fortress54208-tiny-derived-findings-2026-11-16.json`,
`rms-fortress54208-tiny-cache-findings-2026-11-16.json`,
`rms-owned-building-collision-filter-findings-2026-11-16.json`, and
`rms-building-height-findings-2026-11-16.json`. The base-reader seed1/12345
worlds also match privately (1844/1799 objects), but are not counted as Tiny cases.
Reytha/Zaloriis Fortress branches, diagonal gate dispatch, other civilizations,
settings and later gameplay remain outside this newly admitted profile.

The70 packaged cases,29 authored building cases, three authored wall cases and
strict integrated RMS checks pass under ASan/UBSan. The rebuilt production CLI
passes its file/archive tests and accepts56/138 audited attempts; both new
Fortress Tiny cases render successfully. No live Windows UI/audio check was
performed for this change.
The desktop rebuild and headless menu/game/render/audio-stream smoke also pass.


### Fortress Reytha and Zaloriis branches (2026-11-16)

Two more actual derived-reader Tiny cases match complete ordered objects,
startup poses, all grids and RNG, and pass native import/height/health/cache checks:

| Seed | Biome | Objects | RNG | Prefabs, owners1/2 | HUD population, owners1/2 |
| --- | --- | --- | --- | --- | --- |
| 16160 | Reytha | 1857 | 1539544777 | 1 / 4 | 4/10 / 4/25 |
| 8000 | Zaloriis | 1829 | 2585261580 | 3 / 4 | 4/20 / 4/25 |

The loose Prefab group stops when its placement candidates are exhausted. The
native engine does not force the requested fourth building. Remaining capacity
(resource4) is6/21 and16/21 respectively; total capacity shown in the HUD also
includes the four existing population. Recorded total costs (resource98) are
2015/2140 for Reytha and2010/2185 for Zaloriis, with exactly the original research
order and no grants for absent Prefabs.

The implementation reuses the same terrain, elevation, object and wall controllers.
Only the evidenced biome metadata and object-row composition admission changed.
Zaloriis adds the19/46/50/45/14 terrain sequence and1161/1249 object rows; Reytha
uses10/9/10/6/44 and411. Original caches now match seven fixtures (three authored,
four packaged), including every direction byte and obstruction word.

Evidence: `rms-fortress54208-tiny-derived-metadata-{16160,8000}-2026-11-16.json`,
matching full/resource-replay captures, and
`rms-fortress54208-tiny-cache-{16160,8000}-findings-2026-11-16.json`.
The shared suite now contains72 packaged cases; strict RMS and native desktop
smoke checks pass. The production CLI audit accepts57/138 configurations.
Optional resource rows and other randomized quantities are being compared
separately; these samples do not establish every seed or setting as1:1.

### Fortress optional food rows (2026-11-16)

Five additional actual-derived Tiny starts match the original complete ordered
objects, poses, terrain/elevation/slopes and final RNG:

| Seed | Biome | Objects | RNG | Branch exercised |
| --- | --- | --- | --- | --- |
| 0 | Naboo | 1866 | 3055721841 | No near forage or late hunt row |
| 13 | Naboo | 1804 | 1253119621 | Three forage, two hunt animals requested per player |
| 10 | Naboo | 1889 | 527864596 | One hunt animal requested per player |
| 4000 | Reytha | 1844 | 553662306 | Three forage, one Falumpaset requested per player |
| 7000 | Zaloriis | 1910 | 2138941861 | Two Rontos requested per player |

The shipped54103/54208 scripts independently select omitted/3/6/12 forage and
omitted/1/2/4 hunt rows. Zaloriis omits forage; hunt species are822 on Naboo,
822/1364 on Reytha,1249/1364 on Zaloriis. Admission now recognizes those choices
while retaining all other row-field and geometry checks. Placement itself uses
the existing controllers. These requested quantities are not forced when the
original placement queue exhausts.

The five permanent fixtures also retain original cache, resource, research,
owned height/HP and imported population expectations. Seed10 creates only three
Prefabs for owner2: remaining capacity16, HUD4/20. The other new starts have
remaining capacity21 per owner. Original cache observers independently reproduce
the complete baseline worlds before exporting cache hashes.

Evidence: `rms-fortress54208-derived-sweep-findings-2026-11-16.json`,
`rms-fortress-0-13-10-case-initializers-2026-11-16.json`,
`rms-fortress-4000-7000-case-initializers-2026-11-16.json`, and the corresponding
`rms-fortress54208-tiny-derived-native-{seed}-2026-11-16.json` comparisons.
Nine full Fortress samples establish these branches, not arbitrary seeds,
civilizations, map sizes, diagonal gates or subsequent gameplay parity.

Validation:77 packaged cases plus the retained Forest/general-row/authored
building/wall suites pass under ASan/UBSan. The production CLI audit with the
three actual preset symbols still accepts57/138 attempts (19 resources);
acceptance is not full-original comparison coverage. CLI file/archive tests,
strict RMS tests and desktop headless menu/game/render/audio-stream smoke passed.
No live Windows UI or audio comparison was performed in this continuation.

### Shoreline54205 mixed land, cliffs and water connections (2026-11-16)

Six full original comparisons use the actual derived-reader Tiny/fixed/default
presets. Private evidence files use the label `coastal54205`.

| Seed | Biome | Objects | RNG | Gaia unit costs |
| --- | --- | --- | --- | --- |
| 1 | Zaloriis | 1733 | 1818769083 | 2300 |
| 12345 | Reytha | 1715 | 3269173345 | 2000 |
| 16160 | Yavin | 1723 | 31752613 | 1600 |
| 4000 | Kashyyyk | 1689 | 622934016 | 1950 |
| 5000 | Krant | 1713 | 1581241569 | 1450 |
| 8000 | Naboo | 1704 | 1841981369 | 1250 |

The existing land/elevation/terrain/cliff/connection/object algorithms reproduce
every ordered object, XY position, facet, startup pose, grid byte and final RNG
in these references. Native import also matches all823 retained owned/Gaia
poses and health values, deferred foundations, player costs and population.
Player costs remain575 (200 units,375 buildings), population used4 with remaining
capacity1. Gaia costs are observed directly in each full reference; seed1 also
has an independent complete replay matching objects, poses, grids and resources.

The admission predicate checks the six exact biome terrain profiles, seven
height rows, all99 connection settings, and original cliff settings
5..8/4..10/curl10/water2/cliff3. It admits three per-player land targets
3744/4320/4680 and four original border arrangements, keeping both player rows
consistent. A34-case original parser sweep covers every biome and11 of the12
land-target/border combinations; the six full references cover each individual
choice. This is not an exhaustive cross-product or arbitrary-seed proof.

Evidence: `rms-coastal54205-derived-original-findings-2026-11-16.json`,
`rms-coastal54205-derived-geometry-sweep-findings-2026-11-16.json`,
`rms-coastal54205-six-biome-matrix-2026-11-16.json`, and
`rms-coastal54205-branches-4000-5000-8000-findings-2026-11-16.json`.
Shared packaged coverage is83 cases. ASan/UBSan regressions, strict RMS checks,
CLI file/archive checks and rebuilt desktop headless smoke pass. The rebuilt
production audit accepts60/138 configurations across20 resources; acceptance
alone is not original parity evidence. No live Windows/UI/audio comparison.

### Neutral resource islands and player zones (2026-11-16)

Actual derived-reader Tiny/fixed/default full references now cover Satellites54211
and Watermass54202. Both create two neutral resource islands after the player
lands: original source IDs20/23 become finalized IDs30/33. The existing target
selector and object controller preserve those identities and resource placement.

| Resource | Seed | Biome | Player zones | Objects | RNG | Gaia unit costs |
| --- | --- | --- | --- | --- | --- | --- |
| 54211 | 1 | Alaris | 1 / 2 | 1137 | 3534373653 | 1250 |
| 54211 | 4000 | Naboo | 1 / 2 | 1119 | 50054625 | 1150 |
| 54211 | 16160 | Zaloriis | 1 / 2 | 1109 | 4264635455 | 1450 |
| 54202 | 1 | Yavin | 2 / 3 | 1145 | 2022788982 | 1500 |
| 54202 | 4000 | Endor | 18 / 18 | 680 | 194355643 | 1200 |
| 54202 | 16160 | Reytha | 3 / 2 | 687 | 3389064271 | 2250 |
| 54202 | 8000 | Zaloriis | 18 / 18 | 1126 | 810679061 | 2400 |

All seven comparisons match complete ordered records, XY positions, facets,
startup poses, terrain/elevation/slopes and RNG. The permanent native tests also
verify952 retained poses/health values, deferred foundations, land ownership/IDs,
costs and population. Every player retains cost575 split200/375, population used4
and remaining capacity1. Gaia has five holocrons, no population usage, and no
building cost. These counters are captured directly in each full original run.

One bounded geometry predicate reuses the original controllers for both maps.
It validates the shared four-land layout, exact terrain rows, borders and
land-ID order, while distinguishing ordinal Satellites zones from Watermass's
shared18 or randomized2..4 zones. The original runs above cover shared zones and
both observed random assignments; zone4 and arbitrary seed combinations are not
claimed as newly full-map compared. Other sizes, civilizations and team setups
remain outside this profile.

Evidence: `rms-islands54211-derived-original-findings-2026-11-16.json`,
`rms-watermass54202-derived-original-findings-2026-11-16.json`, and corresponding
`rms-{islands54211,watermass54202}-tiny-derived-native-{seed}-2026-11-16.json`.
The90 packaged regressions, retained authored suites, strict RMS checks and
CLI file/archive checks pass. No live Windows UI/audio comparison was performed.
Rebuilt desktop headless menu/game/render/audio-stream checks also pass. The
production CLI audit now accepts66/138 configurations across22 resources;
remaining rejected profiles still fail explicitly rather than silently using
a different generator.

### Arena54223 early trees, holocrons and wider walls (2026-11-16)

Three actual derived-reader Tiny/fixed/default full runs match generated active
objects, original IDs/positions/facets, startup poses, all grids and final RNG:

| Seed | Biome | Active records | Retained state7 records | RNG | Owner costs98,1/2 | Gaia costs98/164 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Endor | 7009 | 157 | 1629495980 | 930 / 1100 | 1500 |
| 4000 | Alaris | 8256 | 141 | 2591708538 | 940 / 1000 | 1300 |
| 16160 | Krant | 6911 | 116 | 3017032858 | 920 / 990 | 1550 |

Raw nonnull world-table counts are7166/8397/7027. Lifecycle captures identify
exactly157/141/116 early-tree IDs becoming state7 during automatic scenery;
they remain state2 through the connection stage in these samples. They are
excluded from active comparisons using the observed states, not inferred from
unit type or position. Native import omits inactive entries while preserving
all creation-ID gaps; it does not claim the same retained-pointer representation.

The map reuses the existing land, dry connection and early-forest controllers.
Connection postprocessing explicitly smooths terrain1 (4e432e/4e4339). Saved
assembly shows its setter is reached only through matching terrain1 neighbors;
a raster without1 and replacements that cannot introduce1 therefore has no
smoothing setter callbacks. The admitted Arena geometry/table enforces those
conditions, and generation also rejects a wet raster before connecting it.

Four separate original controls corroborate the lifecycle distinction. Dry
forest10/20/21 routes repaint429 cells without deleting early trees. One early
tree standing on repainted ground survives final scenery; the other58 are
replaced and become state7. A no-connection sibling replaces all59. This is why
native code must not delete a tree simply because a route repaints its tile.
Water/forest interactions remain rejected outside the evidenced profile.

Arena's holocron row additionally requires the biome's ground terrain, exclusion
30 from every land center, and permanent spacing5. The shared holocron helper
now implements those modifiers and exports the restricted eligibility mask for
later rows; legacy temporary spacing20 and Migration fallback remain unchanged.
The wall controller now accepts the original18..22 bounds and receives bounds
from the RMS row. Original cache observers and native fixtures match obstruction
words and every direction byte for all three Arena starts. The full original
11-stage captures and ordered segments remain local for deeper comparisons.

The reused walled-startup tests cover all active object IDs, native positions,
owned height/HP, completion animation, costs, research order and population.
Arena has used population4, remaining capacity1 and building counter2 for both
players; it does not acquire Fortress's additional building bonuses. Negative
checks cover unsupported holocron modifiers and mismatched wall-distance pairs.

Evidence: `rms-arena54223-actual-tiny-seed1-findings-2026-11-16.json`,
`rms-arena54223-branch-lifecycle-findings-2026-11-16.json`,
`rms-arena54223-case-initializers-2026-11-16.json`,
`rms-arena54223-branch-cache-findings-2026-11-16.json`,
`rms-54223-early-forest-connections-gap-review-2026-11-16.json`, and
`rms-54223-dry-forest-connections-controls-findings-2026-11-16.json`.
Other map sizes, civilizations, diagonal gate emission, wet forest connections
and subsequent gameplay are not established as1:1 by these startup comparisons.

Validation: all93 retained packaged startup cases plus the general/authored
regressions pass, as do strict integrated RMS, production CLI file/archive
checks and the desktop headless menu/render/audio-stream smoke. The actual
Tiny/default production audit accepts69 of138 configurations across23 map
resources; this is an acceptance count, not a percentage of engine parity.

### Tundra54219 barren trees, Cu-pa and ice shores (2026-11-16)

Actual-derived Tiny/default, two-Empire starts now match the full original
runner for seeds1/4000/16160/8000:2765/2704/2723/2692 active objects and final
RNG2106313254/374573099/2541837053/3515303544. These cover all three
source biome branches and both bordered-land orientations. All ordered object
IDs, owners, units, facets and exact positions match, as do all14400 terrain,
elevation and slope cells and captured owned/Gaia startup animation poses.

The geometry reuses existing controllers: two snow32 lands, five elevations,
cliffs and fourteen terrain rows including flat ice patches and barren forests.
The profile retains explicit bounds on map size, civilization, land metadata,
terrain/elevation rows and cliff settings. Native import additionally checks
473 original animation/health records, resource accounting and population.

Explicit barren-tree351 placement now uses its actual static graphic5295 and
carbon100 catalog. Cu-pa1367 uses the existing hunt constructor with verified
HP10, food170, speed1.2, restriction26 and graphic80 (9 frames, duration0.12,
replay0.5). Masters, task headers, graphics and sounds were loaded through the
original routines with byte-consumption and relocation checks before full
startup execution. No placeholder master or skipped constructor was used.

The first comparison exposed a real placement mismatch: original48e33d–48e352
adds side terrain37 when either required side terrain is35. Shore fish69 must
therefore accept cardinal neighbors2/35/37. All33 fish in the first three
references required37; restoring this rule also restores subsequent object
placement and final RNG. This is an explicit original rule, not a rendering
alias approximation. Negative tests reject modified351/1367 health metadata
without changing output or caller RNG.

Evidence: `rms-tundra54219-full-startup-findings-2026-11-16.json`,
`rms-tundra54219-geometry-findings-2026-11-16.json`,
`rms-tundra54219-assets-provenance-2026-11-16.json`, and
`rms-tundra54219-shorefish-side37-findings-2026-11-16.json`. Other sizes,
civilizations and subsequent gameplay are not established as1:1 here.

Validation:97 retained packaged startup cases and the general/authored suites
pass, including native import and invalid-metadata checks. Strict RMS,
production file/archive tests and desktop menu/render/audio-stream smoke pass.
The production audit accepts72 of138 configurations across24 map resources;
acceptance is not a whole-engine parity percentage.

### Yavin55003 temples, ruins and hill placement (2026-11-16)

Actual-derived Tiny/default two-Empire starts match the complete original
runner at seeds1/12345/16160:7881/7917/7891 objects with final RNG
2948084886/449702241/1053053069. Comparisons cover all ordered IDs, owners,
units, facets and exact positions; all14400 terrain/elevation/slope cells; and
captured owned/Gaia animation state. Both mountain1017/1018 alternatives occur.
The geometry uses existing land, elevation, terrain and object controllers.

New catalog entries are Main Temple1295, static Temple1296, Ruins1052,
Plant B1120, water rocks1172/1173 and mountains1017/1018. Their actual master,
graphic, sound and applicable task-header records were loaded through original
readers with exact consumption and relocation checks.1296 is type10 despite
sharing artwork with the type80 ruins; it has no building completion callback.

The full reference exposed two behavioral gaps. Main Temple1295 consumes three
constructor draws followed by a completion draw/reseed; ordinary Ruins1052
consume six plus the completion draw/reseed. An independent authored original
control confirmed both and the single non-reseeded draw for1296. The temple's
hillMode3 gate checks only the footprint's left column and last row against
their shared corner's elevation, allowing a difference of one. This is the
48e697–48e6b6 rule, not an all-flat requirement. Native grouped placement now
accepts an elevation raster for that check and rejects missing/malformed data.
Every captured original temple height-gate success/failure matches the rule.

Native import fixtures include222 startup animation/health records, building
heights and deferred foundation27 for1295. Gaia building costs are4550 in all
three references (4500 temple plus two25-cost ruins); unit costs vary. Both
players retain575 startup cost and population4/5. Negative checks reject
modified new-scenery prototypes and absent/malformed temple elevation while
preserving caller RNG and output state.

The dense forest's original zone calculations exceed the earlier500-million
instruction allowance. The harness retains all guards and executes bounded
500-million-instruction chunks (maximum three), saving progress at each limit;
these runs completed after the first checkpoint. No cache or constructor was
skipped to obtain a result.

Evidence: `rms-yavin55003-full-startup-findings-2026-11-16.json`,
`rms-yavin55003-assets-provenance-2026-11-16.json`,
`rms-yavin-constructor-control-findings-2026-11-16.json`, and
`rms-yavin1295-hillmode3-gate-checks-2026-11-16.json`. These startup cases do
not establish all sizes, civilizations, later terrain changes or gameplay as1:1.

Validation:100 retained packaged startup cases plus the general/authored
regressions pass. Strict integrated RMS, production file/archive checks and
desktop menu/render/audio-stream smoke also pass. The production audit accepts
75 of138 configurations across25 map resources; this is acceptance coverage,
not a percentage of complete engine parity.

### Eredenn55017 neutral lake and frozen scenery (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match the full
original runner:2164/2200/2209 objects, final RNG2210266758/2436159394/3370259064.
All ordered IDs, owners, units, facets and exact positions match, alongside
terrain/elevation/slope grids and captured startup animations. Native import
fixtures retain263 original animation/health records and resource counters.

The bounded profile adds two snow player lands followed by one neutral water
land of288 target tiles. Existing position resolution produces the original
neutral centers(68,71)/(55,51)/(70,93); no new random-placement algorithm was
needed. Six terrain rows, four elevation levels and cliff settings5/8/4/10/10/2/3
are checked explicitly. Unlike Tundra, the ice35 row is not flat-only.

Frozen plant1122 reuses zero-height static decoration placement. Seafood458
reuses animated type30 placement with its actual half-tile footprint, resource17
amount225 and graphic5274 (12 frames, duration0.13). Original master/graphic/
sound captures and static sprite-list layout evidence precede comparison;
pose capture verifies that the selected sprite points at the standing graphic.
These zero-height objects do not block subsequent placement. Prototype mutation
checks reject invalid frozen-plant/seafood data atomically.

Evidence: `rms-eredenn55017-full-startup-findings-2026-11-16.json`,
`rms-eredenn55017-geometry-findings-2026-11-16.json`,
`rms-eredenn55017-assets-provenance-2026-11-16.json`, and
`rms-eredenn55017-sprite-layout-static-2026-11-16.txt`. Other sizes, civilizations
and later gameplay remain outside these startup comparisons.

Validation:103 retained packaged startup cases and the general/authored
regressions pass, plus strict RMS, production file/archive tests and desktop
menu/render/audio-stream smoke. The production audit accepts78 of138
configurations across26 resources; acceptance is not whole-engine parity.

### Geonosis55018 cliffs, wildlife and buildable scenery (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match the original
runner:2377/2392/2382 objects and final RNG3421012888/1724592410/1712025059.
Ordered IDs, owners, units, facets and exact positions match, as do all three
terrain/elevation/slope grids and captured startup poses. Native import retains
194 original animation/health records plus startup resource/population checks.

The bounded geometry profile reuses existing land and cliff algorithms for a
central neutral desert land followed by two bordered player lands with randomized
zones. Cliff settings are5/10/8/25/1/2/2. The secondary carbon row deliberately
retains the shipped script's base44 even though the map starts on terrain26.
No elevation or connection stage is enabled for this profile.

Captured masters/graphics/sounds/task headers add Orray1471, Acklay1459,
Nexu1461, Reek1463, stalagmites573/582/583, vent616, mountain589 and ore1159.
Orray retains its original hunting class and one-tile clearance. Ore uses its
original twelve-angle graphic. Static scenery uses the original radius-based
position snap and consumes no animation draws for sequence0 graphics.

The third seed exposed an existing collision-rule gap: Nova66 overlaps
stalagmite583 at116.5,7.5 in the original. Original placement48e180 skips
blockers whose master canBeBuiltOn byte is1 at48e8b0, before dimension checks.
The actual master vtables confirm that deposits and these stalagmites share
that gate. The shared native controller now omits buildable-over scenery from
both later-row and within-row blockers. This does not broaden the separately
overridden building placement controller.

Evidence: `rms-geonosis55018-full-startup-findings-2026-11-16.json`,
`rms-geonosis55018-geometry-findings-2026-11-16.json`,
`rms-geonosis55018-assets-provenance-2026-11-16.json`, and
`rms-geonosis55018-buildable-collision-findings-2026-11-16.json`.
Other sizes, civilizations, arbitrary seeds and later gameplay remain outside
these three complete startup comparisons.

Validation:106 retained packaged startup cases plus the general/authored
regressions pass under ASan/UBSan. Strict integrated RMS, production file/archive
checks and desktop menu/render/audio-stream smoke pass. The production audit
accepts81 of138 configurations across27 resources; this measures acceptance,
not whole-engine parity.

### Space Mass54214 elevated asteroid startup (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match the full original
runner:1291/1281/1266 objects, RNG3549801262/3681008734/3199497979. Ordered
object records, exact positions, terrain/elevation/slope grids and startup poses
match. The fixtures add82 animation/health/height records, along with transport
population, military counters and cost accounting (735 total per player).

This bounded profile reuses existing geometry for space16 surrounding three
rock42 lands, six terrain rows,27 derived-default elevation rows and kind63
connections replacing space16 with rock42. Complete connection settings and
all land/terrain/elevation parameters are checked before admission. The selected
maps contain heights0 through7; no cliff stage is requested.

Actual captures supply Mynock1203, food crate615, carbon1117 and asteroid
craters1332/1333/1334. Crates retain type20 construction and facet reset; carbon
uses type30 appearance. These crater IDs differ from desert craters1329–1331:
their radii decrease2/1.5/1, their restriction is28, and large crater1332 is not
buildable-over. All three have zero collision height and do not block later
objects. Mynocks use their actual hunting metadata and one-tile clearance.

Air transports retain relative flight height1. Original54fc2c/54fd73 computes
interpolated ground height plus1; placement5c92e0 uses XY overlap, with collision
height only deciding whether an obstacle participates. Nonzero terrain elevation
alone therefore does not change its overlap rule. All six captured transports
spawn on ground height0; nonzero-ground flight arithmetic is statically proven,
not observed in these three startup placements. Native imported object height
is now checked as ground interpolation plus flight offset.

Evidence: `rms-spacemass54214-full-startup-findings-2026-11-16.json`,
`rms-spacemass54214-geometry-findings-2026-11-16.json`,
`rms-spacemass54214-assets-provenance-2026-11-16.json`, and
`rms-spacemass54214-air-height-gate-review-2026-11-16.json`.
Other sizes, civilizations, arbitrary seeds and later gameplay remain unverified.

Validation:109 retained packaged startup cases and the general/authored suites
pass under ASan/UBSan, including native import ownership and absolute object
heights. Strict RMS, production file/archive checks and desktop menu/render/
audio-stream smoke pass. Production accepts84 of138 tested configurations
across28 resources; this is bounded acceptance coverage, not total parity.

### Geddes55007 coupled land-size variant (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match1272/1273/1250
original objects and final RNG55711408/2109840983/3123251575. All ordered object
records, exact positions, three terrain grids and captured startup poses match.
Native fixtures add81 animation/health/height records and transport counters.

Geddes shares Space Mass's elevation, connections, assets and object controllers.
Its coupled geometry variant changes only the neutral target to1440, each player
target to3600 and the final terrain patch to720. These values are admitted
together, not as independently interchangeable choices. One or two trailing
mountain1019 rows reuse their previously captured master/graphic data and
existing constructor; full startup confirms both optional-row outcomes.

Evidence: `rms-geddes55007-full-startup-findings-2026-11-16.json`,
`rms-geddes55007-geometry-findings-2026-11-16.json`, and
`rms-geddes55007-assets-provenance-2026-11-16.json`. These three Tiny/two-Empire
startup comparisons do not establish other sizes, civilizations or later gameplay.

Validation:112 retained packaged cases plus general/authored regressions pass
under ASan/UBSan, as do strict RMS, production file/archive checks and desktop
menu/render/audio-stream smoke. Production accepts87 of138 configurations
across29 resources; acceptance is not a complete-parity percentage.

### Space Satellites54215 specific-land resource placement (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match1037/1049/1035
original objects and final RNG2907172999/1556233438/3567316545. Ordered records,
exact coordinates, all three grids and captured poses match. Native fixtures
retain59 animation/health/height records and transport/resource counters.

The four-land profile places two2880-tile player lands followed by two144-tile
neutral lands with IDs30/33. Its four terrain rows reuse the same27 elevation
rows as Space Mass/Geddes, but omit both connections and cliffs. The shared
elevation validation is reused; all profile-specific land and terrain fields
remain explicit.

Existing general object dispatch places the selected Nova groups on land30 and
Ore groups on land33 without a new placement algorithm. The full comparison
covers target selection, same-zone filtering and grouped placement on the small
islands, followed by transport startup without connected causeways. This is
additional coverage of the general controller, not just terrain-image similarity.

Evidence: `rms-spacesatellites54215-full-startup-findings-2026-11-16.json`,
`rms-satellites54215-geometry-findings-2026-11-16.json`, and reused Space Mass
asset/air-placement provenance. Other sizes, civilizations, arbitrary seeds
and later gameplay remain unverified.

Validation:115 retained packaged startup cases plus general/authored suites
pass under ASan/UBSan. Strict RMS, production file/archive checks and desktop
menu/render/audio-stream smoke pass. Production accepts90 of138 configurations
across30 resources; this remains acceptance coverage rather than total parity.

### Planets and Moons54213 selected-land holocrons and wildlife (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match1207/1214/1230
original objects and final RNG108857928/3607444516/1775171975. Ordered object
records, exact positions, terrain/elevation/slope grids and captured startup
poses match. Native import fixtures add56 animation/health/height records plus
resource and transport counters.

Two2160-tile player lands precede an8640-tile neutral rock42 land with ID31.
The bounded profile checks asymmetric neutral borders, six terrain rows and the
shared27 space elevation rows. Neither cliffs nor connections are present.

The original selected-holocron row has precisely the same parameters as the
previously compared Migration row:land31, five single holocrons, minimum25,
temporary spacing20. Only the profile admission changes; the existing selected
zone and distance controller remains intact. Full comparisons also cover
selected loose Mynock groups of1/2/4 and player-scaled Nova/Ore groups on land31.
No separate algorithm or fabricated object metadata was introduced.

Evidence: `rms-planets54213-full-startup-findings-2026-11-16.json`,
`rms-planets54213-geometry-summary-2026-11-16.json`, and reused Space Mass asset
and air-placement provenance. Other sizes, civilizations, arbitrary seeds and
later gameplay remain outside these startup comparisons.

Validation:118 retained packaged startup cases and the general/authored suites
pass under ASan/UBSan, plus strict RMS, production file/archive checks and
desktop menu/render/audio-stream smoke. Production accepts93 of138 tested
configurations across31 resources; this is acceptance coverage, not total parity.

### Geonosis expanded borders and maximum-RNG zone endpoint (2026-11-16)

Additional full original comparisons for seeds4000/8000/3717145935 match
2278/2353/2323 objects and RNG3481714273/472618545/557507208 respectively,
including ordered records, exact coordinates, all grids and captured poses.
Together with the first three seeds, these cover all four player-border
rectangles, equal player zones and additional food/hunt branches. Geonosis now
retains333 startup pose/health fixtures across six seeds.

The final seed was derived by inverting the original LCG, not found by changing
the script. Original4e30d1–4e3100 computes floor(rand15*playerCount/32767)+2;
with two players, the maximum draw32767 produces zone4. Actual parser execution
confirms zones[0,4,2] at seed3717145935, and the complete original/native startup
matches without an engine change. Other seeds and world configurations remain
outside this bounded coverage.

Evidence: `rms-geonosis55018-expanded-seed-inventory-2026-11-16.json`,
`rms-geonosis55018-zone4-findings-2026-11-16.json`, and
`rms-geonosis55018-expanded-seed-startup-findings-2026-11-16.json`.

Validation:121 retained packaged startup cases and general/authored regressions
pass under ASan/UBSan. This change adds fixtures only; production acceptance
remains93 of138 configurations across31 resources.

### Swamp54221 wet connections, forests and Faamba (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match1873/2332/1757
original objects and final RNG2762582302/3056218634/225713344. Ordered IDs,
owners, units, facets and exact positions match, as do the three grids and
captured poses. Native fixtures retain467 animation/health/height records and
original startup cost/population counters.

Two4680-tile dirt11 player lands precede three144-tile neutral lands (IDs30/35/35).
The bounded profile includes randomized player zones, one excluded map border,
twelve terrain rows and exact kind63 wet connection settings. It has no elevation
or cliffs. The existing connection and automatic forest algorithms already match:
terrain17 creates tree414, while terrain19 creates350. The water base means this
case does not enable the still-unverified early-forest wet-connection lifecycle.

Actual prototype captures add explicit414 tree placement and Faamba48. Faamba
is hunting class4 with radius0.5, clearance1, speed0.8, food250 and cost200 food;
it reuses the validated hunting constructor, not the class60 large-hunt branch.
Invalid tree/Faamba metadata and unverified random-start settings reject atomically.

Evidence: `rms-swamp54221-full-startup-findings-2026-11-16.json`,
`rms-swamp54221-geometry-summary-2026-11-16.json`, and
`rms-swamp54221-assets-provenance-2026-11-16.json`. Other sizes, civilizations,
arbitrary seeds and later gameplay remain outside these comparisons.

Validation:124 packaged startup cases and general/authored regressions pass
under ASan/UBSan; strict map checks, production CLI build and desktop dummy-driver
smoke pass. The production acceptance audit accepts96 of138 configurations across
32 resources; this is acceptance coverage, not a whole-engine parity percentage.

### Naboo Swamp54222 plants and team connections (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160 match5334/5355/5309
original objects and final RNG3553018566/4134424987/1844835132. Ordered object
IDs, owners, types, facets and exact positions match, as do terrain, elevation,
slope grids and captured poses.

Two6840-tile grass player lands precede nine terrain rows. The original includes
a self-replacement grass row and a dirt-forest replacement row even when that
source terrain is absent; neither is simplified away. Two authored connection
commands expand to three finalized records: the enemy-team command produces two
singleton records. Existing native team expansion matches without a new controller.
The new profile checks all99 terrain settings for both source commands.

Original plants1121/1125/1127 use graphics5247/5251/5253, with6/6/12 facets.
They share the existing zero-height, nonblocking plant placement path. Mutated
plant metadata and unverified random starts reject atomically.

Evidence: `rms-nabooswamp54222-full-startup-findings-2026-11-16.json`,
`rms-nabooswamp54222-geometry-summary-2026-11-16.json` and
`rms-nabooswamp54222-assets-provenance-2026-11-16.json`. Other seeds, map sizes,
civilizations and later gameplay remain outside this bounded comparison.

Validation:127 packaged startup cases and general/authored regressions pass
under ASan/UBSan, including274 new pose/health/height fixtures. Strict map checks,
production CLI checks and desktop dummy-driver smoke pass. Production acceptance
is99 of138 configurations across33 resources, not a parity percentage.

### Ice Lake54226 bare connections and alternate patches (2026-11-16)

Actual-derived Tiny/default two-Empire seeds1/12345/16160/8000 match
1764/1777/1810/1716 original objects and final RNG2333833765/1892348138/
3913502968/2653551361. Ordered IDs, owners, types, facets, exact positions,
terrain/elevation/slope grids and captured poses match. The first three seeds
use snow-dirt47; seed8000 exercises the alternate grass41 branch.

The original accepts a bare connection command: Ice Lake comments out its whole
settings block. Native parsing now retains the default99 settings without braces,
including at EOF and before another section. Malformed explicit blocks still fail.
A parser regression compares bare and empty-braced forms. Full original execution
confirms one kind63 record with width1, variance0, replacement-1 and cost1.

The bounded profile has a11520-tile ice35 neutral lake and two2160-tile snow32
player lands, seven elevation layers, original cliffs and five terrain rows.
It reuses existing cliff, snowy forest, connection and object controllers.
Missing cliff metadata and unverified random starts reject atomically.

Evidence: `rms-icelake54226-full-startup-findings-2026-11-16.json` and retained
`rms-icelake54226-settings-tiny-derived-inventory-*` files. Other map sizes,
civilizations, seeds and later gameplay remain outside these comparisons.

Validation:131 packaged startup cases (119 general and12 walled cases), including
276 new pose/health/height fixtures, pass under ASan/UBSan. Strict map checks,
production CLI checks and desktop dummy-driver smoke pass. The production audit
accepts102 of138 configurations across34 resources; this is not a parity score.

### Automated unseen-seed comparisons (2026-11-16)

`script/compare_rms_original.py` replaces the per-profile comparison scripts for
new differential runs. It takes a local JSON manifest, executes at most two
original probes concurrently, then compares the native snapshot against each
original capture. It reports `match`, `mismatch`, `native_rejected`,
`native_error`, `original_error` and harness `error` separately. A rejection is
not a match. Missing object/grid tails count as differences; errors do not
silently skip cases. Reports retain the first object/grid difference, counts,
RNG, pose differences, input hashes and native executable hash. Output directories
must be new; original captures may be replayed with `--existing` only when their
seed, civilizations, probe hash and RMS source hash agree.

The adapter tests Tiny120/default resources, two enemy Empire players, with
fixed or randomized placement selected explicitly per case. It compares ordered object ID/owner/type/facet/XY, complete terrain,
elevation and slope grids, final RNG and captured poses. It does not compare
health, absolute object height, runtime simulation or UI. Wider map sizes and
player/civilization combinations require independently verified original probe
contexts; successful seeds here must not be presented as coverage of those axes.

Build with `cmake --build <build> --target swgb-rms-snapshot`, or on this Mac use
`python3 script/build_rms_snapshot.py`. The latter uses the same core source list
and Apple Clang with ASan/UBSan. CMake is not installed on the reference Mac;
the Apple Clang build is the tested route. Run the asset-independent comparator
checks with `python3 script/test_compare_rms_original.py`.

A local manifest is a JSON array. Each entry contains `id` (unique safe name),
`resource` (DRS RMS ID), `seed` (uint32), optional `positions` (`fixed`, the
default, or `random`), and absolute paths `probe`, `oracle`, `source`. The probe is a retained controlled-original Python runner; `oracle` is
its expected full JSON capture, and `source` is the matching extracted RMS file.
No original executable, game assets or private capture files are committed.

```
python3 script/compare_rms_original.py \
  --manifest /absolute/path/cases.json --output /absolute/path/new-run \
  --native /absolute/path/swgb-rms-snapshot --data /absolute/path/Data \
  --python /absolute/path/unicorn-venv/bin/python --jobs 2
```

The first broad run exposed an export defect in the old diagnostic probe: empty
native elevation/slope vectors mean a flat map, but it wrote zero-byte files.
Old `zip`-based comparisons overlooked their missing tails. The new snapshot
writes every implicit zero tile; the comparator still rejects truncated files.
This changes diagnostic serialization, not terrain generation behavior.

Wave1: seeds20261116 and3141592653 were absent from retained full-map fixtures.
All8 cases across Ice Lake54226, Swamp54221, Space Mass54214 and Geonosis55018
match with the corrected snapshot, without engine or profile changes. Original
captures were newly executed, then replayed against the reproducible repository
build. Evidence: `rms-unseen-seeds-wave1-report-2026-11-16.json`. Eight
asset-independent comparator tests cover truncation, extra objects, RNG, poses,
malformed records, duplicate IDs, complete CLI reporting and stale source rejection.

Wave2: the same two unseen seeds also match all8 cases across Naboo Swamp54222,
Planets and Moons54213, Geddes55007 and Yavin55003. No engine changes or new
profile exceptions were needed. Evidence: `rms-unseen-seeds-wave2-report-2026-11-16.json`.
An initial run failed to import Unicorn because resolving the virtualenv Python
symlink selected the base interpreter; the runner now preserves executable
symlinks. Those failures remain classified as execution errors, not engine
mismatches. Final reports replay against one stable native binary and verified
probe/source hashes.

The subsequent shared-code investigation below tests original field selection
before replacing the bordered-start map-name allowlist.

Wave3: boundary seeds0 and2147483648 match8 cases across Tundra54219, Eredenn55017,
Swamp54221 and Space Mass54214. This brings the unseen matrix to24 comparisons
across10 map families. The two boundary seeds intentionally exercise unsigned
RNG state behavior; they do not represent independent initial random streams.
Evidence: `rms-unseen-seeds-wave3-report-2026-11-16.json`.

### Shared fixed-start bounds aggregation (2026-11-16)

The original4e7750..4e77a7 scans every player-owned land, takes the minimum usable
width/height after subtracting twice each radius, and takes maximum left/top
origins after adding each radius. Neutral lands do not constrain the starts.
This differs from using the first player land and from rectangle intersection.

84 controlled original cases varied first, second and both player lands' borders
and radii, with/without a preceding neutral land, both directly and through full
finalization. A further18 cases distinguish the aggregate formula from rectangle
intersection: shifting only the second rectangle by7 changes both starts by7
without reducing the usable span. These are controlled post-parser mutations,
not a claim that shipped RMS independently specifies each player's rectangle.

The shared fixed-start implementation now computes those aggregates. The existing
map pipeline uses it and no longer has a map-name allowlist for fixed bordered
starts. This fixed-mode checkpoint retained the random owned-border rejection; the next
section establishes that mode separately. Multiple authored player-land blocks
remain unsupported. Other world, prototype,
terrain and lifecycle checks are retained.54 direct original fixtures cover the
aggregate path, alongside existing single-region starts and invalid-input checks.

Two complete original/native runs of an authored120 map with asymmetric shared
borders (left10%,right20%,top15%,bottom5%) match all10 starting objects, terrain,
poses and RNG for seeds1/20261116. This map previously hit the name-based rejection;
no new map predicate was added. All24 unseen packaged comparisons still match
after the shared change.

Evidence: `rms-player-border-structural-differential-oracle-2026-11-16.json`,
`rms-player-border-aggregation-discriminator-oracle-2026-11-16.json`,
`rms-common-border-full-startup-report-2026-11-16.json`, and
`rms-unseen-seeds-region-report-2026-11-16.json`.

Validation: all131 retained packaged startup cases, general/authored regressions
and the two new complete bordered-start fixtures pass under ASan/UBSan. The54
aggregate fixtures, existing288 randomized/35 fixed start fixtures and invalid
inputs pass. Strict integrated map checks, production CLI checks and desktop
dummy-driver smoke pass. All24 new packaged comparisons match unchanged.

### Randomized starts through the same aggregate rule (2026-11-16)

108 original cases verify random4e0740 directly and through full finalization,
with equal/unequal player regions and neutral-first layouts. Original5ef140 reads
the byte at game+0x1438; nonzero dispatches4e76a0 to4e0740. Both complete authored
random-start runs explicitly record this call path and original preset registration
without FIXED_POSITIONS. This is a verified runtime selector, not an inferred label.

The shared map pipeline now uses aggregate regions for both positioning modes.
The separate blanket rejection of random owned bordered starts is removed; other
profile/context limits remain.54 additional random region fixtures cover the
minimum-span/maximum-origin behavior, including split-edge and translation
discriminators. Invalid inputs preserve RNG in both modes.

The same authored bordered map matches complete original startup for seeds1 and
20261116 with10 objects and final RNG2756937157/3500518525 in random mode.
The four full bordered-start fixtures now cover both modes. All24 unseen packaged
fixed-mode cases continue to match. This does not admit every packaged map in
random mode or establish other world-size/civilization/player-count contexts.

The snapshot adapter supports `tiny-random`; comparison manifests use
`positions: "random"`. The comparator checks placement mode against original
preset registrations and rejects mismatched contexts rather than comparing them.

Evidence: `rms-random-player-border-structural-oracle-2026-11-16.json`,
`rms-random-player-mode-selector-static-2026-11-16.txt`,
`rms-common-border-random-startup-report-2026-11-16.json`, and
`rms-unseen-seeds-random-region-report-2026-11-16.json`.

Validation:131 retained packaged startup cases and general/authored regressions,
including all four complete bordered-start fixtures, pass under ASan/UBSan.
The108 aggregate-region fixtures, existing288 randomized/35 fixed fixtures,
invalid-input checks, eight comparator tests (including placement-mode mismatch),
strict map checks, production CLI checks and desktop dummy-driver smoke pass.

### Broader dimensions, player counts and civilizations (2026-11-16)

The original player-placement matrix now covers raw square dimensions 120, 144,
168 and 200; 1, 3, 4 and 8 players; fixed/random positioning; seeds 1/20261116;
equal, split and translated owner regions; and neutral-first layouts. All 384
native cases match start coordinates and RNG. Each original case also matches
its separately executed full-finalizer counterpart (768 original executions).
The raw dimensions have not been mapped to the UI's named size presets.
The committed TSV extends the existing shared player-start tests; it does not
add size-specific production behavior.

A separate 16-case original LAND/TERRAIN matrix covers those four dimensions
and player counts at fixed seed 1. Every full grid and all five scheduler/stage
RNG checkpoints match native land growth, smoothing and terrain placement.
Each land requests 3600 tiles with radius 13 and 10% borders; one terrain patch
requests 10% of the map in seven clumps. The retained tests check grid lengths,
64-bit grid fingerprints and RNG. The local differential run checked every byte.
This is a stage comparison with finalized inputs, not a complete packaged-map
startup comparison: objects, forests, elevations and civilization effects are
excluded. Original scheduler logging consumes one draw before the first stage
and one after each successful stage, including the last.

Fourteen further full RMS comparisons cover civilization pairs 7/7, 1/7, 1/2,
2/1, 3/4, 5/6 and 7/8 in fixed and random modes at seed 20261116. The authored
border map matches ordered starting objects, all grids, captured poses and RNG.
These are healthy pre-bonus constructors. Original civilization/tree/team effects
run after RMS creation; the reference world lacks the full master table needed
to execute them safely. In particular, a zero master count would silently skip
unit effects. These matches do **not** establish civilization bonuses, damaged
building graphics, HP or full game initialization parity. Comparison manifests
now accept `civilizations: [1, 2]` (exactly two IDs from 1 through 8) and preserve
reference limitations in reports. The full-map adapter remains 120×120/two players.

Evidence retained locally: `rms-player-region-dimension-count-matrix-oracle-2026-11-16.json`,
`rms-land-terrain-dimension-expanded-prefix-oracle-2026-11-16.json`,
`rms-land-terrain-native-dimension-comparison-2026-11-16.json`, and build-folder
`rms-civilizations-wave1/report.json` / `rms-civilizations-wave2/report.json`.
The stage findings also retain scoped-hook equivalence against both original
baseline captures; only instrumentation dispatch changed, not hook bodies or guards.

A further 16 LAND/TERRAIN cases vary fixed/random placement and seeds
12345/20261116 at dimensions 144/200 with 3/8 players. All tiles and stage RNG
checkpoints match again; all 32 stage cases are retained in integrated map tests.
Evidence: `rms-land-terrain-mode-seed-prefix-oracle-2026-11-16.json` and
`rms-land-terrain-native-mode-seed-comparison-2026-11-16.json`.

Validation: strict integrated map tests and the retained packaged/general startup
suite pass under ASan/UBSan; the suite includes all 18 authored bordered-start
fixtures (four existing plus fourteen civilization cases). Nine comparator tests
pass, including invalid civilizations, native argument propagation and rejection
of a saved capture from a different civilization pair. No production algorithm
change or map-specific admission was needed for these new cases.

### Larger complete RMS starts and effect dispatch (2026-11-16)

The authored bordered startup now matches complete original RMS execution at
144×144 and 200×200 for seed 1, two Empire players: ten ordered starting objects,
all terrain/elevation/slope bytes, animation poses and final RNG. These worlds
still register TINY_MAP; raw dimensions are deliberately independent of named
UI size presets. The common owned-land admission no longer imposes 120×120;
other terrain, prototype, square-world and owner-count restrictions remain.
This proves RMS creation, not the subsequent ordinary civilization/team pass.

The original reference needed larger, nonoverlapping tile/controller/visibility
and queue arenas. The relocated reference reproduces the prior 120 baseline
exactly on objects, grids, RNG, poses, resources, tokens and preset registrations.
The snapshot keeps its old two-player CLI and adds an explicit form:
`DAT SEED TERRAIN CIVS_CSV DRS RESOURCE WIDTH HEIGHT tiny|tiny-random`.
Comparison manifests accept `width`/`height` (default 120), validate the reference
world context and full grid lengths, and pass raw dimensions to reference probes.
Accepting a context in this diagnostic is not a claim that every map supports it.

A separate full original civilization-6 master-table capture loaded all 1,446
nonnull prototypes, with the true table count 1,596. Actual post-RMS tree 9 then
own-team effect 15 dispatch changes unit 933 and 183 class-targeted units,
respectively. This executes original effect callbacks; it does not yet prove the
whole-world team-selection routine or all civilizations.

Three explicitly synthetic original dispatcher controls establish a shared
selector correction: an in-range unit index takes priority over class; a negative
or out-of-range unit index falls back to a nonnegative class; both negative
selectors affect nothing. The native predicate previously intersected selectors
and treated both negatives as a wildcard. The shared predicate now follows the
original; costs, training time, stats, attack and armour use it. Normal captured
civilization commands use exclusive selectors, so this correction is not presented
as a demonstrated mismatch in their ordinary startup effects.

Local evidence: `rms-commonborder-larger-startup-findings-2026-11-16.json`,
`rms-text-full-commonborder-dimension144-60000-settings-tiny-derived-1-1-1-oracle-2026-11-16.json`,
its dimension200 counterpart, and `rms-civ6-selector-controls-6-1-seed1-oracle-2026-11-16.json`.

The larger-start matrix now contains ten complete matches: fixed seeds
1/12345/20261116 and random seeds 12345/20261116 at both dimensions. Retained
regressions also import these maps into Game and verify every terrain tile plus
object identity, owner and coordinates. Larger worlds with three/eight owners
still require complete reference owner contexts; this work does not waive those
requirements.

Original full-table team15 execution also exposed missing native attribute23
application. Search radius is independent of LOS: the original team effect has
separate commands for each. Automatic healing, next-building/resource searches
and aggressive acquisition now evaluate search-radius effects through the shared
attribute machinery, using the builder/gatherer master where appropriate. Tests
cover the 183 captured team targets and independent LOS/radius commands.

Original live HP controls cover completed fortress933, worker293 and a genuinely
created unfinished fortress. Live HP is updated before the signed-short master:
set uses `s16(trunc(d*H/M))`, add uses `s16(trunc((M+d)*H/M))`, multiplication uses
`s16(trunc(H*d+0.5))`. Set/add with zero old maximum produce zero live HP; there is
no minimum-one clamp. The master separately uses truncated set/add and rounded
multiply, with 16-bit wrapping. The native refresh now applies newly appended
HP commands in order rather than adding an aggregate maximum delta. Attribute0
queries use the same per-command signed-short master arithmetic. The technology
cheat no longer applies a second HP adjustment after ordinary research.

Twenty-one retained original controls cover fractional values, reductions, zero,
negative results and command ordering. Repeated research and unrelated refreshes
must not replay old HP commands. This does not establish combined unit-type
replacement/HP-effect ordering; that remains outside these unchanged-master controls.

Additional evidence: `rms-civ6-postbonus-decoded-findings-2026-11-16.json`,
`rms-hp-command-semantics-findings-2026-11-16.json`, and the linked type70/type80
and unfinished-building captures. Whole-world allied-team selection still needs
an independently initialized reference world; the current normal runtime proof
executes tree9 followed by own-team15 directly after RMS.

Validation after these shared fixes: all ten larger full-RMS differential cases
match; the retained packaged/general/owned startup suite and larger Game imports
pass under ASan/UBSan. Focused cost/time, civilization, HP, combat, healing and
automatic-technology tests pass under the same sanitizers. Ten comparator tests
pass, as do eleven malformed snapshot-argument controls. The rebuilt desktop
passes dummy-driver menu/start/pause navigation and 60 render frames with effect
playback, streamed music and EOF looping. This is not a new live Windows UI test.

### Three through eight owners and whole allied-team selection (2026-11-16)

Sixteen complete authored RMS comparisons now match ordered objects, poses,
terrain/elevation/slope bytes and final RNG: fixed seed 1 at 144/200 for every
player count 3–8, plus random seed 20261116 at both dimensions for counts 3/8.
All players are Empire; each owns a Command Center, three workers and a scout.
Independent original owner contexts include captured masters, resource arrays,
research, visibility, queues, object lists, AI and diplomacy. A relocated
2-player control matches the previous reference exactly. The native shared
owned-land/scout path no longer imposes a two-owner limit; other asset, terrain,
square-map and placement-distance checks remain. The original still registers
TINY_MAP, so these raw dimensions do not establish named UI map-size presets.

The original whole selector at601f90 also executes successfully for civilizations
6/6 and6/1 with enemy, mutual-allied and directional diplomacy. Both recipients
have full original master tables (1596 slots,1446 nonnull records). The actual
getters, alliance predicate and effect callbacks run. The configuration gates
are an explicit zero-valued fixture, not a recovered normal-world option mapping;
CRT allocation boundaries remain modeled. Each case restores verified pre-bonus
state. This establishes selection for those raw inputs, not all session setup.

Observed effect orders:

| Civilizations | Relationship | Player1 | Player2 |
|---|---|---|---|
|6/6|enemy, mutual or directional|9,15|9,15|
|6/1|enemy|9,15|4,10|
|6/1|mutual allies|9,10,15|4,10,15|
|6/1|only1 treats2 as ally|9,10,15|4,10|

Thus own teams apply, duplicate civilizations do not stack, alliances are
recipient-directional, and teams are ordered by civilization rather than by
recipient or effect discovery. The existing native selection agrees; no special
civilization/team branch was added. Retained tests check dispatch order and
recipient-specific LOS/search-radius changes.

Local evidence ledgers: `rms-commonborder-count-startup-context-findings-2026-11-16.json`
and `rms-whole-selector-findings-2026-11-16.json`; native matrix report is
`rms-count-full-matrix/report.json` in the build directory. Full game-session
construction, mixed-civilization3–8-owner worlds and unrelated unit catalogs
remain outside this authored RMS evidence.

### Combined prototype upgrades and HP commands (2026-11-16)

Seven actual original109→71 sequences show in-place source-master replacement,
not an ID alias. The source slot/pointer and ID109 remain; target71 stays a
separate prototype. Existing and newly requested109 objects use the copied
source state. HP changes to109 before an upgrade are overwritten by target71's
current state; changes to71 before the upgrade are copied. Later commands to109
affect the source objects, while later commands to71 do not affect them.

For a damaged109 at1000/2500: adding500 to109 then upgrading restores1000/2500;
upgrading then adding500 to109 produces1200/3000; adding500 to71 before upgrading
also produces1200/3000. Repeated upgrades preserve those stats but consume
original animation-related RNG. Native upgrade sprite/RNG equivalence remains
unverified and is not included in the new stat/identity claim.

The implementation replaces transitive ID aliases with stable per-player
prototype records and copied attribute-command histories. This carries the
source/target distinction through HP, other attributes, costs, training times,
attack and armour. Shared DAT assets remain immutable. Gate variants must use
the same stable per-player prototypes so a research refresh cannot turn an open
gate into a pointer-identity mismatch.

Validation: the sixteen retained count/dimension cases and one fresh three-player
seed12345 original run match. The fresh driver passes the complete civilization
list and player count to a separate immutable Empire-only probe. Ten comparator
tests, the packaged/general/owned startup and import suite, and focused costs,
civilization/team, combined upgrade/HP, combat modifiers, healing and automatic
research checks pass; native suites use ASan/UBSan. The file/DRS CLI suite passes,
including a positive three-player scout render and invalid-civilization rejection.
The rebuilt desktop passes menu navigation and60 render/audio smoke frames.

The broader `swgbtool test-combat` still fails building destruction and attacker
spacing checks. A separately compiled pre-change538778b build produces identical
combat diagnostics and failure; no assertions were relaxed. Logs are retained
as `count-upgrade-combat-check.log` and `count-upgrade-baseline-combat.log` in the
build directory. This is an existing gameplay validation gap, not a passing
end-to-end gameplay claim. No new live Windows UI comparison was performed.

### Mixed-civilization multi-owner startup and combat test repair (2026-11-16)

Sixteen additional complete original/native comparisons match ordered objects,
initial poses, all terrain/elevation/slope cells and final RNG. Twelve use
civilizations1..N for every player count3–8 at144/200, fixed seed1. Four use
reversed civilizations8..1 or the nonconsecutive list8,6,4 at200, seed20261116,
with both fixed and randomized placement. Each original owner has independently
allocated, civilization-specific captured masters, resources and research state;
the previous two-owner3/4 reference is an unchanged relocation control. These
cases require no new native civilization or map-profile special cases.

The retained common-border tests import all sixteen into Game and check owners,
unit identities, coordinates, every terrain tile and final generation RNG. They
also check each owner's original startup cost counters:575 total,200 units and
375 buildings; native post-bonus population is4 used with5 capacity, or200 for
Trade Federation. The whole common-border suite passes ASan/UBSan. Local ledger:
`rms-commonborder-mixed-count-startup-findings-2026-11-16.json`; comparison reports
are `rms-mixed-count-full`, `rms-mixed-count-permutations` and
`rms-mixed-count-reverse-random` in the build directory. These are authored RMS
worlds with enemy diplomacy and actual TINY_MAP registration, not complete
session creation, named UI-size presets or mixed-owner allied bonus proof.

The earlier broad combat failure was traced to fixture assumptions. Projecting
an object's screen coordinates does not establish sight; the test clicked
unseen targets and counted subsequent automatic combat as a manual attack.
The fixture now positions its troops within sight and checks visibility before
issuing the same UI clicks. The target described as a Command Center was in fact
dwelling19; the comment is corrected. No visibility, damage or pathfinding code
was changed. This repairs the test rather than proving original right-click
behavior for remembered buildings, which remains separately unverified.

The spacing failure was also axis-specific: attackers were separated alongX
but the check demanded a2-tileY span. It now measures actual maximum pairwise
distance in2D, retaining the minimum separation and engaged-attacker checks.
The final ASan/UBSan broad combat run passes all three manual attack orders,
damage and destruction deadlines, rubble decay, upgrade menus, garrison,
repair, shields and attack-ground assertions. Dwelling damage reaches588/800
in20.07 simulation seconds, destruction takes another36.23, and eight attackers
remain engaged with0.45 minimum separation and3.05 maximum spread. Evidence:
`combat-checked-pairwise.log`. Initial long-distance approach is no longer an
incidental part of this click fixture; this is not a1:1 combat-pathing claim.

### Session player setup, restart state and mixed allied bonuses (2026-11-16)

The actual selector now has 48 controls: player counts 3–8, alternating
civilizations 6/1, four normal diplomacy configurations and four raw option-gate
configurations at every count. Each recipient uses its own complete captured
1,446-master table. Normal cases cover enemies, mutual allies, directional
alliances with duplicate civilizations, and reversed direction. All retain
unchanged RNG. Twenty-four normal dispatches are retained as native golden
tests, including team15 LOS/search 5→7 and team10 speed 1.5→1.650000095.
This extends player-count and alliance coverage, not full-master coverage for
the other six civilizations. Evidence: local
`rms-multiowner-selector-findings-2026-11-16.json` and its six original oracles.

The original selector traverses configured player records without consulting
starting-object counts. Static evidence supports the inference that empty
players still contribute bonuses; runtime controls here contain starting
objects. Native RMS import incorrectly created players only while importing
owned objects. Explicit player setup now precedes object creation, preserving
civilization, human slot and complete configured diplomacy even when placement
produces no objects. Invalid explicit civilization lists fail before replacing
the current game. Calls without civilization context retain terrain-only
diagnostic behavior. Empty-start native regressions cover 3–8 players.

Repeated-session checks exposed two independent shared reset defects. Trigger
runtime vectors retained elapsed time and fired flags when a new scenario had
the same trigger count. Shield-generator indices retained the previous session's
object ordering when the object count stayed equal. Initializers now reset both
states. Separate pre-change and post-change binaries prove all three regressions:
the previous build fails empty-player bonuses, timer restart and swapped shield
indices; the fixed build passes. Evidence: `session-before-after.log` in the
build directory. The expanded bonus/HP/healing/research suite, packaged RMS
startup/import suite and broad combat check pass ASan/UBSan. The rebuilt desktop
passes menu navigation and 60 render/audio smoke frames.

Full session initialization remains incomplete. The desktop still calls the
custom two-player `initSkirmish` path; `initRms` is a diagnostic world import.
At this checkpoint, starting resources, runtime RNG handoff, AI activation,
multi-player victory, camera and setup options still needed an integrated
session path; the following update adds a bounded normal-mode entry point. Original UI/static
evidence maps game+1439 to All Techs and world.config+5be8 to the scenario Full
Tech Tree setting. Early restrictions and automatic technology eligibility also
consult these settings, so suppressing late bonuses alone would be incorrect.
The runtime full-tech cheat is separate. Raw game+1437 suppresses teams; the
complete game+1664 mode enumeration is still unresolved. See local
`session-option-gates-findings-2026-11-16.json` and
`native-rms-session-initialization-audit-2026-11-16.json`. These investigations do
not establish whole-session parity or a new live Windows UI comparison.

The subsequent resource phase has 15 actual executable controls: five ending at
602484 and ten ending at 60260f, with two recipients and unchanged RNG. Normal
mode overwrites resources 0–3 with current modified resources 91–94 plus:

| Raw resource tier | Added constants, in resource-index order |
|---|---|
| 0 or 1 | 200, 200, 250, 100 |
| 2 | 500, 500, 450, 300 |
| 3 | 1000, 1000, 850, 700 |
| Above 3, unsigned | No tier grant |

Sentinel stocks distinguish overwrite from addition; resource92 offset −25
produces carbon175 in the first tier. Afterward raw map37 clamps carbon to at
least475 and adds100 ore. Raw mode2 then overwrites with offsets plus
[20000,20000,5000,10000], while mode1 uses [500,500,150,0]; these later overrides
erase earlier map adjustments. The first map control exposed an inherited getter
stub, retained as failed evidence; the corrected private runner uses the actual
getter. See `rms-post-selector-resource-findings-2026-11-16.json`. Special tier0
mode3/4 branches remain static-only. This phase is not yet integrated into the
native diagnostic importer and must not be applied to ordinary scenario loads.
The next bounded original segment is starting-age research at60260f..6026be;
its mapped calls are static evidence only so far.


### Normal-mode session funds, starting ages and AI activation (2026-11-16)

`Game::initRmsSession` now composes diagnostic RMS generation/import with normal
starting-resource grants, starting-age research and explicit computer scripts.
It validates player count, tiers, ages and script slots, and parses every AI
before replacing the world. Resource grants overwrite only banks 0–3, preserving
starting-object accounting. Normal tiers 0–3 and the subsequent map37 adjustment
use the original controls above. Scenario loading remains separate.

Ten additional actual-executable controls cover raw starting ages 2–7 in normal
mode and ages 3–6 in raw mode7. The normal advanced-age path temporarily disables
the next age, grants the previous age's dependents in ascending technology order,
follows all six prerequisite slots, then re-enables and completes the next age.
It excludes dependent technology62, reloads resource-selected technology IDs
after effects, and clears research-cost counter99. These are shared algorithms,
not per-civilization upgrade lists. Base age2 does nothing. The new API accepts
normal ages2–6; special modes and All Techs are not exposed.

Native golden checks for civilizations6/1 and ages3–6 match original completion
order, complete researched sets, counters21/99 and live Command Center, worker
and scout HP. They use the same common-border RMS setup as the reference.
Research does not spend starting bank resources. Evidence retained locally:
`rms-starting-age-runtime-findings-2026-11-16.json`,
`rms-starting-age-v6-oracle-2026-11-16.json` and the corresponding static assembly.
The original executes actual research, upgrade and registration callbacks with
captured masters and graphics; the harness still models CRT and configured world
state. This is executable comparison, not a new live Windows UI comparison.

AI is installed after funds and age effects, without evaluating during setup.
Repository checks prove the first native evaluation sees the selected age and
funds, buys a worker, and leaves human player1 uncontrolled. Invalid options or
a missing later AI file preserve the prior world, RNG and computer state. A
private shipped-script smoke covers three and eight players using all eight
civilizations: each computer loads714 rules from original `randomgame.per`,
then executes its initialization goal rule during the next30 native frames.
The original AI source remains outside the repository. Original static ordering
and its unresolved authority/timer gates are recorded in
`rms-ai-activation-order-findings-2026-11-16.json`; exact original evaluation
cadence and complete computer registration remain unproven.

The focused resource/research/bonus/healing suite passes ASan/UBSan, including
all four tiers, map37 and advanced-age goldens. Packaged map/import regressions
and broad combat checks also pass ASan/UBSan; the rebuilt desktop passes menu navigation and60
render/audio frames. Local logs are `rms-session-age-check-v3.log`,
`session-original-ai-active-check.log`, `session-start-packaged-check.log` and
`session-start-desktop-smoke.log` (plus `session-start-combat-check.log`) in the build directory. Full-session parity is not
established: original building upgrades draw and reseed the CRT RNG, while this
API currently returns generation RNG only. Desktop menus still launch the
existing custom skirmish; RMS session UI, multiplayer victory, special modes and
runtime RNG handoff remain separate work. Malformed cyclic technology data may
fail after world import; option/script preflight is not a whole-world rollback.


### Upgrade RNG handoff and whole-program PER evaluation (2026-11-16)

Original executable probes now distinguish the building upgrade wrapper from
its base graphic callbacks. A live building type-3 upgrade refreshes a selected
construction graphic first, saves a CRT draw, replaces its snow overlay, then
reseeds with that saved 15-bit result. Overlay draws are discarded by the reseed;
subsequent base graphic initialization can consume further draws. Mobile units
skip the building wrapper. Self-upgrading a building still reseeds; an unchanged
mobile graphic and a prototype without matching live objects consume no draws.
Ordinary HP/stat effects use separate callbacks and must not reseed.

Thirty-three controls cover seeds1,12345 and UINT32_MAX, normal/construction/snow
states, randomized target graphics, building/mobile self-upgrades, absent live
objects and changed mobile graphics. The actual player upgrade callback,
masters, object callbacks and sprite constructors execute. Altered graphic flags
and construction/snow states are explicit discriminating inputs, not claims of
naturally captured gameplay. Local evidence: `rms-upgrade-rng-findings-2026-11-16.json`,
`rms-upgrade-rng-controls-summary-2026-11-16.json` and both referenced full oracles.

Native upgrades now consume the shared legacy stream in the live-object effect
path, never while replaying prototype caches. Renderer and upgrade handling use
the same active-graphic selection. Changed graphics reset animation and implement
the original random-frame/phase draws; unchanged graphics preserve their state.
Construction precedes the building reseed, followed by attack, movement and base
graphic stages. Alternatives within each stage are mutually exclusive. The
stream enters scenario reconstruction immediately before post-RMS civilization
bonuses: placement/completion draws already accounted for by generation are not
replayed. Diagnostic `initRms` still returns generation RNG; `initRmsSession`
returns the stream after starting-age research, which the AI then continues.

The previous binary fails the new original age-RNG regression. Goldens retain
pre-age state3895143042 and post-age states19483,2618,28080,28080 for raw ages3–6.
Full visual callback parity remains incomplete: snow/delta visuals, foreign-civ
substitution and arbitrary object-list ordering need further comparison. Native
initial replay holds still follow the standing/idle update path; non-idle
randomized replay holds are not yet preserved through later updates. Other
world RNG consumers still use native behavior, so this is not whole-match
randomness parity.

AI investigation also found a separate shared mismatch: original PER evaluation
scans the entire ordered program. Actual original `40f910` controls visit every
record in programs of0,1,96,97,200 and1423 no-op rules. Native evaluation now
removes the arbitrary96-rule slice and persistent cursor; an authored201-rule
regression checks that the last rule sees earlier actions in the same pass.

Timing remains explicitly bounded. Sixty-eight original gate controls establish
first-update flag handling, the20-eligible-call PER counter, outer authority
branches and world-delta conversion. PER has no additional elapsed-time gate;
its constructor argument2000 is rule capacity, not milliseconds. Local host-step
admission accepts whole elapsed intervals of50–100ms, skips shorter intervals,
and caps longer ones without a50ms catch-up loop. Replacing the native0.25s
interval with20 render frames would therefore introduce another approximation.
The interval is retained and marked until world-step admission is integrated.
Original player updates also save/reseed around their AI branch; that outer
wrapper is not yet mapped into native world updates. The two-draw AI regression
proves direct PER-action stream continuity, not full original player-tick RNG.
See `rms-ai-timing-findings-2026-11-16.json`,
`rms-ai-timing-gates-oracle-2026-11-16.json` and
`rms-ai-localstep-per-scan-oracle-2026-11-16.json` for executed/static boundaries.


Validation: the ASan/UBSan suite passes21 native/original upgrade controls, the
mutually exclusive graphic-branch regression, all starting-age RNG goldens and
two consecutive AI draws from each returned post-age state. Existing research,
HP, allied bonuses, healing and controller checks also pass. Shipped714-rule
programs execute initialization rules with three and eight configured players.
Packaged map/import and broad combat regressions pass, and the rebuilt desktop passes menu
navigation plus60 render/audio frames. Local logs in the build directory:
`session-upgrade-rng-before.log`, `session-runtime-after.log`,
`session-runtime-original-ai.log`, `session-runtime-packaged.log` and
`session-runtime-desktop.log` (combat: `session-runtime-combat.log`). The21 native controls exclude the12 original
construction/snow controls pending equivalent native state fixtures.


### Local simulation admission and player/world RNG wrappers (2026-11-16)

`LocalSimulationClock` now implements the verified normal-local host-time branch.
It takes absolute unsigned32-bit host milliseconds, skips intervals below50ms,
accepts the actual50–100ms interval, caps larger gaps at100ms and discards excess
rather than running catch-up ticks. It preserves the original zero-timestamp
sentinel and unsigned wraparound. A fresh clock admits a zero-time first step.
Suspension preserves the last accepted timestamp and sets the original rebase
flag: the next eligible timestamp advances neither simulation nor AI. Short
rejected samples do not consume that flag. Replay-forced and synchronized/network
admission are deliberately outside this path.

Evidence includes32 additional original admission controls, pause/resume and
wrap sequences, and the original delta/body-gate instructions. The raw contexts
are explicit, with actual conversion instructions; this is not a captured full
Windows event loop. See `rms-normal-local-admission-findings-2026-11-16.json` and
its linked probes. The clock is neutral about speed. `Game::updateLocal` uses the
original new-game default1.5: independent writes at61e61f and4247da both store
0x3fc00000. Accepted66 host milliseconds therefore become99 simulation milliseconds.
No native speed-selection UI is added here.

Desktop live play now uses this entry point. Input/HUD handling and camera
movement run between admitted steps; production, triggers, AI, movement and
combat do not. The original order of the existing native phases is retained by
extracting the production/decay phase and gating both simulation blocks. Direct
`Game::update` remains an explicitly admitted simulation-step API for tools.
Pause, menu and focus transitions mark clock suspension immediately, including
transitions within one SDL event batch. Events received after Resume in the same
batch are no longer discarded by the old `wasPlaying` guard. The desktop smoke
loop uses deterministic33ms host samples through the same admission API.
At this milestone native commands still took effect immediately in the input
phase. The subsequent command-dispatch section records the bounded queue
integration; SDL multi-event coalescing remains unproven.

AI's arbitrary0.25s timer is replaced by its original normal first/periodic gate:
the first admitted call evaluates once, and the counter evaluates again on calls
20,40,etc. Initial zero-time steps count; skipped and resume-rebase samples do
not. This ports the normal local gate, not the original nonzero world+29 or
network authority branches. Native economy/tactical policy remains separate.

Actual executable controls establish234 player-wrapper cases and45 world-tail
cases with controlled object/AI callbacks that invoke the real CRT generator.
Gaia has no normal-player reseed. Every instantiated normal player, including
humans and inactive status values, saves one15-bit draw before its AI branch and
restores it afterward. After players, the original world saves/restores another
draw around auxiliary passes and consumes a final draw without reseeding.
See `player-world-rng-wrapper-findings-2026-11-16.json` and its two v2 oracles.
These execute original wrappers with explicit callback bodies; they do not
establish full original AI or active-object gameplay execution.

Native AI updates now apply the normal-player wrapper to configured slots,
including slots without a loaded script, then apply the world-tail RNG steps.
Serialized SCX secondary-player record count is retained independently of
active-player flags; authored diagnostic scenarios infer slots when no table
exists, while RMS supplies the explicit configured count. Malformed counts fail
before replacing the world. Six native regressions match original empty-world
seed1 outcomes for1/2/8 slots with0/4 discarded AI draws, including inactive
normal records. Outcomes are observed through the next real PER draws; script
reload preserves each player's previous random result. Existing upgrade and
starting-age regressions now account for the player wrapper before observing
PER draws, rather than assuming PER is called directly from map generation.

Whole-world parity remains open. The subsequent owner-barrier integration
replaces the global production/object passes described at this milestone.
Original secondary-list/manager callbacks and victory-time RNG
consumers are not fully mapped. The verified empty-world wrapper state must not
be generalized to arbitrary moving/combat worlds or null holes in a player table.


The admission integration also exposed an observable integer-clock defect:
1,000 accepted66ms host intervals at1.5x should reach99,000 world milliseconds,
but accumulating float0.099 seconds made native PER `game-time` report98.
The before-build regression fails this exact boundary. Host-driven updates now
recover integer milliseconds, apply unsigned32-bit addition, and pass the exact
absolute time separately from float physics delta. The regression then reports99;
a near-UINT32_MAX control checks wraparound. Direct diagnostic steps retain their
existing elapsed-time API; switching to host admission rounds its sub-millisecond
remainder at the integer-clock boundary.

Validation logs are local in the build directory: `local-step-gameplay-v2.log`
(full resource/upgrade/research/AI suite), `local-step-focused-final.log`
(normal-speed integration), `local-step-integer-before.log` and
`local-step-integer-final.log` (clock regression), `local-step-packaged.log`,
`local-step-combat.log`, `local-step-original-ai.log`, and desktop smoke logs.
Sanitizer suites cover the normal clock, between-step camera/selection/orders,
frozen production/movement/AI on rejected samples, pause/rebase, calls1/20/40,
inactive configured slots and malformed-count preservation. Packaged maps and
combat pass; shipped714-rule scripts still initialize with three/eight players.
The desktop smoke exercises60 timestamp-driven render/audio frames. These are
bounded executable/static comparisons and native regressions, not a new live
Windows screen or full-world deterministic replay comparison.


### Local command dispatch and owner phase barriers (2026-11-16)

Original command manager `432900/446970/446b90` copies payloads into a
2,000-slot ring and drains by ascending physical slot before player updates
(`61f715`, player loop `61f85e`). Draining keeps the write cursor: after
1,999 consumed commands, the next three dispatch as 2001, 2002, 2000.
Pending commands survive pause and skipped host calls; they do not force
simulation admission. The previously labelled queue-count field at manager
`+1614`, object `+4`, actually enables replay-file handling.

Native context/right-click movement, resource work, healing, repair, attack,
garrison and building rally clicks now capture actor/target IDs and destination
at input time and dispatch only on admitted world steps. Camera/selection
changes cannot redirect the captured command. New sessions clear the ring.
Other HUD/special-mode, production and AI command emitters still use direct
native handlers; replay, network serialization and exact original opcode
validation are not implemented by this queue.

The native world now completes each owner's production, work, movement/combat
and garrison phases before that owner's AI, ascending Gaia then normal owners.
Global target scans remain intact, and occupancy is rebuilt after spawns.
This is an owner barrier correction, not exact within-owner traversal parity.
Original `4af380` walks live lists: appends may execute immediately and removal
swaps the last entry into the vacancy. Eight original-code mutation controls
cover those mechanics. Awake/sleep/auxiliary membership depends on constructor,
wake and deferred-sleep transitions; it cannot be inferred from unit type or
whether the object is alive. Native phases, global capture/group planning and
projectile/remains containers still differ from that lifecycle.

Local evidence: `rms-local-command-{dequeue,ring}-oracle-2026-11-16.json`,
`rms-local-command-ring-findings-2026-11-16.json`, and
`object-update-list-{findings,controls-v1-oracle}-2026-11-16.json` in the private
Windows reference research directory. These execute original queue/list code
with controlled payloads/callbacks, not an end-to-end Windows match.


Validation: ASan/UBSan checks pass for physical-ring wrap/overwrite/lifetime,
command admission, selection/camera snapshots, pause retention, session reset,
deleted actors and rally-point capture. Four owner-barrier controls compose
original upgrade RNG callbacks with real native queued research completion and
PER observations, covering neither/first/second/both owners researching. The
fixture uses a temporary technology because the original chosen technology is
civilization-disabled; the production disable gate is preserved.

The broad cost/bonus/research/healing/controller suite, combat/garrison suite,
and packaged Forest/startup/import regressions pass. Shipped 714-rule AI
initialization passes with three and eight players. The rebuilt desktop passes
menu navigation and 60 render/audio frames with effects and streamed music.
Local build logs: `local-command-check.log`, `owner-order-bonus.log`,
`owner-order-combat.log`, `owner-order-packaged.log`,
`owner-order-original-ai.log`, `owner-order-desktop.log`. This validation does
not establish a complete-match equivalence or a new live Windows UI comparison.

### HUD command dispatch and object sleep/wake evidence (2026-11-16)

Stop, stance, formation movement, alarm/return-to-work, gate locking, rally
removal, garrison ejection, training and production cancellation now enter the
same admitted-step queue as context commands. Payloads retain actor/building
IDs, unit IDs, formation and desired alarm/lock states. Cursor activation and
formation preference remain immediate UI state. Dispatch resolves live owned
objects and retains existing affordability, capacity, eligibility and refund
checks. Individual garrison portraits capture passenger IDs, not changing
portrait indices. Training resolves the captured unit ID again against current
production options rather than replaying a menu index.

Static original-code evidence identifies Stop1 (`44d2e0/44ac50`), stance18
(`44e680/44c310`), training119 (`5bdd30/5bc530`), alarm127 (`5be190`), and
return128 (`5be1e0`) as queued snapshots. This does not establish all native
handler semantics or exact packet serialization. In particular, original Stop
also has last-identical-command suppression within500 host milliseconds;
that submission layer is not yet ported. Formation/ejection/cancellation use
the shared native admission contract, without a claim of complete original
opcode equivalence. See local `rms-hud-command-queue-findings-2026-11-16.json`.

Research is not blindly deferred: original emitter `5bd0c0` immediately marks
the building and reserves pending costs through `5d4f60`; dispatcher `5ba460`
reverses these through `5d5010` before execution. Native research still follows
its previous direct path pending that accounting integration. Target-cursor
orders, construction placement, deletion, cheats and AI emitters also remain
outside this batch. Mixed direct/queued paths are not full command-order parity.

The original awake/sleep investigation now has21 executed flag/counter controls
and12 executed target-reference transitions. Animated/moving units do not sleep
merely because they are idle. `40ad40/40adc0` release the old task **target** and
acquire the new target; an unchanged target is a no-op. Static target acquisition
wakes it, and releasing its final reference requests deferred sleep for the
verified state/HP conditions. Animated release `422e50` decrements its counter
without requesting sleep; combat/building release extends it with notification.
Death wakes the object; destruction/removal occurs later through the update
return and virtual destructor. The native immediate-inactivation/separate
Remains model does not yet represent those original list transitions.

One bounded behavior is corrected now: native base/static types10/15 no longer
advance sprite animation timers. Original base update `54ef00` has no animated
sprite update; animated `422cd0` supplies elapsed time, and drawing reads the
stored frame rather than a wall-clock animation phase. Tests use actual Gaia
base10 decoration/tree411 plus animated20 and mobile70 prototypes. Gaia trees
348/349/411 are type10 in retained original master-loader controls. Type15 is a
separate specialized class, and the inspected original load path does not map
native compatibility type90 to15. No type90 behavior is changed.

Local evidence: `object-sleep-lifecycle-{findings,controls-v1-oracle}-2026-11-16.json`,
`object-task-reference-controls-v1-oracle-2026-11-16.json`,
`object-factory-animation-findings-2026-11-16.json`, and
`object-disk-runtime-type-findings-2026-11-16.json`. Flag/counter and action-setter
controls execute original instructions with explicit ID lookup/registration
boundaries; factory and drawing conclusions are static analysis, not a new live
Windows UI or pixel-rendering comparison. Exact persistent list membership,
deferred migration and dying-object destruction order remain unimplemented.

Validation for this batch: ASan/UBSan passes real HUD press/release controls for
Stop/stance actor capture, deferred formation movement, duplicate desired alarm
state, rally removal, paid training/cancellation/refund, and individual passenger
ejection after selection changes. Static/animated clocks and paused animation
also pass. The full cost/bonus/research/healing/AI suite, owner-barrier/RNG checks
and combat/garrison/formation regressions pass; desktop menu/navigation and
60 render/audio frames pass. Build logs: `hud-queue-local.log`,
`hud-queue-bonus.log`, `hud-queue-combat.log`, `hud-queue-desktop.log`.

A further13 original-code death-boundary controls establish the base-class
sequence needed before list removal can be ported safely. With no remaining
resource and undead mode disabled, state5 advances to7 and invokes dead-unit
creation; the next update advances7→8; only a later update returns nonzero for
iterator removal. Decaying a positive resource to zero adds another update
before that sequence. Animated overrides add dying-graphic states and inspect
sprite completion before advancing its animation. Pending sleep/containment,
descendant overrides, actual dead-unit creation and final destructor effects
remain separate boundaries. See `object-death-boundary-findings-2026-11-16.json`
and `object-death-boundary-controls-v1-oracle-2026-11-16.json`. These controls do
not make the native Remains lifecycle equivalent to those original objects.

Cancellation has two original paths: training119 calls `557440`, scanning
backward for the last matching unit-ID group and cancelling only within that
group; opcode117/submode4 calls `5575a0` with a flattened queue index. The exact
binding of each original UI cancellation control is still unresolved. Native
queue-cell cancellation keeps its existing indexed semantics, now deferred;
it is not silently changed to a type-based cancellation on incomplete evidence.

### Target cursors and final-removal boundaries (2026-11-16)

Target-cursor patrol, guard/follow, heal, unload, repair, garrison and attack-ground
now use the same admitted-step queue. Clicks retain selected actor IDs, the
chosen target ID and/or world destination, not screen coordinates or later
selection. Accepted emission closes targeting immediately; gameplay execution
and its success/failure feedback happen at dispatch. Read-only eligibility
checks preserve targeting for invalid repair/heal/friendly-target clicks;
unload pathfinding remains deferred to its gameplay execution. Dispatch resolves
live targets again, so death or ownership changes do not retarget the command.
This extends native admission consistently, without claiming every original
opcode, pending-state marker or UI failure path is mapped.

Four real-HUD regressions cover patrol, guard, deleted guard targets and repair
through selection/camera changes. ASan/UBSan local admission and complete
cost/bonus/research/healing/AI regressions pass. Desktop launcher/menu and
60 render/audio frames pass. Logs: `target-queue-local.log`,
`target-queue-bonus.log`, `target-queue-combat.log`, `target-queue-desktop.log`.
Research reservations, construction placement, deletion, cheats, AI command
emission and duplicate-command suppression remain separate outstanding paths.

Object lifetime investigation now includes four actual final-removal controls:
`4af380` invokes the state8 update and class-specific virtual destructor for
base10, mobile70 or building80, then `4c4050/4aeff0` removes owner membership and
world deregistration follows. Derived live updates and full descendant cleanup
are not established by these final-callback controls. Existing-selection cleanup
at `4c1960` removes state>=7 or contained objects; HP-driven dying states3..6
alone are different from explicit-kill deselection. Sight-grid changes are not
proof of physical collision removal.

Eight additional controls use retained real Gaia masters411/349, selected and
unselected, with resource amount0 and HP0 or50. HP0 proceeds through owner
updates: state3/dying graphic5242; state7/dead-unit415 request and deselection;
state8 with membership retained; then owner/world removal. HP50/resource0 stays
in state2 with membership unchanged. Graphics, actual dead-unit allocation,
fog and bookkeeping boundaries are explicitly modeled. This proves the
HP-driven entry, not the actual gather-depletion caller, which still needs
tracing. No native awake-list or death-registration change is shipped here:
`active=false` and `Remains.age` are not substituted for these original states.

Evidence is retained locally in `object-membership-removal-findings-2026-11-16.json`,
`object-final-removal-controls-v1-oracle-2026-11-16.json`,
`object-real-resource-depletion-findings-2026-11-16.json` and its v6 oracle.
The next integration prerequisite is the actual depletion/state-entry and
dead-unit creation path, followed by separating actionability from persistent
update-list membership. Rebuilding lists from `active`, or assigning a fixed
removal delay to every exhausted resource, would encode the wrong algorithm.

The subsequent nine actual gather-update controls close the decrement boundary:
`404ee0` through target/worker virtualFC `5519b0` changes resource amount but not
target HP, state or membership. Exact exhaustion requests the worker's
search/drop-off state; it does not invoke target kill. The acquisition
constructor `404160/40ad40` adds a target reference without initiating death.
Thus a target already in state3/5 can finalize on later owner updates, while
state2/HP50 remains alive after exhaustion. The unresolved earlier preparation
path is now narrowed to task selection `409a60 → master+104 task-table virtual+4`
and its felling/attack action. Local evidence:
`gather-depletion-entry-findings-2026-11-16.json`,
`gather-depletion-entry-v1-oracle-2026-11-16.json`, and the companion assembly.
This is an explicit remaining mismatch with native gather's immediate
`killObject` call, not evidence that native resource death is already equivalent.

Regular gathering now uses the original task work factor and owner resource
multiplier, separating source removal from carried credit. The shared arithmetic
retains the original float stores around capacity adjustment and the final
small-remainder flush (which can exceed carrying capacity). All 93 PC64 controls
from `gather-arithmetic-controls-v2-oracle-2026-11-16.json` are embedded as
bitwise checks for remaining amount, carried amount and credited increment.
Double intermediates match these controls; this is not proof of every x87
extended-precision input. Alternate PC53/PC24 controls are not native goldens.
Effective job-prototype research modifiers remain a separate integration gap.

Ten preparation/damage controls further narrow felling. Action110 selects an
ordinary attack9 child for a living tree, rather than treating task workValue1
as a duration. Actual base-resource damage (`5517f0/5518f0`) sums packet attack
amounts without armor-class matching: the supplied worker packet lowers real
Gaia348/349 HP50 to25 then0, leaving resource amount and object state unchanged.
Lethal damage wakes the target; its later owner update admits death. Reload
comes from master+150 through object+174 and is decremented before action update.
The exact selected worker prototype and complete timed attack-to-gather sequence
still need execution. Native fixed-time felling, immediate exhaustion removal,
and owner vector traversal remain explicitly unported. Evidence:
`tree-task-felling-findings-2026-11-16.json` and its preparation/damage controls.

Actual task selection (`409a60 → 5f7a50`) now confirms worker123 selects its
header task0/action110 for real living Gaia349 and retains prototype123. The
original-loaded civilization1 attack packet also matches the DAT packet (sum25).
This closes that bounded worker case, not base-worker83 job switching or timed
attack release. Local controls: `tree-worker-task-selection-v4-oracle-2026-11-16.json`
and `tree-worker123-header-oracle-2026-11-16.json`.

Validation of the gathering integration: the focused and broader gameplay suites
pass under ASan/UBSan, including the 93 arithmetic goldens. Combat checks pass
resource work, automatic gathering, drop-off, construction, repair and garrison.
Logs: `gather-local.log`, `gather-bonus.log`, `gather-combat.log` in the external
build directory. These do not establish full-match or awake-list equivalence.
The rebuilt desktop also passes preference/menu checks and 60 smoke frames with
rendering and effects/music decoding (`gather-desktop.log`). SDL dummy drivers
were used; this is not a new visual or audible comparison with Windows.
Four additional original reload/readiness controls confirm worker123 reload2.0,
with `444da0` decrementing/clamping its counter before `409870`, and `444ae0`
accepting counter<=0. The remaining timed-path prerequisite is actual graphic7245
and sprite/controller release followed by attack-to-parent-gather transition.
Evidence is frozen in `tree-worker-reload-controls-v1-oracle-2026-11-16.json`
and `tree-worker-task-reload-findings-2026-11-16.json`; worker83 fallback remains
untested. No native felling timing change is inferred from reload alone.

### Gathering HP independence and attack animation evidence (2026-11-16)

Native resource work no longer scales HP by the fraction of resources remaining.
The original `404ee0 → 5519b0` controls change amount only. A native regression
uses a temporary positive-HP Gaia102 prototype with its original gathering task:
it confirms partial gathering preserves HP50, and further gathering after
independent damage preserves HP37. The first assertion failed before removing
the proportional-HP block. The fixture restores its DAT prototype on exit and
avoids depletion, so it does not claim the remaining death path is corrected.

Actual graphic7245 loading, sprite construction/update and attack9 now confirm
an eight-frame,0.1-second animation releases the pending hit crossing frame4.
Real damage lowers treeHP50 to25; later frames do not repeat the hit. A further
30-phase restart control reaches hits at phases4 and24. Reload resets before
state7 starts, not on impact. Same-graphic restart does not reset animation:
phase20 consumes frame4 crossing while entering the new attack state, so that
pending hit releases at the phase24 cycle wrap. A reset-per-shot delay would be
incorrect even if its first hit matched. Source controls are retained locally as
`tree-attack-release-v2-oracle-2026-11-16.json` and
`tree-attack-restart-v2-oracle-2026-11-16.json`. These execute original attack
code with explicit lookup/visibility/notification boundaries; target owner
lifecycle and full parent action110 transition are not yet established by them.

The HP-independence change passes the focused ASan/UBSan checks, broad
cost/bonus/research/healing/AI suite and combat/gather/drop-off regressions.
The rebuilt desktop passes its menu and60-frame dummy-driver smoke test.
Logs: `resource-health-{before,local,bonus,combat,desktop}.log` in the external
build directory; `before` retains the intentional failing regression.

Native attack damage now follows the verified target-runtime dispatch boundary:
types10,15,20,25,30,40 inherit base calculator5518f0, which sums signed packet
amounts without armour matching or minimum-one damage. Types50,60,70,80 retain
the combat calculator path. Constructor vtable writes and +EC entries are
recorded with original bytes/hash in
`object-damage-vtable-dispatch-findings-2026-11-16.json`; type90 is not included.
Native checks use shipped Gaia prototypes of the available base runtime classes
and positive, zero and negative class999 attack packets. The old calculator
failed this regression by imposing minimum-one damage. The existing combat
regressions still cover armour matching and technology modifiers; this change
does not newly reproduce terrain/elevation multipliers or health mutation.

The bounded felling trace now executes actual target owner admission, completed
attack destruction/child clearing, parent action110, movement construction and
completion, and initial gathering. Parent progression is12→4→6→7; first transfer
is0.039000000804662704 with tree remaining99.96099853515625. No child-result or
child-clear bypass remains in v5. It still starts from an authored attackstate7,
concatenates phases rather than running the world loop, uses coincident movement
positions, and models movement/gather graphic changes and world side effects.
This closes the action transition, not full elapsed-world timing or native
felling parity. Exact branches, sprite arithmetic, failed probes and source
hashes are in `tree-felling-release-transition-findings-2026-11-16.json`, with
`tree-parent-to-gather-v5-oracle-2026-11-16.json` and the restart-v3 oracle.
