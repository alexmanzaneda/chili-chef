#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "GameScene.h"
#include "Game.h"
#include "Levels.h"
#include "Audio.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2

#define EXIT_MESSAGE_TIME 2000 // ms the "EXIT!" message is shown when the door opens
#define EXIT_BLINK_TIME 100    // ms on / ms off of the fast blink
#define LEVEL_TIME 180000      // ms of the countdown of every level (3 minutes)

#define BOX_BREAK_FRAMES 6     // Cells of <world>_box_break.png (tools/make_disintegrate.py)
#define BOX_BREAK_FPS 18       // 333 ms: the box comes apart while the fire lasts

#define PICKUP_EFFECT_TIME 400    // ms the picture of a picked power-up rises and blinks
#define PICKUP_EFFECT_SPEED -60.f // px/s (upwards)
#define HEART_PROTECT_TIME 2000   // ms the chef cannot be hurt after losing the 'Heart'
#define TIME_BONUS 30000          // ms the '+30 seconds' power-up adds to the countdown

#define BLAST_COLUMNS 4           // chili_explosion.png: 4 frames x 7 pieces (see Explosion.cpp)
#define BLAST_ROWS 7
#define BLAST_FPS 12
#define FLAME_SEARCH_RADIUS 4     // Tiles around the boss where a flame can appear
#define FLAME_MIN_CHEF_DISTANCE 64.f // Px: it does not appear on top of the chef


// Options of the pause menu, in the order they are shown
enum PauseOption { PAUSE_CONTINUE, PAUSE_RESTART, PAUSE_MAIN_MENU };


GameScene::GameScene(int level)
{
	this->level = level;
	map = NULL;
	player = NULL;
	boss = NULL;
	paused = false;
	doorTile = glm::ivec2(-1, -1);
	doorOpen = false;
	doorOpenTime = 0;
	levelCleared = false;
	ChefStats stats = Game::instance().getStats(); // The power-ups of the previous levels
	maxBombs = stats.bombs;
	fireRange = stats.fireRange;
	heart = stats.heart;
	timeLeft = LEVEL_TIME;
	playerSpawn = glm::ivec2(1, 1); // Only used if the dynamic layer has no chef
}

GameScene::~GameScene()
{
	texProgram.free();

	for (int i = 0; i < bombs.size(); i++)
		delete bombs[i];
	bombs.clear();

	for (int i = 0; i < explosions.size(); i++)
		delete explosions[i];
	explosions.clear();

	clearFireballs();

	for (int i = 0; i < effects.size(); i++)
		delete effects[i];
	effects.clear();

	for (int i = 0; i < powerUps.size(); i++)
		delete powerUps[i];
	powerUps.clear();

	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;

	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL)
			delete enemies[i];
	}

	for (int i = 0; i < dynamicTiles.size(); i++) {
		if (dynamicTiles[i].sprite != NULL) {
			delete dynamicTiles[i].sprite;
		}
	}
	dynamicTiles.clear();
}

void GameScene::loadLevelObjects() {
	// 1. Cargar la hoja de sprites de las cajas y la puerta de este mundo
	dynamicSpritesheet.loadFromFile(LEVELS[level - 1].world->tilesheet, TEXTURE_PIXEL_FORMAT_RGBA);
	boxBreakSpritesheet.loadFromFile(LEVELS[level - 1].world->boxBreak, TEXTURE_PIXEL_FORMAT_RGBA);
	boxBreakSpritesheet.setMinFilter(GL_NEAREST);
	boxBreakSpritesheet.setMagFilter(GL_NEAREST);

	// 2. Cargamos el txt dinámico temporalmente para leer sus posiciones
	TileMap* dynMap = TileMap::createTileMap(string(LEVELS[level - 1].file) + "-dynamic.txt", glm::vec2(0.0f, 0.0f), texProgram);
	glm::ivec2 mapTiles = map->getMapSize();

	// 3. Cada objeto de la capa dinámica crea su entidad en la casilla donde está
	for (int y = 0; y < mapTiles.y; y++) {
		for (int x = 0; x < mapTiles.x; x++) {
			switch (dynMap->getTile(x, y)) {
			case OBJ_BOX:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX);
				break;
			case OBJ_BOMB_UP: // Hidden under a box
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_BOMB_UP);
				break;
			case OBJ_FIRE_UP:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_FIRE_UP);
				break;
			case OBJ_HEART:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_HEART);
				break;
			case OBJ_EXTRA_LIFE:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_EXTRA_LIFE);
				break;
			case OBJ_TIME:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_TIME);
				break;
			case OBJ_FULL_FIRE:
				addDynamicTile(glm::ivec2(x, y), TILE_BOX, POWER_UP_FULL_FIRE);
				break;
			case OBJ_DOOR:
				addDynamicTile(glm::ivec2(x, y), TILE_DOOR);
				doorTile = glm::ivec2(x, y);
				break;
			case OBJ_PLAYER:
				playerSpawn = glm::ivec2(x, y);
				break;
			case OBJ_TOASTER:
				spawnEnemy(new Toaster(), glm::ivec2(x, y));
				break;
			case OBJ_PEA:
				spawnEnemy(new Pea(), glm::ivec2(x, y));
				break;
			case OBJ_BOSS:
				boss = new Boss();
				spawnEnemy(boss, glm::ivec2(x, y));
				break;
			case OBJ_FLAME:
				spawnEnemy(new Flame(), glm::ivec2(x, y));
				break;
			}
		}
	}

	// 4. Borramos el mapa temporal de la memoria porque ya los tenemos como entidades
	delete dynMap;
}

void GameScene::addDynamicTile(const glm::ivec2 &tile, int id, int powerUp)
{
	const WorldInfo *world = LEVELS[level - 1].world;
	int tileSize = map->getTileSize();
	float step = float(tileSize) / dynamicSpritesheet.width(); // Las hojas son cuadradas, pero no todas de 8x8 tiles

	Sprite* dynSprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(step, step), &dynamicSpritesheet, &texProgram);
	dynSprite->setPosition(glm::vec2(tile * tileSize));

	dynSprite->setNumberAnimations(2); // Animación 0: Intacto/Cerrado | 1: Roto/Abierto
	dynSprite->setAnimationSpeed(0, 8);
	dynSprite->setAnimationSpeed(1, 8);

	if (id == TILE_BOX) {
		// Animación 0: Caja INTACTA
		dynSprite->addKeyframe(0, glm::vec2(world->box) * step);

		// Animación 1: Caja ROTA (Si tienes un sprite de caja rota)
		//dynSprite->addKeyframe(1, glm::vec2(columna_caja_rota * step, fila_caja_rota * step));
	}
	else if (id == TILE_DOOR) {
		// Animación 0: Puerta CERRADA
		dynSprite->addKeyframe(0, glm::vec2(world->door) * step);

		// Animación 1: Puerta ABIERTA
		dynSprite->addKeyframe(1, glm::vec2(world->openDoor) * step);
	}
	dynSprite->changeAnimation(0); // Que empiece intacta / cerrada

	dynamicTiles.push_back({ dynSprite, tile, id, powerUp });

	// ESTRATEGIA CLAVE: Le pasamos el bloque al mapa principal estático.
	// Esto hace que el motor de físicas detecte la caja como un muro y
	// el cocinero no pueda atravesarla caminando.
	map->setTile(tile.x, tile.y, id);
}

void GameScene::spawnEnemy(Enemy *enemy, const glm::ivec2 &tile)
{
	enemy->init(glm::ivec2(0, 0), texProgram);
	enemy->setTileMap(map);
	enemy->setPosition(glm::vec2(tile * map->getTileSize()));
	enemies.push_back(enemy);
}

void GameScene::init()
{
	initShaders();
	ui.init();
	hud.init(texProgram);
	pauseMenu.init({ "RESUME", "RESTART", "MAIN MENU" });
	map = TileMap::createTileMap(string(LEVELS[level - 1].file) + ".txt", glm::vec2(0.0f, 0.0f), texProgram);
	map->setFreeTiles(LEVELS[level - 1].world->freeTiles);

	loadLevelObjects();
	// Every attempt starts the track from the beginning, also after RESTART or the level keys
	Audio::instance().stopMusic();
	Audio::instance().playMusic(LEVELS[level - 1].world->music);

	// The HUD only counts the kinds of enemy this level has (each world has its own)
	int enemiesInLevel[NUM_ENEMY_TYPES];
	countEnemies(enemiesInLevel);
	std::vector<EnemyType> enemyTypes;
	for (int type = 0; type < NUM_ENEMY_TYPES; type++) {
		// The flames only help the boss and die with it: the goal is the boss, so they are not counted
		if (enemiesInLevel[type] > 0 && !(type == ENEMY_FLAME && boss != NULL))
			enemyTypes.push_back(EnemyType(type));
	}
	hud.setEnemyTypes(enemyTypes);

	// GL_NEAREST: with linear filtering the scaled-up pieces get mixed with the transparent
	// pixels of the neighbour cells and a dark line appears between the explosion tiles
	bombSpritesheet.loadFromFile("images/chili_bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombSpritesheet.setMinFilter(GL_NEAREST);
	bombSpritesheet.setMagFilter(GL_NEAREST);
	explosionSpritesheet.loadFromFile("images/chili_explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionSpritesheet.setMinFilter(GL_NEAREST);
	explosionSpritesheet.setMagFilter(GL_NEAREST);
	powerUpSpritesheet.loadFromFile("images/powerups.png", TEXTURE_PIXEL_FORMAT_RGBA);
	powerUpSpritesheet.setMinFilter(GL_NEAREST);
	powerUpSpritesheet.setMagFilter(GL_NEAREST);
	if (boss != NULL) {
		fireballSpritesheet.loadFromFile("images/fireball.png", TEXTURE_PIXEL_FORMAT_RGBA);
		fireballSpritesheet.setMinFilter(GL_NEAREST);
		fireballSpritesheet.setMagFilter(GL_NEAREST);
	}

	player = new Player();
	player->init(glm::ivec2(0.0f, 0.0f), texProgram);
	player->setTileMap(map);
	player->setPosition(glm::vec2(playerSpawn * map->getTileSize()));

	// Lo que mide tu "lente": el nivel (480 x 480) y debajo la banda del HUD (480 x 40)
	cameraWidth = 480.0f;
	cameraHeight = float(GAME_AREA_HEIGHT + HUD_HEIGHT);
	// Already on the chef: the first frames are seen during the transition, before any update
	updateCamera();

	currentTime = 0.0f;
}

int GameScene::update(int deltaTime)
{
	if (paused)
		return updatePause(deltaTime);

	// The chef reached the door: the level stays frozen while he celebrates, but what
	// was already being destroyed finishes. Then go to the next level
	if (levelCleared) {
		player->update(deltaTime);
		updateBoxes(deltaTime);
		updateExplosions(deltaTime);
		updateEffects(deltaTime);
		updateDyingEnemies(deltaTime);
		updateCamera();
		if (!player->celebrationFinished())
			return SCENE_GAME;
		// Only a cleared level keeps its power-ups: if the chef dies, they are lost
		Game::instance().setStats({ maxBombs, fireRange, heart });
		return SCENE_LEVEL_CLEARED;
	}

	// While the chef dies the rest of the level (enemies, bombs, camera) stays frozen, but
	// what was already being destroyed finishes (fire, boxes, enemies coming apart)
	if (player->isDying()) {
		player->update(deltaTime);
		updateBoxes(deltaTime);
		updateExplosions(deltaTime);
		updateEffects(deltaTime);
		updateDyingEnemies(deltaTime);
		if (player->deathFinished())
			return SCENE_LIFE_LOST;
		return SCENE_GAME;
	}

	// START (ENTER) or ESC pauses the game
	if (Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE)) {
		setPaused(true);
		return SCENE_GAME;
	}

	// 'K' kills all the enemies (key required by the assignment to test the game)
	if (Game::instance().getKeyDown(GLFW_KEY_K))
		killAllEnemies();

	// 'G' turns the god mode on and off (also required by the assignment)
	if (Game::instance().getKeyDown(GLFW_KEY_G))
		Game::instance().toggleGodMode();

	// '1', '2'... jump to that level of LEVELS: '1' is 1-1, '2' is 1-2... (also required by the assignment)
	for (int i = 0; i < NUM_LEVELS && i < 9; i++) {
		if (Game::instance().getKeyDown(GLFW_KEY_1 + i)) {
			Game::instance().setLevel(i + 1);
			return SCENE_RESTART;
		}
	}

	currentTime += deltaTime;

	// The countdown only runs while the chef can play (it stops in god mode);
	// when it reaches 0 the chef dies
	if (!Game::instance().isGodMode()) {
		timeLeft -= deltaTime;
		if (timeLeft <= 0) {
			timeLeft = 0;
			killPlayer();
			return SCENE_GAME;
		}
	}

	hud.update(deltaTime);
	updateBoxes(deltaTime);
	player->update(deltaTime);
	updatePowerUps(deltaTime);
	updateBombs(deltaTime);
	updateExplosions(deltaTime);
	updateEffects(deltaTime);
	updateDyingEnemies(deltaTime);

	if (boss != NULL) {
		int enemiesAlive[NUM_ENEMY_TYPES];
		countEnemies(enemiesAlive);
		boss->setTarget(player->getPosition() + glm::vec2(16.f));
		boss->setFlamesAlive(enemiesAlive[ENEMY_FLAME]);
	}

	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL && !enemies[i]->isDying()) {
			enemies[i]->update(deltaTime);
			if (enemies[i]->isDying()) // It has just finished its own defeat (the boss)
				continue;

			// --- DETECCIÓN DE COLISIÓN ---
			if (!player->isDying() && enemies[i]->isHarmful()) {
				glm::vec2 posPlayer = player->getPosition();
				HitBox box = enemies[i]->contactBox();

				// Ajustamos el tamaño real del impacto ignorando píxeles transparentes
				int hitBoxSize = 20;
				int offset = 6;

				bool collisionX = (posPlayer.x + offset < box.pos.x + box.size.x) &&
					(posPlayer.x + offset + hitBoxSize > box.pos.x);

				bool collisionY = (posPlayer.y + offset < box.pos.y + box.size.y) &&
					(posPlayer.y + offset + hitBoxSize > box.pos.y);

				// Si colisiona con este enemigo en concreto
				if (collisionX && collisionY) {
					hurtPlayer();
				}
			}
		}
	}

	updateBoss();
	updateFireballs(deltaTime);

	// With no enemies left the door opens, and walking into it clears the level.
	// With no door (the boss level) killing them is enough
	bool hasDoor = doorTile.x >= 0;
	if (!doorOpen && !enemiesLeft() && !player->isDying()) {
		if (hasDoor)
			openDoor();
		else
			clearLevel();
	}
	else if (doorOpen)
		doorOpenTime += deltaTime;
	if (doorOpen && !player->isDying() && playerAtDoor())
		clearLevel();

	updateCamera();

	return SCENE_GAME;
}

void GameScene::updateCamera()
{
	// 1. Obtener el centro exacto del jugador (sumando la mitad de sus 32 píxeles)
	glm::vec2 playerPos = player->getPosition();
	float playerCenterX = playerPos.x + 16.0f;
	float playerCenterY = playerPos.y + 16.0f;

	// 2. Centrar la cámara matemáticamente sobre ese punto (en vertical, sobre el área jugable sin la banda)
	cameraX = playerCenterX - (cameraWidth / 2.0f);
	cameraY = playerCenterY - (GAME_AREA_HEIGHT / 2.0f);

	// 3. TOPES DE LOS BORDES (Clamping)
	glm::ivec2 mapTiles = map->getMapSize();
	float maxCameraX = (mapTiles.x * map->getTileSize()) - cameraWidth;
	// Abajo solo cuenta el área jugable: el suelo debe acabar justo donde empieza la banda del HUD
	float maxCameraY = float(mapTiles.y * map->getTileSize() - GAME_AREA_HEIGHT);

	// Evitar que la cámara se salga por la izquierda o la derecha
	if (cameraX < 0.0f) cameraX = 0.0f;
	else if (cameraX > maxCameraX) cameraX = maxCameraX;

	// Evitar que la cámara se salga por arriba o por abajo
	if (cameraY < 0.0f) cameraY = 0.0f;
	else if (cameraY > maxCameraY) cameraY = maxCameraY;

	// Actualizar la matriz de proyección con las nuevas coordenadas
	projection = glm::ortho(cameraX, cameraX + cameraWidth, cameraY + cameraHeight, cameraY);
}

void GameScene::updateBombs(int deltaTime)
{
	int tileSize = map->getTileSize();
	glm::ivec2 posPlayer = glm::ivec2(player->getPosition());

	// SPACE drops a bomb on the tile where the center of the chef is (also in the air)
	if (Game::instance().getKeyDown(GLFW_KEY_SPACE) && int(bombs.size()) < maxBombs) {
		glm::ivec2 tile((posPlayer.x + tileSize / 2) / tileSize, (posPlayer.y + tileSize / 2) / tileSize);
		int tileId = map->getTile(tile.x, tile.y);
		bool freeTile = map->isFree(tileId); // Same rule as the TileMap collisions

		for (int i = 0; i < bombs.size(); i++) {
			if (bombs[i]->getTile() == tile)
				freeTile = false;
		}
		if (freeTile) {
			Bomb *bomb = new Bomb();
			bomb->init(tile, map, bombSpritesheet, texProgram);
			bombs.push_back(bomb);
			Audio::instance().playSound(SOUND_BOMB_PLACE);
		}
	}

	for (int i = int(bombs.size()) - 1; i >= 0; i--) {
		bombs[i]->update(deltaTime);

		// The bomb becomes solid once the chef has left its tile, so he is not trapped inside it
		if (!bombs[i]->isSolid() && !playerOverlapsTile(bombs[i]->getTile()))
			bombs[i]->makeSolid();

		if (bombs[i]->hasExploded()) {
			// Free its tile first so the fire does not stop on the bomb itself
			bombs[i]->removeFromMap();
			Bomb *bomb = bombs[i];
			bombs.erase(bombs.begin() + i);
			explodeBomb(bomb);
			delete bomb;
		}
	}
}

Bomb *GameScene::bombAt(const glm::ivec2 &tile) const
{
	for (int i = 0; i < bombs.size(); i++) {
		if (bombs[i]->getTile() == tile)
			return bombs[i];
	}
	return NULL;
}

void GameScene::explodeBomb(Bomb *bomb)
{
	static const glm::ivec2 DIRECTIONS[4] = { glm::ivec2(-1, 0), glm::ivec2(1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };
	static const int END_PIECES[4] = { PIECE_END_LEFT, PIECE_END_RIGHT, PIECE_END_UP, PIECE_END_DOWN };

	glm::ivec2 center = bomb->getTile();
	glm::ivec2 mapSize = map->getMapSize();
	std::vector<ExplosionCell> cells;

	cells.push_back({ center, PIECE_CENTER });
	for (int d = 0; d < 4; d++) {
		int armPiece = (d < 2) ? PIECE_HORIZONTAL : PIECE_VERTICAL;
		int firstCell = int(cells.size());

		// The fire advances tile by tile until its range or until something stops it
		for (int k = 1; k <= fireRange; k++) {
			glm::ivec2 tile = center + DIRECTIONS[d] * k;
			if (tile.x < 0 || tile.x >= mapSize.x || tile.y < 0 || tile.y >= mapSize.y)
				break;

			Bomb *other = bombAt(tile);
			if (other != NULL) {
				other->detonate(); // Chain reaction: it explodes in this update or the next one
				break;
			}
			int tileId = map->getTile(tile.x, tile.y);
			if (tileId == TILE_BOX) {
				destroyBox(tile);
				break;
			}
			// The fire burns the power-ups already uncovered and stops there, like with a box.
			// The one a box leaves appears after this explosion, so it is not burnt by it
			PowerUp *powerUp = powerUpAt(tile);
			if (powerUp != NULL) {
				removePowerUp(powerUp);
				break;
			}
			if (!map->isFree(tileId)) // Wall, door... (same rule as the TileMap collisions)
				break;

			cells.push_back({ tile, armPiece });
		}
		// The last tile of each arm gets the rounded end, also when a wall cuts it short
		if (int(cells.size()) > firstCell)
			cells.back().piece = END_PIECES[d];
	}

	Explosion *explosion = new Explosion();
	explosion->init(cells, map->getTileSize(), explosionSpritesheet, texProgram);
	explosions.push_back(explosion);
	Audio::instance().playSound(SOUND_EXPLOSION);
}

void GameScene::destroyBox(const glm::ivec2 &tile)
{
	int tileSize = map->getTileSize();
	float step = 1.0f / BOX_BREAK_FRAMES;

	for (int i = 0; i < dynamicTiles.size(); i++) {
		if (dynamicTiles[i].tile == tile && dynamicTiles[i].id == TILE_BOX) {
			// The box comes apart in its tile, playing the frames of the break once
			Sprite *breakSprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(step, 1.f), &boxBreakSpritesheet, &texProgram);
			breakSprite->setNumberAnimations(1);
			breakSprite->setAnimationSpeed(0, BOX_BREAK_FPS);
			breakSprite->setAnimationOnce(0);
			for (int f = 0; f < BOX_BREAK_FRAMES; f++)
				breakSprite->addKeyframe(0, glm::vec2(float(f) * step, 0.f));
			breakSprite->changeAnimation(0);
			breakSprite->setPosition(glm::vec2(tile * tileSize));

			delete dynamicTiles[i].sprite;
			dynamicTiles[i].sprite = breakSprite;
			// It keeps blocking (the chef and other fires) while it comes apart; updateBoxes frees it
			dynamicTiles[i].id = TILE_BOX_BURNING;
			map->setTile(tile.x, tile.y, TILE_BOX_BURNING);
			return;
		}
	}
}

void GameScene::updateBoxes(int deltaTime)
{
	for (int i = int(dynamicTiles.size()) - 1; i >= 0; i--) {
		dynamicTiles[i].sprite->update(deltaTime);

		if (dynamicTiles[i].id == TILE_BOX_BURNING && dynamicTiles[i].sprite->animationFinished()) {
			glm::ivec2 tile = dynamicTiles[i].tile;
			map->setTile(tile.x, tile.y, 0);
			if (dynamicTiles[i].powerUp != NO_POWER_UP)
				spawnPowerUp(PowerUpType(dynamicTiles[i].powerUp), tile);
			delete dynamicTiles[i].sprite;
			dynamicTiles.erase(dynamicTiles.begin() + i);
		}
	}
}

void GameScene::spawnPowerUp(PowerUpType type, const glm::ivec2 &tile)
{
	PowerUp *powerUp = new PowerUp();
	powerUp->init(type, tile, map->getTileSize(), powerUpSpritesheet, texProgram);
	powerUps.push_back(powerUp);
}

PowerUp *GameScene::powerUpAt(const glm::ivec2 &tile) const
{
	for (int i = 0; i < powerUps.size(); i++) {
		if (powerUps[i]->getTile() == tile)
			return powerUps[i];
	}
	return NULL;
}

void GameScene::removePowerUp(PowerUp *powerUp)
{
	for (int i = 0; i < powerUps.size(); i++) {
		if (powerUps[i] == powerUp) {
			powerUps.erase(powerUps.begin() + i);
			delete powerUp;
			return;
		}
	}
}

void GameScene::updatePowerUps(int deltaTime)
{
	int tileSize = map->getTileSize();

	for (int i = int(powerUps.size()) - 1; i >= 0; i--) {
		powerUps[i]->update(deltaTime);
		if (player->isDying() || !playerOverlapsTile(powerUps[i]->getTile()))
			continue;

		// Picked up: its picture rises and blinks away while the effect is applied
		PowerUpType type = powerUps[i]->getType();
		Sprite *sprite = PowerUp::createSprite(type, tileSize, powerUpSpritesheet, texProgram);
		Effect *effect = new Effect(sprite, glm::vec2(powerUps[i]->getTile() * tileSize), PICKUP_EFFECT_TIME);
		effect->setVelocity(glm::vec2(0.f, PICKUP_EFFECT_SPEED));
		effect->setBlinkTime(PICKUP_EFFECT_TIME);
		effects.push_back(effect);

		applyPowerUp(type);
		Audio::instance().playSound(SOUND_POWER_UP);
		delete powerUps[i];
		powerUps.erase(powerUps.begin() + i);
	}
}

// What each power-up does. A new power-up adds its case here
void GameScene::applyPowerUp(PowerUpType type)
{
	switch (type) {
	case POWER_UP_BOMB_UP:
		maxBombs = std::min(maxBombs + 1, MAX_BOMBS);
		break;
	case POWER_UP_FIRE_UP:
		fireRange = std::min(fireRange + 1, MAX_FIRE_RANGE);
		break;
	case POWER_UP_HEART:
		heart = true; // Only one: a second one does nothing
		break;
	case POWER_UP_EXTRA_LIFE:
		Game::instance().addLife();
		break;
	case POWER_UP_TIME:
		timeLeft += TIME_BONUS;
		hud.flashTime();
		break;
	case POWER_UP_FULL_FIRE:
		fireRange = MAX_FIRE_RANGE;
		break;
	}
}

// An enemy or the fire touches the chef. The time running out kills him anyway
void GameScene::hurtPlayer(bool burnt)
{
	if (levelCleared || player->isDying() || player->isProtected() || Game::instance().isGodMode())
		return;
	if (heart) {
		heart = false;
		player->protect(HEART_PROTECT_TIME);
		Audio::instance().playSound(SOUND_HEART_LOST);
		return;
	}
	killPlayer(burnt);
}

void GameScene::killPlayer(bool burnt)
{
	player->die(burnt);
	Audio::instance().stopMusic();
	Audio::instance().playSound(SOUND_CHEF_DEATH);
}

void GameScene::updateDyingEnemies(int deltaTime)
{
	// A dying enemy is harmless; it is deleted once it has come apart
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL && enemies[i]->isDying()) {
			enemies[i]->updateDeath(deltaTime);
			if (enemies[i]->isDead()) {
				if (enemies[i] == boss)
					boss = NULL;
				delete enemies[i];
				enemies[i] = NULL;
			}
		}
	}
}

void GameScene::updateBoss()
{
	if (boss == NULL)
		return;

	glm::vec2 blastCenter;
	if (boss->takeBlast(blastCenter)) {
		spawnBlast(blastCenter);
		Audio::instance().playSound(SOUND_EXPLOSION);
	}

	// Beaten (by the fire or by 'K'): everything it made goes with it
	if (boss->isDefeated() || boss->isDying()) {
		for (int i = 0; i < enemies.size(); i++) {
			if (enemies[i] != NULL && enemies[i] != boss)
				enemies[i]->kill();
		}
		clearFireballs();
		boss->takeShots();
		boss->takeSummon();
		return;
	}

	std::vector<BossShot> shots = boss->takeShots();
	for (int i = 0; i < shots.size(); i++) {
		Fireball *fireball = new Fireball();
		fireball->init(shots[i].origin, shots[i].direction, map, fireballSpritesheet, texProgram);
		fireballs.push_back(fireball);
	}

	glm::ivec2 tile;
	if (boss->takeSummon() && findTileNear(boss->getCenter(), tile))
		spawnEnemy(new Flame(), tile);
}

// The nearest free tile: the boss floats over the walls, and a flame cannot start inside one
bool GameScene::findTileNear(const glm::vec2 &center, glm::ivec2 &tile) const
{
	int tileSize = map->getTileSize();
	glm::ivec2 mapSize = map->getMapSize();
	glm::ivec2 centerTile = glm::ivec2(center) / tileSize;
	glm::vec2 chefCenter = player->getPosition() + glm::vec2(16.f);

	for (int radius = 0; radius <= FLAME_SEARCH_RADIUS; radius++) {
		for (int y = centerTile.y - radius; y <= centerTile.y + radius; y++) {
			for (int x = centerTile.x - radius; x <= centerTile.x + radius; x++) {
				if (std::max(std::abs(x - centerTile.x), std::abs(y - centerTile.y)) != radius)
					continue; // Inside the ring: already checked with a smaller radius
				if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y)
					continue;
				glm::ivec2 candidate(x, y);
				glm::vec2 tileCenter = glm::vec2(candidate * tileSize) + glm::vec2(tileSize / 2.f);
				if (map->isFree(map->getTile(x, y)) && bombAt(candidate) == NULL && glm::length(tileCenter - chefCenter) >= FLAME_MIN_CHEF_DISTANCE) {
					tile = candidate;
					return true;
				}
			}
		}
	}
	return false;
}

void GameScene::updateFireballs(int deltaTime)
{
	for (int i = int(fireballs.size()) - 1; i >= 0; i--) {
		fireballs[i]->update(deltaTime);

		// The same part of the chef the fire must touch
		bool burns = !player->isDying() && fireballs[i]->hits(player->getPosition() + glm::vec2(8.f), glm::vec2(16.f));
		if (burns)
			hurtPlayer(true);
		if (burns || fireballs[i]->hasHitWall()) {
			spawnBlast(fireballs[i]->getCenter());
			delete fireballs[i];
			fireballs.erase(fireballs.begin() + i);
		}
	}
}

void GameScene::clearFireballs()
{
	for (int i = 0; i < fireballs.size(); i++)
		delete fireballs[i];
	fireballs.clear();
}

void GameScene::spawnBlast(const glm::vec2 &center)
{
	int tileSize = map->getTileSize();
	float stepX = 1.f / BLAST_COLUMNS, stepY = 1.f / BLAST_ROWS;

	Sprite *sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(stepX, stepY), &explosionSpritesheet, &texProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, BLAST_FPS);
	sprite->setAnimationOnce(0);
	for (int f = 0; f < BLAST_COLUMNS; f++)
		sprite->addKeyframe(0, glm::vec2(float(f) * stepX, float(PIECE_CENTER) * stepY));
	sprite->changeAnimation(0);
	effects.push_back(new Effect(sprite, center - glm::vec2(tileSize / 2.f), 0));
}

void GameScene::updateEffects(int deltaTime)
{
	for (int i = int(effects.size()) - 1; i >= 0; i--) {
		effects[i]->update(deltaTime);
		if (effects[i]->isFinished()) {
			delete effects[i];
			effects.erase(effects.begin() + i);
		}
	}
}

void GameScene::updateExplosions(int deltaTime)
{
	for (int i = int(explosions.size()) - 1; i >= 0; i--) {
		explosions[i]->update(deltaTime);
		if (explosions[i]->isFinished()) {
			delete explosions[i];
			explosions.erase(explosions.begin() + i);
			continue;
		}

		// The fire kills the enemies and the chef while it lasts
		for (int t = 0; t < enemies.size(); t++) {
			if (enemies[t] == NULL || enemies[t]->isDying())
				continue;
			HitBox box = enemies[t]->fireBox();
			if (explosions[i]->hits(box.pos, box.size))
				enemies[t]->hitByFire();
		}
		if (!player->isDying() && explosions[i]->hits(player->getPosition()))
			hurtPlayer(true); // Burnt
	}
}

bool GameScene::playerOverlapsTile(const glm::ivec2 &tile) const
{
	// Same box Player uses for its collisions: 8 px margin on each side, 4 px over the hat
	glm::ivec2 posPlayer = glm::ivec2(player->getPosition());
	int tileSize = map->getTileSize();
	int left = tile.x * tileSize, top = tile.y * tileSize;

	return posPlayer.x + 8 < left + tileSize && posPlayer.x + 32 - 8 > left &&
		posPlayer.y + 4 < top + tileSize && posPlayer.y + 32 > top;
}

void GameScene::killAllEnemies()
{
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL)
			enemies[i]->kill();
	}
}

// For the HUD: a dying enemy does not count anymore, so the number goes down when it is hit
void GameScene::countEnemies(int count[NUM_ENEMY_TYPES]) const
{
	for (int type = 0; type < NUM_ENEMY_TYPES; type++)
		count[type] = 0;
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL && !enemies[i]->isDying())
			count[enemies[i]->getType()]++;
	}
}

// Dying enemies still count, so the door opens when the last one has come apart
bool GameScene::enemiesLeft() const
{
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] != NULL)
			return true;
	}
	return false;
}

void GameScene::clearLevel()
{
	levelCleared = true;
	player->celebrate(doorTile.x >= 0 ? doorTile.x * map->getTileSize() : -1);
	Audio::instance().stopMusic();
	Audio::instance().playSound(SOUND_LEVEL_CLEAR);
}

void GameScene::openDoor()
{
	for (int i = 0; i < dynamicTiles.size(); i++) {
		if (dynamicTiles[i].id == TILE_DOOR) {
			dynamicTiles[i].sprite->changeAnimation(1); // Animación 1: Puerta ABIERTA
			// The open door is not solid, so the chef can walk into it
			map->setTile(dynamicTiles[i].tile.x, dynamicTiles[i].tile.y, 0);
		}
	}
	doorOpen = true;
	Audio::instance().playSound(SOUND_DOOR_OPEN);
}

bool GameScene::playerAtDoor() const
{
	// The center of the chef must be inside the door tile
	glm::ivec2 posPlayer = glm::ivec2(player->getPosition());
	int tileSize = map->getTileSize();

	return (posPlayer.x + tileSize / 2) / tileSize == doorTile.x && (posPlayer.y + tileSize / 2) / tileSize == doorTile.y;
}

void GameScene::renderExitMessage()
{
	// Tall letters (3x wide, 6x high) in black, blinking fast in the middle of the screen
	if ((doorOpenTime / EXIT_BLINK_TIME) % 2 != 0)
		return;
	ui.begin();
	ui.drawTextCentered("EXIT!", (UI_HEIGHT - 7 * 6) / 2.f, glm::vec4(0.f, 0.f, 0.f, 1.f), 3, 6);
	ui.end();
}

// The music stops while the pause menu is shown and goes on from the same point
void GameScene::setPaused(bool pause)
{
	paused = pause;
	if (pause) {
		pauseMenu.reset();
		Audio::instance().playSound(SOUND_PAUSE);
		Audio::instance().pauseMusic();
	}
	else
		Audio::instance().playMusic(LEVELS[level - 1].world->music);
}

int GameScene::updatePause(int deltaTime)
{
	// ESC closes the pause menu, like choosing RESUME
	if (Game::instance().getKeyDown(GLFW_KEY_ESCAPE)) {
		setPaused(false);
		return SCENE_GAME;
	}

	switch (pauseMenu.update(deltaTime)) {
	case PAUSE_CONTINUE:
		setPaused(false);
		break;
	case PAUSE_RESTART:
		return SCENE_RESTART;
	case PAUSE_MAIN_MENU:
		return SCENE_MENU;
	}
	return SCENE_GAME;
}

void GameScene::renderPause()
{
	float boxWidth = pauseMenu.width() + 20.f, boxHeight = pauseMenu.height() + 30.f;
	float boxX = float(int((UI_WIDTH - boxWidth) / 2.f)), boxY = float(int((UI_HEIGHT - boxHeight) / 2.f));

	ui.begin();
	ui.fillScreen(glm::vec4(0.f, 0.f, 0.f, 0.5f)); // Darken the level
	ui.drawBox(boxX, boxY, boxWidth, boxHeight);
	ui.drawTextCentered("PAUSE", boxY + 7.f, UI_COLOR_RED);
	pauseMenu.render(ui, boxX + 10.f, boxY + 22.f);
	ui.end();
}

void GameScene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	map->render();
	for (int i = 0; i < dynamicTiles.size(); i++) {
		dynamicTiles[i].sprite->render();
	}

	for (int i = 0; i < powerUps.size(); i++) {
		powerUps[i]->render();
	}

	for (int i = 0; i < bombs.size(); i++) {
		bombs[i]->render();
	}

	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] == NULL) continue;
		if (enemies[i]->isDying()) enemies[i]->renderDeath();
		else enemies[i]->render();
	}

	for (int i = 0; i < fireballs.size(); i++) {
		fireballs[i]->render();
	}

	for (int i = 0; i < explosions.size(); i++) {
		explosions[i]->render();
	}

	for (int i = 0; i < effects.size(); i++) {
		effects[i]->render();
	}

	// The chef goes on top of the enemies so the death animation is always visible
	player->render();

	// The band goes after everything of the level, so it covers what scrolls behind it
	int enemiesLeft[NUM_ENEMY_TYPES];
	countEnemies(enemiesLeft);
	hud.render(glm::vec2(cameraX, cameraY), timeLeft, Game::instance().getLives(), heart, maxBombs, fireRange, enemiesLeft);

	if (doorOpen && !levelCleared && doorOpenTime < EXIT_MESSAGE_TIME)
		renderExitMessage();
	if (paused)
		renderPause();
}

void GameScene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



