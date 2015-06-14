#include "stb_font.hh"

#include "stb_error.hh"
#include <algorithm>
#include <limits>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace stb;

namespace stb {
    extern void setError(const char * format, ...);
}
/*
static void debugArray(FILE * f, const unsigned char * b, const size_t rows, const size_t width)
{
    for (size_t r = 0; r < rows; ++r) {
        for (size_t w = 0; w < width; ++w) {
            fprintf(f, "% .3u ", (unsigned)b[r * width + w]);
        }
        fprintf(f, "\n");
    }
}
*/
static void chamferDistanceField(
    const signed char * source,
    const size_t width,
    const size_t height,
    const signed char d1,
    const signed char d2,
    signed char * dest)
{
    for (size_t y = 1; y < (height - 1); ++y) {
        for (size_t x = 1; x < (width - 1); ++x) {

            const size_t index = (y * width) + x;
            const size_t above = ((y + 1) * width) + x;
            const size_t below = ((y - 1) * width) + x;
            const size_t right = index + 1;
            const size_t left = index - 1;

            if ((y != 0) && (x != 0)) {
                if (
                    (source[index] != source[right]) ||
                    (source[index] != source[left]) ||
                    (source[index] != source[above]) ||
                    (source[index] != source[below])
                    ) {
                    dest[index] = 0;
                }
            }
        }
    }

    for (size_t y = 1; y < (height - 1); ++y) {
        for (size_t x = 1; x < (width - 1); ++x) {

            const size_t index = (y * width) + x;

            const size_t i1 = ((y - 1) * width) + x - 1;
            const size_t i2 = ((y - 1) * width) + x;
            const size_t i3 = ((y - 1) * width) + x + 1;
            const size_t i4 = (y * width) + x - 1;

            if ((dest[i1] + d2) < dest[index]) { dest[index] = dest[i1] + d2; }
            if ((dest[i2] + d1) < dest[index]) { dest[index] = dest[i2] + d1; }
            if ((dest[i3] + d2) < dest[index]) { dest[index] = dest[i3] + d2; }
            if ((dest[i4] + d2) < dest[index]) { dest[index] = dest[i4] + d1; }
        }
    }

    for (size_t y = (height - 2); y > 1; --y) {
        for (size_t x = (width - 2); x > 1; --x) {

            const size_t index = (y * width) + x;

            const size_t i1 = (y * width) + x + 1;
            const size_t i2 = ((y + 1) * width) + x - 1;
            const size_t i3 = ((y + 1) * width) + x;
            const size_t i4 = ((y + 1) * width) + x + 1;

            if ((dest[i1] + d1) < dest[index]) { dest[index] = dest[i1] + d1; }
            if ((dest[i2] + d2) < dest[index]) { dest[index] = dest[i2] + d2; }
            if ((dest[i3] + d1) < dest[index]) { dest[index] = dest[i3] + d1; }
            if ((dest[i4] + d2) < dest[index]) { dest[index] = dest[i4] + d2; }
        }
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {

            const size_t index = (y * width) + x;

            if (source[index] == 0) { dest[index] = -dest[index]; }
        }
    }
}

static void createDistanceFieldArray(
    const unsigned char * source,
    const size_t height,
    const size_t width,
    std::vector<signed char> & buffer1,
    std::vector<signed char> & buffer2,
    unsigned char * output)
{
    // Freetype packs fonts quite tight,
    // add "extraElements" above, below, left, and right to give the algorithm some extra space
    const size_t extraElements = 1;
    const size_t h = height + (extraElements * 2);
    const size_t w = width + (extraElements * 2);
    buffer1.assign(w * h, 0);
    buffer2.assign(w * h, std::numeric_limits<signed char>::max());

    // Copy source into work buffer and format it to work in buffer
    // At the same time, convert source into binary image
    // and flip it horizontally so for OpenGL
    for (size_t hi = 0; hi < height; ++hi) {

        const unsigned char * s = source +                // Beginning of the row in source 
            ((height - hi - 1) * width);                  // from the end image
        signed char * d = buffer1.data() +                // Beginning of the row in workArray
            (extraElements * w) + extraElements +         // (take into account the added elements)
            hi * w;

        for (size_t wi = 0; wi < width; ++wi) {
            if (*(s + wi) != 0) {*(d + wi) = 1;}
        }
    }

    // Actual implementation for algorithm
    chamferDistanceField(buffer1.data(), w, h, 3*1, 4*1, buffer2.data());

    /*
    FILE * f = fopen("D:/Projects/stoolbox/build/l.log", "w");
    debugArray(f, (unsigned char *)source, height, width);
    fprintf(f, "----------------------------------\n");
    debugArray(f, (unsigned char *)buffer1.data(), h, w);
    fprintf(f, "----------------------------------\n");
    debugArray(f, (unsigned char *)buffer2.data(), h, w);
    */

    // Remove the extra lines which are not wished for output,
    // and at the same timw convert result to unsigned char
    for (size_t hi = 0; hi < height; ++hi) {

        unsigned char * d = output + (hi * width);        // Beginning of the row in output
        const signed char * s = buffer2.data() +          // Beginning of the row in workArray
            (extraElements * w) + extraElements +         // (take into account the added elements)
            hi * w;

        for (size_t wi = 0; wi < width; ++wi) {
            // Signed char to unsinged char conversion
            *(d + wi) = static_cast<unsigned char>(std::numeric_limits<signed char>::min() + *(s + wi));
        }
    }
    /*
    fprintf(f, "----------------------------------\n");
    debugArray(f, (unsigned char *)output, height, width);
    fclose(f);
    */
}

static void copyFontToAtlas(
    const unsigned char * source,
    const size_t sourceWidth,
    const size_t sourceHeight,
    unsigned char * destination,
    const size_t offsetX,
    const size_t offsetY,
    const size_t atlasWidth)
{
    for (size_t h = 0; h < sourceHeight; ++h) {
        const unsigned char * s = source + (h * sourceWidth);
        unsigned char * d = destination + ((h + offsetY) * atlasWidth) + offsetX;
        memcpy(d, s, sourceWidth);
    }
}

static void createCharacterAtlasImpl(
    CharacterAtlas & characterAtlas,
    FT_Library & ftLib,
    FT_Face & ftFace,
    const char * ttfFontData,
    const size_t sizeOfTtfFontData,
    const char * charactersToRender,
    const size_t numberOfCharasterToRender,
    const size_t charHeight,
    const size_t dpiX,
    const size_t dpiY,
    const size_t atlasItemsPerRow,
    const size_t atlasItemsRows
    )
{
    int ftErrorCode = 0;
    if ((ftErrorCode = FT_Init_FreeType(&ftLib)) != 0) {
        stb::setError("FT_Init_FreeType failed, code=%d\n", ftErrorCode);
        return;
    }

    if ((ftErrorCode = FT_New_Memory_Face(ftLib, (const FT_Byte *)ttfFontData, sizeOfTtfFontData, 0, &ftFace)) != 0) {
        stb::setError("FT_New_Face failed, code=%d\n", ftErrorCode);
        return;
    }

    if ((ftErrorCode = FT_Set_Char_Size(ftFace, 0, charHeight * 64, dpiY, dpiX)) != 0) {
        stb::setError("FT_Set_Pixel_Sizes failed, code=%d\n", ftErrorCode);
        return;
    }

    size_t charMaxWidth = 0;
    size_t charMaxAscender = 0;
    size_t charMaxDescender = 0;

    /*
    * There is a comment on Freetype2 documentation: "Indeed, the only way to get the
    * exact metrics is to render all glyphs. As this would be a definite performance
    * hit, it is up to client applications to perform such computations."
    *
    * So, as this function should be called at init stage before any performance critical parts,
    * it should be safe to loop thru all characters and render them in order to know their real sizes
    */
    for (size_t i = 0; i < numberOfCharasterToRender; ++i) {

        char c = charactersToRender[i];
        int index = FT_Get_Char_Index(ftFace, c);
        if (index == 0) {
            stb::setError("FT_Get_Char_Index failed for char=%c, code=%d\n", c, ftErrorCode);
            return;
        }

        if ((ftErrorCode = FT_Load_Glyph(ftFace, index, FT_LOAD_DEFAULT)) != 0) {
            stb::setError("FT_Load_Glyph failed for char=%c, code=%d\n", c, ftErrorCode);
            return;
        }

        if ((ftErrorCode = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL)) != 0) {
            stb::setError("FT_Render_Glyph failed for char=%c, code=%d\n", c, ftErrorCode);
            return;
        }

        size_t d = 0;
        if (ftFace->glyph->metrics.height > ftFace->glyph->metrics.horiBearingY &&
            ftFace->glyph->metrics.horiBearingY > 0) {
            d = static_cast<size_t>(ftFace->glyph->metrics.height >> 6) -
                static_cast<size_t>(ftFace->glyph->metrics.horiBearingY >> 6);
        }
        size_t a = static_cast<size_t>(ftFace->glyph->metrics.height >> 6) - d;

        charMaxWidth = std::max(static_cast<size_t>(ftFace->glyph->metrics.width >> 6), charMaxWidth);
        charMaxAscender = std::max(a, charMaxAscender);
        charMaxDescender = std::max(d, charMaxDescender);
    }

    characterAtlas.atlasWidth = atlasItemsPerRow * charMaxWidth;
    characterAtlas.atlasHeight = atlasItemsRows * (charMaxAscender + charMaxDescender);

    characterAtlas.buffer.assign(characterAtlas.atlasWidth * characterAtlas.atlasHeight, 0);
    std::vector<signed char> buffer1(charMaxWidth * (charMaxAscender + charMaxDescender), 0);
    std::vector<signed char> buffer2(charMaxWidth * (charMaxAscender + charMaxDescender), 0);
    std::vector<unsigned char> buffer3(charMaxWidth * (charMaxAscender + charMaxDescender), 0);

    for (size_t i = 0; i < numberOfCharasterToRender; ++i) {

        char c = charactersToRender[i];
        int index = FT_Get_Char_Index(ftFace, c);
        FT_Load_Glyph(ftFace, index, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        createDistanceFieldArray(ftFace->glyph->bitmap.buffer,
            ftFace->glyph->bitmap.rows,
            ftFace->glyph->bitmap.width,
            buffer1,
            buffer2,
            &buffer3[0]);

        const size_t atlasLocationX = i % atlasItemsPerRow;
        const size_t atlasLocationY = i / atlasItemsPerRow;

        const size_t descender = static_cast<size_t>(ftFace->glyph->metrics.height >> 6) -
            static_cast<size_t>(ftFace->glyph->metrics.horiBearingY >> 6);
        const size_t descenderOffset = charMaxDescender - descender;

        copyFontToAtlas(
            buffer3.data(),
            ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows,
            (unsigned char *)characterAtlas.buffer.data(),
            atlasLocationX * charMaxWidth, atlasLocationY * (charMaxAscender + charMaxDescender) + descenderOffset,
            characterAtlas.atlasWidth);
        
        const float horizontalAdvanceAsPercentageOfWidth = (float)(ftFace->glyph->metrics.horiAdvance >> 6) / (float)(charMaxWidth);

        characterAtlas.characters[c] = stb::Character(
            i,
            horizontalAdvanceAsPercentageOfWidth,
            (float)(ftFace->glyph->metrics.horiBearingX >> 6) / (float)(charMaxWidth)
            );

        characterAtlas.maxHorizontalAdvanceAsPercentageOfWidth = std::max(horizontalAdvanceAsPercentageOfWidth, characterAtlas.maxHorizontalAdvanceAsPercentageOfWidth);
    }
}

void stb::createCharacterAtlas(
    CharacterAtlas & characterAtlas,
    const char * ttfFontData,
    const size_t sizeOfTtfFontData,
    const char * charactersToRender,
    const size_t numberOfCharasterToRender,
    const size_t charHeight,
    const size_t dpiX,
    const size_t dpiY
    )
{
    FT_Library ftLib;
    FT_Face ftFace;

    // Number of elements in atlas is hardcoded for time being
    const size_t atlasItemsPerRow = 10;
    const size_t atlasItemsRows = 10;

    if ((atlasItemsPerRow * atlasItemsRows) < numberOfCharasterToRender) {
        stb::setError("%s: numberOfChars %u exeeds atlas size %u", __FUNCTION__, numberOfCharasterToRender, (atlasItemsPerRow * atlasItemsRows));
        return;
    }

    createCharacterAtlasImpl(
        characterAtlas,
        ftLib,
        ftFace,
        ttfFontData,
        sizeOfTtfFontData,
        charactersToRender,
        numberOfCharasterToRender,
        charHeight,
        dpiX,
        dpiY,
        atlasItemsPerRow,
        atlasItemsRows
        );

    if (stb::isError()) {
        return ;
    }

    characterAtlas.charactersPerRow = atlasItemsPerRow;
    characterAtlas.characterRows = atlasItemsRows;

    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLib);
}

void stb::renderText(
    CharacterAtlas & atlas,
    const std::string & text,
    std::string & buffer,
    size_t & textureWidthTexels,
    size_t & textureHeightTexels,
    size_t & numberOfhorizontalSurplusTexels
    )
{
    const size_t characterHeight = static_cast<size_t>(static_cast<double>(atlas.atlasHeight) / static_cast<double>(atlas.characterRows));
    const size_t characterWidth = static_cast<size_t>(static_cast<double>(atlas.atlasWidth) / static_cast<double>(atlas.charactersPerRow));
    const size_t maxAdvanceTexels = static_cast<size_t>(static_cast<double>(characterWidth) * atlas.maxHorizontalAdvanceAsPercentageOfWidth);

    // Estimate needed memory by worst case
    textureWidthTexels = maxAdvanceTexels * text.length();
    textureHeightTexels = characterHeight;
    buffer.assign(textureHeightTexels * textureWidthTexels, 0);

    size_t positionHorizontal = 0;
    for (const char & t : text) {

        const stb::Character & c = atlas.characters[t];
        const size_t atlasPosX = c.atlasIndex % atlas.charactersPerRow;
        const size_t atlasPosY = c.atlasIndex / atlas.charactersPerRow;
        const char * beginningOfCharacterAtAtlas = atlas.buffer.data() + (atlasPosY * atlas.atlasWidth) + (atlasPosX * characterWidth);

        for (size_t row = 0; row < characterHeight; ++row) {
            char * dest = &buffer[0] + (row * textureWidthTexels) + positionHorizontal;
            const char * source = beginningOfCharacterAtAtlas + (row * atlas.atlasWidth);
            memcpy(dest, source, characterWidth);
        }
        const size_t advance = static_cast<size_t>(static_cast<double>(characterWidth) * c.advanceAsPercentageOfWidth);
        positionHorizontal += advance;
    }

    // Return number of texels the estimate was wrong
    numberOfhorizontalSurplusTexels = textureWidthTexels - positionHorizontal;
}
