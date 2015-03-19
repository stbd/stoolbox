#ifndef STB_ERROR_HH_
#define STB_ERROR_HH_

namespace stb
{
    bool isError();
    const char * getErrorDescription();
    void clearError();
}

#endif