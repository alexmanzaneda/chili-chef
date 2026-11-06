#include "Toaster.h"
#include <iostream>
#include <cmath>

#define FALL_STEP 4
#define MARGIN_X 3 // Píxeles transparentes a cada lado del sprite: choca con su tamaño visible

enum ToasterAnims {
    MOVE_LEFT, MOVE_RIGHT
};

Toaster::Toaster() {
    sprite = NULL;
    map = NULL;
    direction = -1; // Comienza moviéndose hacia la izquierda por defecto
    speed = 0.51;      // Velocidad reducida para el primer enemigo
}

Toaster::~Toaster() {
    if (sprite != NULL)
        delete sprite;
}

void Toaster::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) {
    // Asegúrate de cambiar la ruta a la imagen real de tu enemigo
    spritesheet.loadFromFile("images/toaster.png", TEXTURE_PIXEL_FORMAT_RGBA);
    initDeath("images/toaster_death.png", 12, shaderProgram); // One row per frame of toaster.png

    float stepX = 1.0f / 12.0f; // 12 sprites de ancho
    float stepY = 1.0f;        // 1 sola fila

    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(2);

    // 1. ANDAR IZQUIERDA (Fila 1: Y = 0. Primeros 8 sprites)
    sprite->setAnimationSpeed(MOVE_LEFT, 8);
    for (int i = 0; i < 6; i++) {
        sprite->addKeyframe(MOVE_LEFT, glm::vec2(float(i) * stepX, 0.f));
    }

    // 2. ANDAR DERECHA (Fila 1: Y = 0. Siguientes 6 sprites)
    sprite->setAnimationSpeed(MOVE_RIGHT, 8);
    for (int i = 6; i < 12; i++) {
        sprite->addKeyframe(MOVE_RIGHT, glm::vec2(float(i) * stepX, 0.f));
    }

  
    sprite->changeAnimation(MOVE_LEFT);
    tileMapDispl = tileMapPos;
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posToaster.x), float(tileMapDispl.y + posToaster.y)));
}

void Toaster::update(int deltaTime) {
    sprite->update(deltaTime);

    // 1. Gravedad (Comprobamos si está tocando el suelo primero)
    posToaster.y += FALL_STEP;
    int correctedY = posToaster.y;
    bool isGrounded = map->collisionMoveDown(posToaster, glm::ivec2(32, 32), &correctedY);

    if (isGrounded) {
        // Está pisando suelo firme, corregimos su posición Y
        posToaster.y = correctedY;

        // 2. IA de Patrullaje: avanza si puede y si no se da la vuelta. En un hueco de 1x1
        // va de punta a punta. Si no puede ir a ningún lado se queda (no gira en cada frame)
        if (canWalk(direction, speed))
            posToaster.x += direction * speed;
        else if (canWalk(-direction, speed))
            turn();
    }

    // 3. Sincronizar coordenadas con la pantalla
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posToaster.x), float(tileMapDispl.y + posToaster.y)));
}

// Puede avanzar 'distance' píxeles hacia 'dir': no hay pared y el sensor adelantado
// encuentra suelo. Usa la posición real (con decimales), así nunca se mete en la pared
bool Toaster::canWalk(int dir, float distance) const {
    glm::ivec2 size(32, 32);
    int tileSize = map->getTileSize();
    int x = int(std::floor(posToaster.x));
    glm::ivec2 nextPos(int(std::floor(posToaster.x + dir * distance)), int(posToaster.y));

    // Solo cuenta la columna en la que entra: si le ponen una bomba encima, sale de ella
    int edge = (dir == -1) ? (x + MARGIN_X) / tileSize : (x + 31 - MARGIN_X) / tileSize;
    int nextEdge = (dir == -1) ? (nextPos.x + MARGIN_X) / tileSize : (nextPos.x + 31 - MARGIN_X) / tileSize;
    bool hitWall = nextEdge != edge &&
        ((dir == -1) ? map->collisionMoveLeft(nextPos, size, MARGIN_X) : map->collisionMoveRight(nextPos, size, MARGIN_X));

    // Empujamos el sensor 24 píxeles hacia adelante para que detecte el borde antes
    glm::ivec2 sensorPos(nextPos.x + dir * 24, nextPos.y + FALL_STEP);
    int tempY = sensorPos.y;
    bool hasGround = map->collisionMoveDown(sensorPos, size, &tempY);

    return !hitWall && hasGround;
}

void Toaster::turn() {
    direction = -direction;
    sprite->changeAnimation(direction == -1 ? MOVE_LEFT : MOVE_RIGHT);
}

void Toaster::render() {
    sprite->render();
}

void Toaster::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

void Toaster::setPosition(const glm::vec2& pos) {
    posToaster = pos;
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posToaster.x), float(tileMapDispl.y + posToaster.y)));
}

glm::vec2 Toaster::getPosition() const {
    return posToaster;
}

int Toaster::deathRow() const {
    // The rows follow the columns of toaster.png: 0-5 walking left, 6-11 walking right
    return (sprite->animation() == MOVE_LEFT ? 0 : 6) + sprite->keyframe();
}