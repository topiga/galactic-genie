// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Native continuous-collision regression, not original pathfinder parity.
// Real siege-duel positions plus a mirrored copy. The orthogonal local-grid
// samples are blocked, but a diagonal between circular footprints is clear.
[[gnu::noinline]] static void circularObstacleDetourChecks(swgb::Assets &assets) {
    for (int direction : {1, -1}) for (int type : {307, 993, 713})
        for (bool convoy : {false, true}) {
        if (convoy && type != 713) continue;
        swgb::Scenario scenario;
        scenario.map.width = scenario.map.height = 96;
        scenario.map.tiles.resize(96 * 96);
        scenario.playerSlotCount = 2;
        scenario.nextUnitId = 1;
        scenario.civilizations.fill(1);
        for (int player = 0; player < 2; ++player) {
            auto &p = scenario.players[player];
            p.active = p.human = true;
            p.civilization = 1;
            p.diplomacy.fill(3);
            p.diplomacy[player + 1] = 0;
        }
        auto game = std::make_unique<swgb::Game>(assets);
        auto &g = *game;
        std::string error;
        check(g.initScenario(scenario, &error), error.c_str());
        const auto axis = [&](float value) { return direction > 0 ? value : 96 - value; };
        const auto a = g.spawnObjectForTesting(0, 1249, 0, axis(45.5f), axis(30.5f));
        const auto b = g.spawnObjectForTesting(0, 1249, 0, axis(46.5f), axis(29.5f));
        const auto c = g.spawnObjectForTesting(0, 1364, 0, axis(46.5f), axis(28.5f));
        check(a && b && c, "Restore actual Gaia animal blockers");
        const auto target = g.spawnObjectForTesting(1, 109, 2, axis(55), axis(60));
        const auto observer = g.spawnObjectForTesting(1, 83, 1, axis(55), axis(63));
        const auto front = convoy ? g.spawnObjectForTesting(1, 993, 1, axis(45.81f), axis(29.8f)) : 0;
        const auto source = g.spawnObjectForTesting(1, type, 1,
            axis(type == 713 ? 45.67f : 45.81f), axis(type == 713 ? 28.94f : 29.8f));
        check(target && observer && source && (!convoy || front), "Restore siege and its visible attack target");
        for (auto id : {a, b, c, target, observer, source, front})
            if (id) g.setAttackModeForTesting(id, 3);
        g.update(0, {});
        check(!g.segmentClearForTesting(source, axis(52), axis(57), axis(58), axis(63)),
              "A diagonal crossing a solid building remains forbidden");
        const std::vector<std::array<float, 2>> route = {
            {axis(45.5f), axis(44.5f)}, {axis(55), axis(57)}};
        if (front) {
            check(g.issueAttackForTesting(front, target) &&
                  g.restoreMovementRouteForTesting(front, axis(55), axis(60), route),
                  "Restore the front soldier's attack and retained route");
        }
        check(g.issueAttackForTesting(source, target) &&
              g.restoreMovementRouteForTesting(source, axis(55), axis(60), route),
              "Restore siege attack without changing ordinary pursuit retry timing");
        const auto initial = g.objectPosition(source);
        const auto &unit = assets.dat().civs[1].units[type];
        const float radius = std::max(.1f, std::max(unit.collisionSize[0], unit.collisionSize[1]));
        for (int frame = 0; frame < 2700; ++frame) {
            const auto before = g.objectPosition(source);
            g.update(1.f / 30, {});
            const auto after = g.objectPosition(source);
            check(g.segmentClearForTesting(source, before[0], before[1], after[0], after[1]),
                  "Every detour movement segment preserves static clearance");
            for (auto id : {a, b, c}) {
                const auto position = g.objectPosition(id);
                const auto &blocker = assets.dat().civs[0].units[g.objectUnitId(id)];
                const float separation = radius + .05f +
                    std::max(.1f, std::max(blocker.collisionSize[0], blocker.collisionSize[1]));
                const float vx = after[0] - before[0], vy = after[1] - before[1];
                const float ox = before[0] - position[0], oy = before[1] - position[1];
                const float dot = ox * vx + oy * vy, length = vx * vx + vy * vy;
                // Existing overlapping starts may separate monotonically, but
                // may not move deeper into the circular blocker.
                if (ox * ox + oy * oy < separation * separation && dot >= 0) continue;
                const float t = length > 0 ? std::max(0.f, std::min(1.f, -dot / length)) : 0;
                check(length < 1e-12f || (ox + t * vx) * (ox + t * vx) +
                      (oy + t * vy) * (oy + t * vy) >= separation * separation - 1e-5f,
                      "Every detour segment preserves continuous animal-footprint clearance");
            }
        }
        const auto position = g.objectPosition(source);
        check(std::hypot(position[0] - initial[0], position[1] - initial[1]) > 5,
              "Attack movement escapes the circular obstacle pinch within ninety seconds");
    }
}
