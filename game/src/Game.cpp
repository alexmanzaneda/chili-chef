#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "MenuScene.h"
#include "GameScene.h"
#include "InfoScene.h"
#include "GameOverScene.h"
#include "LifeLostScene.h"
#include "Levels.h"


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	changeScene(SCENE_MENU);
}

bool Game::update(int deltaTime)
{
	int nextScene = scene->update(deltaTime);
	if(nextScene != currentScene)
		changeScene(nextScene);

	// A key press is only reported during the update that follows it
	for(int i = 0; i <= GLFW_KEY_LAST; i++)
		keysDown[i] = false;

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->render();
}

void Game::changeScene(int sceneId)
{
	if(sceneId == SCENE_EXIT)
	{
		bPlay = false;
		return;
	}
	if(sceneId == SCENE_RESTART)
		sceneId = currentScene;
	if(sceneId == SCENE_LIFE_LOST && --lives == 0)
		sceneId = SCENE_GAME_OVER;
	if(sceneId == SCENE_LEVEL_CLEARED)
	{
		currentLevel++;
		sceneId = (currentLevel > NUM_LEVELS) ? SCENE_VICTORY : SCENE_GAME;
	}
	else if(sceneId == SCENE_GAME && currentScene == SCENE_MENU)
	{
		// START always begins a new game at the first level
		currentLevel = 1;
		lives = START_LIVES;
		stats = { START_BOMBS, START_FIRE_RANGE };
	}

	delete scene;
	switch(sceneId)
	{
	case SCENE_MENU:
		scene = new MenuScene(currentScene);
		break;
	case SCENE_GAME:
		scene = new GameScene(currentLevel);
		break;
	case SCENE_INSTRUCTIONS:
	case SCENE_CREDITS:
		scene = new InfoScene(sceneId);
		break;
	case SCENE_LIFE_LOST:
		scene = new LifeLostScene(lives);
		break;
	case SCENE_GAME_OVER:
	case SCENE_VICTORY:
		scene = new GameOverScene(sceneId);
		break;
	}
	currentScene = sceneId;
	scene->init();
}

void Game::keyPressed(int key)
{
	if(key < 0) // GLFW_KEY_UNKNOWN
		return;
	keys[key] = true;
	keysDown[key] = true;
}

void Game::keyReleased(int key)
{
	if(key < 0)
		return;
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}

bool Game::getKeyDown(int key) const
{
	return keysDown[key];
}



