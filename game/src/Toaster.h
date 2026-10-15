#ifndef TOASTER_H
#define TOASTER_H

#include "Sprite.h"
#include "TileMap.h"

class Toaster {
public:
    Toaster();
    ~Toaster();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
    void update(int deltaTime);
    void render();
    void setTileMap(TileMap* tileMap);
    void setPosition(const glm::vec2& pos);
    glm::vec2 getPosition() const;

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 posToaster;

    int direction; // 1 para moverse a la derecha, -1 para la izquierda
    float speed;     // Píxeles por frame que avanza

    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // TOASTER_H
