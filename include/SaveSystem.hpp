#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace ReligionSim {

class Game;

// Metadatos de un Mundo Guardado (inspirado en level.dat y lista de mundos de Minecraft)
struct WorldSaveMeta {
    std::string folderName;       // p. ej. "Mundo_Solarius_1"
    std::string worldName;        // Nombre visible: "Imperio de Solarius"
    std::string godName;          // Dios rector
    std::string religionName;     // Culto principal
    uint32_t seed{42};            // Semilla del generador
    int currentYear{-4000};       // Año de la partida
    HistoricalEra era{HistoricalEra::PrimordialAnimism};
    int totalPopulation{280000};
    int activeReligions{5};
    int64_t lastPlayedTimestamp{0}; // Epoch milisegundos
    std::string iconPPMPath;      // Ruta a icon.ppm / icon.png
    bool hasIcon{false};
};

class SaveSystem {
public:
    static std::string getSavesRootDirectory();

    // Descubrimiento y lectura de mundos existentes
    static std::vector<WorldSaveMeta> scanSaveSlots();

    // Guardar partida completa con captura de snapshot
    static bool saveWorld(Game& game, const std::string& worldFolderName);

    // Cargar partida completa
    static bool loadWorld(Game& game, const std::string& worldFolderName);

    // Eliminar mundo guardado
    static bool deleteWorld(const std::string& worldFolderName);

    // Formatear fecha y hora estilo Minecraft (ej. "24/10/2026 01:15")
    static std::string formatTimestamp(int64_t epochMs);

    // Formatear tamaño o resumen (ej. "48x48 | 280k hab | 4000 BCE")
    static std::string formatSummary(const WorldSaveMeta& meta);
};

} // namespace ReligionSim
