#include "HUD.h"


// Layout of images/hud.png (see tools/make_hud.py)
#define ATLAS_WIDTH 238.f
#define ATLAS_HEIGHT 56.f
#define ICON_WIDTH 34  // Icons in the first row: timer, hat, bomb, fire, shield and one per EnemyType
#define ICON_HEIGHT 32
#define GLYPH_WIDTH 18 // Characters in the second row: 0-9 and ':'
#define GLYPH_HEIGHT 24

// Layout of the band. Icons and glyphs touch the bottom of their cells in the atlas,
// so drawing every cell ending at BASELINE aligns all of them at the bottom
#define BASELINE (HUD_HEIGHT - 4)
#define TIME_X 8         // Timer icon; the countdown goes right after it
#define FIRST_SLOT_X 136 // Lives, bombs, fire and the enemies left
#define SLOT_WIDTH 68
#define SHIELD_X (FIRST_SLOT_X - 20) // Cell of the shield: it overlaps the hat a bit, like a badge
#define NUM_SLOTS 5      // The band has room for two kinds of enemy after lives, bombs and fire
#define TEXT_GAP 2       // Pixels between an icon and its number

#define HURRY_UP_TIME 30000 // ms left when the countdown turns red
#define TIME_FLASH_TIME 1000 // ms the countdown blinks green after getting time
#define TIME_FLASH_STEP 100  // ms green / ms white while it blinks


// The icon of each EnemyType is ICON_FIRST_ENEMY + type
enum HudIcon { ICON_TIMER, ICON_LIVES, ICON_BOMBS, ICON_FIRE, ICON_SHIELD, ICON_FIRST_ENEMY, NUM_ICONS = ICON_FIRST_ENEMY + NUM_ENEMY_TYPES };


HUD::HUD()
{
	program = NULL;
	band = NULL;
	timeFlash = 0;
	icon = NULL;
	glyph = NULL;
}

HUD::~HUD()
{
	if (band != NULL)
		delete band;
	if (icon != NULL)
		delete icon;
	if (glyph != NULL)
		delete glyph;
}

void HUD::init(ShaderProgram &shaderProgram)
{
	program = &shaderProgram;

	// GL_NEAREST keeps the pixel art sharp when the logical pixels are scaled up
	bandImage.loadFromFile("images/band.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bandImage.setMinFilter(GL_NEAREST);
	bandImage.setMagFilter(GL_NEAREST);
	atlas.loadFromFile("images/hud.png", TEXTURE_PIXEL_FORMAT_RGBA);
	atlas.setMinFilter(GL_NEAREST);
	atlas.setMagFilter(GL_NEAREST);

	band = Sprite::createSprite(glm::ivec2(480, HUD_HEIGHT), glm::vec2(1.f, 1.f), &bandImage, program);

	icon = Sprite::createSprite(glm::ivec2(ICON_WIDTH, ICON_HEIGHT), glm::vec2(ICON_WIDTH / ATLAS_WIDTH, ICON_HEIGHT / ATLAS_HEIGHT), &atlas, program);
	icon->setNumberAnimations(NUM_ICONS);
	for (int i = 0; i < NUM_ICONS; i++)
		icon->addKeyframe(i, glm::vec2(i * ICON_WIDTH / ATLAS_WIDTH, 0.f));

	glyph = Sprite::createSprite(glm::ivec2(GLYPH_WIDTH, GLYPH_HEIGHT), glm::vec2(GLYPH_WIDTH / ATLAS_WIDTH, GLYPH_HEIGHT / ATLAS_HEIGHT), &atlas, program);
	glyph->setNumberAnimations(11);
	for (int i = 0; i < 11; i++)
		glyph->addKeyframe(i, glm::vec2(i * GLYPH_WIDTH / ATLAS_WIDTH, ICON_HEIGHT / ATLAS_HEIGHT));
}

void HUD::update(int deltaTime)
{
	if (timeFlash > 0)
		timeFlash -= deltaTime;
}

void HUD::flashTime()
{
	timeFlash = TIME_FLASH_TIME;
}

void HUD::render(const glm::vec2 &camera, int timeLeft, int lives, bool heart, int bombs, int fireRange, const int enemiesLeft[NUM_ENEMY_TYPES])
{
	// The band follows the camera, always right below the playable area
	glm::vec2 origin = camera + glm::vec2(0.f, float(GAME_AREA_HEIGHT));
	band->setPosition(origin);
	band->render();

	// Countdown as m:ss, rounding up so it shows 0:00 only when the time is over
	int seconds = (timeLeft + 999) / 1000;
	std::string time = std::to_string(seconds / 60) + ":" + (seconds % 60 < 10 ? "0" : "") + std::to_string(seconds % 60);

	renderIcon(ICON_TIMER, origin + glm::vec2(TIME_X, 0.f));
	if (timeFlash > 0) {
		if ((timeFlash / TIME_FLASH_STEP) % 2 == 0)
			program->setUniform4f("color", 0.35f, 1.f, 0.35f, 1.f); // Green: time was added
	}
	else if (timeLeft < HURRY_UP_TIME)
		program->setUniform4f("color", 1.f, 0.2f, 0.15f, 1.f); // The white glyphs are tinted red
	renderText(time, origin + glm::vec2(TIME_X + ICON_WIDTH + TEXT_GAP, 0.f));
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);

	renderItem(ICON_LIVES, lives, origin + glm::vec2(FIRST_SLOT_X, 0.f));
	if (heart)
		renderIcon(ICON_SHIELD, origin + glm::vec2(SHIELD_X, 0.f)); // After the hat, so it goes on top
	renderItem(ICON_BOMBS, bombs, origin + glm::vec2(FIRST_SLOT_X + SLOT_WIDTH, 0.f));
	renderItem(ICON_FIRE, fireRange, origin + glm::vec2(FIRST_SLOT_X + 2 * SLOT_WIDTH, 0.f));

	// The rest of the slots: the enemies left of each kind that appears in the level
	for (int i = 0; i < int(enemyTypes.size()) && 3 + i < NUM_SLOTS; i++) {
		EnemyType type = enemyTypes[i];
		renderItem(ICON_FIRST_ENEMY + type, enemiesLeft[type], origin + glm::vec2(FIRST_SLOT_X + (3 + i) * SLOT_WIDTH, 0.f));
	}
}

void HUD::renderItem(int iconId, int value, const glm::vec2 &pos)
{
	renderIcon(iconId, pos);
	renderText(std::to_string(value), pos + glm::vec2(ICON_WIDTH + TEXT_GAP, 0.f));
}

// pos.x is the left of the cell and pos.y the top of the band
void HUD::renderIcon(int iconId, const glm::vec2 &pos)
{
	icon->changeAnimation(iconId);
	icon->setPosition(pos + glm::vec2(0.f, float(BASELINE - ICON_HEIGHT)));
	icon->render();
}

void HUD::renderText(const std::string &text, const glm::vec2 &pos)
{
	for (int i = 0; i < int(text.size()); i++) {
		glyph->changeAnimation(text[i] == ':' ? 10 : text[i] - '0');
		glyph->setPosition(pos + glm::vec2(float(i * GLYPH_WIDTH), float(BASELINE - GLYPH_HEIGHT)));
		glyph->render();
	}
}
