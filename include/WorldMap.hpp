#pragma once

#include "Cell.hpp"
#include "Religion.hpp"
#include <vector>
#include <memory>

namespace ReligionSim {

class WorldMap {
public:
    WorldMap(int width = 48, int height = 48);

    void generateProceduralWorld(uint32_t seed = 42);
    void seedPrimordialFaiths(ReligionRegistry& registry, int currentYear);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    Cell& getCell(int x, int y);
    const Cell& getCell(int x, int y) const;

    bool isValid(int x, int y) const {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    std::vector<Point2i> getNeighbors(int x, int y, bool includeDiagonals = false) const;
    float getTravelEase(const Cell& from, const Cell& to) const;

    int getTotalWorldPopulation() const;
    void recalculateReligions(ReligionRegistry& registry);

    // Holy site actions
    void consecrateHolySite(int x, int y, const std::string& name);
    void upgradeHolySite(int x, int y);

private:
    int m_width{48};
    int m_height{48};
    std::vector<Cell> m_cells;

    void assignBiomesAndElevations(uint32_t seed);
    void assignPopulationsAndRivers();
};

} // namespace ReligionSim
