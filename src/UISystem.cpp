#include "UISystem.hpp"
#include <cmath>

namespace ReligionSim::UI {

// -----------------------------------------------------------------------------
// Nine-Slice (9-Patch) Panel Renderer
// -----------------------------------------------------------------------------
void DrawNinePatchPanel(PixelCanvas& canvas,
                        const uint32_t* atlasPixels, int atlasW, int atlasH,
                        TileRect srcPatch, Margins margins,
                        Rect targetDest, ColorRGB tint) {
    const int dx = static_cast<int>(targetDest.x);
    const int dy = static_cast<int>(targetDest.y);
    const int dw = static_cast<int>(targetDest.w);
    const int dh = static_cast<int>(targetDest.h);

    if (dw < (margins.left + margins.right) || dh < (margins.top + margins.bottom)) {
        // Fallback: Si el tamaño es menor a las esquinas mínimas, rellenar caja sólida
        canvas.fillRect(dx, dy, dw, dh, tint);
        return;
    }

    // Source subdivisions
    const int sx0 = srcPatch.x;
    const int sx1 = srcPatch.x + margins.left;
    const int sx2 = srcPatch.x + srcPatch.width - margins.right;
    const int swCenter = srcPatch.width - (margins.left + margins.right);

    const int sy0 = srcPatch.y;
    const int sy1 = srcPatch.y + margins.top;
    const int sy2 = srcPatch.y + srcPatch.height - margins.bottom;
    const int shCenter = srcPatch.height - (margins.top + margins.bottom);

    // Destination subdivisions (Corners keep exact 1:1 pixel proportions)
    const int dx0 = dx;
    const int dx1 = dx + margins.left;
    const int dx2 = dx + dw - margins.right;
    const int dwCenter = dw - (margins.left + margins.right);

    const int dy0 = dy;
    const int dy1 = dy + margins.top;
    const int dy2 = dy + dh - margins.bottom;
    const int dhCenter = dh - (margins.top + margins.bottom);

    // Si no hay atlas de textura provisto, renderizar 9-patch procedural de piedra/mármol
    if (!atlasPixels) {
        auto clampByte = [](int val) -> uint8_t {
            return static_cast<uint8_t>(std::clamp(val, 0, 255));
        };

        // 1. Centro sólido (Basalto / Fondo del panel)
        canvas.fillRect(dx1, dy1, dwCenter, dhCenter, tint);

        // 2. Bordes estirados
        canvas.fillRect(dx1, dy0, dwCenter, margins.top, ColorRGB{clampByte(tint.r + 30), clampByte(tint.g + 30), clampByte(tint.b + 30)});
        canvas.fillRect(dx1, dy2, dwCenter, margins.bottom, ColorRGB{clampByte(tint.r - 10), clampByte(tint.g - 10), clampByte(tint.b - 10)});
        canvas.fillRect(dx0, dy1, margins.left, dhCenter, ColorRGB{clampByte(tint.r + 15), clampByte(tint.g + 15), clampByte(tint.b + 15)});
        canvas.fillRect(dx2, dy1, margins.right, dhCenter, ColorRGB{clampByte(tint.r - 5), clampByte(tint.g - 5), clampByte(tint.b - 5)});

        // 3. Cuatro Esquinas no estiradas (1:1 píxel)
        canvas.fillRect(dx0, dy0, margins.left, margins.top, ColorRGB{clampByte(tint.r + 45), clampByte(tint.g + 45), clampByte(tint.b + 45)});
        canvas.fillRect(dx2, dy0, margins.right, margins.top, ColorRGB{clampByte(tint.r + 25), clampByte(tint.g + 25), clampByte(tint.b + 25)});
        canvas.fillRect(dx0, dy2, margins.left, margins.bottom, ColorRGB{clampByte(tint.r + 10), clampByte(tint.g + 10), clampByte(tint.b + 10)});
        canvas.fillRect(dx2, dy2, margins.right, margins.bottom, ColorRGB{clampByte(tint.r - 15), clampByte(tint.g - 15), clampByte(tint.b - 15)});
        return;
    }

    // Render con atlas de textura:
    // --- 4 Esquinas fijas (1:1 píxel) ---
    canvas.blitSubImage(dx0, dy0, margins.left, margins.top, atlasPixels, atlasW, atlasH, sx0, sy0, margins.left, margins.top, tint);
    canvas.blitSubImage(dx2, dy0, margins.right, margins.top, atlasPixels, atlasW, atlasH, sx2, sy0, margins.right, margins.top, tint);
    canvas.blitSubImage(dx0, dy2, margins.left, margins.bottom, atlasPixels, atlasW, atlasH, sx0, sy2, margins.left, margins.bottom, tint);
    canvas.blitSubImage(dx2, dy2, margins.right, margins.bottom, atlasPixels, atlasW, atlasH, sx2, sy2, margins.right, margins.bottom, tint);

    // --- 4 Bordes estirados ---
    canvas.blitSubImage(dx1, dy0, dwCenter, margins.top, atlasPixels, atlasW, atlasH, sx1, sy0, swCenter, margins.top, tint);
    canvas.blitSubImage(dx1, dy2, dwCenter, margins.bottom, atlasPixels, atlasW, atlasH, sx1, sy2, swCenter, margins.bottom, tint);
    canvas.blitSubImage(dx0, dy1, margins.left, dhCenter, atlasPixels, atlasW, atlasH, sx0, sy1, margins.left, shCenter, tint);
    canvas.blitSubImage(dx2, dy1, margins.right, dhCenter, atlasPixels, atlasW, atlasH, sx2, sy1, margins.right, shCenter, tint);

    // --- Centro interior ---
    canvas.blitSubImage(dx1, dy1, dwCenter, dhCenter, atlasPixels, atlasW, atlasH, sx1, sy1, swCenter, shCenter, tint);
}

// -----------------------------------------------------------------------------
// Panel Widget Implementation
// -----------------------------------------------------------------------------
void Panel::render(PixelCanvas& canvas, const UIThemeColors& theme) {
    if (!m_visible) return;

    const int px = static_cast<int>(m_rect.x);
    const int py = static_cast<int>(m_rect.y);
    const int pw = static_cast<int>(m_rect.w);
    const int ph = static_cast<int>(m_rect.h);

    // 1. Marco exterior con Basalto (#1A1A1A) y bisel doble
    canvas.fillRect(px, py, pw, ph, theme.background);
    canvas.strokeRect(px, py, pw, ph, theme.border);
    canvas.strokeRect(px + 2, py + 2, pw - 4, ph - 4, theme.borderAccent);

    // Esquinas ornamentales estilo relicario
    constexpr int CORNER_SZ = 4;
    canvas.fillRect(px, py, CORNER_SZ, CORNER_SZ, theme.borderAccent);
    canvas.fillRect(px + pw - CORNER_SZ, py, CORNER_SZ, CORNER_SZ, theme.borderAccent);
    canvas.fillRect(px, py + ph - CORNER_SZ, CORNER_SZ, CORNER_SZ, theme.borderAccent);
    canvas.fillRect(px + pw - CORNER_SZ, py + ph - CORNER_SZ, CORNER_SZ, CORNER_SZ, theme.borderAccent);

    // Título de la cabecera si existe
    if (!m_title.empty()) {
        canvas.drawText(px + 12, py + 10, m_title, theme.borderAccent, 1);
        canvas.fillRect(px + 10, py + 24, pw - 20, 1, theme.border);
    }

    // Renderizar hijos
    for (const auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render(canvas, theme);
        }
    }
}

bool Panel::handleMouseDown(float mx, float my) {
    if (!m_visible || !m_rect.contains(mx, my)) return false;
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleMouseDown(mx, my)) return true;
    }
    return true; // Panel consumes click
}

bool Panel::handleMouseUp(float mx, float my) {
    if (!m_visible) return false;
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleMouseUp(mx, my)) return true;
    }
    return m_rect.contains(mx, my);
}

void Panel::handleMouseMove(float mx, float my) {
    if (!m_visible) return;
    for (const auto& child : m_children) {
        child->handleMouseMove(mx, my);
    }
}

// -----------------------------------------------------------------------------
// Button Widget Implementation
// -----------------------------------------------------------------------------
void Button::render(PixelCanvas& canvas, const UIThemeColors& theme) {
    if (!m_visible) return;

    const int bx = static_cast<int>(m_rect.x);
    const int by = static_cast<int>(m_rect.y);
    const int bw = static_cast<int>(m_rect.w);
    const int bh = static_cast<int>(m_rect.h);

    ColorRGB bg = theme.background;
    ColorRGB border = theme.border;
    ColorRGB textCol = theme.textPrimary;

    switch (m_state) {
        case ButtonState::Idle:
            bg = ColorRGB{static_cast<uint8_t>(theme.background.r + 14),
                          static_cast<uint8_t>(theme.background.g + 14),
                          static_cast<uint8_t>(theme.background.b + 14)};
            border = theme.border;
            textCol = theme.textSecondary;
            break;

        case ButtonState::Hover:
            bg = ColorRGB{static_cast<uint8_t>(theme.background.r + 28),
                          static_cast<uint8_t>(theme.background.g + 24),
                          static_cast<uint8_t>(theme.background.b + 18)};
            border = theme.borderAccent; // Divine Gold / Accent highlight
            textCol = theme.borderAccent;
            break;

        case ButtonState::Clicked:
            bg = ColorRGB{static_cast<uint8_t>(theme.borderAccent.r / 3),
                          static_cast<uint8_t>(theme.borderAccent.g / 3),
                          static_cast<uint8_t>(theme.borderAccent.b / 3)};
            border = theme.borderAccent;
            textCol = ColorRGB{255, 255, 255};
            break;
    }

    // Caja biselada de piedra/relicario
    canvas.fillRect(bx, by, bw, bh, bg);
    canvas.strokeRect(bx, by, bw, bh, border);

    // Sombra interna y esquinas
    if (m_state == ButtonState::Hover || m_state == ButtonState::Clicked) {
        canvas.fillRect(bx, by, 3, 3, theme.borderAccent);
        canvas.fillRect(bx + bw - 3, by, 3, 3, theme.borderAccent);
        canvas.fillRect(bx, by + bh - 3, 3, 3, theme.borderAccent);
        canvas.fillRect(bx + bw - 3, by + bh - 3, 3, 3, theme.borderAccent);
    }

    // Texto centrado
    const int textX = bx + std::max(6, (bw - static_cast<int>(m_label.size() * 6)) / 2);
    const int textY = by + (bh - 7) / 2;
    canvas.drawText(textX, textY, m_label, textCol, 1);
}

bool Button::handleMouseDown(float mx, float my) {
    if (!m_visible) return false;
    if (m_rect.contains(mx, my)) {
        m_state = ButtonState::Clicked;
        return true;
    }
    return false;
}

bool Button::handleMouseUp(float mx, float my) {
    if (!m_visible) return false;
    if (m_state == ButtonState::Clicked && m_rect.contains(mx, my)) {
        m_state = ButtonState::Hover;
        if (m_onClick) m_onClick();
        return true;
    }
    m_state = m_rect.contains(mx, my) ? ButtonState::Hover : ButtonState::Idle;
    return false;
}

void Button::handleMouseMove(float mx, float my) {
    if (!m_visible) return;
    if (m_state != ButtonState::Clicked) {
        m_state = m_rect.contains(mx, my) ? ButtonState::Hover : ButtonState::Idle;
    }
}

// -----------------------------------------------------------------------------
// Dual-Layer ProgressBar Implementation (Economic vs Faith)
// -----------------------------------------------------------------------------
void ProgressBar::render(PixelCanvas& canvas, const UIThemeColors& theme) {
    if (!m_visible) return;

    const int px = static_cast<int>(m_rect.x);
    const int py = static_cast<int>(m_rect.y);
    const int pw = static_cast<int>(m_rect.w);
    const int ph = static_cast<int>(m_rect.h);

    const int halfH = std::max(4, (ph - 3) / 2);

    // Fondo oscuro con bisel de mármol
    canvas.fillRect(px, py, pw, ph, ColorRGB{20, 20, 24});
    canvas.strokeRect(px, py, pw, ph, theme.border);

    // Capa Superior: Productividad Económica (Oro / Ámbar)
    const int ecoFillW = std::clamp(static_cast<int>((pw - 2) * m_economicRatio), 0, pw - 2);
    if (ecoFillW > 0) {
        canvas.fillRect(px + 1, py + 1, ecoFillW, halfH, theme.energyBar);
        // Resplandor superior
        canvas.fillRect(px + 1, py + 1, ecoFillW, 1, ColorRGB{255, 240, 180});
    }

    // Capa Inferior: Devoción / Fe Espiritual (Rubí Sacrificial / Carmín)
    const int faithFillW = std::clamp(static_cast<int>((pw - 2) * m_faithRatio), 0, pw - 2);
    const int faithY = py + 1 + halfH + 1;
    if (faithFillW > 0) {
        canvas.fillRect(px + 1, faithY, faithFillW, halfH, theme.faithBar);
        // Resplandor sagrado
        canvas.fillRect(px + 1, faithY, faithFillW, 1, ColorRGB{255, 120, 140});
    }

    // Divisor central horizontal
    canvas.fillRect(px + 1, py + 1 + halfH, pw - 2, 1, theme.background);
}

} // namespace ReligionSim::UI
