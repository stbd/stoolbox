#include "stb_gl.hh"

#include "stb_types.hh"
#include "stb_error.hh"

namespace stb {
    extern void setError(const char * format, ...);
}

GLenum stb::getGlError(const char * file, const int lineNumber)
{
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR) {
        stb::setError("GL error: %d at %s:%d\n", glError, file, lineNumber);
    }
    return glError;
}

static_assert(sizeof(GLuint) == sizeof(GL_U),
              "Assumption failed: Size of GLuint and GL_U is not equal");

static_assert(sizeof(GLint) == sizeof(GL_I),
              "Assumption failed: Size of GLuint and GL_I is not equal");
