#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "GameScene.h"
#include "Game.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2


// Options of the pause menu, in the order they are shown
enum PauseOption { PAUSE_CONTINUE, PAUSE_RESTART, PAUSE_MAIN_MENU };


GameScene::GameScene()
{
	map = NULL;
	player = NULL;
	paused = false;
	for (int i = 0; i < 3; i++) {
		toasters.push_back(NULL);
	}
}

GameScene::~GameScene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;

	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL)
			delete toasters[i];
	}

	for (int i = 0; i < dynamicTiles.size(); i++) {
		if (dynamicTiles[i] != NULL) {
			delete dynamicTiles[i];
		}
	}
	dynamicTiles.clear();
}

void GameScene::initDynamicTiles() {
	// 1. Cargar la hoja de sprites
	dynamicSpritesheet.loadFromFile("images/kitchen.png", TEXTURE_PIXEL_FORMAT_RGBA);

	// 2. Cargamos el txt dinámico temporalmente para leer sus posiciones
	TileMap* dynMap = TileMap::createTileMap("levels/level1-dynamic.txt", glm::vec2(0.0f, 0.0f), texProgram);

	glm::ivec2 mapTiles = map->getMapSize();
	int tileSize = map->getTileSize();
	float step = 1.0f / 8.0f; // Ajusta esto según las columnas de tu imagen de bloques

	// 3. Escanear la cuadrícula buscando cajas y puertas
	for (int y = 0; y < mapTiles.y; y++) {
		for (int x = 0; x < mapTiles.x; x++) {

			int tileId = dynMap->getTile(x, y);

			if (tileId == 12 || tileId == 39) {

				// Creamos el Sprite y lo guardamos en el vector
				Sprite* dynSprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(step, step), &dynamicSpritesheet, &texProgram);
				dynSprite->setPosition(glm::vec2(x * tileSize, y * tileSize));

				dynSprite->setNumberAnimations(2); // Animación 0: Intacto/Cerrado | 1: Roto/Abierto
				dynSprite->setAnimationSpeed(0, 8);
				dynSprite->setAnimationSpeed(1, 8);

				if (tileId == 12) { // Caja
					
					// Animación 0: Caja INTACTA
					dynSprite->setAnimationSpeed(0, 8);
					dynSprite->addKeyframe(0, glm::vec2(3.f * step, 1.f * step));

					// Animación 1: Caja ROTA (Si tienes un sprite de caja rota)
					//dynSprite->setAnimationSpeed(1, 8);
					//dynSprite->addKeyframe(1, glm::vec2(columna_caja_rota * step, fila_caja_rota * step));

					dynSprite->changeAnimation(0); // Que empiece intacta
				}
				else if (tileId == 39) { // Puerta
					
					// Animación 0: Puerta CERRADA
					dynSprite->setAnimationSpeed(0, 8);
					dynSprite->addKeyframe(0, glm::vec2(5.f * step, 2.f * step));

					// Animación 1: Puerta ABIERTA
					// dynSprite->setAnimationSpeed(1, 8);
					// dynSprite->addKeyframe(1, glm::vec2(columna_puerta_abierta * step, fila_puerta_abierta * step));

					dynSprite->changeAnimation(0); // Que empiece cerrada
				}


				dynamicTiles.push_back(dynSprite);

				// ESTRATEGIA CLAVE: Le pasamos el bloque al mapa principal estático.
				// Esto hace que el motor de físicas detecte la caja como un muro y 
				// el cocinero no pueda atravesarla caminando.
				map->setTile(x, y, tileId);
			}
		}
	}

	// 4. Borramos el mapa temporal de la memoria porque ya los tenemos como Sprites
	delete dynMap;
}

void GameScene::init()
{
	initShaders();
	ui.init();
	pauseMenu.init({ "RESUME", "RESTART", "MAIN MENU" });
	map = TileMap::createTileMap("levels/level1.txt", glm::vec2(0.0f, 0.0f), texProgram);

	initDynamicTiles();

	player = new Player();
	player->init(glm::ivec2(0.0f, 0.0f), texProgram);
	player->setTileMap(map);


	for (int i = 0; i < toasters.size(); i++) {
		toasters[i] = new Toaster();
		toasters[i]->init(glm::ivec2(0.0f, 0.0f), texProgram);
		toasters[i]->setTileMap(map);
	}
	
	cameraWidth = 416.0f;  // Lo que mide tu "lente"
	cameraHeight = 416.0f;
	
	// Calculamos el tamaño total del mapa a partir del TileMap cargado
	glm::ivec2 mapTiles = map->getMapSize();
	float mapPixelWidth = float(mapTiles.x * map->getTileSize());
	float mapPixelHeight = float(mapTiles.y * map->getTileSize());

	// Centrar la cámara horizontalmente: la mitad del mapa menos la mitad de la cámara
	cameraX = 0.0f;

	// Centrar la cámara abajo:
	// Como OpenGL pone el (0,0) abajo a la izquierda, la cámara en Y = 0 enfocará el suelo.
	cameraY = mapPixelHeight - cameraHeight;
	projection = glm::ortho(cameraX, cameraX + cameraWidth, cameraY + cameraHeight, cameraY);

	
	int playerX = 7;
	int playerY = 10;

	player->setPosition(glm::vec2(playerX * map->getTileSize(), playerY * map->getTileSize()));

	int toaster01X = 2; // Que aparezca en el bloque 2 hacia la derecha
	int toaster01Y = 10;     // Que aparezca en el bloque 10 desde arriba (déjalo caer desde un poco alto)

	toasters[0]->setPosition(glm::vec2(toaster01X * map->getTileSize(), toaster01Y * map->getTileSize()));

	int toaster02X = 7; // Que aparezca en el bloque 5 hacia la derecha
	int toaster02Y = 5;     // Que aparezca en el bloque 10 desde arriba (déjalo caer desde un poco alto)

	toasters[1]->setPosition(glm::vec2(toaster02X * map->getTileSize(), toaster02Y * map->getTileSize()));
	
	int toaster03X = 10; // Que aparezca en el bloque 12 hacia la derecha
	int toaster03Y = 1;     // Que aparezca en el bloque 5 desde arriba (déjalo caer desde un poco alto)

	toasters[2]->setPosition(glm::vec2(toaster03X * map->getTileSize(), toaster03Y * map->getTileSize()));
	
	currentTime = 0.0f;
}

int GameScene::update(int deltaTime)
{
	if (paused)
		return updatePause(deltaTime);

	// While the chef dies the rest of the level (enemies, camera) stays frozen
	if (player->isDying()) {
		player->update(deltaTime);
		if (player->deathFinished())
			return SCENE_GAME_OVER;
		return SCENE_GAME;
	}

	// START (ENTER) or ESC pauses the game
	if (Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE)) {
		paused = true;
		pauseMenu.reset();
		return SCENE_GAME;
	}

	currentTime += deltaTime;

	for (int i = 0; i < dynamicTiles.size(); i++) {
		dynamicTiles[i]->update(deltaTime);
	}

	player->update(deltaTime);

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

	// 1. Obtener el centro exacto del jugador (sumando la mitad de sus 32 píxeles)
	glm::vec2 playerPos = player->getPosition();
	float playerCenterX = playerPos.x + 16.0f;
	float playerCenterY = playerPos.y + 16.0f;

	// 2. Centrar la cámara matemáticamente sobre ese punto
	cameraX = playerCenterX - (cameraWidth / 2.0f);
	cameraY = playerCenterY - (cameraHeight / 2.0f);

	// 3. TOPES DE LOS BORDES (Clamping)
	glm::ivec2 mapTiles = map->getMapSize();
	float maxCameraX = (mapTiles.x * map->getTileSize()) - cameraWidth;
	float maxCameraY = (mapTiles.y * map->getTileSize()) - cameraHeight;

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
	ui.drawRect(0.f, 0.f, UI_WIDTH, UI_HEIGHT, glm::vec4(0.f, 0.f, 0.f, 0.5f)); // Darken the level
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
		dynamicTiles[i]->render();
	}

	for (int i = 0; i < toasters.size(); i++) {
		if (toasters[i] != NULL) toasters[i]->render();
	}

	// The chef goes on top of the enemies so the death animation is always visible
	player->render();

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



