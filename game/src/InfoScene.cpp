#include "InfoScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"
#include "PowerUp.h"


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

// After the controls come the power-ups, POWER_UPS_PER_PAGE on each page, in the order of
// PowerUpType (their row in powerups.png). Each one is its box, its name in yellow and what
// it does in white: the name and the description must be at most 19 characters
struct PowerUpInfo
{
	const char *name, *description;
};

static const PowerUpInfo POWER_UP_INFO[NUM_POWER_UPS] = {
	{ "BOMB UP",    "+1 BOMB (MAX 8)" },
	{ "FIRE UP",    "+1 RANGE (MAX 4)" },
	{ "SHIELD",     "SURVIVES ONE HIT" },
	{ "EXTRA LIFE", "+1 LIFE (MAX 9)" },
	{ "TIMER",      "+30 SECONDS" },
	{ "FULL FIRE",  "MAXIMUM RANGE" },
};

// The lines over the power-ups of each page
static const char *POWER_UP_PAGE_HEADERS[] = {
	"POWER-UPS:\n"
	"HIDDEN IN THE BOXES.",
	"POWER-UPS:\n"
	"KEPT IN THE NEXT LEVEL.",
};

#define POWER_UPS_PER_PAGE 3
#define POWER_UP_PAGES ((NUM_POWER_UPS + POWER_UPS_PER_PAGE - 1) / POWER_UPS_PER_PAGE)
#define POWER_UP_SHEET_COLUMNS 2 // The box and the box shining (PowerUp.cpp)
#define POWER_UP_ICON_SIZE 20.f
#define POWER_UP_FIRST_ROW (PANEL_Y + 34.f)
#define POWER_UP_ROW_HEIGHT 26.f

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
	page = 0;
	numPages = (sceneId == SCENE_INSTRUCTIONS) ? 1 + POWER_UP_PAGES : 1; // The controls, then the power-ups
}

void InfoScene::init()
{
	ui.init();
	UIRenderer::loadImage(title, sceneId == SCENE_INSTRUCTIONS ? "images/ui/screen_title_how_to_play.png" : "images/ui/screen_title_credits.png");
	if(sceneId == SCENE_INSTRUCTIONS)
		UIRenderer::loadImage(powerUpIcons, "images/powerups.png");
}

int InfoScene::update(int deltaTime)
{
	currentTime += deltaTime;

	// LEFT/RIGHT turn the pages; ENTER or ESC (also X, BACKSPACE) go back from any page
	if(Game::instance().getKeyDown(GLFW_KEY_LEFT) && page > 0)
		changePage(page - 1);
	else if(Game::instance().getKeyDown(GLFW_KEY_RIGHT) && page < numPages - 1)
		changePage(page + 1);
	else if(Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE) ||
	        Game::instance().getKeyDown(GLFW_KEY_X) || Game::instance().getKeyDown(GLFW_KEY_BACKSPACE))
		return SCENE_MENU;
	return sceneId;
}

void InfoScene::changePage(int newPage)
{
	page = newPage;
	Audio::instance().playSound(SOUND_MENU_MOVE);
}

void InfoScene::render()
{
	ui.begin();
	ui.drawPaper();
	ui.drawTitle(title, 4.f);
	ui.drawPanel(PANEL_X, PANEL_Y, UI_WIDTH - 2.f * PANEL_X, PANEL_HEIGHT);

	if(page == 0)
		renderText(body, PANEL_Y + 9.f);
	else
		renderPowerUps(page - 1);

	bool blinkOn = int(currentTime / 500.f) % 2 == 0;
	if(numPages == 1)
	{
		if(blinkOn)
			ui.drawTextOutlinedCentered("ENTER: BACK", 150.f, UI_COLOR_YELLOW);
	}
	else
	{
		// The page on the left, with arrows only towards the pages there are, and the hint on the right
		std::string pageText = std::string(page > 0 ? "<" : " ") + " " + std::to_string(page + 1) + "/" +
		                       std::to_string(numPages) + " " + (page < numPages - 1 ? ">" : " ");
		ui.drawTextOutlined(pageText, PANEL_X + 4.f, 150.f, UI_COLOR_WHITE);
		if(blinkOn)
			ui.drawTextOutlined("ENTER: BACK", UI_WIDTH - PANEL_X - 4.f - UIRenderer::textWidth("ENTER: BACK"), 150.f, UI_COLOR_YELLOW);
	}
	ui.end();
}

void InfoScene::renderText(const std::string &text, float y)
{
	// Line by line: the headings in yellow
	size_t start = 0;
	while(start <= text.size())
	{
		size_t end = text.find('\n', start);
		std::string line = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
		bool heading = !line.empty() && line.back() == ':';
		ui.drawTextOutlined(line, PANEL_X + 8.f, y, heading ? UI_COLOR_YELLOW : UI_COLOR_WHITE);
		y += UI_LINE_HEIGHT;
		if(end == std::string::npos)
			break;
		start = end + 1;
	}
}

void InfoScene::renderPowerUps(int powerUpPage)
{
	renderText(POWER_UP_PAGE_HEADERS[powerUpPage], PANEL_Y + 9.f);

	float cellWidth = 1.f / POWER_UP_SHEET_COLUMNS, cellHeight = 1.f / NUM_POWER_UPS;
	for(int row = 0; row < POWER_UPS_PER_PAGE; row++)
	{
		int type = powerUpPage * POWER_UPS_PER_PAGE + row;
		if(type >= NUM_POWER_UPS)
			break;
		float y = POWER_UP_FIRST_ROW + row * POWER_UP_ROW_HEIGHT;
		// The plain box (first column of the sheet), as the chef finds it in the level
		ui.drawImageRegion(powerUpIcons, glm::vec4(0.f, type * cellHeight, cellWidth, (type + 1) * cellHeight),
		                   PANEL_X + 8.f, y, POWER_UP_ICON_SIZE, POWER_UP_ICON_SIZE);
		ui.drawTextOutlined(POWER_UP_INFO[type].name, PANEL_X + 32.f, y + 1.f, UI_COLOR_YELLOW);
		ui.drawTextOutlined(POWER_UP_INFO[type].description, PANEL_X + 32.f, y + 11.f, UI_COLOR_WHITE);
	}
}
