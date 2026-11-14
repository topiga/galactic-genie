# Clone Campaigns random-map parity

## Current complete-map result: 2026-11-14

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

This proves this complete configuration, **not general 1:1 terrain generation**.
Other seeds, sizes, civilizations, team setups and map families still need complete
comparisons. Full map startup with terrain 22 remains guarded; this verified
Forest seed contains no 22 tiles and the original wave row creates zero objects.
The engine executes its candidate shuffle, preserving subsequent RNG exactly.
Historical checkpoints below describe earlier coverage, not the current result.

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
| Team/random zones and flat-terrain flags | Still blocked in the integrated importer |
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
hunting species. `tools/rms_large_hunt_checks.h` checks terrain/elevation, every
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
