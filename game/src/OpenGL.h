#ifndef _OPENGL_INCLUDE
#define _OPENGL_INCLUDE


// The desktop build uses GLEW to reach OpenGL 3.3. The web build (Emscripten) uses WebGL 2,
// which is OpenGL ES 3.0: its functions come straight from the GLES3 header
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif


#endif // _OPENGL_INCLUDE
