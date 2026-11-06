#include "InfoScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


#define PANEL_X 3.f
#define PANEL_Y 30.f
#define PANEL_HEIGHT 114.f

// Lines must be at most 23 characters to fit the panel. The lines that end with ':' are
// headings, drawn in yellow

static const char *INSTRUCTIONS_TEXT =
	"GOAL:\n"
	"DEFEAT ALL THE ENEMIES\n"
	"AND REACH THE DOOR\n"
	"BEFORE TIME RUNS OUT.\n"
	"\n"
	"CONTROLS:\n"
	"LEFT/RIGHT  MOVE\n"
	"UP          JUMP\n"
	"SPACE       BOMB\n"
	"ENTER       PAUSE";

static const char *CREDITS_TEXT =
	"PROGRAMMING AND ART:\n"
	"ALEX MANZANEDA\n"
	"SARA VIDAL\n"
	"\n"
	"BASED ON:\n"
	"POCKET BOMBERMAN 1998\n"
	"\n"
	"MADE FOR:\n"
	"VIDEOJOCS - FIB UPC\n"
	"2026/27";


InfoScene::InfoScene(int sceneId)
{
	this->sceneId = sceneId;
	currentTime = 0.f;
	body = (sceneId == SCENE_INSTRUCTIONS) ? INSTRUCTIONS_TEXT : CREDITS_TEXT;
}

void InfoScene::init()
{
	ui.init();
	UIRenderer::loadImage(title, sceneId == SCENE_INSTRUCTIONS ? "images/ui/screen_title_how_to_play.png" : "images/ui/screen_title_credits.png");
}

int InfoScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE) ||
	   Game::instance().getKeyDown(GLFW_KEY_X) || Game::instance().getKeyDown(GLFW_KEY_BACKSPACE))
		return SCENE_MENU;
	return sceneId;
}

void InfoScene::render()
{
	ui.begin();
	ui.drawPaper();
	ui.drawTitle(title, 4.f);
	ui.drawPanel(PANEL_X, PANEL_Y, UI_WIDTH - 2.f * PANEL_X, PANEL_HEIGHT);

	// The text line by line: the headings in yellow
	float y = PANEL_Y + 9.f;
	size_t start = 0;
	while(start <= body.size())
	{
		size_t end = body.find('\n', start);
		std::string line = body.substr(start, end == std::string::npos ? std::string::npos : end - start);
		bool heading = !line.empty() && line.back() == ':';
		ui.drawTextOutlined(line, PANEL_X + 8.f, y, heading ? UI_COLOR_YELLOW : UI_COLOR_WHITE);
		y += UI_LINE_HEIGHT;
		if(end == std::string::npos)
			break;
		start = end + 1;
	}

	if(int(currentTime / 500.f) % 2 == 0)
		ui.drawTextOutlinedCentered("ENTER: BACK", 150.f, UI_COLOR_YELLOW);
	ui.end();
}
