#include "Enemy.h"
#include "Audio.h"

#define DEATH_FRAMES 6       // Columns of <enemy>_death.png (tools/make_disintegrate.py)
#define DEATH_FPS 18         // 333 ms coming apart, like the boxes
#define DEATH_FLASH_TIME 400 // ms frozen and blinking white before coming apart
#define DEATH_FLASH_STEP 50  // ms white / ms normal while it blinks

enum DeathAnims { DEATH_HIT, DEATH_BREAK };


Enemy::Enemy() {
    program = NULL;
    deathRows = 1;
    deathSprite = NULL;
    deathTime = 0;
}

Enemy::~Enemy() {
    if (deathSprite != NULL)
        delete deathSprite;
}

void Enemy::initDeath(const std::string& file, int rows, ShaderProgram& shaderProgram) {
    deathSpritesheet.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
    deathSpritesheet.setMinFilter(GL_NEAREST);
    deathSpritesheet.setMagFilter(GL_NEAREST);
    deathRows = rows;
    program = &shaderProgram;
}

void Enemy::kill() {
    if (isDying())
        return;
    Audio::instance().playSound(SOUND_ENEMY_DEATH);

    float stepX = 1.0f / DEATH_FRAMES, stepY = 1.0f / deathRows;
    float row = float(deathRow()) * stepY;

    deathSprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(stepX, stepY), &deathSpritesheet, program);
    deathSprite->setNumberAnimations(2);
    // The first column is the frame as it was, used while it blinks
    deathSprite->setAnimationSpeed(DEATH_HIT, DEATH_FPS);
    deathSprite->addKeyframe(DEATH_HIT, glm::vec2(0.f, row));
    deathSprite->setAnimationSpeed(DEATH_BREAK, DEATH_FPS);
    deathSprite->setAnimationOnce(DEATH_BREAK);
    for (int f = 1; f < DEATH_FRAMES; f++)
        deathSprite->addKeyframe(DEATH_BREAK, glm::vec2(float(f) * stepX, row));
    deathSprite->changeAnimation(DEATH_HIT);
    deathSprite->setPosition(getPosition());
    deathTime = 0;
}

bool Enemy::isDead() const {
    return isDying() && deathSprite->animation() == DEATH_BREAK && deathSprite->animationFinished();
}

void Enemy::updateDeath(int deltaTime) {
    deathTime += deltaTime;
    if (deathTime < DEATH_FLASH_TIME)
        deathSprite->setFlash((deathTime / DEATH_FLASH_STEP) % 2 == 0);
    else if (deathSprite->animation() == DEATH_HIT) {
        deathSprite->setFlash(false);
        deathSprite->changeAnimation(DEATH_BREAK);
    }
    deathSprite->update(deltaTime);
}

void Enemy::renderDeath() {
    deathSprite->render();
}
