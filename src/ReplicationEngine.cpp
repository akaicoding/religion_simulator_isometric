#include "ReplicationEngine.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace ReligionSim {

namespace {
    std::mt19937 g_repRng(777);
}

ReplicationEngine::ReplicationEngine() {
    m_caravans.reserve(64);
}

void ReplicationEngine::update(WorldMap& map, ReligionRegistry& registry, float dtYears,
                               std::vector<GameEvent>& outEvents, int currentYear) {
    processDemographics(map, dtYears);
    processSpatialDiffusion(map, registry, dtYears);
    processMissionaryCaravans(map, registry, dtYears, outEvents, currentYear);
}

void ReplicationEngine::processDemographics(WorldMap& map, float dtYears) {
    int w = map.getWidth();
    int h = map.getHeight();
    (void)dtYears;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Cell& c = map.getCell(x, y);
            if (!c.isHabitable()) continue;

            // La población de cada celda está determinada 1:1 por los aldeanos reales.
            int maxPop = std::max(1, c.maxPopulation);
            c.population = std::clamp(c.population, 0, maxPop * 2);

            // Development scales with sustained real population
            c.development = std::clamp(static_cast<float>(c.population) / static_cast<float>(maxPop), 0.05f, 1.0f);
        }
    }
}

void ReplicationEngine::processSpatialDiffusion(WorldMap& map, ReligionRegistry& registry, float dtYears) {
    int w = map.getWidth();
    int h = map.getHeight();

    // Delta buffer to apply changes simultaneously
    struct CellDiff {
        int religionId;
        float delta;
    };
    std::vector<std::vector<CellDiff>> diffs(w * h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Cell& c = map.getCell(x, y);
            // Las zonas de influencia solo existen y difunden donde hay aldeanos (nodos de fe)
            if (!c.isHabitable() || c.population <= 0 || c.faithShare.empty()) continue;

            auto neighbors = map.getNeighbors(x, y, true);

            for (const auto& sharePair : c.faithShare) {
                int rId = sharePair.first;
                float shareSrc = sharePair.second;
                if (shareSrc < 0.02f) continue;

                const Religion* rel = registry.getReligion(rId);
                if (!rel || rel->isExtinct) continue;

                float zeal = rel->zeal;
                float shrineBonus = c.hasHolySite ? (1.0f + 0.35f * c.shrineLevel) : 1.0f;
                float baseDiffusion = 0.025f * zeal * shrineBonus * dtYears;

                for (const auto& nb : neighbors) {
                    const Cell& target = map.getCell(nb.x, nb.y);
                    // Solo difundir hacia celdas que también tengan aldeanos presentes
                    if (!target.isHabitable() || target.population <= 0) continue;

                    float travel = map.getTravelEase(c, target);
                    float targetShare = target.getShare(rId);

                    // Gradient diffusion: spread when source has higher concentration or target is open
                    if (shareSrc > targetShare + 0.05f) {
                        float gradient = shareSrc - targetShare;

                        // Target resistance from rival dominant faith
                        float resistance = 1.0f;
                        if (target.dominantReligionId > 0 && target.dominantReligionId != rId) {
                            const Religion* rival = registry.getReligion(target.dominantReligionId);
                            if (rival) {
                                resistance = 1.0f + rival->cohesion * target.devotion * 2.0f;
                            }
                        }

                        float transfer = (gradient * baseDiffusion * travel) / resistance;
                        transfer = std::clamp(transfer, 0.0f, 0.15f);

                        if (transfer > 0.0005f) {
                            diffs[nb.y * w + nb.x].push_back({rId, transfer});
                        }
                    }
                }
            }
        }
    }

    // Apply accumulated diffusion (only on cells with villagers)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Cell& c = map.getCell(x, y);
            if (!c.isHabitable() || c.population <= 0) {
                c.faithShare.clear();
                c.dominantReligionId = 0;
                c.dominantFraction = 0.0f;
                continue;
            }

            const auto& cellDiffs = diffs[y * w + x];
            for (const auto& cd : cellDiffs) {
                c.addShare(cd.religionId, cd.delta);
            }

            c.normalizeShares();
            c.updateDominance();
        }
    }
}

void ReplicationEngine::processMissionaryCaravans(WorldMap& map, ReligionRegistry& registry, float dtYears,
                                                 std::vector<GameEvent>& outEvents, int currentYear) {
    int w = map.getWidth();
    int h = map.getHeight();

    // Advance existing caravans
    for (auto it = m_caravans.begin(); it != m_caravans.end();) {
        it->progress += 0.25f * dtYears; // Travel duration ~ 4 years
        if (it->progress >= 1.0f) {
            // Caravan arrived at target cell!
            if (map.isValid(it->to.x, it->to.y)) {
                Cell& target = map.getCell(it->to.x, it->to.y);
                if (target.isHabitable() && target.population > 0) {
                    target.addShare(it->religionId, it->power);
                    target.normalizeShares();
                    target.updateDominance();

                    const Religion* r = registry.getReligion(it->religionId);
                    if (r && target.dominantReligionId == it->religionId && target.dominantFraction > 0.6f) {
                        GameEvent ev;
                        ev.year = currentYear;
                        ev.category = EventCategory::PilgrimageWave;
                        ev.location = it->to;
                        ev.religionAId = it->religionId;
                        ev.badgeColor = r->color;
                        ev.title = "Missionary Vanguard Triumphs";
                        ev.description = "Missionaries of " + r->name + " have established a major congregation in new lands.";
                        outEvents.push_back(ev);
                    }
                }
            }
            it = m_caravans.erase(it);
        } else {
            ++it;
        }
    }

    // Spawn new caravans from high-piety Holy Sites
    if (m_caravans.size() < 12) {
        std::uniform_int_distribution<int> rx(0, w - 1);
        std::uniform_int_distribution<int> ry(0, h - 1);
        std::uniform_real_distribution<float> probDist(0.0f, 1.0f);

        for (int i = 0; i < 5; ++i) {
            int sx = rx(g_repRng);
            int sy = ry(g_repRng);
            const Cell& c = map.getCell(sx, sy);
            if (c.hasHolySite && c.population > 0 && c.dominantReligionId > 0 && probDist(g_repRng) < 0.25f) {
                // Pick a target habitable cell with villagers
                int tx = rx(g_repRng);
                int ty = ry(g_repRng);
                const Cell& tc = map.getCell(tx, ty);
                if (tc.isHabitable() && tc.population > 0 && tc.dominantReligionId != c.dominantReligionId) {
                    MissionaryCaravan car;
                    car.from = {sx, sy};
                    car.to = {tx, ty};
                    car.religionId = c.dominantReligionId;
                    car.progress = 0.0f;
                    car.power = 0.25f + 0.05f * c.shrineLevel;
                    m_caravans.push_back(car);
                }
            }
        }
    }
}

} // namespace ReligionSim
