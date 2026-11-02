#include <iostream>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include "UIRenderer.h"


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
	float texVertices[24] = {0.f, 0.f, 0.f, 0.f,  1.f, 0.f, 1.f, 0.f,  1.f, 1.f, 1.f, 1.f,
	                         0.f, 0.f, 0.f, 0.f,  1.f, 1.f, 1.f, 1.f,  0.f, 1.f, 0.f, 1.f};

	glGenVertexArrays(1, &texVao);
	glBindVertexArray(texVao);
	glGenBuffers(1, &texVbo);
	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), texVertices, GL_STATIC_DRAW);
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

void UIRenderer::drawBlackBox(float x, float y, float width, float height)
{
	drawRect(x + 1, y, width - 2, height, UI_COLOR_BLACK);
	drawRect(x, y + 1, width, height - 2, UI_COLOR_BLACK);
	drawRect(x + 2, y + 2, width - 4, height - 4, UI_COLOR_ORANGE);
	drawRect(x + 3, y + 3, width - 6, height - 6, UI_COLOR_BLACK);
}

void UIRenderer::drawScreenImage(const Texture &image)
{
	drawImage(image, 0.f, -UI_MARGIN_Y, UI_WIDTH, UI_HEIGHT + 2.f * UI_MARGIN_Y);
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

void UIRenderer::drawText(const std::string &text, float x, float y, const glm::vec4 &color, int scale, int scaleY)
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
						drawRect(penX + start * scale, penY + row * scaleY, float((col - start) * scale), float(scaleY), color);
					}
					else
						col++;
				}
			}
		}
		penX += UI_CHAR_WIDTH * scale;
	}
}

void UIRenderer::drawTextCentered(const std::string &text, float y, const glm::vec4 &color, int scale, int scaleY)
{
	drawText(text, float(int((UI_WIDTH - textWidth(text, scale)) / 2.f)), y, color, scale, scaleY);
}

float UIRenderer::textWidth(const std::string &text, int scale)
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
	return float((longest * UI_CHAR_WIDTH - 1) * scale); // No spacing after the last char
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
