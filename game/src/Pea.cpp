#include "Pea.h"
#include <cmath>

#define PEA_SIZE 32
#define GRAVITY 1400.f        // px/s^2
#define MAX_FALL_SPEED 600.f  // px/s, so it never goes through the floor in one frame

#define IDLE_TIME 500   // ms on the ground between jumps (the moment to attack it)
#define CROUCH_TIME 240 // ms crouching before a jump (the warning for the player)
#define LAND_TIME 150   // ms squashed after landing

#define MAX_DROP 64       // Pixels it accepts to land below where it jumps (2 tiles)
#define SIM_STEP (1.f / 120.f)
#define SIM_MAX_TIME 2.f  // s; a jump that has not landed by then falls into a pit

// Frames of each direction in images/pea-anim.png (columns 0-6 right, 8-14 left)
#define FRAMES_PER_DIR 7
#define LEFT_FIRST_COLUMN 8

// Tiles of 32 px: the short jump advances 2 tiles and the long one 3 tiles and
// goes up 2.5, enough to climb a wall of 2 blocks
static const float SHORT_DISTANCE = 64.f, SHORT_HEIGHT = 48.f;
static const float LONG_DISTANCE = 96.f, LONG_HEIGHT = 80.f;

// Rhythm of the jumps: short, short, long, and again
static const bool JUMP_CYCLE[] = { false, false, true }; // true = long jump
static const int JUMP_CYCLE_SIZE = sizeof(JUMP_CYCLE) / sizeof(JUMP_CYCLE[0]);


Pea::Pea() {
    sprite = NULL;
    map = NULL;
    direction = -1;
    cycleIndex = 0;
    state = IDLE;
    stateTime = 0;
    velocity = glm::vec2(0.f);
    nextJump = { SHORT_DISTANCE, SHORT_HEIGHT };
}

Pea::~Pea() {
    if (sprite != NULL)
        delete sprite;
}

void Pea::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) {
    spritesheet.loadFromFile("images/pea-anim.png", TEXTURE_PIXEL_FORMAT_RGBA);
    initDeath("images/pea_death.png", 2 * FRAMES_PER_DIR, shaderProgram);

    float stepX = 1.0f / 16.0f; // 16 columns of 32 px
    float stepY = 1.0f / 4.0f;  // 4 rows, only the first one is used

    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);

    // One animation per frame and direction: the frame depends on the state and the
    // speed of the jump, not on time, so updateFrame() picks it every update
    sprite->setNumberAnimations(2 * FRAMES_PER_DIR);
    for (int i = 0; i < FRAMES_PER_DIR; i++) {
        sprite->setAnimationSpeed(i, 8);
        sprite->addKeyframe(i, glm::vec2(float(i) * stepX, 0.f));
        sprite->setAnimationSpeed(FRAMES_PER_DIR + i, 8);
        sprite->addKeyframe(FRAMES_PER_DIR + i, glm::vec2(float(LEFT_FIRST_COLUMN + i) * stepX, 0.f));
    }

    tileMapDispl = tileMapPos;
    updateFrame();
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPea.x), float(tileMapDispl.y + posPea.y)));
}

void Pea::update(int deltaTime) {
    stateTime += deltaTime;

    switch (state) {
    case IDLE:
        // If the floor disappears (a box blown up) it falls
        if (!isGrounded()) {
            velocity = glm::vec2(0.f);
            setState(AIRBORNE);
        }
        else if (stateTime >= IDLE_TIME)
            chooseJump();
        break;

    case CROUCH:
        if (stateTime >= CROUCH_TIME) {
            velocity = launchVelocity(direction, nextJump);
            cycleIndex = (cycleIndex + 1) % JUMP_CYCLE_SIZE;
            setState(AIRBORNE);
        }
        break;

    case AIRBORNE:
        // A long frame (window dragged...) must not make it go through a wall
        if (step(posPea, velocity, glm::min(deltaTime / 1000.f, 1.f / 30.f)))
            setState(LAND);
        break;

    case LAND:
        if (stateTime >= LAND_TIME)
            setState(IDLE);
        break;
    }

    updateFrame();
    sprite->update(deltaTime);
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPea.x), float(tileMapDispl.y + posPea.y)));
}

// Picks the jump of the rhythm, or the other one if that one is not safe.
// With no safe jump ahead it turns around and waits again, so it never turns in the air
void Pea::chooseJump() {
    bool isLong = JUMP_CYCLE[cycleIndex];
    Jump preferred = isLong ? Jump{ LONG_DISTANCE, LONG_HEIGHT } : Jump{ SHORT_DISTANCE, SHORT_HEIGHT };
    Jump other = isLong ? Jump{ SHORT_DISTANCE, SHORT_HEIGHT } : Jump{ LONG_DISTANCE, LONG_HEIGHT };

    if (canJump(direction, preferred)) {
        nextJump = preferred;
        setState(CROUCH);
    }
    else if (canJump(direction, other)) {
        nextJump = other;
        setState(CROUCH);
    }
    else if (canJump(-direction, preferred) || canJump(-direction, other)) {
        direction = -direction;
        setState(IDLE);
    }
    else
        setState(IDLE); // Trapped: try again later (a box may be destroyed)
}

// Simulates the jump with the same physics: it must land on the floor, not too low,
// and advance at least half of its distance (if not, a wall stops it)
bool Pea::canJump(int dir, const Jump& jump) const {
    glm::vec2 pos = posPea;
    glm::vec2 vel = launchVelocity(dir, jump);

    for (float t = 0.f; t < SIM_MAX_TIME; t += SIM_STEP) {
        if (step(pos, vel, SIM_STEP))
            return std::fabs(pos.x - posPea.x) >= jump.distance / 2.f && pos.y - posPea.y <= MAX_DROP;
    }
    return false;
}

// Speed to go up 'height' and advance 'distance' landing at the same height
glm::vec2 Pea::launchVelocity(int dir, const Jump& jump) const {
    float flightTime = 2.f * std::sqrt(2.f * jump.height / GRAVITY);
    return glm::vec2(dir * jump.distance / flightTime, -std::sqrt(2.f * GRAVITY * jump.height));
}

// Moves the pea in the air for dt seconds. Walls stop it horizontally and the
// ceiling makes it start falling
bool Pea::step(glm::vec2& pos, glm::vec2& vel, float dt) const {
    glm::ivec2 size(PEA_SIZE, PEA_SIZE);
    vel.y = glm::min(vel.y + GRAVITY * dt, MAX_FALL_SPEED);

    glm::ivec2 nextX(int(pos.x + vel.x * dt), int(pos.y));
    if ((vel.x < 0.f && map->collisionMoveLeft(nextX, size)) || (vel.x > 0.f && map->collisionMoveRight(nextX, size)))
        vel.x = 0.f;
    else
        pos.x += vel.x * dt;

    pos.y += vel.y * dt;
    int correctedY = int(pos.y);
    if (vel.y < 0.f) {
        if (map->collisionMoveUp(glm::ivec2(pos), size, &correctedY)) {
            pos.y = float(correctedY);
            vel.y = 0.f;
        }
    }
    else if (map->collisionMoveDown(glm::ivec2(pos), size, &correctedY)) {
        pos.y = float(correctedY);
        return true;
    }
    return false;
}

bool Pea::isGrounded() const {
    glm::ivec2 below(int(posPea.x), int(posPea.y) + 1);
    int correctedY = below.y;
    return map->collisionMoveDown(below, glm::ivec2(PEA_SIZE, PEA_SIZE), &correctedY);
}

void Pea::setState(State newState) {
    state = newState;
    stateTime = 0;
}

// Crouch 1-2-3, take off 4, go up 5, top of the jump 6, and squash 3-2-1 when landing
void Pea::updateFrame() {
    int newFrame = 0;
    switch (state) {
    case IDLE:
        newFrame = 0;
        break;
    case CROUCH:
        newFrame = 1 + glm::min(2, stateTime * 3 / CROUCH_TIME);
        break;
    case AIRBORNE:
        if (velocity.y < -250.f) newFrame = 4;
        else if (std::fabs(velocity.y) < 150.f) newFrame = 6;
        else newFrame = 5;
        break;
    case LAND:
        newFrame = 3 - glm::min(2, stateTime * 3 / LAND_TIME);
        break;
    }

    int anim = (direction == 1 ? 0 : FRAMES_PER_DIR) + newFrame;
    if (anim != sprite->animation())
        sprite->changeAnimation(anim);
}

void Pea::render() {
    sprite->render();
}

void Pea::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

void Pea::setPosition(const glm::vec2& pos) {
    posPea = pos;
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPea.x), float(tileMapDispl.y + posPea.y)));
}

glm::vec2 Pea::getPosition() const {
    return posPea;
}

int Pea::deathRow() const {
    // pea_death.png has one row per animation: 0-6 right, 7-13 left
    return sprite->animation();
}
