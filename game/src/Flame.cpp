#include "Flame.h"
#include <cstdlib>

#define FLAME_SIZE 32
#define FLAME_FRAMES 4    // images/flame.png
#define FLAME_FPS 8
#define FLAME_SPEED 70.f  // px/s on each axis
#define APPEAR_TIME 700   // ms blinking and harmless when it appears
#define APPEAR_BLINK_STEP 70
#define LAUNCH_SPEED 120.f   // px/s on each axis while it flies out of the boss
#define MIN_LAUNCH_TIME 350  // ms: enough to leave the body of the boss


Flame::Flame() {
    posFlame = glm::vec2(0.f);
    // Each one starts in its own diagonal
    velocity = glm::vec2(rand() % 2 == 0 ? FLAME_SPEED : -FLAME_SPEED, rand() % 2 == 0 ? FLAME_SPEED : -FLAME_SPEED);
    appearTime = APPEAR_TIME;
    launched = false;
    launchTime = 0;
    sprite = NULL;
    map = NULL;
}

Flame::~Flame() {
    if (sprite != NULL)
        delete sprite;
}

void Flame::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) {
    spritesheet.loadFromFile("images/flame.png", TEXTURE_PIXEL_FORMAT_RGBA);
    spritesheet.setMinFilter(GL_NEAREST);
    spritesheet.setMagFilter(GL_NEAREST);
    initDeath("images/flame_death.png", FLAME_FRAMES, shaderProgram); // One row per frame of flame.png

    float stepX = 1.0f / FLAME_FRAMES;
    sprite = Sprite::createSprite(glm::ivec2(FLAME_SIZE, FLAME_SIZE), glm::vec2(stepX, 1.f), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, FLAME_FPS);
    for (int i = 0; i < FLAME_FRAMES; i++)
        sprite->addKeyframe(0, glm::vec2(float(i) * stepX, 0.f));
    sprite->changeAnimation(0);

    tileMapDispl = tileMapPos;
    sprite->setPosition(glm::vec2(tileMapDispl) + posFlame);
}

void Flame::launch(int direction) {
    launched = true;
    launchTime = 0;
    appearTime = 0;
    velocity = glm::vec2(direction * LAUNCH_SPEED, LAUNCH_SPEED);
}

void Flame::update(int deltaTime) {
    sprite->update(deltaTime);

    if (launched) {
        updateLaunch(deltaTime);
        return;
    }
    if (appearTime > 0) {
        appearTime -= deltaTime;
        sprite->setFlash(appearTime > 0 && (appearTime / APPEAR_BLINK_STEP) % 2 == 0);
        return;
    }

    // A long frame (window dragged...) must not make it go through a wall
    float dt = glm::min(deltaTime / 1000.f, 1.f / 30.f);

    float nextX = posFlame.x + velocity.x * dt;
    if (blockedX(nextX))
        velocity.x = -velocity.x;
    else
        posFlame.x = nextX;

    float nextY = posFlame.y + velocity.y * dt;
    if (blockedY(nextY))
        velocity.y = -velocity.y;
    else
        posFlame.y = nextY;

    sprite->setPosition(glm::vec2(tileMapDispl) + posFlame);
}

// The boss floats over the walls, so the flame goes through them until it is out in a free place.
// The walls around the level keep it in: there it bounces
void Flame::updateLaunch(int deltaTime) {
    float dt = glm::min(deltaTime / 1000.f, 1.f / 30.f);
    int tileSize = map->getTileSize();
    glm::vec2 minPos = glm::vec2(float(tileSize));
    glm::vec2 maxPos = glm::vec2(map->getMapSize() * tileSize - tileSize - FLAME_SIZE);

    launchTime += deltaTime;
    posFlame += velocity * dt;
    for (int axis = 0; axis < 2; axis++) {
        if (posFlame[axis] < minPos[axis] || posFlame[axis] > maxPos[axis]) {
            posFlame[axis] = glm::clamp(posFlame[axis], minPos[axis], maxPos[axis]);
            velocity[axis] = -velocity[axis];
        }
    }

    sprite->setFlash((launchTime / APPEAR_BLINK_STEP) % 2 == 0);
    if (launchTime >= MIN_LAUNCH_TIME && !insideWall(posFlame)) {
        launched = false;
        sprite->setFlash(false);
        velocity = glm::sign(velocity) * FLAME_SPEED;
    }
    sprite->setPosition(glm::vec2(tileMapDispl) + posFlame);
}

bool Flame::insideWall(const glm::vec2& pos) const {
    int tileSize = map->getTileSize();
    glm::ivec2 first = glm::ivec2(pos) / tileSize;
    glm::ivec2 last = (glm::ivec2(pos) + FLAME_SIZE - 1) / tileSize;
    for (int y = first.y; y <= last.y; y++) {
        for (int x = first.x; x <= last.x; x++) {
            if (!map->isFree(map->getTile(x, y)))
                return true;
        }
    }
    return false;
}

bool Flame::blockedX(float x) const {
    glm::ivec2 pos(int(x), int(posFlame.y));
    glm::ivec2 size(FLAME_SIZE, FLAME_SIZE);
    if (velocity.x < 0.f)
        return map->collisionMoveLeft(pos, size, 0, 0);
    return map->collisionMoveRight(pos, size, 0, 0);
}

bool Flame::blockedY(float y) const {
    glm::ivec2 pos(int(posFlame.x), int(y));
    glm::ivec2 size(FLAME_SIZE, FLAME_SIZE);
    int correctedY = pos.y;
    if (velocity.y < 0.f)
        return map->collisionMoveUp(pos, size, &correctedY, 0, 0);
    return map->collisionMoveDown(pos, size, &correctedY, 0);
}

void Flame::render() {
    sprite->render();
}

void Flame::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

void Flame::setPosition(const glm::vec2& pos) {
    posFlame = pos;
    sprite->setPosition(glm::vec2(tileMapDispl) + posFlame);
}

glm::vec2 Flame::getPosition() const {
    return posFlame;
}

int Flame::deathRow() const {
    return sprite->keyframe();
}
