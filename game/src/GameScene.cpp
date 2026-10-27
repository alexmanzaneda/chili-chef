#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "GameScene.h"
#include "Game.h"
#include "Levels.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2

#define LEVEL_CLEAR_TIME 2000 // ms the "STAGE CLEAR" message is shown
#define EXIT_MESSAGE_TIME 2000 // ms the "EXIT!" message is shown when the door opens
#define EXIT_BLINK_TIME 100    // ms on / ms off of the fast blink
#define LEVEL_TIME 180000      // ms of the countdown of every level (3 minutes)


// Options of the pause menu, in the order they are shown
enum PauseOption { PAUSE_CONTINUE, PAUSE_RESTART, PAUSE_MAIN_MENU };


GameScene::GameScene(int level)
{
	this->level = level;
	map = NULL;
	player = NULL;
	paused = false;
	doorTile = glm::ivec2(-1, -1);
	doorOpen = false;
	doorOpenTime = 0;
	levelCleared = false;
	clearTime = 0;
	maxBombs = 1;
	fireRange = 1;
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

	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;

	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL)
			delete toasters[i];
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
			case OBJ_DOOR:
				addDynamicTile(glm::ivec2(x, y), TILE_DOOR);
				doorTile = glm::ivec2(x, y);
				break;
			case OBJ_PLAYER:
				playerSpawn = glm::ivec2(x, y);
				break;
			case OBJ_TOASTER:
				spawnToaster(glm::ivec2(x, y));
				break;
			}
		}
	}

	// 4. Borramos el mapa temporal de la memoria porque ya los tenemos como entidades
	delete dynMap;
}

void GameScene::addDynamicTile(const glm::ivec2 &tile, int id)
{
	const WorldInfo *world = LEVELS[level - 1].world;
	int tileSize = map->getTileSize();
	float step = 1.0f / 8.0f; // Ajusta esto según las columnas de tu imagen de bloques

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

	dynamicTiles.push_back({ dynSprite, tile, id });

	// ESTRATEGIA CLAVE: Le pasamos el bloque al mapa principal estático.
	// Esto hace que el motor de físicas detecte la caja como un muro y
	// el cocinero no pueda atravesarla caminando.
	map->setTile(tile.x, tile.y, id);
}

void GameScene::spawnToaster(const glm::ivec2 &tile)
{
	Toaster *toaster = new Toaster();
	toaster->init(glm::ivec2(0, 0), texProgram);
	toaster->setTileMap(map);
	toaster->setPosition(glm::vec2(tile * map->getTileSize()));
	toasters.push_back(toaster);
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
	// GL_NEAREST: with linear filtering the scaled-up pieces get mixed with the transparent
	// pixels of the neighbour cells and a dark line appears between the explosion tiles
	bombSpritesheet.loadFromFile("images/chili_bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombSpritesheet.setMinFilter(GL_NEAREST);
	bombSpritesheet.setMagFilter(GL_NEAREST);
	explosionSpritesheet.loadFromFile("images/chili_explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionSpritesheet.setMinFilter(GL_NEAREST);
	explosionSpritesheet.setMagFilter(GL_NEAREST);

	player = new Player();
	player->init(glm::ivec2(0.0f, 0.0f), texProgram);
	player->setTileMap(map);
	player->setPosition(glm::vec2(playerSpawn * map->getTileSize()));

	// Lo que mide tu "lente": el nivel (480 x 480) y debajo la banda del HUD (480 x 40)
	cameraWidth = 480.0f;
	cameraHeight = float(GAME_AREA_HEIGHT + HUD_HEIGHT);
	
	// Calculamos el tamaño total del mapa a partir del TileMap cargado
	glm::ivec2 mapTiles = map->getMapSize();
	float mapPixelWidth = float(mapTiles.x * map->getTileSize());
	float mapPixelHeight = float(mapTiles.y * map->getTileSize());

	// Centrar la cámara horizontalmente: la mitad del mapa menos la mitad de la cámara
	cameraX = 0.0f;

	// Centrar la cámara abajo: el suelo del nivel justo encima de la banda del HUD
	cameraY = mapPixelHeight - GAME_AREA_HEIGHT;
	projection = glm::ortho(cameraX, cameraX + cameraWidth, cameraY + cameraHeight, cameraY);

	currentTime = 0.0f;
}

int GameScene::update(int deltaTime)
{
	if (paused)
		return updatePause(deltaTime);

	// The chef went through the door: show "STAGE CLEAR" and then go to the next level
	if (levelCleared) {
		clearTime += deltaTime;
		if (clearTime >= LEVEL_CLEAR_TIME)
			return SCENE_LEVEL_CLEARED;
		return SCENE_GAME;
	}

	// While the chef dies the rest of the level (enemies, camera) stays frozen
	if (player->isDying()) {
		player->update(deltaTime);
		if (player->deathFinished())
			return SCENE_LIFE_LOST;
		return SCENE_GAME;
	}

	// START (ENTER) or ESC pauses the game
	if (Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE)) {
		paused = true;
		pauseMenu.reset();
		return SCENE_GAME;
	}

	// 'K' kills all the enemies (key required by the assignment to test the game)
	if (Game::instance().getKeyDown(GLFW_KEY_K))
		killAllEnemies();

	// '1', '2'... jump to that level of LEVELS: '1' is 1-1, '2' is 1-2... (also required by the assignment)
	for (int i = 0; i < NUM_LEVELS && i < 9; i++) {
		if (Game::instance().getKeyDown(GLFW_KEY_1 + i)) {
			Game::instance().setLevel(i + 1);
			return SCENE_RESTART;
		}
	}

	currentTime += deltaTime;

	// The countdown only runs while the chef can play; when it reaches 0 the chef dies
	timeLeft -= deltaTime;
	if (timeLeft <= 0) {
		timeLeft = 0;
		player->die();
		return SCENE_GAME;
	}

	for (int i = 0; i < dynamicTiles.size(); i++) {
		dynamicTiles[i].sprite->update(deltaTime);
	}

	player->update(deltaTime);
	updateBombs(deltaTime);
	updateExplosions(deltaTime);

	for (size_t i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL) {
			toasters[i]->update(deltaTime);

			// --- DETECCIÓN DE COLISIÓN ---
			if (!player->isDying()) {
				glm::vec2 posPlayer = player->getPosition();
				glm::vec2 posEnemy = toasters[i]->getPosition();

				// Ajustamos el tamaño real del impacto ignorando píxeles transparentes
				int hitBoxSize = 20;
				int offset = 6;

				bool collisionX = (posPlayer.x + offset < posEnemy.x + offset + hitBoxSize) &&
					(posPlayer.x + offset + hitBoxSize > posEnemy.x + offset);

				bool collisionY = (posPlayer.y + offset < posEnemy.y + offset + hitBoxSize) &&
					(posPlayer.y + offset + hitBoxSize > posEnemy.y + offset);

				// Si colisiona con esta tostadora en concreto
				if (collisionX && collisionY) {
					player->die();
				}
			}
		}
	}

	// With no enemies left the door opens, and walking into it clears the level
	if (!doorOpen && !enemiesLeft())
		openDoor();
	else if (doorOpen)
		doorOpenTime += deltaTime;
	if (doorOpen && !player->isDying() && playerAtDoor())
		levelCleared = true;

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

	return SCENE_GAME;
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
}

void GameScene::destroyBox(const glm::ivec2 &tile)
{
	for (int i = 0; i < dynamicTiles.size(); i++) {
		if (dynamicTiles[i].tile == tile && dynamicTiles[i].id == TILE_BOX) {
			// TODO: breaking animation and power-ups
			delete dynamicTiles[i].sprite;
			dynamicTiles.erase(dynamicTiles.begin() + i);
			map->setTile(tile.x, tile.y, 0);
			return;
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
		for (int t = 0; t < toasters.size(); t++) {
			if (toasters[t] != NULL && explosions[i]->hits(toasters[t]->getPosition())) {
				delete toasters[t]; // TODO: death animation
				toasters[t] = NULL;
			}
		}
		if (!player->isDying() && explosions[i]->hits(player->getPosition()))
			player->die(); // TODO: skip in god mode
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
	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL) {
			delete toasters[i];
			toasters[i] = NULL;
		}
	}
}

bool GameScene::enemiesLeft() const
{
	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL)
			return true;
	}
	return false;
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
}

bool GameScene::playerAtDoor() const
{
	// The center of the chef must be inside the door tile
	glm::ivec2 posPlayer = glm::ivec2(player->getPosition());
	int tileSize = map->getTileSize();

	return (posPlayer.x + tileSize / 2) / tileSize == doorTile.x && (posPlayer.y + tileSize / 2) / tileSize == doorTile.y;
}

void GameScene::renderLevelCleared()
{
	ui.begin();
	ui.fillScreen(glm::vec4(0.f, 0.f, 0.f, 0.5f)); // Darken the level
	ui.drawBox(8.f, 60.f, UI_WIDTH - 16.f, 40.f);
	ui.drawTextCentered("STAGE CLEAR!", 74.f, UI_COLOR_DARK, 2);
	ui.drawTextCentered("STAGE CLEAR!", 73.f, UI_COLOR_RED, 2);
	ui.end();
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

int GameScene::updatePause(int deltaTime)
{
	// ESC closes the pause menu, like choosing RESUME
	if (Game::instance().getKeyDown(GLFW_KEY_ESCAPE)) {
		paused = false;
		return SCENE_GAME;
	}

	switch (pauseMenu.update(deltaTime)) {
	case PAUSE_CONTINUE:
		paused = false;
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

	for (int i = 0; i < bombs.size(); i++) {
		bombs[i]->render();
	}

	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL) toasters[i]->render();
	}

	for (int i = 0; i < explosions.size(); i++) {
		explosions[i]->render();
	}

	// The chef goes on top of the enemies so the death animation is always visible.
	// Once he goes through the door he is not drawn anymore
	if (!levelCleared)
		player->render();

	// The band goes after everything of the level, so it covers what scrolls behind it
	hud.render(glm::vec2(cameraX, cameraY), timeLeft, Game::instance().getLives(), maxBombs, fireRange);

	if (doorOpen && !levelCleared && doorOpenTime < EXIT_MESSAGE_TIME)
		renderExitMessage();
	if (levelCleared)
		renderLevelCleared();
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



