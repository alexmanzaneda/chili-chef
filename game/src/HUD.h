#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "Sprite.h"
#include "Enemy.h"


#define GAME_AREA_HEIGHT 480 // Logical pixels of the level shown above the band
#define HUD_HEIGHT 40        // Logical pixels of the band


// HUD draws the dark band at the bottom of the screen: the countdown, the lives (with
// the shield of the 'Heart' when he has it), the bombs, the fire range and the enemies
// left of each kind in the level. It is drawn in the coordinates of the level, right
// below the camera's playable area, so it must be rendered after the level.


class HUD
{

public:
	HUD();
	~HUD();

	void init(ShaderProgram &shaderProgram);
	// Kinds of enemy of the level, in the order they are shown (the ones that fit in the band)
	void setEnemyTypes(const std::vector<EnemyType> &types) { enemyTypes = types; }
	// camera is the top-left corner of the camera; timeLeft is in ms;
	// enemiesLeft has the enemies left of each EnemyType
	void render(const glm::vec2 &camera, int timeLeft, int lives, bool heart, int bombs, int fireRange, const int enemiesLeft[NUM_ENEMY_TYPES]);

private:
	void renderItem(int icon, int value, const glm::vec2 &pos); // Icon and its number
	void renderIcon(int icon, const glm::vec2 &pos);
	void renderText(const std::string &text, const glm::vec2 &pos);

private:
	ShaderProgram *program;
	Texture bandImage, atlas; // images/band.png and images/hud.png (tools/make_hud.py)
	Sprite *band;
	Sprite *icon;  // One animation per icon of the atlas
	Sprite *glyph; // One animation per character: 0-9 and ':'
	std::vector<EnemyType> enemyTypes;

};


#endif // _HUD_INCLUDE
