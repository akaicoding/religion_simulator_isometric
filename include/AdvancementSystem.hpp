#pragma once

#include "Types.hpp"
#include "Religion.hpp"
#include <vector>
#include <string>

namespace ReligionSim {

struct AdvancementDoc {
    DoctrineType type;
    std::string name;
    std::string description;
    EraType era;
    int theologyCost;
    float spreadModifier;
    float syncretismModifier;
    float cohesionModifier;
};

class AdvancementSystem {
public:
    AdvancementSystem();

    void updateAdvancements(ReligionRegistry& registry, int currentYear,
                            std::vector<GameEvent>& outEvents);

    static std::string getEraName(HistoricalEra era);
    static std::string getEraDescription(HistoricalEra era);
    static HistoricalEra getEraForYear(int year);
    static const EraConfig& getEraConfig(HistoricalEra era);

    const AdvancementDoc* getDocInfo(DoctrineType doc) const;

private:
    std::vector<AdvancementDoc> m_doctrineCatalog;
};

} // namespace ReligionSim
