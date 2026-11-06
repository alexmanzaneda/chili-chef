#include <iostream>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include "UIRenderer.h"
#include "Levels.h"


// 5x7 pixel font. Each glyph is 7 rows; bit 4 of each row is the leftmost pixel.

static const unsigned char FONT_LETTERS[26][7] = {
	{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // A
	{0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
	{0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
	{0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}, // D
	{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
	{0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
	{0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}, // G
	{0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
	{0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // I
	{0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // J
	{0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
	{0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
	{0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // M
	{0x11,0x11,0x19,0x15,0x13,0x11,0x11}, // N
	{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
	{0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
	{0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
	{0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
	{0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, // S
	{0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
	{0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
	{0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, // V
	{0x11,0x11,0x11,0x15,0x15,0x15,0x0A}, // W
	{0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
	{0x11,0x11,0x11,0x0A,0x04,0x04,0x04}, // Y
	{0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}  // Z
};

static const unsigned char FONT_DIGITS[10][7] = {
	{0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
	{0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, // 1
	{0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}, // 2
	{0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}, // 3
	{0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 4
	{0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, // 5
	{0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, // 6
	{0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 7
	{0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
	{0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}  // 9
};

static const unsigned char FONT_SYMBOLS[][8] = {
	// First byte is the character, the other 7 are the rows
	{'.', 0x00,0x00,0x00,0x00,0x00,0x0C,0x0C},
	{',', 0x00,0x00,0x00,0x00,0x0C,0x04,0x08},
	{'!', 0x04,0x04,0x04,0x04,0x04,0x00,0x04},
	{'?', 0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
	{':', 0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00},
	{'-', 0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
	{'>', 0x08,0x04,0x02,0x01,0x02,0x04,0x08},
	{'<', 0x02,0x04,0x08,0x10,0x08,0x04,0x02},
	{'/', 0x01,0x01,0x02,0x04,0x08,0x10,0x10},
	{'(', 0x02,0x04,0x08,0x08,0x08,0x04,0x02},
	{')', 0x08,0x04,0x02,0x02,0x02,0x04,0x08},
	{'\'', 0x04,0x04,0x08,0x00,0x00,0x00,0x00}
};


// Unit square with texture coordinates, for drawImage. drawImageRegion changes them for a while
static const float TEX_VERTICES[24] = {0.f, 0.f, 0.f, 0.f,  1.f, 0.f, 1.f, 0.f,  1.f, 1.f, 1.f, 1.f,
                                       0.f, 0.f, 0.f, 0.f,  1.f, 1.f, 1.f, 1.f,  0.f, 1.f, 0.f, 1.f};

// Where the outline of drawTextOutlined is drawn, in pixels of the font: around every pixel
// and one more to the right and down, as a shadow
static const int OUTLINE_OFFSETS[][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1},
                                         {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}};

#define TITLE_PIXEL 3.f   // Pixels of the window per pixel of the title images
#define BACKDROP_TILE 48.f // Pixels of the window per tile: the 32 px tiles at x1.5, like the levels
#define PANEL_BORDER 3.f   // Pixels of the window of each line of the panels, like the HUD band at x1.5


// Returns the 7 rows of a character, or NULL if it has no pixels (space, unknown)
static const unsigned char *glyph(char c)
{
	c = char(toupper((unsigned char)c));
	if(c >= 'A' && c <= 'Z')
		return FONT_LETTERS[c - 'A'];
	if(c >= '0' && c <= '9')
		return FONT_DIGITS[c - '0'];
	for(int i = 0; i < int(sizeof(FONT_SYMBOLS) / sizeof(FONT_SYMBOLS[0])); i++)
		if(FONT_SYMBOLS[i][0] == c)
			return &FONT_SYMBOLS[i][1];
	return NULL;
}


UIRenderer::UIRenderer()
{
	initialized = false;
}

UIRenderer::~UIRenderer()
{
	if(initialized)
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &texVao);
		glDeleteBuffers(1, &texVbo);
		program.free();
		texProgram.free();
	}
}

void UIRenderer::init()
{
	initShaders(program, "shaders/simple.vert", "shaders/simple.frag");
	initShaders(texProgram, "shaders/texture.vert", "shaders/texture.frag");

	// Unit square; drawRect scales it to the requested size
	float vertices[12] = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
	                      0.f, 0.f, 1.f, 1.f, 0.f, 1.f};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 0, 0);

	// Same square with texture coordinates, for drawImage
	glGenVertexArrays(1, &texVao);
	glBindVertexArray(texVao);
	glGenBuffers(1, &texVbo);
	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), TEX_VERTICES, GL_DYNAMIC_DRAW);
	texPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	initialized = true;
}

void UIRenderer::begin()
{
	glm::mat4 projection = glm::ortho(0.f, UI_WIDTH, UI_HEIGHT + UI_MARGIN_Y, -UI_MARGIN_Y);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	program.use();
	program.setUniformMatrix4f("projection", projection);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UIRenderer::end()
{
	glDisable(GL_BLEND);
}

void UIRenderer::drawRect(float x, float y, float width, float height, const glm::vec4 &color)
{
	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.f));
	modelview = glm::scale(modelview, glm::vec3(width, height, 1.f));

	program.setUniformMatrix4f("modelview", modelview);
	program.setUniform4f("color", color.r, color.g, color.b, color.a);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::fillScreen(const glm::vec4 &color)
{
	drawRect(0.f, -UI_MARGIN_Y, UI_WIDTH, UI_HEIGHT + 2.f * UI_MARGIN_Y, color);
}

void UIRenderer::drawBox(float x, float y, float width, float height)
{
	drawRect(x, y, width, height, UI_COLOR_DARK);
	drawRect(x + 1, y + 1, width - 2, height - 2, UI_COLOR_ORANGE);
	drawRect(x + 2, y + 2, width - 4, height - 4, UI_COLOR_LIGHT);
}

void UIRenderer::drawFrameBox(float x, float y, float width, float height)
{
	drawRect(x + 2, y + height, width - 1, 1, UI_COLOR_SHADOW);
	drawRect(x + width, y + 2, 1, height - 1, UI_COLOR_SHADOW);
	drawRect(x + 1, y, width - 2, height, UI_COLOR_NAVY);
	drawRect(x, y + 1, width, height - 2, UI_COLOR_NAVY);
	drawRect(x + 1, y + 1, width - 2, height - 2, UI_COLOR_SAGE);
	drawRect(x + 2, y + 2, width - 4, height - 4, UI_COLOR_LIGHT);
}

void UIRenderer::drawPanel(float x, float y, float width, float height)
{
	float b = PANEL_BORDER * UI_PIXEL;

	drawRect(x, y, width, height, UI_COLOR_PANEL_EDGE);
	drawRect(x + b, y + b, width - 2 * b, height - 2 * b, UI_COLOR_PANEL_DARK);
	drawRect(x + b, y + b, width - 3 * b, height - 3 * b, UI_COLOR_PANEL_LIGHT);
	drawRect(x + 2 * b, y + 2 * b, width - 4 * b, height - 4 * b, UI_COLOR_PANEL);
}

void UIRenderer::drawScreenImage(const Texture &image)
{
	drawImage(image, 0.f, -UI_MARGIN_Y, UI_WIDTH, UI_HEIGHT + 2.f * UI_MARGIN_Y);
}

void UIRenderer::drawImageRegion(const Texture &image, const glm::vec4 &region, float x, float y, float width, float height)
{
	float u0 = region.x, v0 = region.y, u1 = region.z, v1 = region.w;
	float vertices[24] = {0.f, 0.f, u0, v0,  1.f, 0.f, u1, v0,  1.f, 1.f, u1, v1,
	                      0.f, 0.f, u0, v0,  1.f, 1.f, u1, v1,  0.f, 1.f, u0, v1};

	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	drawImage(image, x, y, width, height);
	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TEX_VERTICES), TEX_VERTICES);
}

void UIRenderer::drawBackdrop(const WorldInfo &world)
{
	if(backdropFile != world.tilesheet)
	{
		loadImage(backdrop, world.tilesheet);
		backdropFile = world.tilesheet;
	}

	float tile = BACKDROP_TILE * UI_PIXEL;
	float step = 1.f / world.sheetColumns;
	float u = (world.floorTile % world.sheetColumns) * step, v = (world.floorTile / world.sheetColumns) * step;

	for(float y = -UI_MARGIN_Y; y < UI_HEIGHT + UI_MARGIN_Y; y += tile)
		for(float x = 0.f; x < UI_WIDTH; x += tile)
			drawImageRegion(backdrop, glm::vec4(u, v, u + step, v + step), x, y, tile, tile);
	fillScreen(glm::vec4(1.f, 1.f, 1.f, world.veil));
}

void UIRenderer::drawPaper()
{
	fillScreen(UI_COLOR_PAPER);
}

void UIRenderer::drawTitle(const Texture &title, float y)
{
	float width = title.width() * TITLE_PIXEL * UI_PIXEL, height = title.height() * TITLE_PIXEL * UI_PIXEL;

	drawImage(title, (UI_WIDTH - width) / 2.f, y, width, height);
}

void UIRenderer::drawImage(const Texture &image, float x, float y, float width, float height)
{
	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.f));
	modelview = glm::scale(modelview, glm::vec3(width, height, 1.f));

	texProgram.use();
	texProgram.setUniformMatrix4f("modelview", modelview);
	image.use();
	glBindVertexArray(texVao);
	glEnableVertexAttribArray(texPosLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_TEXTURE_2D);
	program.use(); // Back to flat color for the next rectangles and text
}

void UIRenderer::drawText(const std::string &text, float x, float y, const glm::vec4 &color, float scale, float scaleY)
{
	float penX = x, penY = y;

	if(scaleY <= 0)
		scaleY = scale;

	for(char c : text)
	{
		if(c == '\n')
		{
			penX = x;
			penY += UI_LINE_HEIGHT * scaleY;
			continue;
		}
		const unsigned char *rows = glyph(c);
		if(rows != NULL)
		{
			for(int row = 0; row < 7; row++)
			{
				// Draw each horizontal run of pixels as a single rectangle
				int col = 0;
				while(col < 5)
				{
					if(rows[row] & (0x10 >> col))
					{
						int start = col;
						while(col < 5 && (rows[row] & (0x10 >> col)))
							col++;
						drawRect(penX + start * scale, penY + row * scaleY, (col - start) * scale, scaleY, color);
					}
					else
						col++;
				}
			}
		}
		penX += UI_CHAR_WIDTH * scale;
	}
}

void UIRenderer::drawTextCentered(const std::string &text, float y, const glm::vec4 &color, float scale, float scaleY)
{
	drawText(text, float(int((UI_WIDTH - textWidth(text, scale)) / 2.f)), y, color, scale, scaleY);
}

void UIRenderer::drawTextOutlined(const std::string &text, float x, float y, const glm::vec4 &color, float scale)
{
	for(const int *offset : OUTLINE_OFFSETS)
		drawText(text, x + offset[0] * scale, y + offset[1] * scale, UI_COLOR_OUTLINE, scale);
	drawText(text, x, y, color, scale);
}

void UIRenderer::drawTextOutlinedCentered(const std::string &text, float y, const glm::vec4 &color, float scale)
{
	drawTextOutlined(text, float(int((UI_WIDTH - textWidth(text, scale)) / 2.f)), y, color, scale);
}

float UIRenderer::textWidth(const std::string &text, float scale)
{
	int longest = 0, current = 0;

	for(char c : text)
	{
		if(c == '\n')
			current = 0;
		else if(++current > longest)
			longest = current;
	}
	if(longest == 0)
		return 0.f;
	return (longest * UI_CHAR_WIDTH - 1) * scale; // No spacing after the last char
}

bool UIRenderer::loadImage(Texture &image, const std::string &filename)
{
	if(!image.loadFromFile(filename, TEXTURE_PIXEL_FORMAT_RGBA))
	{
		cout << "Could not load " << filename << endl;
		return false;
	}
	image.setMinFilter(GL_NEAREST);
	image.setMagFilter(GL_NEAREST);
	image.setWrapS(GL_CLAMP_TO_EDGE);
	image.setWrapT(GL_CLAMP_TO_EDGE);
	return true;
}

void UIRenderer::initShaders(ShaderProgram &shaderProgram, const std::string &vertexFile, const std::string &fragmentFile)
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, vertexFile);
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, fragmentFile);
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	shaderProgram.init();
	shaderProgram.addShader(vShader);
	shaderProgram.addShader(fShader);
	shaderProgram.link();
	if(!shaderProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << shaderProgram.log() << endl << endl;
	}
	shaderProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
