#ifndef _UI_RENDERER_INCLUDE
#define _UI_RENDERER_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "Game.h"


// Logical resolution of the UI. Every UI pixel is drawn as a block of
// (window size / UI size) real pixels, which gives the retro look.
// The window is taller than wide, so the 160x160 layout is centered vertically and
// UI_MARGIN_Y rows are left above and below it (only fillScreen draws there).
#define UI_WIDTH 160.f
#define UI_HEIGHT 160.f
#define UI_MARGIN_Y ((UI_WIDTH * SCREEN_HEIGHT / SCREEN_WIDTH - UI_HEIGHT) / 2.f)

#define UI_CHAR_WIDTH 6   // 5 px glyph + 1 px spacing
#define UI_LINE_HEIGHT 10 // 7 px glyph + 3 px spacing


// Palette shared by every screen
const glm::vec4 UI_COLOR_DARK(0.16f, 0.07f, 0.06f, 1.f);
const glm::vec4 UI_COLOR_RED(0.75f, 0.15f, 0.10f, 1.f);
const glm::vec4 UI_COLOR_ORANGE(0.95f, 0.55f, 0.20f, 1.f);
const glm::vec4 UI_COLOR_LIGHT(0.98f, 0.92f, 0.78f, 1.f);
const glm::vec4 UI_COLOR_BLACK(0.f, 0.f, 0.f, 1.f);


// UIRenderer draws flat rectangles, images and text with a built-in 5x7 pixel
// font. It is meant for menus and the HUD.


class UIRenderer
{

public:
	UIRenderer();
	~UIRenderer();

	void init();

	// Every draw call must be between begin() and end()
	void begin();
	void end();

	void drawRect(float x, float y, float width, float height, const glm::vec4 &color);
	void fillScreen(const glm::vec4 &color); // The whole window, margins included
	void drawBox(float x, float y, float width, float height); // Panel with border
	void drawBlackBox(float x, float y, float width, float height); // Black panel with an orange line and cut corners
	void drawImage(const Texture &image, float x, float y, float width, float height);
	void drawScreenImage(const Texture &image); // The whole window, margins included
	// scale enlarges the font; scaleY (if > 0) uses a different vertical scale for tall letters
	void drawText(const std::string &text, float x, float y, const glm::vec4 &color, int scale = 1, int scaleY = 0);
	void drawTextCentered(const std::string &text, float y, const glm::vec4 &color, int scale = 1, int scaleY = 0);

	static float textWidth(const std::string &text, int scale = 1);

	// Loads an image for the UI with sharp pixels (no filtering)
	static bool loadImage(Texture &image, const std::string &filename);

private:
	void initShaders(ShaderProgram &shaderProgram, const std::string &vertexFile, const std::string &fragmentFile);

private:
	ShaderProgram program;    // Flat color (rectangles and text)
	ShaderProgram texProgram; // Textured (images)
	GLuint vao, vbo, texVao, texVbo;
	GLint posLocation, texPosLocation, texCoordLocation;
	bool initialized;

};


#endif // _UI_RENDERER_INCLUDE
