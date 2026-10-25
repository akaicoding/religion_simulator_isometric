#pragma once

#include "Types.hpp"
#include "IsometricCamera.hpp"
#include "WorldMap.hpp"
#include "Religion.hpp"
#include "AdvancementSystem.hpp"
#include "ReplicationEngine.hpp"
#include "AgglutinationEngine.hpp"
#include "Villager.hpp"
#include "Humano.hpp"
#include "Timeline.hpp"
#include "TimeEngine.hpp"
#include "EventLine.hpp"
#include "Renderer.hpp"
#include "FogOfWar.hpp"
#include <memory>
#include <string>

namespace ReligionSim {

class Game {
public:
    Game(int windowWidth = 1280, int windowHeight = 720);
    ~Game();

    bool init();
    void run();
    void runHeadless(int maxTicks = 200, const std::string& exportPPM = "snapshot.ppm");

    void update(float deltaTime);
    void render();

    // Input handlers
    void onMouseDown(int button, float x, float y);
    void onMouseUp(int button, float x, float y);
    void onMouseMove(float x, float y);
    void onMouseWheel(float delta, float x, float y);
    void onKeyPress(int keySym);

    // Dynamic resize handler
    void onWindowResize(int newWidth, int newHeight);

    // Gameplay commands
    void selectCell(int gx, int gy);
    void triggerSyncretismAtSelection();
    void triggerSchismAtSelection();
    void buildHolySiteAtSelection();

    // Accessors for testing
    const WorldMap& getMap() const { return m_map; }
    const ReligionRegistry& getRegistry() const { return m_registry; }
    const Timeline& getTimeline() const { return m_timeline; }
    const EventLine& getEventLine() const { return m_eventLine; }
    const TimeEngine& getTimeEngine() const { return m_timeEngine; }
    TimeEngine& getTimeEngine() { return m_timeEngine; }
    const GestorHumanos& getGestorHumanos() const { return m_gestorHumanos; }
    GestorHumanos& getGestorHumanos() { return m_gestorHumanos; }
    RenderMapMode getMapMode() const { return m_mapMode; }
    GameState getGameState() const { return m_gameState; }
    void setGameState(GameState state) { m_gameState = state; }
    const WorldCreationConfig& getCreationConfig() const { return m_creationConfig; }
    WorldCreationConfig& getCreationConfig() { return m_creationConfig; }
    const Renderer& getRenderer() const { return m_renderer; }
    Renderer& getRenderer() { return m_renderer; }
    const FaithFogOfWar& getFogOfWar() const { return m_fog; }
    bool isLuminusActive() const { return m_cheatLuminusActive; }
    void runCommand(const std::string& cmd) { executeCommand(cmd); }

    void startNewCustomWorld();
    void loadStateFromSave(const WorldMap& map, const ReligionRegistry& registry,
                           int currentYear, const std::string& godName,
                           const std::string& religionName, uint32_t seed);
    void saveCurrentGame(const std::string& slotName = "");

private:
    int m_windowWidth{1280};
    int m_windowHeight{720};
    bool m_isRunning{true};
    GameState m_gameState{GameState::MainMenu};
    int m_hoveredMenuButtonId{0};
    int m_activeInputField{0}; // 501: GodName, 502: ReligionName, 503: MapSeed
    int m_selectedSaveIndex{0};

    WorldCreationConfig m_creationConfig;

    WorldMap m_map;
    ReligionRegistry m_registry;
    IsometricCamera m_camera;
    AdvancementSystem m_advancement;
    ReplicationEngine m_replication;
    AgglutinationEngine m_agglutination;
    GestorHumanos m_gestorHumanos;
    Timeline m_timeline;
    TimeEngine m_timeEngine;
    EventLine m_eventLine;
    Renderer m_renderer;

    Point2i m_selectedCell{24, 24};
    RenderMapMode m_mapMode{RenderMapMode::ReligionDominance};
    bool m_scrubbingTimeline{false};
    float m_mouseX{640.0f};
    float m_mouseY{360.0f};

    // Consola de comandos/trucos (tecla Enter) y truco "SELECTOR"
    bool m_commandBoxOpen{false};
    std::string m_commandInput{};
    bool m_cheatSelectorActive{false};
    std::string m_commandFeedback{};
    float m_commandFeedbackTimer{0.0f};

    // Niebla de Guerra de la Fe: sólo se ve y controla lo que alcanza la fe del pueblo del jugador.
    // El truco "LUMINUS" la disipa por completo.
    FaithFogOfWar m_fog;
    bool m_cheatLuminusActive{false};
    static constexpr int PLAYER_RELIGION_ID = 1;

    void processButtonAction(int buttonId);
    void executeCommand(const std::string& rawCmd);
    void handleX11Events();
    void initFaithFog(int playerRelId);
    void refreshFaithFog();
    void focusCameraOnPlayerFaith();
    int resolvePlayerReligionId() const;
    bool isCellHiddenByFog(int gx, int gy) const { return m_fog.isEnabled() && !m_fog.isVisible(gx, gy); }
    void showFeedback(const std::string& msg, float seconds = 4.0f);
};

} // namespace ReligionSim
