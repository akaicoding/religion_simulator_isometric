#pragma once

#include "Types.hpp"
#include "TileSet.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <algorithm>

namespace ReligionSim::UI {

// -----------------------------------------------------------------------------
// 1. Rectangles & Margins for 9-Patch and Layouts
// -----------------------------------------------------------------------------
struct Rect {
    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    [[nodiscard]] constexpr bool contains(float px, float py) const noexcept {
        return px >= x && px <= (x + w) && py >= y && py <= (y + h);
    }
};

struct Margins {
    int left{8};
    int right{8};
    int top{8};
    int bottom{8};
};

// -----------------------------------------------------------------------------
// 2. Liturgical Theme Manager & Era Palettes
// -----------------------------------------------------------------------------
struct UIThemeColors {
    ColorRGB background;    // Basalt / Obsidian / Void
    ColorRGB border;        // Relic Bone / Polished Gold / Cyber Cyan
    ColorRGB borderAccent;  // Divine Gold / Neon Pulse
    ColorRGB textPrimary;   // Relic Bone / Ivory
    ColorRGB textSecondary; // Aged Linen / Muted Silver
    ColorRGB accentAlert;   // Blood Ruby / Warning Crimson
    ColorRGB energyBar;     // Divine Gold / Sacred Amber
    ColorRGB faithBar;      // Blood Ruby / Sacred Carmine
};

// Constexpr palettes per era archetype
namespace LiturgicalPalettes {
    // Eras 1 - 3: Tribal, Animist, Neolithic (Raw Stone, Bone, Earth)
    inline constexpr UIThemeColors TRIBAL_SHAMANIC = {
        .background    = Palette::BASALT,      // #1A1A1A
        .border        = ColorRGB{120, 115, 105},
        .borderAccent  = Palette::RELIC_BONE,  // #E2DAC4
        .textPrimary   = Palette::RELIC_BONE,
        .textSecondary = ColorRGB{170, 160, 145},
        .accentAlert   = Palette::BLOOD_RUBY,  // #800020
        .energyBar     = ColorRGB{180, 140, 60},
        .faithBar      = Palette::BLOOD_RUBY
    };

    // Eras 4 - 5: Classical Temples & Monastic Empires (Polished Gold, Relic Bone)
    inline constexpr UIThemeColors IMPERIAL_SACRED = {
        .background    = Palette::BASALT,
        .border        = ColorRGB{90, 85, 75},
        .borderAccent  = Palette::DIVINE_GOLD, // #E6A13B
        .textPrimary   = ColorRGB{255, 245, 225},
        .textSecondary = ColorRGB{200, 190, 170},
        .accentAlert   = Palette::BLOOD_RUBY,
        .energyBar     = Palette::DIVINE_GOLD,
        .faithBar      = ColorRGB{190, 30, 45} // Sacrificial Ruby
    };

    // Eras 6 - 7: Secular & Industrial (Iron, Brass, Scholastic Ink)
    inline constexpr UIThemeColors SECULAR_REASON = {
        .background    = ColorRGB{24, 26, 32},
        .border        = ColorRGB{70, 75, 88},
        .borderAccent  = ColorRGB{180, 160, 120},
        .textPrimary   = ColorRGB{235, 238, 245},
        .textSecondary = ColorRGB{150, 158, 175},
        .accentAlert   = ColorRGB{210, 40, 50},
        .energyBar     = ColorRGB{70, 140, 200},
        .faithBar      = ColorRGB{200, 140, 50}
    };

    // Era 8: Transhumanist Cyber-Epoch (Dark Obsidian, Nanotech Neon)
    inline constexpr UIThemeColors POST_BIOLOGICAL = {
        .background    = ColorRGB{12, 14, 20},
        .border        = ColorRGB{35, 55, 75},
        .borderAccent  = ColorRGB{0, 235, 215},  // Cyber Cyan
        .textPrimary   = ColorRGB{230, 255, 250},
        .textSecondary = ColorRGB{100, 180, 190},
        .accentAlert   = ColorRGB{255, 40, 110}, // Cyber Pink
        .energyBar     = ColorRGB{0, 235, 215},
        .faithBar      = ColorRGB{170, 40, 255}  // Psionic Violet
    };
}

[[nodiscard]] constexpr UIThemeColors GetThemeForEra(HistoricalEra era) noexcept {
    switch (era) {
        case HistoricalEra::PrimordialAnimism:
        case HistoricalEra::TotemismAncestorWorship:
        case HistoricalEra::StructuralShamanism:
            return LiturgicalPalettes::TRIBAL_SHAMANIC;

        case HistoricalEra::TerritorialPolytheism:
        case HistoricalEra::UniversalReligions:
            return LiturgicalPalettes::IMPERIAL_SACRED;

        case HistoricalEra::RationalizationSecularization:
        case HistoricalEra::PostSecular:
            return LiturgicalPalettes::SECULAR_REASON;

        case HistoricalEra::TranshumanistPostBiological:
            return LiturgicalPalettes::POST_BIOLOGICAL;

        default:
            return LiturgicalPalettes::IMPERIAL_SACRED;
    }
}

// -----------------------------------------------------------------------------
// 3. Pixel-Perfect Rendering Canvas Abstraction
// -----------------------------------------------------------------------------
class PixelCanvas {
public:
    virtual ~PixelCanvas() = default;

    virtual void fillRect(int x, int y, int w, int h, ColorRGB color) = 0;
    virtual void strokeRect(int x, int y, int w, int h, ColorRGB color) = 0;
    virtual void drawText(int x, int y, std::string_view text, ColorRGB color, int scale = 1) = 0;
    virtual void blitSubImage(int dstX, int dstY, int dstW, int dstH,
                              const uint32_t* srcPixels, int srcStrideW, int srcStrideH,
                              int srcX, int srcY, int srcW, int srcH,
                              ColorRGB tint) = 0;
};

// -----------------------------------------------------------------------------
// 4. Nine-Patch (Nine-Slice) Panel Renderer
// -----------------------------------------------------------------------------
void DrawNinePatchPanel(PixelCanvas& canvas,
                        const uint32_t* atlasPixels, int atlasW, int atlasH,
                        TileRect srcPatch, Margins margins,
                        Rect targetDest, ColorRGB tint);

// -----------------------------------------------------------------------------
// 5. Core Modular UI Widgets: Widget, Panel, Button, ProgressBar
// -----------------------------------------------------------------------------
class Widget {
public:
    virtual ~Widget() = default;
    virtual void render(PixelCanvas& canvas, const UIThemeColors& theme) = 0;
    virtual bool handleMouseDown(float mx, float my) { (void)mx; (void)my; return false; }
    virtual bool handleMouseUp(float mx, float my) { (void)mx; (void)my; return false; }
    virtual void handleMouseMove(float mx, float my) { (void)mx; (void)my; }

    void setRect(Rect r) { m_rect = r; }
    [[nodiscard]] const Rect& getRect() const noexcept { return m_rect; }
    void setVisible(bool v) noexcept { m_visible = v; }
    [[nodiscard]] bool isVisible() const noexcept { return m_visible; }

protected:
    Rect m_rect{0.0f, 0.0f, 0.0f, 0.0f};
    bool m_visible{true};
};

// --- Panel Container ---
class Panel : public Widget {
public:
    Panel(Rect rect, std::string title = "")
        : m_title(std::move(title)) { m_rect = rect; }

    void addChild(std::shared_ptr<Widget> child) {
        m_children.push_back(std::move(child));
    }

    void render(PixelCanvas& canvas, const UIThemeColors& theme) override;
    bool handleMouseDown(float mx, float my) override;
    bool handleMouseUp(float mx, float my) override;
    void handleMouseMove(float mx, float my) override;

private:
    std::string m_title;
    std::vector<std::shared_ptr<Widget>> m_children;
};

// --- Button Widget ---
enum class ButtonState : std::uint8_t { Idle, Hover, Clicked };

class Button : public Widget {
public:
    Button(Rect rect, std::string label, std::function<void()> onClick = nullptr)
        : m_label(std::move(label)), m_onClick(std::move(onClick)) { m_rect = rect; }

    void render(PixelCanvas& canvas, const UIThemeColors& theme) override;
    bool handleMouseDown(float mx, float my) override;
    bool handleMouseUp(float mx, float my) override;
    void handleMouseMove(float mx, float my) override;

    void setLabel(std::string label) { m_label = std::move(label); }
    void setOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }

private:
    std::string m_label;
    std::function<void()> m_onClick;
    ButtonState m_state{ButtonState::Idle};
};

// --- Dual-Layer Progress Bar (Economic Productivity vs Faith/Devotion) ---
class ProgressBar : public Widget {
public:
    ProgressBar(Rect rect, float initialEco = 0.5f, float initialFaith = 0.5f)
        : m_economicRatio(initialEco), m_faithRatio(initialFaith) { m_rect = rect; }

    void setRatios(float eco, float faith) noexcept {
        m_economicRatio = std::clamp(eco, 0.0f, 1.0f);
        m_faithRatio = std::clamp(faith, 0.0f, 1.0f);
    }

    void render(PixelCanvas& canvas, const UIThemeColors& theme) override;

private:
    float m_economicRatio{0.5f};
    float m_faithRatio{0.5f};
};

} // namespace ReligionSim::UI
