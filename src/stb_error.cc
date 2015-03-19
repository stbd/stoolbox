#include "stb_error.hh"

#include <mutex>
#include <string>
#include <cstdio>
#include <cstdarg>

using namespace stb;

namespace stb
{
    std::mutex lock;
    typedef std::lock_guard<std::mutex> Guard;
    static std::string errorDescriptor;
}

bool stb::isError()
{
    Guard guard(lock);
    return !errorDescriptor.empty();
}

const char * stb::getErrorDescription()
{
    Guard guard(lock);
    return errorDescriptor.c_str();
}

void stb::clearError()
{
    Guard guard(lock);
    errorDescriptor.clear();
}

/*
 * The following function is meant be used only by
 * STB libary, thus it is hidden.
 * To use it, copy:
namespace stb {
    extern void setError(const char * format, ...);
}
*/
namespace stb
{
void setError(const char * format, ...)
{
    const size_t bufferSize = 1024;
    std::string buffer(bufferSize, ' ');
    va_list argptr;

    va_start(argptr, format);
    vsnprintf(&buffer[0], bufferSize, format, argptr);
    va_end(argptr);

    {
        Guard guard(lock);
        if (errorDescriptor.empty()) {
            errorDescriptor = buffer;
        }
    }
}
}
