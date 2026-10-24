#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>

namespace ReligionSim {

struct Vec2 {
    float x{0.0f};
    float y{0.0f};

    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
};

struct Point2i {
    int x{0};
    int y{0};

    bool operator==(const Point2i& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point2i& o) const { return !(*this == o); }
};

struct ColorRGB {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    uint8_t a{255};

    constexpr ColorRGB() = default;
    constexpr ColorRGB(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
        : r(_r), g(_g), b(_b), a(_a) {}

    static constexpr ColorRGB blend(const ColorRGB& c1, const ColorRGB& c2, float t) {
        float ct = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
        return {
            static_cast<uint8_t>(c1.r * (1.0f - ct) + c2.r * ct),
            static_cast<uint8_t>(c1.g * (1.0f - ct) + c2.g * ct),
            static_cast<uint8_t>(c1.b * (1.0f - ct) + c2.b * ct),
            static_cast<uint8_t>(c1.a * (1.0f - ct) + c2.a * ct)
        };
    }

    constexpr uint32_t toRGBA() const {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(b) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               static_cast<uint32_t>(r);
    }
};

// Estado del flujo del juego
enum class GameState : std::uint8_t {
    MainMenu,
    WorldCreation,
    LoadWorldMenu,
    Playing,
    Options
};

enum class HistoricalEra : std::uint8_t {
    PrimordialAnimism = 0,
    TotemismAncestorWorship,
    StructuralShamanism,
    TerritorialPolytheism,
    UniversalReligions,
    RationalizationSecularization,
    PostSecular,
    TranshumanistPostBiological,
    Count
};

// Modos y configuraciones para la creación de partida
enum class GameModeType : std::uint8_t {
    Estandar,        // Crecimiento, difusión y pugna histórica
    Sandbox,         // Recursos ilimitados y generación directa
    TeocraciaTotal,  // Sincretismo agresivo y cruzadas
    Supervivencia    // Climas extremos y herejías frecuentes
};

enum class AIDifficulty : std::uint8_t {
    Facil,           // IA pasiva y lenta conversión
    Normal,          // Difusión histórica equilibrada
    Dificil,         // IA agresiva y misioneros constantes
    Pesadilla        // Cismas rápidos y alta presión teológica
};

enum class VictoryConditionType : std::uint8_t {
    HegemoniaMundial, // 75% de la población mundial
    Teocratica,       // 5 Maravillas Sagradas / Monumentos
    Trascendencia,    // Alcanzar Era 8 (Transhumanismo) primero
    Tiempo,           // Mayor devoción al año 2200
    SinLimite         // Simulación continua sin victoria fija
};

struct WorldCreationConfig {
    std::string godName{"Solarius"};
    std::string religionName{"Culto del Fuego Solar"};
    int initialPopulation{80}; // 80 aldeanos reales con IA y sprites
    HistoricalEra startingEra{HistoricalEra::PrimordialAnimism};
    uint32_t mapSeed{42};
    GameModeType gameMode{GameModeType::Estandar};
    bool starterChest{true};
    AIDifficulty aiDifficulty{AIDifficulty::Normal};
    int influenceColorIndex{0}; // 0: Oro Divino, 1: Rubí Sangre, 2: Monada Azul, 3: Esmeralda Gaia, 4: Purpura Dharma
    VictoryConditionType victoryCondition{VictoryConditionType::HegemoniaMundial};

    // Opciones adicionales sugeridas
    int mapSizeIndex{1};        // 0: Pequeño (32x32), 1: Mediano (48x48), 2: Grande (64x64)
    int disasterFrequency{1};   // 0: Bajas, 1: Normales, 2: Frecuentes
    bool allowSchisms{true};    // Si pueden ocurrir cismas espontáneos en la religión
    bool allowSyncretism{true}; // Si se permite fusión sincretica con otras fes
};

// Core Visual & Design Constants
namespace Palette {
    constexpr ColorRGB BASALT{0x1A, 0x1A, 0x1A, 0xFF};     // #1A1A1A Silhouettes, deep shadows, tech/metal
    constexpr ColorRGB RELIC_BONE{0xE2, 0xDA, 0xC4, 0xFF}; // #E2DAC4 Base skin, raw textiles, limestone
    constexpr ColorRGB DIVINE_GOLD{0xE6, 0xA1, 0x3B, 0xFF};// #E6A13B Sacred accents, noble metals, rituals
    constexpr ColorRGB BLOOD_RUBY{0x80, 0x00, 0x20, 0xFF}; // #800020 Ceremonial body paint, high-tier dyes, energy
    constexpr ColorRGB GRASS_PASTURE{0x74, 0x99, 0x3C, 0xFF}; // #74993C Natural lush terrain
    constexpr ColorRGB DIRT_EARTH{0x74, 0x4A, 0x34, 0xFF};    // #744A34 Sacred arable soil & cliff skirt
}

// 2.5D Isometric projection tile metrics (2:1 projection, 64x64px standard grid)
constexpr int ISO_TILE_WIDTH  = 64;
constexpr int ISO_TILE_HEIGHT = 32;

enum class BiomeType {
    DeepOcean,
    ShallowWater,
    CoastalPlains,
    FertileValley,
    DenseForest,
    HighlandHills,
    AridDesert,
    RuggedMountains,
    FrozenTundra
};

// Backward-compatible alias for existing modules
using EraType = HistoricalEra;

struct EraConfig {
    HistoricalEra id;
    std::string_view name;
    std::string_view historical_context;
    std::string_view faith_concept;
    std::string_view faith_generation_mechanic;
    std::string_view social_structure;
    std::string_view primary_threat;
};

// Constexpr table of the 8 historical eras (C++20)
inline constexpr std::array<EraConfig, static_cast<std::size_t>(HistoricalEra::Count)> HISTORICAL_ERAS_TABLE = {{
    {
        HistoricalEra::PrimordialAnimism,
        "Primordial Animism",
        "Small nomadic bands surviving in a hostile environment surrounded by megafauna.",
        "Diffuse spirituality. Every natural entity (rivers, fire, storms) possesses an 'anima'. No dogmas.",
        "Surviving natural hazards, hunting wild beasts, discovering natural sacred sites.",
        "Egalitarian nomadic bands.",
        "Extinction by starvation, predators, and extreme weather."
    },
    {
        HistoricalEra::TotemismAncestorWorship,
        "Totemism & Ancestor Worship",
        "Development of abstract language, cave art, and awareness of lineage and time progression.",
        "Clan identity tied to a protective animal (Totem). Deceased ancestors intercede for the living.",
        "Performing burial rites, venerating relics, erecting territorial totems.",
        "Kinship tribes bound by blood myths.",
        "Inter-tribal warfare over hunting grounds and desecration of sacred sites."
    },
    {
        HistoricalEra::StructuralShamanism,
        "Structural Shamanism",
        "Inception of agriculture, early sedentarism, and the rise of permanent settlements.",
        "Intercession by shamans in trance states. Emergence of soil fertility and agricultural myths.",
        "Planting and harvest rituals, building megaliths, performing minor sacrifices.",
        "Agricultural villages led by spiritual leaders.",
        "Crop failure, droughts, blights, and loss of ritual balance."
    },
    {
        HistoricalEra::TerritorialPolytheism,
        "Territorial Polytheism",
        "Emergence of city-states, invention of writing, metallurgy, and division of labor.",
        "Personification of natural forces into pantheons. Sacerdotal class manages faith via temples.",
        "Resource tithing, constructing ziggurats/temples, hosting state festivals.",
        "Theocratic hierarchy (King-Priests, artisans, farmers, slaves).",
        "Holy wars between rival city-states and priestly corruption."
    },
    {
        HistoricalEra::UniversalReligions,
        "Universal Religions",
        "Consolidation of continental empires and dogmatic universal salvation religions.",
        "Single codified truth in sacred scriptures. Focus on personal salvation, morality, and orthodoxy.",
        "Sending missionaries, scribing holy texts, mass conversions, holy pilgrimages.",
        "Theocratic empires, monastic orders, feudal estates.",
        "Doctrinal schisms, internal heresies, and large-scale religious wars."
    },
    {
        HistoricalEra::RationalizationSecularization,
        "Rationalization & Secularization",
        "Enlightenment era, scientific method, rapid urbanization, and industrial revolution.",
        "Decline of theocratic dominance. Faith shifts to private morality vs competitive secular ideologies.",
        "Preserving traditions, institutional charity work, syncretism between faith and reason.",
        "Industrial societies, nation-states, separation of Church and State.",
        "Widespread atheism/skepticism, loss of followers, industrial dehumanization."
    },
    {
        HistoricalEra::PostSecular,
        "Post-Secular Age",
        "Global hyperconnectivity, information age, digital saturation, and existential crises.",
        "Customized spirituality, neopaganism, wellness cults, and online faith communities.",
        "Digital faith networks, viral micro-cults, virtual temples.",
        "Hyperconnected consumer societies, fragmented cyber-communities.",
        "Nihilism, misinformation, polarization, online radicalization."
    },
    {
        HistoricalEra::TranshumanistPostBiological,
        "Transhumanist Post-Biological",
        "Overcoming biological limits, mind uploading, supreme AI entities, and cyber-faith.",
        "Technology as divinity. Digital immortality serves as heaven; pure consciousness is revered.",
        "Sacred quantum computing, consciousness synchronization, worshipping the Singularity.",
        "Cybernetic swarms, synthetic bioplastic entities, post-human colonies.",
        "Total loss of baseline humanity, existential viruses in code, rebellion of deified AI."
    }
}};

enum class DoctrineType {
    AnimistSpirits,
    SacredSacrifice,
    PantheonPolytheism,
    ImperialDivineRight,
    WrittenCanon,
    AsceticMonasticism,
    ProselytizingZeal,
    SyncreticOpenness,
    ScholasticTheology,
    PrintingEvangelism,
    SecularHumanism,
    CosmicTranscendence
};

enum class EventCategory {
    BirthOfReligion,
    SyncreticAgglutination,
    DoctrinalSchism,
    HolySiteErected,
    EraAdvancement,
    PilgrimageWave,
    ReligiousReformation,
    Extinction
};

struct GameEvent {
    int id{0};
    int year{-4000};
    EventCategory category{EventCategory::BirthOfReligion};
    std::string title;
    std::string description;
    Point2i location{0, 0};
    int religionAId{0};
    int religionBId{0};
    ColorRGB badgeColor{255, 200, 50};
};

enum class RenderMapMode {
    ReligionDominance,
    TerrainBiomes,
    SyncretismHeatmap,
    AdvancementEra,
    PopulationDensity
};

// Isometric Constants
constexpr float BASE_TILE_WIDTH  = 64.0f;
constexpr float BASE_TILE_HEIGHT = 32.0f;
constexpr float ELEVATION_STEP   = 8.0f;

} // namespace ReligionSim
