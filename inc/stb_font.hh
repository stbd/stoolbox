#ifndef STB_FONT_HH_
#define STB_FONT_HH_

#include <unordered_map>
#include <string>

namespace stb
{
    struct Character
    {
        Character(const size_t atlasIndex, const float advanceAsPercentageOfWidth, const float horizontalOffsetAsPercentageOfWidth)
        : atlasIndex(atlasIndex), advanceAsPercentageOfWidth(advanceAsPercentageOfWidth), horizontalOffsetAsPercentageOfWidth(horizontalOffsetAsPercentageOfWidth)
        {}
        Character()
        : atlasIndex(0), advanceAsPercentageOfWidth(0.0f), horizontalOffsetAsPercentageOfWidth(0.0f)
        {}

        size_t atlasIndex;
        float advanceAsPercentageOfWidth;
        float horizontalOffsetAsPercentageOfWidth;
    };

    typedef std::unordered_map<char, stb::Character> AtlasCharacters;
    struct CharacterAtlas
    {
        std::string buffer;
        size_t atlasWidth;
        size_t atlasHeight;
        size_t charactersPerRow;
        size_t characterRows;
        float maxHorizontalAdvanceAsPercentageOfWidth;
        AtlasCharacters characters;
    };

    void createCharacterAtlas(
        CharacterAtlas & characterAtlas,
        const char * pathToTtf,
        const char * charactersToRender,
        const size_t numberOfCharasterToRender,
        const size_t charHeight = 512,
        const size_t dpiX = 72,
        const size_t dpiY = 72
        );
    void renderText(
        CharacterAtlas & atlas,
        const std::string & text,
        std::string & buffer,
        size_t & textureWidthTexels,
        size_t & textureHeightTexels,
        size_t & numberOfhorizontalSurplusTexels
        );
}

#endif
