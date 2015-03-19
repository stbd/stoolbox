#include "stb_gl.hh"

#include "stb_types.hh"

static_assert(sizeof(GLuint) == sizeof(GL_U),
              "Assumption failed: Size of GLuint and GL_U is not equal");

static_assert(sizeof(GLint) == sizeof(GL_I),
              "Assumption failed: Size of GLuint and GL_I is not equal");
