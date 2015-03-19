#ifndef STB_TYPES_HH_
#define STB_TYPES_HH_

#include <cstdint>

// System default sizes
typedef unsigned U;
typedef int I;

typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;
typedef int8_t I8;
typedef int32_t I32;
typedef int16_t I16;

/*
 * Purpose of these typedefs is to allow header files to use
 * types combatible with OpenGL without including OpenGL headers
 */
typedef uint32_t GL_U;
typedef int32_t GL_I;

#endif
