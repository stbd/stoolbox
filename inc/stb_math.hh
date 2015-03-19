#ifndef STB_MATH_HH_
#define STB_MATH_HH_

#include <cmath>
#include <algorithm>
#include <type_traits>

namespace stb
{

//sign
template <typename T> inline
int sign(T x, std::false_type) {
    return T(0) < x;
}
template <typename T> inline
int sign(T x, std::true_type) {
    return (T(0) < x) - (x < T(0));
}
template <typename T> inline
int sign(T x) {
    return sign(x, std::is_signed<T>());
}

inline int iabs(int x) {
    return (x > 0) ? x : -x;
}

}

#endif
