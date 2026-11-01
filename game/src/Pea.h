#ifndef PEA_H
#define PEA_H

#include "Enemy.h"

// Enemy of the second world. It patrols jumping from side to side with a fixed
// rhythm (short, short, long). Before every jump it crouches, so the player can
// see it coming, and it simulates the jump to never land in a pit.
class Pea : public Enemy {
public:
    Pea();
    ~Pea();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;
    void render() override;
    void setTileMap(TileMap* tileMap) override;
    void setPosition(const glm::vec2& pos) override;
    glm::vec2 getPosition() const override;
    EnemyType getType() const override { return ENEMY_PEA; }

private:
    enum State { IDLE, CROUCH, AIRBORNE, LAND, CRAWL, SLIDE };

    struct Jump {
        float distance; // Horizontal pixels it advances when it lands at the same height
        float height;   // Pixels it goes up
    };

    void chooseJump();
    bool findJump(int dir, float maxDrop); // Sets nextJump if it finds one
    bool canJump(int dir, const Jump& jump, float maxDrop) const;
    glm::vec2 launchVelocity(int dir, const Jump& jump) const;
    bool step(glm::vec2& pos, glm::vec2& vel, float dt) const; // true when it lands
    bool isHanging(const glm::vec2& pos) const; // Half of the body out of the tile it stands on
    float columnBelowCenter(const glm::vec2& pos) const;
    void startSlide();
    bool canCrawl(int dir) const;
    bool isGrounded() const;
    void setState(State newState);
    void updateFrame();
    int deathRow() const override;

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 posPea;
    glm::vec2 velocity; // px/s, only used while it is in the air

    State state;
    int stateTime;   // ms in the current state
    int direction;   // 1 right, -1 left
    int cycleIndex;  // Next jump of the rhythm
    Jump nextJump;   // Chosen in IDLE, done after CROUCH
    float slideTargetX; // X where it stops being held by the edge it slides off

    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // PEA_H
