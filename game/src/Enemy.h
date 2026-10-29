#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include "Sprite.h"
#include "TileMap.h"

// Common interface of the enemies, so GameScene can keep all of them in the same list.
// It also does their death, the same for all of them: the enemy freezes, blinks white and
// comes apart like the boxes (images/<enemy>_death.png, made by tools/make_disintegrate.py)
class Enemy {
public:
    Enemy();
    virtual ~Enemy();

    virtual void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) = 0;
    virtual void update(int deltaTime) = 0;
    virtual void render() = 0;
    virtual void setTileMap(TileMap* tileMap) = 0;
    virtual void setPosition(const glm::vec2& pos) = 0;
    virtual glm::vec2 getPosition() const = 0;

    void kill(); // Starts the death from the pose it has now
    bool isDying() const { return deathSprite != NULL; }
    bool isDead() const; // The death animation is over, GameScene can delete it
    void updateDeath(int deltaTime);
    void renderDeath();

protected:
    // Called from init: the death image has one row for each frame of the enemy
    void initDeath(const std::string& file, int rows, ShaderProgram& shaderProgram);
    virtual int deathRow() const = 0; // Row of the death image with the frame shown now

private:
    Texture deathSpritesheet;
    ShaderProgram* program;
    int deathRows;
    Sprite* deathSprite; // NULL while it is alive
    int deathTime;
};

#endif // ENEMY_H
