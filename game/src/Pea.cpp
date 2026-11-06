#include "Pea.h"
#include <cmath>

#define PEA_SIZE 32
#define GRAVITY 1400.f        // px/s^2
#define MAX_FALL_SPEED 600.f  // px/s, so it never goes through the floor in one frame

#define IDLE_TIME 750   // ms on the ground between jumps (the moment to attack it)
#define CROUCH_TIME 240 // ms crouching before a jump (the warning for the player)
#define LAND_TIME 150   // ms squashed after landing
#define CRAWL_TIME 530  // ms it crawls before looking for a jump again (1 tile)
#define CRAWL_SPEED 60.f // px/s
#define SLIDE_SPEED 90.f // px/s it slides off the edge of a tile

#define MAX_DROP 64       // Pixels it prefers not to go below when it jumps (2 tiles)
#define NO_DROP_LIMIT 100000.f // For a pea with no other way out: better to drop than to stay
#define SIM_STEP (1.f / 120.f)
#define SIM_MAX_TIME 2.f  // s; a jump that has not landed by then falls into a pit

// Frames of each direction in images/pea-anim.png (columns 0-6 right, 8-14 left)
#define FRAMES_PER_DIR 7
#define LEFT_FIRST_COLUMN 8

// Tiles of 32 px: the short jump advances 2 tiles and the long one 3 tiles and
// goes up 2.5, enough to climb a wall of 2 blocks. The hop advances 1 tile, for the
// places where the others do not fit (stairs, narrow platforms)
static const float SHORT_DISTANCE = 64.f, SHORT_HEIGHT = 48.f;
static const float LONG_DISTANCE = 96.f, LONG_HEIGHT = 80.f;
static const float HOP_DISTANCE = 32.f, HOP_HEIGHT = 40.f;

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
    slideTargetX = 0.f;
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
        else if (isHanging(posPea))
            startSlide();
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
        if (step(posPea, velocity, glm::min(deltaTime / 1000.f, 1.f / 30.f))) {
            if (isHanging(posPea))
                startSlide();
            else
                setState(LAND);
        }
        break;

    case LAND:
        if (stateTime >= LAND_TIME)
            setState(IDLE);
        break;

    case CRAWL:
        if (stateTime >= CRAWL_TIME || !canCrawl(direction))
            setState(IDLE);
        else
            posPea.x += direction * CRAWL_SPEED * glm::min(deltaTime / 1000.f, 1.f / 30.f);
        break;

    case SLIDE:
        // It only crosses its own columns, so no wall can stop it. At slideTargetX
        // nothing holds it any more and it falls
        if (!isGrounded()) {
            velocity = glm::vec2(0.f);
            setState(AIRBORNE);
        }
        else {
            float move = SLIDE_SPEED * glm::min(deltaTime / 1000.f, 1.f / 30.f);
            posPea.x = glm::clamp(slideTargetX, posPea.x - move, posPea.x + move);
        }
        break;
    }

    updateFrame();
    sprite->update(deltaTime);
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPea.x), float(tileMapDispl.y + posPea.y)));
}

// With no safe jump ahead it turns around and waits again, so it never turns in the air.
// With none on either side it accepts any drop. Under a low ceiling it cannot jump, so it
// crawls out. If it is walled in it jumps on the spot until a box is destroyed
void Pea::chooseJump() {
    if (findJump(direction, MAX_DROP))
        setState(CROUCH);
    else if (findJump(-direction, MAX_DROP)) {
        direction = -direction;
        setState(IDLE);
    }
    else if (findJump(direction, NO_DROP_LIMIT))
        setState(CROUCH);
    else if (findJump(-direction, NO_DROP_LIMIT)) {
        direction = -direction;
        setState(CROUCH);
    }
    else if (canCrawl(direction))
        setState(CRAWL);
    else if (canCrawl(-direction)) {
        direction = -direction;
        setState(CRAWL);
    }
    else {
        nextJump = { 0.f, SHORT_HEIGHT };
        setState(CROUCH);
    }
}

// Looks for a jump in that direction: the one of the rhythm, the other one or the hop
bool Pea::findJump(int dir, float maxDrop) {
    bool isLong = JUMP_CYCLE[cycleIndex];
    Jump shortJump = { SHORT_DISTANCE, SHORT_HEIGHT }, longJump = { LONG_DISTANCE, LONG_HEIGHT };
    Jump options[] = { isLong ? longJump : shortJump, isLong ? shortJump : longJump, { HOP_DISTANCE, HOP_HEIGHT } };

    for (const Jump& jump : options) {
        if (canJump(dir, jump, maxDrop)) {
            nextJump = jump;
            return true;
        }
    }
    return false;
}

// Simulates the jump with the same physics: it must land on the floor, not too low,
// and advance at least half of its distance (if not, a wall stops it)
bool Pea::canJump(int dir, const Jump& jump, float maxDrop) const {
    glm::vec2 pos = posPea;
    glm::vec2 vel = launchVelocity(dir, jump);

    for (float t = 0.f; t < SIM_MAX_TIME; t += SIM_STEP) {
        if (step(pos, vel, SIM_STEP)) {
            // Landing with half of the body out means sliding and falling: the jump goes on
            if (isHanging(pos)) {
                pos.x = columnBelowCenter(pos);
                vel = glm::vec2(0.f);
            }
            else
                return std::fabs(pos.x - posPea.x) >= jump.distance / 2.f && pos.y - posPea.y <= maxDrop;
        }
    }
    return false;
}

// Speed to go up 'height' and advance 'distance' landing at the same height
glm::vec2 Pea::launchVelocity(int dir, const Jump& jump) const {
    float flightTime = 2.f * std::sqrt(2.f * jump.height / GRAVITY);
    return glm::vec2(dir * jump.distance / flightTime, -std::sqrt(2.f * GRAVITY * jump.height));
}

// Moves the pea in the air for dt seconds. The ceiling makes it start falling. A wall
// only holds it while it is in front: vel.x stays, so it gets over a step when it is high enough
bool Pea::step(glm::vec2& pos, glm::vec2& vel, float dt) const {
    glm::ivec2 size(PEA_SIZE, PEA_SIZE);
    vel.y = glm::min(vel.y + GRAVITY * dt, MAX_FALL_SPEED);

    // Only the column it enters counts: a bomb that becomes solid on top of it does not trap it
    int tileSize = map->getTileSize();
    glm::ivec2 nextX(int(pos.x + vel.x * dt), int(pos.y));
    int edge = vel.x < 0.f ? int(pos.x) / tileSize : (int(pos.x) + PEA_SIZE - 1) / tileSize;
    int nextEdge = vel.x < 0.f ? nextX.x / tileSize : (nextX.x + PEA_SIZE - 1) / tileSize;
    bool blocked = nextEdge != edge &&
        ((vel.x < 0.f && map->collisionMoveLeft(nextX, size)) || (vel.x > 0.f && map->collisionMoveRight(nextX, size)));
    if (!blocked)
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

// Standing on a tile with the center of the body out of it: there is no floor below the center
bool Pea::isHanging(const glm::vec2& pos) const {
    int tileSize = map->getTileSize();
    int feetRow = (int(pos.y) + PEA_SIZE) / tileSize;
    return map->isFree(map->getTile((int(pos.x) + PEA_SIZE / 2) / tileSize, feetRow));
}

// X where the pea is exactly over the column of its center
float Pea::columnBelowCenter(const glm::vec2& pos) const {
    int tileSize = map->getTileSize();
    return float((int(pos.x) + PEA_SIZE / 2) / tileSize * tileSize);
}

void Pea::startSlide() {
    slideTargetX = columnBelowCenter(posPea);
    setState(SLIDE);
}

// There is no wall ahead (if the floor ends it falls when it goes back to IDLE)
bool Pea::canCrawl(int dir) const {
    glm::ivec2 size(PEA_SIZE, PEA_SIZE);
    glm::ivec2 next(int(posPea.x) + dir, int(posPea.y));
    return dir < 0 ? !map->collisionMoveLeft(next, size) : !map->collisionMoveRight(next, size);
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

// Crouch 1-2-3, take off 4, go up 5, top of the jump 6, and squash 3-2-1 when landing.
// It crawls flat (3) and slides a bit squashed (1)
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
    case CRAWL:
        newFrame = 3;
        break;
    case SLIDE:
        newFrame = 1;
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
