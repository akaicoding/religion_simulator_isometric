#pragma once

#include "Types.hpp"
#include "WorldMap.hpp"
#include "Religion.hpp"
#include <vector>

namespace ReligionSim {

struct MissionaryCaravan {
    Point2i from;
    Point2i to;
    int religionId;
    float progress; // 0.0 to 1.0
    float power;
};

class ReplicationEngine {
public:
    ReplicationEngine();

    void update(WorldMap& map, ReligionRegistry& registry, float dtYears,
                std::vector<GameEvent>& outEvents, int currentYear);

    const std::vector<MissionaryCaravan>& getActiveCaravans() const { return m_caravans; }

private:
    std::vector<MissionaryCaravan> m_caravans;

    void processDemographics(WorldMap& map, float dtYears);
    void processSpatialDiffusion(WorldMap& map, ReligionRegistry& registry, float dtYears);
    void processMissionaryCaravans(WorldMap& map, ReligionRegistry& registry, float dtYears,
                                  std::vector<GameEvent>& outEvents, int currentYear);
};

} // namespace ReligionSim
