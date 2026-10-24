#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace ReligionSim {

enum class AccionHumano {
    IDLE,               // De pie descansando (Fila 0, Col 0)
    CAMINAR,            // Caminando hacia adelante (Fila 0, Col 1)
    CORRER,             // Corriendo veloz (Fila 0, Col 3 y Fila 1, Col 0)
    TALAR,              // Golpe descendente con hacha de piedra (Fila 1, Col 3)
    RECOLECTAR,         // Recolectando / inclinándose con herramienta (Fila 1, Col 3)
    CARGAR,             // Transportando leños o reliquias sagradas (Fila 0, Col 1)
    REZAR,              // En cuclillas / orando ante el tótem o fogata (Fila 2, Col 1 y 2)
    ATACAR,             // En guardia / golpe frontal con mazo (Fila 1, Col 1 y 2 / Fila 2, Col 0)
    SALUDAR,            // Celebración / brazos en alto / victoria (Fila 3, Col 3)
    MORIR,              // Tendido en el suelo inerte (Fila 3, Col 1)
    HERIDO              // Tambaleándose / noqueado (Fila 2, Col 3 y Fila 3, Col 0)
};

enum class DireccionIso {
    SW, // Suroeste (hacia abajo-izquierda)
    SE, // Sureste (hacia abajo-derecha)
    NE, // Noreste (hacia arriba-derecha)
    NW  // Noroeste (hacia arriba-izquierda)
};

struct TileRect {
    int x{0};
    int y{0};
    int width{0};
    int height{0};
};

class TileSet {
public:
    TileSet();

    // Carga la imagen del tileset desde archivo (RGBA binario, BMP, o genera datos si no existe)
    bool cargarDesdeArchivo(const std::string& rutaArchivo);

    int getAncho() const { return m_width; }
    int getAlto() const { return m_height; }
    const std::vector<uint32_t>& getPixeles() const { return m_pixels; }

    uint32_t getPixel(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 0;
        return m_pixels[y * m_width + x];
    }

    // Obtiene el rectángulo exacto dentro del tileset de 1024x558 para una acción y dirección
    TileRect obtenerRectangulo(AccionHumano accion, DireccionIso direccion) const;

    // Dibuja el tile sobre un framebuffer destino con mezcla alfa, soporte de escala y contorno de selección
    void blitTile(uint32_t* dstBuffer, int dstW, int dstH,
                  const TileRect& rect, float pieX, float pieY,
                  float escala, ColorRGB auraColor, bool seleccionado = false) const;

private:
    int m_width{1024};
    int m_height{558};
    std::vector<uint32_t> m_pixels;

    void inicializarMapaRectangulos();
    std::unordered_map<int, TileRect> m_rectMap;

    int hashAccionDir(AccionHumano a, DireccionIso d) const {
        return (static_cast<int>(a) << 8) | static_cast<int>(d);
    }
};

} // namespace ReligionSim
