#include "Flame.h"
#include <cstdlib>

#define FLAME_SIZE 32
#define FLAME_FRAMES 4    // images/flame.png
#define FLAME_FPS 8
#define FLAME_SPEED 70.f  // px/s on each axis
#define APPEAR_TIME 700   // ms blinking and harmless when it appears
#define APPEAR_BLINK_STEP 70


Flame::Flame() {
    posFlame = glm::vec2(0.f);
    // Each one starts in its own diagonal
    velocity = glm::vec2(rand() % 2 == 0 ? FLAME_SPEED : -FLAME_SPEED, rand() % 2 == 0 ? FLAME_SPEED : -FLAME_SPEED);
    appearTime = APPEAR_TIME;
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

void Flame::update(int deltaTime) {
    sprite->update(deltaTime);

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
