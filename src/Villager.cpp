#include "Villager.hpp"
#include "WorldMap.hpp"
#include "Religion.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace ReligionSim {

namespace {
    std::mt19937 g_villRng(404);

    const std::vector<std::string> GROG_TITLES = {
        "Grog the Hunter", "Grog the Devotee", "Grog the Axeman", "Grog of the Sun",
        "Grog the Gatherer", "Grog the Pilgrim", "Grog the Shaman", "Grog the Nomad",
        "Grog the Builder", "Grog the Faithful", "Grog the Elder", "Grog the Seeker"
    };
}

std::string Villager::getSpriteName() const {
    std::string dirStr = "sw";
    switch (direction) {
        case IsoDirection::SW: dirStr = "sw"; break;
        case IsoDirection::SE: dirStr = "se"; break;
        case IsoDirection::NE: dirStr = "ne"; break;
        case IsoDirection::NW: dirStr = "nw"; break;
    }

    switch (activity) {
        case VillagerActivity::Walking:
            return "walk_" + dirStr;
        case VillagerActivity::PrayingAtShrine:
            return "pray_" + dirStr;
        case VillagerActivity::ChoppingGathering:
            return (dirStr == "sw" || dirStr == "se") ? ("chop_" + dirStr) : ("chop_sw");
        case VillagerActivity::CarryingRelic:
            return (dirStr == "sw" || dirStr == "se") ? ("carry_" + dirStr) : ("carry_sw");
        case VillagerActivity::Greeting:
            return "greet_sw";
        case VillagerActivity::Idle:
        default:
            return "idle_" + dirStr;
    }
}

void Villager::update(float dt, const WorldMap& map) {
    stateTimer -= dt;
    animTimer += dt;

    if (activity == VillagerActivity::Walking) {
        float dx = targetX - worldX;
        float dy = targetY - worldY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > 0.05f) {
            float step = moveSpeed * dt;
            if (step > dist) step = dist;

            worldX += (dx / dist) * step;
            worldY += (dy / dist) * step;

            // Determine isometric direction
            // In isometric: x+ is SE, x- is NW, y+ is SW, y- is NE
            if (std::abs(dx) > std::abs(dy)) {
                direction = (dx > 0) ? IsoDirection::SE : IsoDirection::NW;
            } else {
                direction = (dy > 0) ? IsoDirection::SW : IsoDirection::NE;
            }

            cellX = static_cast<int>(std::floor(worldX));
            cellY = static_cast<int>(std::floor(worldY));
        } else {
            worldX = targetX;
            worldY = targetY;
            pickNewBehavior(map);
        }
    } else {
        if (stateTimer <= 0.0f) {
            pickNewBehavior(map);
        }
    }
}

void Villager::pickNewBehavior(const WorldMap& map) {
    std::uniform_real_distribution<float> fDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> durDist(2.5f, 6.0f);
    stateTimer = durDist(g_villRng);

    int curX = static_cast<int>(std::floor(worldX));
    int curY = static_cast<int>(std::floor(worldY));

    if (!map.isValid(curX, curY)) {
        worldX = 20.5f; worldY = 20.5f;
        targetX = 20.5f; targetY = 20.5f;
        activity = VillagerActivity::Idle;
        return;
    }

    const Cell& curCell = map.getCell(curX, curY);

    float roll = fDist(g_villRng);

    // If standing on or next to a Holy Site -> Pray or Carry Relic!
    if (curCell.hasHolySite && roll < (0.45f + piety * 0.35f)) {
        activity = (fDist(g_villRng) < 0.65f) ? VillagerActivity::PrayingAtShrine : VillagerActivity::CarryingRelic;
        return;
    }

    // If in dense forest -> Chop / Gather timber!
    if (curCell.biome == BiomeType::DenseForest && roll < 0.55f) {
        activity = VillagerActivity::ChoppingGathering;
        return;
    }

    // Otherwise roam / walk to a neighbor cell or within cell
    if (roll < 0.70f) {
        activity = VillagerActivity::Walking;
        auto neighbors = map.getNeighbors(curX, curY, false);
        std::vector<Point2i> validNbs;
        for (const auto& nb : neighbors) {
            if (map.getCell(nb.x, nb.y).isHabitable()) {
                validNbs.push_back(nb);
            }
        }

        if (!validNbs.empty() && fDist(g_villRng) < 0.60f) {
            std::uniform_int_distribution<size_t> nDist(0, validNbs.size() - 1);
            Point2i dest = validNbs[nDist(g_villRng)];
            targetX = static_cast<float>(dest.x) + 0.3f + fDist(g_villRng) * 0.4f;
            targetY = static_cast<float>(dest.y) + 0.3f + fDist(g_villRng) * 0.4f;
        } else {
            // Wander within current tile
            targetX = static_cast<float>(curX) + 0.2f + fDist(g_villRng) * 0.6f;
            targetY = static_cast<float>(curY) + 0.2f + fDist(g_villRng) * 0.6f;
        }
    } else {
        // Rest / Idle
        activity = (fDist(g_villRng) < 0.5f) ? VillagerActivity::Idle : VillagerActivity::Greeting;
    }
}

VillagerManager::VillagerManager() {
    m_villagers.reserve(128);
}

void VillagerManager::initVillagers(const WorldMap& map, const ReligionRegistry& registry, int count) {
    (void)registry;
    m_villagers.clear();
    int w = map.getWidth();
    int h = map.getHeight();

    std::uniform_int_distribution<int> rx(2, w - 3);
    std::uniform_int_distribution<int> ry(2, h - 3);
    std::uniform_int_distribution<size_t> tDist(0, GROG_TITLES.size() - 1);
    std::uniform_real_distribution<float> subOffset(0.2f, 0.8f);
    std::uniform_real_distribution<float> pDist(0.4f, 0.95f);

    int spawned = 0;
    int attempts = 0;

    while (spawned < count && attempts < 500) {
        attempts++;
        int x = rx(g_villRng);
        int y = ry(g_villRng);
        const Cell& c = map.getCell(x, y);

        if (c.isHabitable()) {
            Villager v;
            v.id = m_nextId++;
            v.name = GROG_TITLES[tDist(g_villRng)];
            v.cellX = x;
            v.cellY = y;
            v.worldX = static_cast<float>(x) + subOffset(g_villRng);
            v.worldY = static_cast<float>(y) + subOffset(g_villRng);
            v.targetX = v.worldX;
            v.targetY = v.worldY;
            v.religionId = (c.dominantReligionId > 0) ? c.dominantReligionId : 1;
            v.piety = pDist(g_villRng);
            v.activity = VillagerActivity::Idle;
            v.direction = IsoDirection::SW;
            v.stateTimer = 1.0f + subOffset(g_villRng) * 3.0f;

            m_villagers.push_back(v);
            spawned++;
        }
    }
}

void VillagerManager::update(float dt, const WorldMap& map) {
    for (auto& v : m_villagers) {
        v.update(dt, map);
    }
}

const Villager* VillagerManager::getSelectedVillager() const {
    if (m_selectedId < 0) return nullptr;
    for (const auto& v : m_villagers) {
        if (v.id == m_selectedId) return &v;
    }
    return nullptr;
}

void VillagerManager::selectVillager(int id) {
    m_selectedId = id;
}

} // namespace ReligionSim
