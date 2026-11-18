// SPDX-License-Identifier: GPL-3.0-or-later
#include "ai_group_targets.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace swgb {
std::optional<AiTargetGroup> aggregateAiTargetGroup(const std::vector<AiGroupMemberInput> &members) {
    if (members.empty()) return std::nullopt;
    AiTargetGroup group;
    group.region = members.front().region;
    group.landMode = !(members.front().trait & 2);
    for (const auto &member : members) {
        if (!member.resolved || member.region != group.region ||
            bool(!(member.trait & 2)) != group.landMode) return std::nullopt;
        for (size_t axis = 0; axis < 3; ++axis) {
            if (!std::isfinite(member.position[axis])) return std::nullopt;
            group.centroid[axis] = static_cast<float>(group.centroid[axis] + member.position[axis]);
        }
        if (member.unitClass != 50) group.allClass50 = false;
        else if (group.firstClass50 == -1) group.firstClass50 = member.id;
        if (!std::isfinite(member.range)) return std::nullopt;
        group.minRange = std::min(group.minRange, member.range);
    }
    for (float &axis : group.centroid) axis = static_cast<float>(static_cast<double>(axis) / members.size());
    return group;
}

AiGroupTargetAdmission filterAiGroupTarget(const AiTargetGroup &group, const AiGroupTargetFilter &t) {
    using A = AiGroupTargetAdmission;
    if (t.recordId == -1 || t.recordOwner == 0 ||
        (t.requestedOwner != -1 && t.recordOwner != t.requestedOwner) || !t.resolved ||
        t.ownerState == 2 || t.allied || t.status > 2 || t.hp == 0) return A::Rejected;
    if (t.unitType != 80 && (!t.visibility ||
        (t.visibility == 128 && !t.fogMode) || t.excludedByVirtual300)) return A::Rejected;
    if (!group.landMode) return A::Unsupported; // Naval wall-neighborhood branch remains unclosed.
    if ((t.trait & 2) || t.unitType == 45) return A::Rejected;
    if (t.transportOption && t.region == group.region) return A::Rejected;
    if (group.allClass50) return A::Unsupported; // Requires first converter's virtual c0.
    if (t.worldMode >= 3 && t.unitClass == 58) return A::Rejected;
    return A::Accepted;
}

bool aiGroupNormalPrioritySupported(int prototypeId, int unitClass) {
    return unitClass != 46 && unitClass != 51 && unitClass != 8 &&
           prototypeId != 104 && prototypeId != 276;
}

std::optional<AiTargetPriority> resolveAiGroupTargetPriority(
    int prototype, int unitClass, const AiGroupTargetPriorityInputs &input) {
    if (unitClass == 46) {
        if (!input.terminateMode) return std::nullopt;
        if (*input.terminateMode) return AiTargetPriority::Force100;
    }
    if (unitClass == 51 || prototype == 104) {
        if (!input.ownerFlag114) return std::nullopt;
        if (*input.ownerFlag114) {
            if (!input.rememberedMarkerOne) return std::nullopt;
            return *input.rememberedMarkerOne ? AiTargetPriority::Force100 : AiTargetPriority::Force99Point9;
        }
    }
    if (prototype == 104) {
        if (!input.sn106) return std::nullopt;
        if (*input.sn106 == 1) return AiTargetPriority::Force100;
    }
    if (unitClass == 8) {
        if (!input.sn107) return std::nullopt;
        if (*input.sn107 == 1) return AiTargetPriority::Force100;
    }
    if (prototype == 276) {
        if (!input.sn108) return std::nullopt;
        if (*input.sn108 == 1) return AiTargetPriority::Force100;
        if (!input.ownerFlag109) return std::nullopt;
        if (*input.ownerFlag109) return AiTargetPriority::Force100;
    }
    return AiTargetPriority::Normal;
}

bool aiGroupThreatInRange(bool enemy, bool resolved, int status, float hp, float distance, float range) {
    return enemy && resolved && status <= 2 && hp >= 1 && distance <= range;
}

std::optional<AiTargetCandidate> buildAiGroupTargetMetric(
    const std::vector<AiGroupCombatMember> &members, bool threatListEmpty,
    float targetHp, float distance, bool sameRegion, int prototype, int unitClass,
    std::optional<AiTargetPriority> resolvedPriority) {
    if (!resolvedPriority && !aiGroupNormalPrioritySupported(prototype, unitClass)) return std::nullopt;
    constexpr float epsilon = 1.0e-7f;
    const float maximum = std::numeric_limits<float>::max();
    auto dps = [](const AiGroupDamageInput &input) -> std::optional<double> {
        if (!std::isfinite(input.damage) || !std::isfinite(input.reload) || input.reload < 0 ||
            (input.damage <= epsilon && input.conversionEligible)) return std::nullopt;
        if (input.reload == 0) {
            if (input.damage == 0) return std::numeric_limits<double>::quiet_NaN();
            return std::copysign(std::numeric_limits<double>::infinity(), input.damage);
        }
        return static_cast<double>(input.damage) / input.reload;
    };
    if (!std::isfinite(targetHp) || !std::isfinite(distance)) return std::nullopt;
    float damage = 0;
    for (const auto &member : members) {
        if (!member.resolved) continue;
        auto contribution = dps(member.attack);
        if (!contribution) return std::nullopt;
        damage = static_cast<float>(damage + *contribution);
    }
    const float killTime = damage >= epsilon ? static_cast<float>(static_cast<double>(targetHp) / damage) : maximum;
    float vulnerability = threatListEmpty ? maximum : 0;
    if (!threatListEmpty) for (const auto &member : members) {
        if (!member.resolved) continue;
        float threatDamage = 0;
        for (const auto &threat : member.threats) {
            auto contribution = dps(threat);
            if (!contribution) return std::nullopt;
            threatDamage = static_cast<float>(threatDamage + *contribution);
        }
        if (!(threatDamage >= epsilon)) { vulnerability = maximum; break; }
        vulnerability = static_cast<float>(static_cast<double>(vulnerability) +
                                           static_cast<double>(member.hp) / threatDamage);
    }
    bool preferred = true;
    if (unitClass == 18 || unitClass == 7) {
        constexpr int excluded[] = {70,562,584,323,103,50,209,68};
        preferred = std::find(std::begin(excluded), std::end(excluded), prototype) == std::end(excluded);
    }
    return AiTargetCandidate{killTime, vulnerability, distance, preferred,
        unitClass == 6 || unitClass == 8, sameRegion, unitClass == 6,
        resolvedPriority.value_or(AiTargetPriority::Normal)};
}

AiGroupTargetRetention retainAiGroupTarget(const AiGroupRetainedTarget &target) {
    if (!target.resolved || target.status > 2 || target.hp < 1 || target.allied)
        return AiGroupTargetRetention::ResetAndStop;
    if (!target.memberResolved || (target.unitType != 80 &&
        (!target.visibility || (target.visibility == 128 && !target.fogMode))))
        return AiGroupTargetRetention::Reset;
    if(target.groupFlags&1u) {
        if(!target.pathResult)return AiGroupTargetRetention::NeedsPath;
        return *target.pathResult?AiGroupTargetRetention::Redispatch:AiGroupTargetRetention::Reset;
    }
    return AiGroupTargetRetention::Retain;
}

AiGroupTargetSelection selectAiGroupTarget(const AiTargetRanking &ranking,
    AiGroupTargetContinuation &state, uint32_t start, uint32_t budget,
    const std::function<uint32_t()> &now,
    const std::function<AiGroupTargetAdmission(size_t)> &attempt) {
    size_t cursor = state.active ? state.cursor : 0;
    bool first = true;
    for (; cursor < ranking.order.size(); ++cursor) {
        if (!first && uint32_t(now() - start) >= budget) {
            state = {cursor, true};
            return {AiGroupTargetSelectionStatus::Budget};
        }
        first = false;
        const auto result = attempt(ranking.order[cursor]);
        if (result == AiGroupTargetAdmission::Unsupported) {
            state = {cursor, true};
            return {AiGroupTargetSelectionStatus::Unsupported};
        }
        if (result == AiGroupTargetAdmission::Accepted) {
            state = {};
            return {AiGroupTargetSelectionStatus::Selected, ranking.order[cursor]};
        }
    }
    state = {};
    return {AiGroupTargetSelectionStatus::Exhausted};
}
} // namespace swgb
