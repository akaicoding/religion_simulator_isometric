#pragma once

#include "Types.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <mutex>
#include <chrono>

namespace ReligionSim {

enum class TimeMultiplier : std::uint8_t {
    Pause = 0,
    Normal = 1,
    Fast = 2,
    Ultra = 5
};

[[nodiscard]] constexpr double ToNumericMultiplier(TimeMultiplier mult) noexcept {
    switch (mult) {
        case TimeMultiplier::Pause:  return 0.0;
        case TimeMultiplier::Normal: return 1.0;
        case TimeMultiplier::Fast:   return 2.0;
        case TimeMultiplier::Ultra:  return 5.0;
        default: return 1.0;
    }
}

// -----------------------------------------------------------------------------
// Elastic Time Scaling Table (Years per Second at 1x Multiplier)
// -----------------------------------------------------------------------------
inline constexpr std::array<double, static_cast<std::size_t>(HistoricalEra::Count)> ERA_YEARS_PER_SECOND_TABLE = {{
    500.0,      // Era 1 (PrimordialAnimism): ~500.0 yr/s
    100.0,      // Era 2 (TotemismAncestorWorship): ~100.0 yr/s
    10.0,       // Era 3 (StructuralShamanism): ~10.0 yr/s
    2.0,        // Era 4 (TerritorialPolytheism): ~2.0 yr/s
    1.0,        // Era 5 (UniversalReligions): ~1.0 yr/s
    1.0 / 12.0, // Era 6 (RationalizationSecularization): ~0.0833 yr/s (1 month/s)
    1.0 / 365.0,// Era 7 (PostSecular): ~0.0027 yr/s (1 day/s)
    1.0 / 8760.0// Era 8 (TranshumanistPostBiological): ~0.0001 yr/s (1 hour/s)
}};

[[nodiscard]] constexpr double GetYearsPerSecond(HistoricalEra era) noexcept {
    const auto idx = static_cast<std::size_t>(era);
    if (idx < ERA_YEARS_PER_SECOND_TABLE.size()) {
        return ERA_YEARS_PER_SECOND_TABLE[idx];
    }
    return 1.0;
}

// -----------------------------------------------------------------------------
// GameTime Data Structure (Decoupled Macro / Micro time state)
// -----------------------------------------------------------------------------
struct GameTime {
    double current_year{-4000.0};
    std::uint64_t total_ticks{0};
    TimeMultiplier time_multiplier{TimeMultiplier::Normal};
    HistoricalEra active_era{HistoricalEra::PrimordialAnimism};
    double year_fraction{0.0}; // Sub-year accumulator
};

// -----------------------------------------------------------------------------
// Time Progression & Scaling Engine
// -----------------------------------------------------------------------------
class TimeEngine {
public:
    static constexpr double FIXED_MICRO_TIMESTEP = 1.0 / 60.0; // 60 Hz micro tick rate

    explicit TimeEngine(double startYear = -4000.0);

    // Thread-safe state update
    // Advances micro-time ticks and computes the decoupled macro-calendar delta
    void update(double deltaRealSeconds, double& outMacroDeltaYears) noexcept;

    // Time controls
    void setMultiplier(TimeMultiplier mult) noexcept;
    void togglePause() noexcept;
    [[nodiscard]] bool isPaused() const noexcept;
    [[nodiscard]] TimeMultiplier getMultiplier() const noexcept;

    // Direct calendar scrubbing
    void setYear(double year) noexcept;
    void reset(double year = -4000.0) noexcept {
        setYear(year);
        setMultiplier(TimeMultiplier::Normal);
    }
    [[nodiscard]] GameTime getTimeState() const noexcept;

    // UI Formatting
    [[nodiscard]] static std::string GetFormattedDate(const GameTime& time) noexcept;
    [[nodiscard]] std::string getFormattedCurrentDate() const noexcept;

    // Check if the current year triggers an era change
    [[nodiscard]] static HistoricalEra CalculateEraForYear(double year) noexcept;

private:
    mutable std::mutex m_mutex;
    GameTime m_state;
    double m_accumulator{0.0};
};

} // namespace ReligionSim
