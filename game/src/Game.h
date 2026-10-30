#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"


#define SCREEN_WIDTH 720 // 480 x 520 logical pixels at x1.5: the level (480 x 480) and the HUD band below
#define SCREEN_HEIGHT 780

#define START_LIVES 3
#define MAX_LIVES 9 // 'Extra life' does not go further, so the HUD shows a single digit
#define START_BOMBS 1
#define START_FIRE_RANGE 1


// What the power-ups have improved. It goes on to the next level; when the chef
// dies the level starts again with what he had when he entered it
struct ChefStats
{
	int bombs;     // Simultaneous bombs ('Bomb Up')
	int fireRange; // Tiles the fire reaches in each direction ('Fire Up')
	bool heart;    // The next hit only takes the 'Heart' away
};


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() : scene(NULL), currentScene(-1), currentLevel(1), lives(START_LIVES), stats({ START_BOMBS, START_FIRE_RANGE, false }) {}
	~Game() { delete scene; }
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;     // Is the key held down? (movement)
	bool getKeyDown(int key) const; // Was the key pressed this frame? (menus)

	int getLives() const { return lives; }
	void addLife() { if (lives < MAX_LIVES) lives++; } // 'Extra life' power-up
	void setLevel(int level) { currentLevel = level; } // Used by the keys that jump to a level
	ChefStats getStats() const { return stats; }
	void setStats(const ChefStats &stats) { this->stats = stats; } // Saved by GameScene when the level is cleared

private:
	void changeScene(int sceneId);

private:
	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that
							    // we can have access at any time
	bool keysDown[GLFW_KEY_LAST+1]; // Keys pressed since the last update
	Scene *scene; // Active scene, Game delegates to it
	int currentScene; // SceneId of the active scene
	int currentLevel; // Level loaded by GameScene (1..NUM_LEVELS, see Levels.h)
	int lives;        // Lives of the chef; they last between levels until the game ends
	ChefStats stats;  // Power-ups the chef had when the current level started

};


#endif // _GAME_INCLUDE


