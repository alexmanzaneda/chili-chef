#include "Toaster.h"
#include <iostream>

#define FALL_STEP 4

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

        // 2. IA de Patrullaje: Simulamos dónde estaría en el siguiente fotograma
        glm::ivec2 nextPos = posToaster;
        nextPos.x += (direction == -1) ? -speed : speed;

        // A) ¿Hay una pared en ese siguiente paso?
        bool hitWall = false;
        if (direction == -1)
            hitWall = map->collisionMoveLeft(nextPos, glm::ivec2(32, 32));
        else
            hitWall = map->collisionMoveRight(nextPos, glm::ivec2(32, 32));

        // B) ¿Hay suelo en ese siguiente paso? (Sensor adelantado)
        glm::ivec2 sensorPos = nextPos;
        // Empujamos el sensor 14 píxeles hacia adelante para que detecte el borde antes
        sensorPos.x += (direction == -1) ? -24 : 24;
        sensorPos.y += FALL_STEP;

        int tempY = sensorPos.y;
        bool hasGround = map->collisionMoveDown(sensorPos, glm::ivec2(32, 32), &tempY);

        // 3. Tomar decisión
        if (hitWall || !hasGround) {
            // Peligro: Hay un muro o el sensor detecta precipicio. Nos damos la vuelta.
            if (direction == -1) {
                direction = 1;
                sprite->changeAnimation(MOVE_RIGHT);
            }
            else {
                direction = -1;
                sprite->changeAnimation(MOVE_LEFT);
            }
        }
        else {
            // Todo despejado: Aplicamos el movimiento real
            posToaster.x += (direction == -1) ? -speed : speed;
        }
    }

    // 4. Sincronizar coordenadas con la pantalla
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posToaster.x), float(tileMapDispl.y + posToaster.y)));
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