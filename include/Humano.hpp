#pragma once

#include "Types.hpp"
#include "TileSet.hpp"
#include "FirstEraAIController.hpp"
#include "HumanAIController.hpp"
#include <string>
#include <vector>

namespace ReligionSim {

class WorldMap;
class ReligionRegistry;
class IsometricCamera;
class EventLine;

class Humano {
public:
    Humano(int id = 1, const std::string& nombre = "Grog", float inicioX = 20.5f, float inicioY = 20.5f);

    // Getters y Setters
    int getId() const { return m_id; }
    const std::string& getNombre() const { return m_nombre; }
    void setNombre(const std::string& nombre) {
        m_nombre = nombre;
        m_humanUnit.unitName = nombre;
    }

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    int getCeldaX() const { return m_celdaX; }
    int getCeldaY() const { return m_celdaY; }

    AccionHumano getAccion() const { return m_accion; }
    DireccionIso getDireccion() const { return m_direccion; }

    int getReligionId() const { return m_religionId; }
    void setReligionId(int rId) { m_religionId = rId; }

    float getDevocion() const { return m_devocion; }
    void setDevocion(float d) {
        m_devocion = d;
        m_humanUnit.devotion = std::clamp(d * 100.0f, 0.0f, 100.0f);
    }

    bool esChaman() const { return m_esChaman; }
    void setEsChaman(bool chaman) {
        m_esChaman = chaman;
        if (chaman) {
            m_humanUnit.role = CommunityRole::SHAMAN;
            m_humanUnit.conversionPower = 38.0f;
            m_humanUnit.conversionInertia = 85.0f;
            m_humanUnit.willpower = 92.0f;
        }
    }

    float getTimerSenalDivina() const { return m_timerSenalDivina; }
    void activarSenalDivina(float duracion = 5.5f) {
        m_timerSenalDivina = duracion;
        m_timerEstado = duracion;
        m_accion = AccionHumano::SALUDAR;
        m_devocion = 1.0f;
        m_humanUnit.devotion = 100.0f;
        m_humanUnit.energy = 100.0f;
        m_humanUnit.currentState = HumanAIState::WORSHIPING;
    }

    float getSalud() const { return m_salud; }
    float getEnergia() const { return m_energia; }

    // Estructura demográfica y motivacional de la Primera Era (FirstEraAIController)
    const RealisticPopGroup& getPopGroup() const { return m_popGroup; }
    RealisticPopGroup& getPopGroupMutable() { return m_popGroup; }
    RealisticVillagerState getRealisticState() const { return m_popGroup.state; }

    // Estructura sistémica individual de la unidad humana (HumanAIController)
    const HumanUnit& getHumanUnit() const { return m_humanUnit; }
    HumanUnit& getHumanUnitMutable() { return m_humanUnit; }
    CommunityRole getRole() const { return m_humanUnit.role; }
    void setRole(CommunityRole r) { m_humanUnit.role = r; }
    HumanAIState getHumanAIState() const { return m_humanUnit.currentState; }

    // Control de comportamiento y animación
    void actualizar(float dt, const WorldMap& mapa);
    void cambiarAccion(AccionHumano nuevaAccion);
    void cambiarDireccion(DireccionIso nuevaDir);
    void caminarHacia(float destX, float destY, bool corriendo = false);
    void ordenarMovimientoManual(float destX, float destY);
    bool tieneOrdenManual() const { return m_ordenManual; }
    float getDestinoX() const { return m_destinoX; }
    float getDestinoY() const { return m_destinoY; }

    // Obtener el recorte del tileset para este Humano
    TileRect getTileRect(const TileSet& tileset) const;

    // Renderizar al humano en el buffer usando el TileSet del usuario
    void renderizar(uint32_t* dstBuffer, int dstW, int dstH,
                    const IsometricCamera& camara, const WorldMap& mapa,
                    const TileSet& tileset, ColorRGB auraColor, bool seleccionado = false) const;

private:
    int m_id{1};
    std::string m_nombre{"Grog"};

    float m_x{20.5f};
    float m_y{20.5f};
    int m_celdaX{20};
    int m_celdaY{20};

    float m_destinoX{20.5f};
    float m_destinoY{20.5f};

    AccionHumano m_accion{AccionHumano::IDLE};
    DireccionIso m_direccion{DireccionIso::SW};

    float m_velocidad{0.65f};
    int m_religionId{1};
    float m_devocion{0.80f};
    bool m_esChaman{false};
    bool m_ordenManual{false};
    float m_timerSenalDivina{0.0f};
    float m_salud{100.0f};
    float m_energia{100.0f};

    float m_timerEstado{2.5f};
    float m_animTimer{0.0f};
    RealisticPopGroup m_popGroup{};
    HumanUnit m_humanUnit{};

    void tomarDecisionIA(const WorldMap& mapa);
};

class GestorHumanos {
public:
    GestorHumanos();

    void inicializarPoblacion(WorldMap& mapa, const ReligionRegistry& registry, int cantidad = 50);
    void actualizar(float dt, WorldMap& mapa);
    void renderizarTodos(uint32_t* dstBuffer, int dstW, int dstH,
                         const IsometricCamera& camara, const WorldMap& mapa,
                         const TileSet& tileset, const ReligionRegistry& registry) const;

    const std::vector<Humano>& getHumanos() const { return m_humanos; }
    std::vector<Humano>& getHumanos() { return m_humanos; }

    void agregarHumano(const Humano& h);
    const Humano* getHumanoSeleccionado() const;
    Humano* getHumanoSeleccionadoMutable();
    void seleccionarHumano(int id);
    int buscarChamanDeReligion(int religionId) const;
    void ejecutarSenalDivina(int tipoSenal, WorldMap& mapa, ReligionRegistry& registry,
                             EventLine& eventLine, int currentYear);
    int buscarHumanoCercano(float worldX, float worldY, float radioMax = 1.0f, int soloReligionId = -1) const;
    int buscarHumanoEnPantalla(float screenX, float screenY, const IsometricCamera& camara,
                               const WorldMap& mapa, int soloReligionId = -1) const;

private:
    std::vector<Humano> m_humanos;
    int m_seleccionadoId{-1};
    int m_siguienteId{1};
    FirstEraAIController m_firstEraAI{};
    HumanAIController m_humanAI{};
};

} // namespace ReligionSim
