#ifndef STB_UTIL_HH_
#define STB_UTIL_HH_

#include <cstdio>

#if defined(STB_WINDOWS)
/*
* Windows does not implement snprintf, as a workaround use _snprintf
* which is not exactly the same but quite close
*/
#define stb_snprintf _snprintf
#else
#define stb_snprintf snprintf
#endif

namespace stb
{

//Number to string
template <typename T>
inline bool numberToStr(T, char *, const size_t) {
    return false;
}
template <>
inline bool numberToStr<unsigned>(unsigned value, char * buffer, const size_t bufferSize) {
    return (stb_snprintf(buffer, bufferSize, "%u", value) > 0);
}
template <>
inline bool numberToStr<float>(float value, char * buffer, const size_t bufferSize) {
    return (stb_snprintf(buffer, bufferSize, "%f", value) > 0);
}
template <>
inline bool numberToStr<int>(int value, char * buffer, const size_t bufferSize) {
    return (stb_snprintf(buffer, bufferSize, "%d", value) > 0);
}
template <>
inline bool numberToStr<double>(double value, char * buffer, const size_t bufferSize) {
    return (stb_snprintf(buffer, bufferSize, "%f", value) > 0);
}

//String to number
template <typename T>
inline bool strToNumber(const char *, T &) {
    return false;
}
template <>
inline bool strToNumber<unsigned>(const char * source, unsigned & destination) {
    return (std::sscanf(source, "%u", &destination) == 1);
}
template <>
inline bool strToNumber<int>(const char * source, int & destination) {
    return (std::sscanf(source, "%d", &destination) == 1);
}

template<typename T>
void emptyDeleter(const T * /*t*/){}

}
/*
 * Following types are handy tools to be used with Main class
 * They are behind macro to avoid unneeded include for chrono
 */
#ifdef STB_DEFAULT_TIMER_TYPES
#include <chrono>
typedef std::chrono::steady_clock Clock;
typedef Clock::time_point TimePoint;
typedef std::chrono::nanoseconds TimeDurationNano;
TimePoint getTime() {
    return Clock::now();
}
#endif

#endif
