#include "TimeEngine.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace ReligionSim {

TimeEngine::TimeEngine(double startYear) {
    m_state.current_year = startYear;
    m_state.total_ticks = 0;
    m_state.time_multiplier = TimeMultiplier::Normal;
    m_state.active_era = CalculateEraForYear(startYear);
    m_state.year_fraction = 0.0;
}

void TimeEngine::update(double deltaRealSeconds, double& outMacroDeltaYears) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);

    outMacroDeltaYears = 0.0;
    if (m_state.time_multiplier == TimeMultiplier::Pause || deltaRealSeconds <= 0.0) {
        return;
    }

    // 1. Fixed Micro-Time Tick Rate (60 Hz)
    m_accumulator += deltaRealSeconds;
    constexpr double EPSILON = 1e-9;
    while (m_accumulator >= FIXED_MICRO_TIMESTEP - EPSILON) {
        m_state.total_ticks++;
        m_accumulator -= FIXED_MICRO_TIMESTEP;
        if (m_accumulator < 0.0) m_accumulator = 0.0;
    }

    // 2. Elastic Macro-Time Scaling (Years per second)
    const double mult = ToNumericMultiplier(m_state.time_multiplier);
    const double yearsPerSec = GetYearsPerSecond(m_state.active_era);
    const double deltaYears = yearsPerSec * mult * deltaRealSeconds;

    m_state.current_year += deltaYears;
    outMacroDeltaYears = deltaYears;

    // Sub-year fractional tracking for exact day/hour resolution in late eras
    double intPart = 0.0;
    m_state.year_fraction = std::modf(m_state.current_year, &intPart);
    if (m_state.year_fraction < 0.0) {
        m_state.year_fraction += 1.0;
    }

    // 3. Update active era boundary based on current year
    m_state.active_era = CalculateEraForYear(m_state.current_year);
}

void TimeEngine::setMultiplier(TimeMultiplier mult) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.time_multiplier = mult;
}

void TimeEngine::togglePause() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state.time_multiplier == TimeMultiplier::Pause) {
        m_state.time_multiplier = TimeMultiplier::Normal;
    } else {
        m_state.time_multiplier = TimeMultiplier::Pause;
    }
}

bool TimeEngine::isPaused() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.time_multiplier == TimeMultiplier::Pause;
}

TimeMultiplier TimeEngine::getMultiplier() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.time_multiplier;
}

void TimeEngine::setYear(double year) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.current_year = year;
    m_state.active_era = CalculateEraForYear(year);
}

GameTime TimeEngine::getTimeState() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state;
}

HistoricalEra TimeEngine::CalculateEraForYear(double year) noexcept {
    if (year < -3000.0) return HistoricalEra::PrimordialAnimism;
    if (year < -2000.0) return HistoricalEra::TotemismAncestorWorship;
    if (year < -1000.0) return HistoricalEra::StructuralShamanism;
    if (year < 200.0)   return HistoricalEra::TerritorialPolytheism;
    if (year < 1500.0)  return HistoricalEra::UniversalReligions;
    if (year < 1980.0)  return HistoricalEra::RationalizationSecularization;
    if (year < 2060.0)  return HistoricalEra::PostSecular;
    return HistoricalEra::TranshumanistPostBiological;
}

std::string TimeEngine::GetFormattedDate(const GameTime& time) noexcept {
    std::ostringstream ss;
    const double yr = time.current_year;

    switch (time.active_era) {
        case HistoricalEra::PrimordialAnimism: {
            // e.g. "300,000 BCE" or "4,000 BCE"
            long long absYear = static_cast<long long>(std::abs(yr));
            ss << absYear << " BCE (Primordial Time)";
            return ss.str();
        }

        case HistoricalEra::TotemismAncestorWorship: {
            long long absYear = static_cast<long long>(std::abs(yr));
            long long clanYear = static_cast<long long>(std::abs(yr + 3000.0)) + 1;
            ss << absYear << " BCE [Year " << clanYear << " of the Sacred Ancestors]";
            return ss.str();
        }

        case HistoricalEra::StructuralShamanism: {
            long long absYear = static_cast<long long>(std::abs(yr));
            ss << absYear << " BCE [Harvest Cycle]";
            return ss.str();
        }

        case HistoricalEra::TerritorialPolytheism:
        case HistoricalEra::UniversalReligions: {
            if (yr < 0.0) {
                ss << static_cast<long long>(std::abs(yr)) << " BCE";
            } else {
                ss << static_cast<long long>(yr) << " CE";
            }
            return ss.str();
        }

        case HistoricalEra::RationalizationSecularization: {
            // Month + Year precision: e.g. "Month 8, 1845 CE"
            int yearInt = static_cast<int>(yr);
            int month = static_cast<int>(time.year_fraction * 12.0) + 1;
            ss << "Month " << month << ", " << yearInt << " CE";
            return ss.str();
        }

        case HistoricalEra::PostSecular: {
            // Day + Month + Year precision: e.g. "2024-06-15"
            int yearInt = static_cast<int>(yr);
            int totalDays = static_cast<int>(time.year_fraction * 365.0);
            int month = (totalDays / 30) + 1;
            int day = (totalDays % 30) + 1;
            if (month > 12) month = 12;
            ss << yearInt << "-"
               << std::setfill('0') << std::setw(2) << month << "-"
               << std::setfill('0') << std::setw(2) << day;
            return ss.str();
        }

        case HistoricalEra::TranshumanistPostBiological: {
            // High-precision Cyber-Epoch: e.g. "2088-11-24 14:02:00 [Singularity Cycle]"
            int yearInt = static_cast<int>(yr);
            int totalHours = static_cast<int>(time.year_fraction * 8760.0);
            int month = (totalHours / 730) + 1;
            int day = ((totalHours % 730) / 24) + 1;
            int hour = totalHours % 24;
            int minute = static_cast<int>((time.year_fraction * 525600.0)) % 60;
            if (month > 12) month = 12;
            ss << yearInt << "-"
               << std::setfill('0') << std::setw(2) << month << "-"
               << std::setfill('0') << std::setw(2) << day << " "
               << std::setfill('0') << std::setw(2) << hour << ":"
               << std::setfill('0') << std::setw(2) << minute << ":00 [Singularity]";
            return ss.str();
        }

        default:
            ss << static_cast<long long>(yr);
            return ss.str();
    }
}

std::string TimeEngine::getFormattedCurrentDate() const noexcept {
    return GetFormattedDate(getTimeState());
}

} // namespace ReligionSim
