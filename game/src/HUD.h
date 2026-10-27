#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "Sprite.h"


#define GAME_AREA_HEIGHT 480 // Logical pixels of the level shown above the band
#define HUD_HEIGHT 40        // Logical pixels of the band


// HUD draws the dark band at the bottom of the screen: the countdown, the lives,
// the bombs and the fire range. It is drawn in the coordinates of the level,
// right below the camera's playable area, so it must be rendered after the level.


class HUD
{

public:
	HUD();
	~HUD();

	void init(ShaderProgram &shaderProgram);
	// camera is the top-left corner of the camera; timeLeft is in ms
	void render(const glm::vec2 &camera, int timeLeft, int lives, int bombs, int fireRange);

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

};


#endif // _HUD_INCLUDE
