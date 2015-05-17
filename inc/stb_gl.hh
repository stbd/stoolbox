#ifndef STB_GL_
#define STB_GL_

#include <gl_stb.h>

namespace stb
{
#define STB_GL_DEBUG_ERROR stb::getGlError(__FILE__, __LINE__)
    GLenum getGlError(const char * file, const int lineNumber);

    inline bool initGl()
    { return ogl_LoadFunctions() == ogl_LOAD_SUCCEEDED; }
}

#endif
