#ifndef TOASTER_H
#define TOASTER_H

#include "Enemy.h"

class Toaster : public Enemy {
public:
    Toaster();
    ~Toaster();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;
    void render() override;
    void setTileMap(TileMap* tileMap) override;
    void setPosition(const glm::vec2& pos) override;
    glm::vec2 getPosition() const override;

private:
    int deathRow() const override;

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
