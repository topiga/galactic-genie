// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_targets.h"
#include <array>
#include <cstdint>
#include <functional>

namespace swgb {
// Original 5941a0 inputs. Coordinates are original x,z,y, not native map axes.
struct AiGroupMemberInput {
    int32_t id = -1;
    bool resolved = true;
    uint8_t region = 0, trait = 0;
    int unitClass = 0;
    std::array<float, 3> position{};
    float hp = 0, range = 0;
};
struct AiTargetGroup {
    std::array<float, 3> centroid{};
    uint8_t region = 0;
    bool landMode = true, allClass50 = true;
    int32_t firstClass50 = -1;
    float minRange = 16;
};
std::optional<AiTargetGroup> aggregateAiTargetGroup(const std::vector<AiGroupMemberInput> &members);

enum class AiGroupTargetAdmission { Accepted, Rejected, Unsupported };
struct AiGroupTargetFilter {
    int32_t recordId = -1;
    int recordOwner = 0, requestedOwner = -1, ownerState = 0;
    bool resolved = true, allied = false;
    int status = 2;
    float hp = 0;
    int unitType = 70, prototypeId = -1, unitClass = 0;
    uint8_t trait = 0, visibility = 0, fogMode = 0, region = 0;
    bool excludedByVirtual300 = false, transportOption = false;
    int worldMode = 0;
    std::optional<bool> conversionEligible; // First converter+c0, ignoring charge for candidate admission.
};
// Standard land subset of 5934a9. Priority overrides must be
// supplied separately to rankAiTargets; this filter does not infer victory mode.
AiGroupTargetAdmission filterAiGroupTarget(const AiTargetGroup &, const AiGroupTargetFilter &);
// Normal-priority scope: these classes/prototypes require victory-mode, owner
// information flags, strategic-number or remembered-record overrides at 592a02.
// Until callers resolve those inputs, they must defer them rather than use Normal.
bool aiGroupNormalPrioritySupported(int prototypeId, int unitClass);
// Recovered 592a02 override inputs. nullopt means the caller has not resolved
// that input; unrelated unknowns do not prevent a decision. rememberedMarkerOne
// is false for a missing original record as well as record+28 != 1.
struct AiGroupTargetPriorityInputs {
    std::optional<bool> terminateMode, ownerFlag114, ownerFlag109, rememberedMarkerOne;
    std::optional<int> sn106, sn107, sn108;
};
std::optional<AiTargetPriority> resolveAiGroupTargetPriority(
    int prototypeId, int unitClass, const AiGroupTargetPriorityInputs &);

// Original 591580 membership test; callers preserve known-record order and dedup IDs.
bool aiGroupThreatInRange(bool enemy, bool resolved, int status, float hp,
                          float footprintDistance, float range);
struct AiGroupDamageInput {
    float damage = 0, reload = 1;
    std::optional<bool> conversionEligible = false; // Unresolved controller+c0 defers zero-damage metrics.
};
struct AiGroupCombatMember {
    bool resolved = true;
    float hp = 0;
    AiGroupDamageInput attack;
    std::vector<AiGroupDamageInput> threats;
};
// 592430 scalar inputs use the caller's resolved 190/18c virtuals and 551c60
// distance. Nonempty original threat lists remain explicit even if all lookups
// failed: threatListEmpty is not inferred from filtered contribution vectors.
std::optional<AiTargetCandidate> buildAiGroupTargetMetric(
    const std::vector<AiGroupCombatMember> &, bool threatListEmpty,
    float targetHp, float distance, bool sameRegion, int prototypeId, int unitClass,
    std::optional<AiTargetPriority> resolvedPriority = std::nullopt);

enum class AiGroupTargetRetention { Retain, Reset, ResetAndStop, NeedsPath, Redispatch };
struct AiGroupRetainedTarget {
    bool resolved = true;
    int status = 2;
    float hp = 1;
    bool allied = false, memberResolved = true;
    uint8_t visibility = 0, fogMode = 0;
    int unitType = 70;
    uint32_t groupFlags = 0;
    std::optional<int> pathResult; // Original4bd560: zero fails; every nonzero result succeeds.
};
// 5a31c0. Flag0 requests path validation only after ordinary retention gates.
// NeedsPath leaves state untouched until the caller can supply that provider. Reset clears target
// and initialization; ResetAndStop dispatches state1 before the caller restores
// state2 when its prior target-list count was nonzero.
AiGroupTargetRetention retainAiGroupTarget(const AiGroupRetainedTarget &);

enum class AiGroupTargetSelectionStatus { Selected, Exhausted, Budget, Unsupported };
struct AiGroupTargetContinuation { size_t cursor = 0; bool active = false; };
struct AiGroupTargetSelection {
    AiGroupTargetSelectionStatus status;
    size_t index = 0; // Candidate input index, not rank index; valid only Selected.
};
// Original 593938 continuation. The first attempted ranked record is exempt
// from the host budget. attempt must report Unsupported when an unimplemented
// blocker/transport fallback could select a target, rather than silently skip it.
AiGroupTargetSelection selectAiGroupTarget(const AiTargetRanking &,
    AiGroupTargetContinuation &, uint32_t start, uint32_t budget,
    const std::function<uint32_t()> &now,
    const std::function<AiGroupTargetAdmission(size_t)> &attempt);
} // namespace swgb
