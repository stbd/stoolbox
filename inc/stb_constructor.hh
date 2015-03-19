#ifndef STB_GENERATOR_HH_
#define STB_GENERATOR_HH_

#include "stb_types.hh"

namespace stb
{
    class ModelData;

    stb::ModelData generateCube(const U subdivides);
    stb::ModelData generateSphere(const U subdivides, const float radius = .5f);
}

#endif
