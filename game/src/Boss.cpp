#include "Boss.h"
#include "Audio.h"
#include <cmath>
#include <cstdlib>

#define BOSS_SIZE 42
#define BOSS_LIVES 5
#define MAX_FLAMES 3         // It does not summon more while these are alive

// A slow oven gives time to lure it over a bomb (their fuse is 3 s); it hurries as it loses lives
#define SPEED_FULL 30.f      // px/s with 5-4 lives
#define SPEED_ANGRY 38.f     // with 3 lives
#define SPEED_FURIOUS 46.f   // with 2-1 lives

#define FIRST_ATTACK_TIME 3000 // ms before its first attack, so the chef can get his bearings
#define PAUSE_FULL 4000        // ms between attacks with 5-4 lives
#define PAUSE_ANGRY 3500
#define PAUSE_FURIOUS 2800
#define PAUSE_AFTER_HIT 1000

#define WARN_TIME 700        // ms with the door opening before the attack
#define CLOSE_TIME 300       // ms closing the door after it
#define CHARGE_WARN_TIME 800 // ms shaking before charging
#define CHARGE_TIME 900      // ms of the charge at most
#define CHARGE_SPEED 200.f   // px/s (the chef runs at 120)
#define HURT_TIME 1000       // ms it cannot be hurt after a hit: a fire lasts 333 ms and would take all its lives
#define HURT_BLINK_STEP 60   // ms white / ms normal while it is hurt
#define DEFEAT_TIME 1600     // ms blowing up before it comes apart
#define BLAST_STEP 180       // ms between the small explosions of its defeat

#define FAN_ANGLE 0.45f      // Radians between the fireballs of the fan
#define VIEW_RANGE 220.f     // Px from the chef: the camera shows at least this much, so nothing shoots from off screen
#define SHAKE_SIZE 2         // Px it shakes
#define BOB_SIZE 2.f         // Px it goes up and down while it floats
#define BOB_SPEED 0.004f     // Rad/ms

// images/boss.png: cells of 42x42, the rows in between are empty
#define SHEET_CELLS 6
#define ROW_LIVES 0          // Columns: 5, 4, 3, 2, 1 and 0 lamps lit
#define ROW_HURT 2           // Column 1 (column 0 is the white one, the game does it with setFlash)
#define ROW_ATTACK 4         // Columns: the door opening
#define ATTACK_FRAMES 4
#define DOOR_FPS 6
#define CLOSE_FPS 13

// The door the fireballs and the flames come out of, from its top left corner
#define DOOR_X 21.f
#define DOOR_Y 26.f

enum BossAnims { ANIM_LAMPS_FIRST, ANIM_HURT = ANIM_LAMPS_FIRST + BOSS_LIVES + 1, ANIM_OPEN, ANIM_CLOSE, NUM_ANIMS };


Boss::Boss() {
    state = FOLLOW;
    attack = ATTACK_SHOT;
    lives = BOSS_LIVES;
    stateTime = 0;
    attackCooldown = FIRST_ATTACK_TIME;
    attackCycle = 0;
    flamesAlive = 0;
    roared = false;
    blastTimer = 0;
    floatTime = 0;
    posBoss = glm::vec2(0.f);
    target = glm::vec2(0.f);
    chargeDirection = glm::vec2(0.f);
    summonPending = false;
    blastPending = false;
    blastCenter = glm::vec2(0.f);
    sprite = NULL;
    map = NULL;
}

Boss::~Boss() {
    if (sprite != NULL)
        delete sprite;
}

void Boss::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) {
    spritesheet.loadFromFile("images/boss.png", TEXTURE_PIXEL_FORMAT_RGBA);
    spritesheet.setMinFilter(GL_NEAREST);
    spritesheet.setMagFilter(GL_NEAREST);
    initDeath("images/boss_death.png", BOSS_LIVES + 1, shaderProgram, BOSS_SIZE); // One row per lamps lit

    float step = 1.0f / SHEET_CELLS;
    sprite = Sprite::createSprite(glm::ivec2(BOSS_SIZE, BOSS_SIZE), glm::vec2(step, step), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(NUM_ANIMS);

    for (int i = 0; i <= BOSS_LIVES; i++) {
        sprite->setAnimationSpeed(ANIM_LAMPS_FIRST + i, 8);
        sprite->addKeyframe(ANIM_LAMPS_FIRST + i, glm::vec2(float(i) * step, ROW_LIVES * step));
    }
    sprite->setAnimationSpeed(ANIM_HURT, 8);
    sprite->addKeyframe(ANIM_HURT, glm::vec2(step, ROW_HURT * step));

    sprite->setAnimationSpeed(ANIM_OPEN, DOOR_FPS);
    sprite->setAnimationOnce(ANIM_OPEN);
    sprite->setAnimationSpeed(ANIM_CLOSE, CLOSE_FPS);
    sprite->setAnimationOnce(ANIM_CLOSE);
    for (int i = 0; i < ATTACK_FRAMES; i++) {
        sprite->addKeyframe(ANIM_OPEN, glm::vec2(float(i) * step, ROW_ATTACK * step));
        sprite->addKeyframe(ANIM_CLOSE, glm::vec2(float(ATTACK_FRAMES - 1 - i) * step, ROW_ATTACK * step));
    }

    showIdleFrame();
    tileMapDispl = tileMapPos;
    sprite->setPosition(glm::vec2(tileMapDispl) + posBoss);
}

void Boss::update(int deltaTime) {
    // A long frame (window dragged...) must not make it jump
    float dt = glm::min(deltaTime / 1000.f, 1.f / 30.f);
    glm::vec2 shake(0.f);

    stateTime += deltaTime;
    floatTime += deltaTime;
    if (!roared) {
        roared = true;
        Audio::instance().playSound(SOUND_BOSS_ROAR);
    }

    switch (state) {
    case FOLLOW:
        moveTowardsTarget(followSpeed(), dt);
        attackCooldown -= deltaTime;
        if (attackCooldown <= 0 && chefInView())
            startNextAttack();
        break;

    case WARN:
        if (stateTime >= WARN_TIME) {
            releaseAttack();
            sprite->changeAnimation(ANIM_CLOSE);
            setState(CLOSING);
        }
        break;

    case CLOSING:
        if (stateTime >= CLOSE_TIME) {
            attackCooldown = attackPause();
            showIdleFrame();
            setState(FOLLOW);
        }
        break;

    case CHARGE_WARN:
        shake = glm::vec2(float(rand() % (2 * SHAKE_SIZE + 1) - SHAKE_SIZE), 0.f);
        if (stateTime >= CHARGE_WARN_TIME) {
            glm::vec2 toChef = target - getCenter();
            float length = glm::length(toChef);
            chargeDirection = (length > 1.f) ? toChef / length : glm::vec2(0.f, 1.f);
            setState(CHARGE);
            Audio::instance().playSound(SOUND_BOSS_DASH);
        }
        break;

    case CHARGE:
        posBoss += chargeDirection * CHARGE_SPEED * dt;
        if (clampToMap() || stateTime >= CHARGE_TIME) {
            sprite->changeAnimation(ANIM_CLOSE);
            setState(CLOSING);
        }
        break;

    case HURT:
        sprite->setFlash((stateTime / HURT_BLINK_STEP) % 2 == 0);
        if (stateTime >= HURT_TIME) {
            sprite->setFlash(false);
            attackCooldown = glm::max(attackCooldown, PAUSE_AFTER_HIT);
            showIdleFrame();
            setState(FOLLOW);
        }
        break;

    case DEFEATED:
        shake = glm::vec2(float(rand() % (2 * SHAKE_SIZE + 1) - SHAKE_SIZE), float(rand() % (2 * SHAKE_SIZE + 1) - SHAKE_SIZE));
        sprite->setFlash((stateTime / HURT_BLINK_STEP) % 2 == 0);
        blastTimer -= deltaTime;
        if (blastTimer <= 0) {
            blastTimer = BLAST_STEP;
            blastPending = true;
            blastCenter = posBoss + glm::vec2(float(rand() % BOSS_SIZE), float(rand() % BOSS_SIZE));
        }
        if (stateTime >= DEFEAT_TIME) {
            Audio::instance().playSound(SOUND_BOSS_EXPLODE);
            kill();
        }
        break;
    }

    sprite->update(deltaTime);
    float bob = BOB_SIZE * std::sin(floatTime * BOB_SPEED);
    sprite->setPosition(glm::vec2(tileMapDispl) + posBoss + shake + glm::vec2(0.f, bob));
}

void Boss::render() {
    sprite->render();
}

void Boss::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

void Boss::setPosition(const glm::vec2& pos) {
    posBoss = pos;
    target = getCenter();
    sprite->setPosition(glm::vec2(tileMapDispl) + posBoss);
}

glm::vec2 Boss::getPosition() const {
    return posBoss;
}

glm::vec2 Boss::getDoor() const {
    return posBoss + glm::vec2(DOOR_X, DOOR_Y);
}

glm::vec2 Boss::getCenter() const {
    return posBoss + glm::vec2(BOSS_SIZE / 2.f);
}

void Boss::hitByFire() {
    if (state == HURT || state == DEFEATED)
        return;

    lives--;
    sprite->setFlash(false);
    if (lives > 0) {
        sprite->changeAnimation(ANIM_HURT);
        setState(HURT);
        Audio::instance().playSound(SOUND_BOSS_HIT);
        // With 3 and with 2 lives it starts a new phase: faster and with more attacks
        if (lives == 3 || lives == 2)
            Audio::instance().playSound(SOUND_BOSS_PHASE);
    }
    else {
        showIdleFrame();
        setState(DEFEATED);
        Audio::instance().playSound(SOUND_BOSS_ROAR);
    }
}

// All its body, so the fire does not have to reach its middle
HitBox Boss::fireBox() const {
    return { posBoss + glm::vec2(2.f, 0.f), glm::vec2(38.f, 36.f) };
}

// Smaller: jumping over it and passing near it must look safe
HitBox Boss::contactBox() const {
    return { posBoss + glm::vec2(6.f, 4.f), glm::vec2(30.f, 28.f) };
}

std::vector<BossShot> Boss::takeShots() {
    std::vector<BossShot> taken;
    taken.swap(shots);
    return taken;
}

bool Boss::takeSummon() {
    bool pending = summonPending;
    summonPending = false;
    return pending;
}

bool Boss::takeBlast(glm::vec2& center) {
    bool pending = blastPending;
    blastPending = false;
    center = blastCenter;
    return pending;
}

int Boss::deathRow() const {
    // boss_death.png has one row per column of ROW_LIVES
    return BOSS_LIVES - lives;
}

void Boss::setState(State newState) {
    state = newState;
    stateTime = 0;
}

void Boss::moveTowardsTarget(float speed, float dt) {
    glm::vec2 toTarget = target - getCenter();
    float length = glm::length(toTarget);
    if (length > 1.f)
        posBoss += toTarget / length * glm::min(speed * dt, length);
    clampToMap();
}

bool Boss::clampToMap() {
    int tileSize = map->getTileSize();
    glm::ivec2 mapSize = map->getMapSize() * tileSize;
    glm::vec2 minPos = glm::vec2(float(tileSize));
    glm::vec2 maxPos = glm::vec2(mapSize - tileSize - BOSS_SIZE);
    glm::vec2 clamped = glm::clamp(posBoss, minPos, maxPos);
    bool changed = (clamped != posBoss);
    posBoss = clamped;
    return changed;
}

bool Boss::chefInView() const {
    glm::vec2 toChef = target - getCenter();
    return std::fabs(toChef.x) <= VIEW_RANGE && std::fabs(toChef.y) <= VIEW_RANGE;
}

// With fewer lives it has more attacks
void Boss::startNextAttack() {
    static const Attack ATTACKS_ANGRY[] = { ATTACK_FAN, ATTACK_SUMMON };
    static const Attack ATTACKS_FURIOUS[] = { ATTACK_FAN, ATTACK_CHARGE, ATTACK_SUMMON };

    if (lives >= 4)
        attack = ATTACK_SHOT;
    else if (lives == 3)
        attack = ATTACKS_ANGRY[attackCycle % 2];
    else
        attack = ATTACKS_FURIOUS[attackCycle % 3];
    attackCycle++;

    if (attack == ATTACK_SUMMON && flamesAlive >= MAX_FLAMES)
        attack = ATTACK_FAN;

    sprite->changeAnimation(ANIM_OPEN);
    if (attack == ATTACK_CHARGE) {
        setState(CHARGE_WARN);
        Audio::instance().playSound(SOUND_BOSS_CHARGE);
    }
    else
        setState(WARN);
}

void Boss::releaseAttack() {
    switch (attack) {
    case ATTACK_SHOT:
        shoot(0.f);
        break;
    case ATTACK_FAN:
        shoot(-FAN_ANGLE);
        shoot(0.f);
        shoot(FAN_ANGLE);
        break;
    case ATTACK_SUMMON:
        summonPending = true;
        Audio::instance().playSound(SOUND_BOSS_SUMMON);
        return;
    case ATTACK_CHARGE:
        return;
    }
    Audio::instance().playSound(SOUND_BOSS_SHOOT);
}

// Angle in radians from the direction of the chef
void Boss::shoot(float angle) {
    glm::vec2 origin = getDoor();
    glm::vec2 toChef = target - origin;
    float aim = std::atan2(toChef.y, toChef.x) + angle;
    shots.push_back({ origin, glm::vec2(std::cos(aim), std::sin(aim)) });
}

float Boss::followSpeed() const {
    if (lives >= 4)
        return SPEED_FULL;
    return (lives == 3) ? SPEED_ANGRY : SPEED_FURIOUS;
}

int Boss::attackPause() const {
    if (lives >= 4)
        return PAUSE_FULL;
    return (lives == 3) ? PAUSE_ANGRY : PAUSE_FURIOUS;
}

// The lamps show the lives it has left
void Boss::showIdleFrame() {
    sprite->changeAnimation(ANIM_LAMPS_FIRST + BOSS_LIVES - lives);
}
