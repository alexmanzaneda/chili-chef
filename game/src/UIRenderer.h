#ifndef _UI_RENDERER_INCLUDE
#define _UI_RENDERER_INCLUDE


#include <string>
#include <vector>
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
const glm::vec4 UI_COLOR_NAVY(0.01f, 0.01f, 0.12f, 1.f);     // Outline of the logo and the frame of the menu image
const glm::vec4 UI_COLOR_SAGE(0.34f, 0.40f, 0.31f, 1.f);     // Line of the frame of the menu image
const glm::vec4 UI_COLOR_SHADOW(0.78f, 0.77f, 0.83f, 1.f);   // Soft shadow over the paper of the menu image
const glm::vec4 UI_COLOR_WHITE(1.f, 1.f, 1.f, 1.f);
const glm::vec4 UI_COLOR_PAPER(0.98f, 0.95f, 0.894f, 1.f);  // Behind the screens that are not in a world
const glm::vec4 UI_COLOR_YELLOW(1.f, 0.82f, 0.25f, 1.f);     // Selected option and headings
const glm::vec4 UI_COLOR_OUTLINE(0.04f, 0.03f, 0.09f, 1.f);  // Around the text, like the digits of the HUD
// The plate of the HUD band (band.png): edge, light top-left, body and dark bottom-right
const glm::vec4 UI_COLOR_PANEL_EDGE(0.f, 0.f, 0.f, 1.f);
const glm::vec4 UI_COLOR_PANEL_LIGHT(0.76f, 0.76f, 0.77f, 1.f);
const glm::vec4 UI_COLOR_PANEL(0.46f, 0.47f, 0.49f, 1.f);
const glm::vec4 UI_COLOR_PANEL_DARK(0.25f, 0.26f, 0.27f, 1.f);

#define UI_PIXEL (UI_WIDTH / SCREEN_WIDTH) // UI units of a pixel of the window


struct WorldInfo;


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
	void drawFrameBox(float x, float y, float width, float height); // Panel with the frame of the menu image, cut corners and a shadow
	void drawPanel(float x, float y, float width, float height);    // Plate like the HUD band: the panels of every screen
	void drawImage(const Texture &image, float x, float y, float width, float height);
	// Only the part (u0, v0, u1, v1) of the texture, in texture coordinates
	void drawImageRegion(const Texture &image, const glm::vec4 &region, float x, float y, float width, float height);
	void drawScreenImage(const Texture &image); // The whole window, margins included
	// The floor of the world in every tile of the window, as big as in the levels, under a white veil:
	// the background of the screens
	void drawBackdrop(const WorldInfo &world);
	// The paper of the screens that are not in a world (menu, credits...): plain UI_COLOR_PAPER
	void drawPaper();
	// A title of tools/title_font.py centered at y, with a pixel of it every 3 pixels of the window
	void drawTitle(const Texture &title, float y);
	// scale enlarges the font (it can be fractional: 2/3 gives pixels of 3 px of window);
	// scaleY (if > 0) uses a different vertical scale for tall letters
	void drawText(const std::string &text, float x, float y, const glm::vec4 &color, float scale = 1.f, float scaleY = 0.f);
	void drawTextCentered(const std::string &text, float y, const glm::vec4 &color, float scale = 1.f, float scaleY = 0.f);
	// The same with a dark outline and a shadow down and to the right, like the digits of the HUD
	void drawTextOutlined(const std::string &text, float x, float y, const glm::vec4 &color, float scale = 1.f);
	void drawTextOutlinedCentered(const std::string &text, float y, const glm::vec4 &color, float scale = 1.f);

	static float textWidth(const std::string &text, float scale = 1.f);

	// Loads an image for the UI with sharp pixels (no filtering)
	static bool loadImage(Texture &image, const std::string &filename);

private:
	void initShaders(ShaderProgram &shaderProgram, const std::string &vertexFile, const std::string &fragmentFile);
	// The text is drawn in batches: its rectangles are collected and then drawn with a single call.
	// With outline, the rectangles of the outline of drawTextOutlined instead of the letters
	void addTextRects(std::vector<float> &vertices, const std::string &text, float x, float y, float scale, float scaleY, bool outline = false);
	void drawRects(const std::vector<float> &vertices, const glm::vec4 &color);

private:
	ShaderProgram program;    // Flat color (rectangles and text)
	ShaderProgram texProgram; // Textured (images)
	GLuint vao, vbo, texVao, texVbo;
	GLuint textVao, textVbo; // Rectangles of the text, refilled by every drawText
	GLint posLocation, texPosLocation, texCoordLocation, textPosLocation;
	bool initialized;
	Texture backdrop;          // Tilesheet of the last world drawn by drawBackdrop
	std::string backdropFile;

};


#endif // _UI_RENDERER_INCLUDE
