#include "OpenGL.h"
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "Game.h"
#include "Audio.h"


#define TARGET_FRAMERATE 60.0f


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().keyPressed(key);
	else if (action == GLFW_RELEASE)
		Game::instance().keyReleased(key);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game::instance().mouseMove(int(xpos), int(ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().mousePress(button);
	else if (action == GLFW_RELEASE)
		Game::instance().mouseRelease(button);
}


// After a long pause (loading a level, the tab in the background) the game does not try
// to catch up all the lost time: at most this many updates in a row, the rest is skipped
#define MAX_UPDATES_PER_FRAME 5


#ifdef __EMSCRIPTEN__
// The touch buttons of the page (web/shell.html) call it: button is a GLFW_GAMEPAD_BUTTON_*
extern "C" EMSCRIPTEN_KEEPALIVE void setTouchButton(int button, int pressed)
{
	Game::instance().setTouchButton(button, pressed != 0);
}
#endif


static GLFWwindow* window;
static double timePerFrame = 1.0 / TARGET_FRAMERATE, timePreviousFrame;
static double pendingTime = 0.0; // Real time not simulated yet, in s
static double simulatedTime = 0.0; // Total time simulated, in s


// One pass of the game loop. The movement of the game is per update (the chef moves 2 px,
// gravity adds the same each update...), so it must run exactly TARGET_FRAMERATE updates per
// second of real time, however often this is called. The browser calls it once per screen
// refresh (30, 60, 144 Hz...), so a frame may need 0, 1 or several updates
static void frame()
{
	double currentTime = glfwGetTime();
	pendingTime += currentTime - timePreviousFrame;
	timePreviousFrame = currentTime;
	if (pendingTime > MAX_UPDATES_PER_FRAME * timePerFrame)
		pendingTime = MAX_UPDATES_PER_FRAME * timePerFrame;

	if (pendingTime >= timePerFrame)
	{
		/* Update steps of the game loop */
		while (pendingTime >= timePerFrame)
		{
			// 16 or 17 ms, so that the timers of the game add up exactly to the real time
			int deltaTime = int(1000.0 * (simulatedTime + timePerFrame)) - int(1000.0 * simulatedTime);
			simulatedTime += timePerFrame;
			pendingTime -= timePerFrame;
			if(!Game::instance().update(deltaTime))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		/* Render step: only the last state is shown */
		Game::instance().render();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);
	}

	/* Poll for and process events */
	glfwPollEvents();
}


int main(void)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

#ifdef __EMSCRIPTEN__
	/* WebGL 2 (OpenGL ES 3.0) */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chili Chef", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

#ifndef __EMSCRIPTEN__
	/* Set window initial position */
	glfwSetWindowPos(window, 100, 100);
#endif
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Set callbacks */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

#ifndef __EMSCRIPTEN__
	/* Init glew to have access to GL extensions */
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	/* Init step of the game loop */
	Game::instance().init();
	timePreviousFrame = glfwGetTime();

#ifdef __EMSCRIPTEN__
	/* The browser owns the loop: it calls frame() once per screen refresh. The page
	   cannot be closed from the game, so there is nothing to free afterwards */
	emscripten_set_main_loop(frame, 0, true);
#else
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
		frame();

	Audio::instance().free();
	glfwTerminate();
#endif
	return 0;
}
