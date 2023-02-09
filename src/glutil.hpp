// Copied from SuperTux, GPLv3
#include <sstream>
#include <stdexcept>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

inline void check_gl_error(const char* filename, int line)
{
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::ostringstream msg;
    msg << filename << ":" << line << ": " << "glGetError: ";
    switch (error) {
      case GL_INVALID_ENUM:
        msg << "INVALID_ENUM: An unacceptable value is specified for an "
          "enumerated argument.";
        break;
      case GL_INVALID_VALUE:
        msg << "INVALID_VALUE: A numeric argument is out of range.";
        break;
      case GL_INVALID_OPERATION:
        msg << "INVALID_OPERATION: The specified operation is not allowed "
          "in the current state.";
        break;
#ifdef GL_STACK_OVERFLOW
      case GL_STACK_OVERFLOW:
        msg << "STACK_OVERFLOW: This command would cause a stack overflow.";
        break;
#endif
#ifdef GL_STACK_UNDERFLOW
      case GL_STACK_UNDERFLOW:
        msg << "STACK_UNDERFLOW: This command would cause a stack underflow.";
        break;
#endif
      case GL_OUT_OF_MEMORY:
        msg << "OUT_OF_MEMORY: There is not enough memory left to execute the "
          "command.";
        break;
#ifdef GL_TABLE_TOO_LARGE
      case GL_TABLE_TOO_LARGE:
        msg << "TABLE_TOO_LARGE: table is too large";
        break;
#endif
      default:
        msg << "Unknown error (code " << error << ")";
    }

    throw std::runtime_error(msg.str());
  }
}

#define assert_gl() check_gl_error(__FILE__, __LINE__)

