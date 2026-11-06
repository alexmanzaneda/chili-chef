#ifndef FLAME_H
#define FLAME_H

#include "Enemy.h"

// A little flame that flies in diagonal and bounces against everything solid. It does not need
// a floor. The boss throws them out of its door: they fly out through anything, blinking and
// harmless, until they are in a free place. One placed in the map just blinks for a moment
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
    bool isHarmful() const override { return appearTime <= 0 && !launched; }

    void launch(int direction); // Thrown by the boss: downwards, to that side (-1 left, 1 right)

private:
    int deathRow() const override;

    bool blockedX(float x) const;
    bool blockedY(float y) const;
    bool insideWall(const glm::vec2& pos) const;
    void updateLaunch(int deltaTime);

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 posFlame;
    glm::vec2 velocity; // px/s
    int appearTime;     // ms left of the blinking
    bool launched;      // Still flying out of the boss
    int launchTime;     // ms since it was thrown

    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // FLAME_H
