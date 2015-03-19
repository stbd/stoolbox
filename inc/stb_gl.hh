#ifndef STB_GL_
#define STB_GL_

#include <gl_stb.h>

namespace stb
{
    inline bool initGl()
    { return ogl_LoadFunctions() == ogl_LOAD_SUCCEEDED; }
}

#endif
