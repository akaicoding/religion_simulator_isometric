#pragma once

#include "WorldMap.hpp"
#include "Humano.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace ReligionSim {

// =============================================================================
// Niebla de Guerra de la Fe (Faith Fog of War)
// -----------------------------------------------------------------------------
// La visión del jugador nace exclusivamente de la fe de su propio pueblo:
//   - Cada creyente ilumina un radio proporcional a su devoción.
//   - El Chamán del pueblo ve mucho más lejos (y aún más al canalizar señales divinas).
//   - Los santuarios/templos de su fe irradian visión según su nivel.
//   - Las celdas donde su fe es mayoritaria aportan una pequeña luz propia.
// Las zonas ya vistas quedan "en la memoria" (exploradas) con tono apagado; las
// nunca vistas permanecen cubiertas por la niebla. El truco "LUMINUS" la desactiva.
// =============================================================================
class FaithFogOfWar {
public:
    static constexpr float VISIBLE_THRESHOLD = 0.5f;

    void reset(int w, int h) {
        m_w = std::max(0, w);
        m_h = std::max(0, h);
        m_visible.assign(static_cast<size_t>(m_w * m_h), 0.0f);
        m_explored.assign(static_cast<size_t>(m_w * m_h), 0.0f);
    }

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setPlayerReligionId(int relId) { m_playerReligionId = relId; }
    int getPlayerReligionId() const { return m_playerReligionId; }

    // Recalcula la visión a partir de los creyentes, el chamán y los templos del jugador
    void update(const WorldMap& map, const GestorHumanos& humanos) {
        const int w = map.getWidth();
        const int h = map.getHeight();
        if (w != m_w || h != m_h || m_visible.size() != static_cast<size_t>(w * h)) {
            reset(w, h);
        }
        std::fill(m_visible.begin(), m_visible.end(), 0.0f);

        // 1. Creyentes y Chamán del pueblo del jugador
        for (const auto& hObj : humanos.getHumanos()) {
            if (hObj.getReligionId() != m_playerReligionId) continue;
            float dev = std::clamp(hObj.getDevocion(), 0.0f, 1.0f);
            float radius = 2.6f + 3.2f * dev;
            if (hObj.esChaman()) radius = 6.0f + 2.0f * dev;
            if (hObj.getTimerSenalDivina() > 0.0f) radius *= 1.5f;
            reveal(hObj.getX(), hObj.getY(), radius);
        }

        // 2. Santuarios de su fe y celdas donde su fe es mayoritaria
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const Cell& c = map.getCell(x, y);
                float cx = static_cast<float>(x) + 0.5f;
                float cy = static_cast<float>(y) + 0.5f;
                if (c.hasHolySite) {
                    int hsRel = (c.holySiteReligionId > 0) ? c.holySiteReligionId : c.dominantReligionId;
                    if (hsRel == m_playerReligionId) {
                        reveal(cx, cy, 4.5f + static_cast<float>(c.shrineLevel));
                        continue;
                    }
                }
                if (c.population > 0 && c.getShare(m_playerReligionId) >= 0.35f) {
                    reveal(cx, cy, 1.6f);
                }
            }
        }

        // 3. Memoria de exploración: lo que se vio una vez queda registrado
        for (size_t i = 0; i < m_visible.size(); ++i) {
            if (m_visible[i] > 0.35f) m_explored[i] = 1.0f;
        }
    }

    // Visibilidad actual de la celda (0 = oculta, 1 = plenamente visible)
    float visibility(int x, int y) const {
        if (!m_enabled) return 1.0f;
        if (!valid(x, y)) return 0.0f;
        return m_visible[static_cast<size_t>(y * m_w + x)];
    }
    bool isVisible(int x, int y) const { return visibility(x, y) >= VISIBLE_THRESHOLD; }

    // Memoria de exploración de la celda (0 = nunca vista, 1 = ya explorada)
    float explored(int x, int y) const {
        if (!m_enabled) return 1.0f;
        if (!valid(x, y)) return 0.0f;
        return m_explored[static_cast<size_t>(y * m_w + x)];
    }
    bool isExplored(int x, int y) const { return explored(x, y) > 0.5f; }

    // Muestreo bilineal continuo (coordenadas de grilla) para bordes de niebla suaves
    float sampleVisibility(float gx, float gy) const {
        if (!m_enabled) return 1.0f;
        return sampleBilinear(m_visible, gx, gy);
    }
    float sampleExplored(float gx, float gy) const {
        if (!m_enabled) return 1.0f;
        return sampleBilinear(m_explored, gx, gy);
    }

private:
    int m_w{0};
    int m_h{0};
    bool m_enabled{true};
    int m_playerReligionId{1};
    std::vector<float> m_visible;
    std::vector<float> m_explored;

    bool valid(int x, int y) const { return x >= 0 && y >= 0 && x < m_w && y < m_h; }

    void reveal(float cx, float cy, float radius) {
        int x0 = std::max(0, static_cast<int>(std::floor(cx - radius - 1.0f)));
        int x1 = std::min(m_w - 1, static_cast<int>(std::ceil(cx + radius + 1.0f)));
        int y0 = std::max(0, static_cast<int>(std::floor(cy - radius - 1.0f)));
        int y1 = std::min(m_h - 1, static_cast<int>(std::ceil(cy + radius + 1.0f)));
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                float dx = static_cast<float>(x) + 0.5f - cx;
                float dy = static_cast<float>(y) + 0.5f - cy;
                float d = std::sqrt(dx * dx + dy * dy);
                float v = std::clamp((radius - d) / 1.25f, 0.0f, 1.0f);
                v = v * v * (3.0f - 2.0f * v); // smoothstep: borde de visión suave
                float& cell = m_visible[static_cast<size_t>(y * m_w + x)];
                if (v > cell) cell = v;
            }
        }
    }

    float sampleBilinear(const std::vector<float>& grid, float gx, float gy) const {
        if (m_w <= 0 || m_h <= 0) return 0.0f;
        float fx = gx - 0.5f;
        float fy = gy - 0.5f;
        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        float tx = fx - static_cast<float>(x0);
        float ty = fy - static_cast<float>(y0);
        auto at = [&](int x, int y) -> float {
            x = std::clamp(x, 0, m_w - 1);
            y = std::clamp(y, 0, m_h - 1);
            return grid[static_cast<size_t>(y * m_w + x)];
        };
        float a = at(x0, y0) * (1.0f - tx) + at(x0 + 1, y0) * tx;
        float b = at(x0, y0 + 1) * (1.0f - tx) + at(x0 + 1, y0 + 1) * tx;
        return a * (1.0f - ty) + b * ty;
    }
};

} // namespace ReligionSim
