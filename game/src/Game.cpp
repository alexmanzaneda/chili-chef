#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include "Game.h"
#include "UIRenderer.h"
#include "MenuScene.h"
#include "GameScene.h"
#include "InfoScene.h"
#include "GameOverScene.h"
#include "LifeLostScene.h"
#include "LevelIntroScene.h"
#include "WorldClearScene.h"
#include "Levels.h"
#include "Audio.h"


Game::~Game()
{
	delete scene;
	delete overlay;
}

void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	Audio::instance().init();
	overlay = new UIRenderer();
	overlay->init();
	changeScene(SCENE_MENU);
	transition = TRANSITION_IN;
}

bool Game::update(int deltaTime)
{
	// The scenes stay frozen during a transition, so their timers do not lose
	// that time and the keys pressed meanwhile are ignored
	if(transition != TRANSITION_NONE)
		updateTransition(deltaTime);
	else
	{
		int newScene = scene->update(deltaTime);
		if(newScene == SCENE_EXIT)
			changeScene(newScene);
		else if(newScene != currentScene)
		{
			nextScene = newScene;
			transition = TRANSITION_OUT;
			transitionTime = 0;
		}
	}

	// A key press is only reported during the update that follows it
	for(int i = 0; i <= GLFW_KEY_LAST; i++)
		keysDown[i] = false;

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->render();
	if(transition != TRANSITION_NONE)
		renderTransition();
}

void Game::updateTransition(int deltaTime)
{
	// Loading a scene makes the next frame very long: it must not skip the fade
	transitionTime += std::min(deltaTime, 33);

	if(transition == TRANSITION_OUT && transitionTime >= TRANSITION_OUT_TIME)
	{
		changeScene(nextScene);
		transition = TRANSITION_IN;
		transitionTime = 0;
	}
	else if(transition == TRANSITION_IN && transitionTime >= TRANSITION_IN_TIME)
		transition = TRANSITION_NONE;
}

void Game::renderTransition()
{
	float white = (transition == TRANSITION_OUT) ? float(transitionTime) / TRANSITION_OUT_TIME
	                                             : 1.f - float(transitionTime) / TRANSITION_IN_TIME;

	overlay->begin();
	overlay->fillScreen(glm::vec4(1.f, 1.f, 1.f, std::ceil(white * TRANSITION_STEPS) / TRANSITION_STEPS));
	overlay->end();
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
	bool enteringLevel = false; // Repeating it after losing a life does not show its intro again
	// The last level of a world shows its gem first; that scene comes back here to go on
	if(sceneId == SCENE_LEVEL_CLEARED && currentScene == SCENE_GAME
		&& (currentLevel == NUM_LEVELS || LEVELS[currentLevel].world != LEVELS[currentLevel - 1].world))
		sceneId = SCENE_WORLD_CLEAR;
	else if(sceneId == SCENE_LEVEL_CLEARED)
	{
		currentLevel++;
		sceneId = (currentLevel > NUM_LEVELS) ? SCENE_VICTORY : SCENE_GAME;
		enteringLevel = true;
	}
	else if(sceneId == SCENE_GAME && currentScene == SCENE_MENU)
	{
		// START always begins a new game at the first level
		currentLevel = 1;
		lives = START_LIVES;
		stats = { START_BOMBS, START_FIRE_RANGE, false };
		godMode = false;
		enteringLevel = true;
	}
	if(enteringLevel && sceneId == SCENE_GAME && LEVELS[currentLevel - 1].introScreen != NULL)
		sceneId = SCENE_LEVEL_INTRO;

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
	case SCENE_LEVEL_INTRO:
		scene = new LevelIntroScene(currentLevel);
		break;
	case SCENE_WORLD_CLEAR:
		scene = new WorldClearScene(currentLevel);
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



