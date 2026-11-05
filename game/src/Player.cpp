#include <algorithm>
#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"
#include "Audio.h"


// In pixels and frames, like his walk (2 px/frame). At 60 FPS: 1200 px/s^2, 492 px/s and 360 px/s
#define GRAVITY (1200.f / 3600.f)
#define JUMP_SPEED 8.2f       // Goes up 3 tiles
#define MAX_FALL_SPEED 6.f    // collisionMoveDown misses the floor above 16 px in one frame
#define GROUND_PROBE 4        // Px below his feet where he still counts as standing on the floor

#define DEATH_FREEZE_TIME 500   // ms the chef stays still before the death jump
#define DEATH_TOTAL_TIME 2000   // ms until the game over screen
#define DEATH_JUMP_SPEED 400.f  // px/s upwards at the start of the death jump
#define DEATH_GRAVITY 1200.f    // px/s^2
#define DEATH_BLINK_STEP 60     // ms of each color while he blinks frozen

#define PROTECT_BLINK_STEP 80   // ms visible / ms hidden while he is protected
#define GOD_BLINK_STEP 150      // ms of each color while he is in god mode

#define BURN_FRAMES 6          // Columns of chef_death.png (tools/make_disintegrate.py)
#define BURN_FPS 12             // A bit slower than the enemies: it is the chef

#define CHEER_ROW 6             // Row of chef.png: columns 0-4 looking left, 8-12 looking right
#define CHEER_FPS 10             // About 140 ms per frame, so the gesture can be followed
#define CELEBRATE_WALK_STEP 2   // px per update walking to the center of the door

// He crouches, throws his fist to the sky and keeps it up until the next level
static const int CHEER_FRAMES[] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4 };

// The chef is almost white, so a white flash would not show: he blinks red when he is
// hit, and between hot orange and charred when he is burnt
static const glm::vec4 HURT_TINT(1.f, 0.4f, 0.35f, 1.f);
static const glm::vec4 HOT_TINT(2.f, 1.f, 0.45f, 1.f);
static const glm::vec4 CHARRED_TINT(0.45f, 0.3f, 0.25f, 1.f);
static const glm::vec4 GOD_TINT(1.f, 0.8f, 0.2f, 1.f);

enum BurnAnims { BURN_FROZEN, BURN_BREAK };


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_LEFT, JUMP_RIGHT, 
	FALL_LEFT, FALL_RIGHT, LAND_LEFT, LAND_RIGHT, CHEER_LEFT, CHEER_RIGHT
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
	bDying = false;
	bBurnt = false;
	bCelebrating = false;
	bCheerLeft = false;
	celebrateX = -1;
	deathTime = 0;
	deathStartY = 0;
	protectTime = 0;
	godBlinkTime = 0;
	burnSprite = NULL;
	program = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
	if (burnSprite != NULL)
		delete burnSprite;
}

void Player::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
	bJumping = false;
	velY = 0.f;
	spritesheet.loadFromFile("images/chef.png", TEXTURE_PIXEL_FORMAT_RGBA);
	burnSpritesheet.loadFromFile("images/chef_death.png", TEXTURE_PIXEL_FORMAT_RGBA);
	burnSpritesheet.setMinFilter(GL_NEAREST);
	burnSpritesheet.setMagFilter(GL_NEAREST);
	program = &shaderProgram;

	// Calculamos la proporción exacta de 1 frame sobre la cuadrícula de 16x16
	float step = 1.0f / 16.0f;
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(step, step), &spritesheet, &shaderProgram);

	landTimer = 0; // Inicializamos el temporizador a 0
	sprite->setNumberAnimations(12); // Ahora son 12 animaciones

	// 1. REPOSO IZQUIERDA (Fila 5: Y = 4 * step. Primeros 8 sprites)
	sprite->setAnimationSpeed(STAND_LEFT, 8);
	for (int i = 0; i < 8; i++) {
		sprite->addKeyframe(STAND_LEFT, glm::vec2(float(i) * step, 4.f * step));
	}

	// 1. REPOSO DERECHA (Fila 5: Y = 4 * step. Siguientes 8 sprites)
	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	for (int i = 8; i < 16; i++) {
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(float(i) * step, 4.f * step));
	}

	// 2. ANDAR IZQUIERDA (Fila 1: Y = 0. Primeros 8 sprites)
	sprite->setAnimationSpeed(MOVE_LEFT, 16);
	for (int i = 0; i < 8; i++) {
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(float(i) * step, 0.f));
	}

	// 3. ANDAR DERECHA (Fila 1: Y = 0. Siguientes 8 sprites)
	sprite->setAnimationSpeed(MOVE_RIGHT, 16);
	for (int i = 8; i < 16; i++) {
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(float(i) * step, 0.f));
	}

	// 4. SALTAR IZQUIERDA (Fila 3: Y = 2 * step. Ahora coge 8 sprites)
	sprite->setAnimationSpeed(JUMP_LEFT, 8);
	for (int i = 0; i < 8; i++) {
		sprite->addKeyframe(JUMP_LEFT, glm::vec2(float(i) * step, 2.f * step));
	}

	// 5. SALTAR DERECHA (Fila 3: Y = 2 * step. Coge los 8 sprites restantes)
	sprite->setAnimationSpeed(JUMP_RIGHT, 8);
	for (int i = 8; i < 16; i++) {
		sprite->addKeyframe(JUMP_RIGHT, glm::vec2(float(i) * step, 2.f * step));
	}

	// 6. CAER IZQUIERDA (En el aire) 
	sprite->setAnimationSpeed(FALL_LEFT, 4); // A 4 FPS se verá cómo pasa lentamente del 4º al 6º
	for (int i = 3; i <= 5; i++) {
		sprite->addKeyframe(FALL_LEFT, glm::vec2(float(i) * step, 2.f * step));
	}
	sprite->addKeyframe(FALL_LEFT, glm::vec2(5.f * step, 2.f * step)); // Repetimos el 6º por si cae de muy alto

	// 7. CAER DERECHA (En el aire)
	sprite->setAnimationSpeed(FALL_RIGHT, 4);
	for (int i = 11; i <= 13; i++) {
		sprite->addKeyframe(FALL_RIGHT, glm::vec2(float(i) * step, 2.f * step));
	}
	sprite->addKeyframe(FALL_RIGHT, glm::vec2(13.f * step, 2.f * step));

	// 8. ATERRIZAR IZQUIERDA (Toca el suelo)
	sprite->setAnimationSpeed(LAND_LEFT, 12); 
	sprite->addKeyframe(LAND_LEFT, glm::vec2(6.f * step, 2.f * step)); 
	sprite->addKeyframe(LAND_LEFT, glm::vec2(7.f * step, 2.f * step)); 

	// 9. ATERRIZAR DERECHA (Toca el suelo)
	sprite->setAnimationSpeed(LAND_RIGHT, 12);
	sprite->addKeyframe(LAND_RIGHT, glm::vec2(14.f * step, 2.f * step)); 
	sprite->addKeyframe(LAND_RIGHT, glm::vec2(15.f * step, 2.f * step)); 


	// 10. PUÑO AL CIELO (Nivel superado)
	sprite->setAnimationSpeed(CHEER_LEFT, CHEER_FPS);
	sprite->setAnimationOnce(CHEER_LEFT);
	sprite->setAnimationSpeed(CHEER_RIGHT, CHEER_FPS);
	sprite->setAnimationOnce(CHEER_RIGHT);
	for (int frame : CHEER_FRAMES) {
		sprite->addKeyframe(CHEER_LEFT, glm::vec2(float(frame) * step, float(CHEER_ROW) * step));
		sprite->addKeyframe(CHEER_RIGHT, glm::vec2(float(8 + frame) * step, float(CHEER_ROW) * step));
	}

	sprite->changeAnimation(1);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::update(int deltaTime)
{
	if (bDying) {
		updateDeath(deltaTime);
		return;
	}
	if (bCelebrating) {
		updateCelebration(deltaTime);
		return;
	}

	sprite->update(deltaTime);
	if (protectTime > 0)
		protectTime -= deltaTime;

	// In god mode he blinks gold. He is always visible, unlike when he is protected by the 'Heart'
	godBlinkTime += deltaTime;
	bool godBlinkOn = Game::instance().isGodMode() && (godBlinkTime / GOD_BLINK_STEP) % 2 == 0;
	sprite->setTint(godBlinkOn ? GOD_TINT : glm::vec4(1.f));

	// Temporizador de aterrizaje
	if (landTimer > 0) {
		landTimer -= deltaTime;
		if (landTimer <= 0) {
			// Cuando acaba el tiempo del impacto, lo ponemos en reposo
			if (sprite->animation() == LAND_LEFT) sprite->changeAnimation(STAND_LEFT);
			else if (sprite->animation() == LAND_RIGHT) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	if (Game::instance().getKey(GLFW_KEY_LEFT))
	{
		// 1. Solo cambiamos a la animación de andar si tocamos el suelo
		if (sprite->animation() != MOVE_LEFT && !bJumping && sprite->animation() != FALL_LEFT && sprite->animation() != FALL_RIGHT)
			sprite->changeAnimation(MOVE_LEFT);

		// 2. Si nos movemos en el aire, giramos el sprite hacia la izquierda
		if (sprite->animation() == JUMP_RIGHT) sprite->changeAnimation(JUMP_LEFT);
		if (sprite->animation() == FALL_RIGHT) sprite->changeAnimation(FALL_LEFT);

		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32), 8, 4))
		{
			posPlayer.x += 2;
			// Solo pasamos a reposo si no estamos en el aire
			if (!bJumping && sprite->animation() != FALL_LEFT) sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if (sprite->animation() != MOVE_RIGHT && !bJumping && sprite->animation() != FALL_LEFT && sprite->animation() != FALL_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);

		if (sprite->animation() == JUMP_LEFT) sprite->changeAnimation(JUMP_RIGHT);
		if (sprite->animation() == FALL_LEFT) sprite->changeAnimation(FALL_RIGHT);

		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32), 8, 4))
		{
			posPlayer.x -= 2;
			if (!bJumping && sprite->animation() != FALL_RIGHT) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		// Solo pasamos a reposo al soltar la tecla si estábamos andando
		if (sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if (sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
	}
	
	if (bJumping)
	{
		// 1. Primero calculamos dónde quiere estar el jugador según su velocidad
		velY = glm::min(velY + GRAVITY, MAX_FALL_SPEED);
		posY += velY;
		posPlayer.y = int(posY);

		if (velY > 0.f)
		{
			// MITAD DESCENDENTE: Comprobamos el suelo al caer
			bool landed = map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y, 8);
			bJumping = !landed;
			if (landed)
			{
				posY = float(posPlayer.y);
				velY = 0.f;
				if (sprite->animation() == FALL_LEFT || sprite->animation() == JUMP_LEFT) {
					sprite->changeAnimation(LAND_LEFT);
					landTimer = 150;
				}
				else if (sprite->animation() == FALL_RIGHT || sprite->animation() == JUMP_RIGHT) {
					sprite->changeAnimation(LAND_RIGHT);
					landTimer = 150;
				}
			}
			else
			{
				if (sprite->animation() == JUMP_LEFT) sprite->changeAnimation(FALL_LEFT);
				else if (sprite->animation() == JUMP_RIGHT) sprite->changeAnimation(FALL_RIGHT);
			}
		}
		else
		{
			// MITAD ASCENDENTE: Comprobamos si nos damos con el techo mientras subimos
			bool hitCeiling = map->collisionMoveUp(posPlayer, glm::ivec2(32, 32), &posPlayer.y, 8, 4);
			if (hitCeiling)
			{
				// CONDICIÓN CRÍTICA: Cortamos el salto al instante.
				// Sin velocidad y con bJumping en false, la gravedad lo empujará hacia abajo.
				bJumping = false;
				posY = float(posPlayer.y);
				velY = 0.f;

				// Lo pasamos a estado de caída (para que aletee al caer)
				if (sprite->animation() == JUMP_LEFT) sprite->changeAnimation(FALL_LEFT);
				else if (sprite->animation() == JUMP_RIGHT) sprite->changeAnimation(FALL_RIGHT);
			}
		}
	}
	else
	{
		// Calculamos el estado tras aplicar gravedad este frame
		float nextVelY = glm::min(velY + GRAVITY, MAX_FALL_SPEED);
		glm::ivec2 posAfterFall = posPlayer;
		posAfterFall.y = int(posY + glm::max(nextVelY, float(GROUND_PROBE)));
		int correctedY = posAfterFall.y;

		// Si colisiona con el suelo a esa altura, estamos en suelo/aterrizando
		if(map->collisionMoveDown(posAfterFall, glm::ivec2(32, 32), &correctedY, 8))
		{
			posPlayer.y = correctedY;
			posY = float(correctedY);
			velY = 0.f;
			if (Game::instance().getKey(GLFW_KEY_UP))
			{
				// 1. Simulamos la posición de la cabeza 1 píxel más arriba
				glm::ivec2 posAbove = posPlayer;
				posAbove.y -= 6;
				int tempY = posAbove.y;

				// 2. Solo permitimos iniciar el salto si NO hay techo pegado (Le pasamos los márgenes 8 y 4)
				if (!map->collisionMoveUp(posAbove, glm::ivec2(32, 32), &tempY, 8, 4))
				{
					bJumping = true;
					velY = -JUMP_SPEED;
					Audio::instance().playSound(SOUND_JUMP);

					// Iniciar animación de salto según la orientación actual
					if (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT || sprite->animation() == LAND_LEFT)
						sprite->changeAnimation(JUMP_LEFT);
					else
						sprite->changeAnimation(JUMP_RIGHT);
				}
			}
			else
			{
				// Si acabamos de aterrizar desde el aire, reproducimos aterrizaje
				if (sprite->animation() == FALL_LEFT || sprite->animation() == JUMP_LEFT) {
					sprite->changeAnimation(LAND_LEFT);
					landTimer = 150;
				}
				else if (sprite->animation() == FALL_RIGHT || sprite->animation() == JUMP_RIGHT) {
					sprite->changeAnimation(LAND_RIGHT);
					landTimer = 150;
				}
			}
		}
		else
		{
			// Seguimos en el aire: aplicamos caída y animación correspondiente
			velY = nextVelY;
			posY += velY;
			posPlayer.y = int(posY);
			if (sprite->animation() == STAND_LEFT || sprite->animation() == MOVE_LEFT || sprite->animation() == JUMP_LEFT)
				sprite->changeAnimation(FALL_LEFT);
			else if (sprite->animation() == STAND_RIGHT || sprite->animation() == MOVE_RIGHT || sprite->animation() == JUMP_RIGHT)
				sprite->changeAnimation(FALL_RIGHT);
		}
	}
	
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::die(bool burnt)
{
	if (bDying)
		return;
	bDying = true;
	bBurnt = burnt;
	bJumping = false;
	deathTime = 0;
	deathStartY = posPlayer.y;

	if (burnt) {
		// He stays in the pose he had and comes apart from it once the blinking is over
		float stepX = 1.0f / BURN_FRAMES, stepY = 1.0f / (3 * 16);
		float row = float(burnRow()) * stepY;

		burnSprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(stepX, stepY), &burnSpritesheet, program);
		burnSprite->setNumberAnimations(2);
		burnSprite->setAnimationSpeed(BURN_FROZEN, BURN_FPS);
		burnSprite->addKeyframe(BURN_FROZEN, glm::vec2(0.f, row));
		burnSprite->setAnimationSpeed(BURN_BREAK, BURN_FPS);
		burnSprite->setAnimationOnce(BURN_BREAK);
		for (int f = 1; f < BURN_FRAMES; f++)
			burnSprite->addKeyframe(BURN_BREAK, glm::vec2(float(f) * stepX, row));
		burnSprite->changeAnimation(BURN_FROZEN);
		burnSprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		return;
	}

	// Jump pose looking to the same side; the sprite stays frozen in it
	if (lookingLeft())
		sprite->changeAnimation(JUMP_LEFT);
	else
		sprite->changeAnimation(JUMP_RIGHT);
}

bool Player::deathFinished() const
{
	return bDying && deathTime >= DEATH_TOTAL_TIME;
}

void Player::updateDeath(int deltaTime)
{
	deathTime += deltaTime;
	bool frozen = deathTime <= DEATH_FREEZE_TIME;
	bool blinkOn = (deathTime / DEATH_BLINK_STEP) % 2 == 0;

	if (bBurnt) {
		// Frozen blinking hot and charred, then he comes apart in his place
		if (frozen)
			burnSprite->setTint(blinkOn ? HOT_TINT : CHARRED_TINT);
		else if (burnSprite->animation() == BURN_FROZEN) {
			burnSprite->setTint(glm::vec4(1.f));
			burnSprite->changeAnimation(BURN_BREAK);
		}
		burnSprite->update(deltaTime);
		return;
	}

	// Classic platformer death: freeze blinking red, small jump up, then fall through the floor
	sprite->setTint((frozen && blinkOn) ? HURT_TINT : glm::vec4(1.f));
	if (!frozen) {
		float t = (deathTime - DEATH_FREEZE_TIME) / 1000.f;
		posPlayer.y = int(deathStartY - DEATH_JUMP_SPEED * t + 0.5f * DEATH_GRAVITY * t * t);
	}
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::celebrate(int doorX)
{
	bCelebrating = true;
	bCheerLeft = lookingLeft();
	bJumping = false;
	celebrateX = doorX;
	protectTime = 0;
	sprite->setTint(glm::vec4(1.f));
}

bool Player::celebrationFinished() const
{
	return bCelebrating && (sprite->animation() == CHEER_LEFT || sprite->animation() == CHEER_RIGHT) && sprite->animationFinished();
}

void Player::updateCelebration(int deltaTime)
{
	sprite->update(deltaTime);

	bool walking = celebrateX >= 0 && posPlayer.x != celebrateX;
	if (walking) {
		int distance = celebrateX - posPlayer.x;
		int walkStep = (distance > 0) ? std::min(distance, CELEBRATE_WALK_STEP) : std::max(distance, -CELEBRATE_WALK_STEP);

		bCheerLeft = distance < 0;
		posPlayer.x += walkStep;
		if (bCheerLeft ? map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32), 8, 4) : map->collisionMoveRight(posPlayer, glm::ivec2(32, 32), 8, 4)) {
			posPlayer.x -= walkStep;
			celebrateX = posPlayer.x; // Something is in the way: he celebrates here
			walking = false;
		}
	}

	glm::ivec2 posAfterFall = posPlayer;
	int correctedY;
	posAfterFall.y += FALL_STEP;
	correctedY = posAfterFall.y;
	bool grounded = map->collisionMoveDown(posAfterFall, glm::ivec2(32, 32), &correctedY, 8);
	posPlayer.y = grounded ? correctedY : posAfterFall.y;

	int anim;
	if (!grounded)
		anim = bCheerLeft ? FALL_LEFT : FALL_RIGHT;
	else if (walking)
		anim = bCheerLeft ? MOVE_LEFT : MOVE_RIGHT;
	else
		anim = bCheerLeft ? CHEER_LEFT : CHEER_RIGHT;
	if (sprite->animation() != anim)
		sprite->changeAnimation(anim);

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

bool Player::lookingLeft() const
{
	int anim = sprite->animation();
	return anim == STAND_LEFT || anim == MOVE_LEFT || anim == JUMP_LEFT || anim == FALL_LEFT || anim == LAND_LEFT;
}

void Player::protect(int time)
{
	protectTime = time;
}

void Player::render()
{
	// The shader discards the half-transparent pixels, so he blinks instead of fading out
	if (!bDying && protectTime > 0 && (protectTime / PROTECT_BLINK_STEP) % 2 != 0)
		return;
	if (!bBurnt)
		sprite->render();
	else if (!burnSprite->animationFinished()) // Once he has come apart nothing is left
		burnSprite->render();
}

int Player::burnRow() const
{
	// chef_death.png has one row per frame of the rows 0, 2 and 4 of chef.png (16 columns)
	float step = 1.0f / 16.0f;
	int column = int(sprite->frame().x / step + 0.5f);
	int row = int(sprite->frame().y / step + 0.5f);
	return (row / 2) * 16 + column;
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	posY = float(posPlayer.y);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




