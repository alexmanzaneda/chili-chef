#ifndef BOSS_H
#define BOSS_H

#include <vector>
#include "Enemy.h"

// A fireball the boss wants to shoot (direction is normalized)
struct BossShot {
    glm::vec2 origin, direction;
};

// The oven at the end of the last level. It floats over everything towards the chef, and it
// needs several hits. It does not know the rest of the level: GameScene tells it where the
// chef is, and picks up the fireballs, flames and blasts it asks for
class Boss : public Enemy {
public:
    Boss();
    ~Boss();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;
    void render() override;
    void setTileMap(TileMap* tileMap) override;
    void setPosition(const glm::vec2& pos) override;
    glm::vec2 getPosition() const override;
    EnemyType getType() const override { return ENEMY_BOSS; }

    void hitByFire() override; // Takes a life instead of dying at once
    HitBox fireBox() const override;
    HitBox contactBox() const override;
    bool isHarmful() const override { return state != DEFEATED; }

    void setTarget(const glm::vec2& chefCenter) { target = chefCenter; }
    void setFlamesAlive(int flames) { flamesAlive = flames; }
    bool isDefeated() const { return state == DEFEATED; } // Out of lives, blowing up before it comes apart

    std::vector<BossShot> takeShots();
    bool takeSummon();                      // True once for each flame it wants to summon
    bool takeBlast(glm::vec2& center);      // True once for each small explosion of its defeat
    glm::vec2 getCenter() const;
    glm::vec2 getDoor() const; // Where the fireballs and the flames come out

private:
    enum State { FOLLOW, WARN, CLOSING, CHARGE_WARN, CHARGE, HURT, DEFEATED };
    enum Attack { ATTACK_SHOT, ATTACK_FAN, ATTACK_SUMMON, ATTACK_CHARGE };

    int deathRow() const override;

    void setState(State newState);
    void moveTowardsTarget(float speed, float dt);
    bool clampToMap(); // Keeps it inside the walls of the level; true if it had to
    bool chefInView() const;
    void startNextAttack();
    void releaseAttack();
    void shoot(float angle);
    float followSpeed() const;
    int attackPause() const;
    void showIdleFrame();

private:
    State state;
    Attack attack;
    int lives;
    int stateTime;
    int attackCooldown;
    int attackCycle;    // Which attack comes next
    int flamesAlive;
    bool roared;
    int blastTimer;
    int floatTime;      // ms it has been floating, for the bobbing

    glm::vec2 posBoss;
    glm::vec2 target;
    glm::vec2 chargeDirection;
    glm::ivec2 tileMapDispl;

    std::vector<BossShot> shots;
    bool summonPending;
    bool blastPending;
    glm::vec2 blastCenter;

    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // BOSS_H
