#ifndef FLAME_H
#define FLAME_H

#include "Enemy.h"

// A little flame that flies in diagonal and bounces against everything solid. It does not need
// a floor, so the boss can summon it anywhere. It blinks harmless for a moment when it appears
class Flame : public Enemy {
public:
    Flame();
    ~Flame();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;
    void render() override;
    void setTileMap(TileMap* tileMap) override;
    void setPosition(const glm::vec2& pos) override;
    glm::vec2 getPosition() const override;
    EnemyType getType() const override { return ENEMY_FLAME; }
    bool isHarmful() const override { return appearTime <= 0; }

private:
    int deathRow() const override;

    bool blockedX(float x) const;
    bool blockedY(float y) const;

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 posFlame;
    glm::vec2 velocity; // px/s
    int appearTime;     // ms left of the blinking

    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // FLAME_H
