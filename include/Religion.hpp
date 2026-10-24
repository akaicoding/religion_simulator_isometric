#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <unordered_set>

namespace ReligionSim {

struct Religion {
    int id{0};
    std::string name;
    std::string founderName;
    std::string symbol{"Sun"};
    int foundedYear{-4000};
    ColorRGB color{255, 180, 50};

    // Core traits (0.0 to 1.0)
    float zeal{0.5f};                  // Conversion pressure and replication velocity
    float syncretismTolerance{0.5f};   // Willingness to blend, absorb, and agglutinate with other faiths
    float cohesion{0.7f};              // Dogmatic stability and resistance to schisms / heresies
    float spiritualPiety{0.6f};        // Devotion and generation of holy wonders / relics
    float scholasticism{0.2f};         // Written doctrine, theology development, speed of era advance

    // Current status
    HistoricalEra currentEra{HistoricalEra::PrimordialAnimism};
    int totalFollowers{0};
    int peakFollowers{0};
    int holySitesCount{0};
    int totalCellsDominant{0};
    bool isExtinct{false};

    // Advancement / Tree
    std::vector<DoctrineType> doctrines;
    int theologyPoints{0};
    int advancementLevel{1};

    // Lineage & Agglutination history
    int parentReligionId{0};          // 0 if primordial, or parent if schismatic branch
    int syncreticParentA{0};          // Non-zero if created via syncretic agglutination
    int syncreticParentB{0};
    int schismCount{0};

    bool hasDoctrine(DoctrineType d) const {
        for (auto doc : doctrines) {
            if (doc == d) return true;
        }
        return false;
    }

    void addDoctrine(DoctrineType d) {
        if (!hasDoctrine(d)) {
            doctrines.push_back(d);
        }
    }
};

class ReligionRegistry {
public:
    ReligionRegistry();

    int createPrimordialReligion(const std::string& name, const std::string& founder,
                                 ColorRGB color, const std::string& symbol, int year);

    int createSyncreticReligion(int parentAId, int parentBId, int year, const Point2i& origin);

    int createSchismReligion(int parentId, int year, const Point2i& origin);

    Religion* getReligion(int id);
    const Religion* getReligion(int id) const;

    const std::vector<Religion>& getAllReligions() const { return m_religions; }
    std::vector<Religion>& getAllReligions() { return m_religions; }

    void updateStats(int totalWorldPop);

    static std::string generateName(const std::string& prefix, const std::string& suffix);
    static std::string generateProphetName();
    static ColorRGB generateDistinctColor(size_t index);

private:
    std::vector<Religion> m_religions;
    int m_nextId{1};
};

} // namespace ReligionSim
