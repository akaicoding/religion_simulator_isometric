#pragma once

#include "Types.hpp"
#include "WorldMap.hpp"
#include "Religion.hpp"
#include <vector>

namespace ReligionSim {

class AgglutinationEngine {
public:
    AgglutinationEngine();

    void update(WorldMap& map, ReligionRegistry& registry, float dtYears,
                std::vector<GameEvent>& outEvents, int currentYear);

    // Manual god/player actions
    int triggerForcedSyncretism(WorldMap& map, ReligionRegistry& registry,
                               int x, int y, std::vector<GameEvent>& outEvents, int currentYear);

    int triggerForcedSchism(WorldMap& map, ReligionRegistry& registry,
                           int x, int y, std::vector<GameEvent>& outEvents, int currentYear);

private:
    float m_agglutinationAccumulator{0.0f};

    void checkSyncreticAgglutination(WorldMap& map, ReligionRegistry& registry, float dtYears,
                                     std::vector<GameEvent>& outEvents, int currentYear);
    void checkDoctrinalSchisms(WorldMap& map, ReligionRegistry& registry, float dtYears,
                               std::vector<GameEvent>& outEvents, int currentYear);
};

} // namespace ReligionSim
