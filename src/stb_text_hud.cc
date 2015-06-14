#include "stb_text_hud.hh"

#include "stb_font.hh"
#include "stb_error.hh"
#include "stb_buffer.hh"
#include "stb_model.hh"
#include "stb_gl.hh"
#include "stb_util.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cassert>

using namespace stb;

namespace stb {
    extern void setError(const char * format, ...);
}

static const char * vertexShader =
"#version 150\n \
in vec3 position; \
in vec2 texturePosition; \
out vec2 texturePositionShared; \
uniform mat4 projection = mat4(1.0);\n \
uniform vec2 cursorScale = vec2(1.0, 1.0); \
uniform vec2 cursorPostion = vec2(.0, .0); \
void main()\n \
{\n \
    texturePositionShared = texturePosition; \
    gl_Position = projection * vec4(position * vec3(cursorScale, 1.0) + vec3(cursorPostion, 0.0), 1);\n \
}\n";

static const char * fragmentShader =
"#version 150\n \
in vec2 texturePositionShared; \
out vec4 color; \
uniform sampler2D sampler; \
uniform vec2 textureSize = vec2(0.0f, 0.0f); \
uniform vec2 texelSize = vec2(0.0f, 0.0f); \
uniform vec2 fontScale = vec2(1.0f, 1.0f); \
uniform uint fontIndex = uint(0.0f); \
uniform uint fontsPerRow = uint(10.0f); \
uniform vec3 fontColor = vec3(0.0f); \
\
float aastep(float threshold, float value) { \
    float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value))); \
    return smoothstep(threshold - afwidth, threshold + afwidth, value); \
}\
\
void main() \
{ \
    vec2 atlasFontIndex = vec2(fontIndex % fontsPerRow, fontIndex / fontsPerRow); \
    vec2 atlasTextureCoordinate = vec2(texturePositionShared + atlasFontIndex) * fontScale; \
\
    vec2 textureCoordinate = atlasTextureCoordinate * textureSize; \
    vec2 lowerLeftCoordinate = floor(textureCoordinate) * texelSize; \
\
    float t = texture(sampler, atlasTextureCoordinate).r; \
    float lowerLeft = texture(sampler, lowerLeftCoordinate).r; \
    float lowerRight = texture(sampler, lowerLeftCoordinate + vec2(1.0f, 0.0f) * texelSize).r; \
    float upperLeft = texture(sampler, lowerLeftCoordinate + vec2(0.0f, 1.f) * texelSize).r; \
    float upperRight = texture(sampler, lowerLeftCoordinate + vec2(1.f, 1.f) * texelSize).r; \
    float dx = (atlasTextureCoordinate.x - lowerLeftCoordinate.x); \
    float dy = (atlasTextureCoordinate.y - lowerLeftCoordinate.y); \
    float lower = mix(lowerLeft, lowerRight, dx / texelSize.x); \
    float upper = mix(upperLeft, upperRight, dx / texelSize.x); \
    float d = mix(lower, upper, dy / texelSize.y); \
\
    float pattern = aastep(0.5f, d); \
    if (pattern > 0.0f) { \
        //color = vec4(vec3(float(d) * fontColor), 1.0f); \n\
        //color = vec4(vec3(float(d > 0.5) * fontColor), 1.0f); \n\
        //color = vec4(vec3(float(lowerLeft > 0.5) * fontColor), 1.0f); \n\
        color = vec4(vec3(pattern * fontColor), 1.0f); \n\
    } else { \
        discard ; \
    } \
}";

static void initVertexArrayObject(
    stb::TextHud & hud,
    const GL_I shaderVertexInputLayout,
    const GL_I shaderTextureInputLayout
    )
{
    const float attributes[] = {
        .0f, .0f, -1.0f, 0.0f, 0.0f,
        1.0f, .0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        .0f, 1.f, -1.0f, 0.0f, 1.0f
    };

    const U32 indices[] = {
        0, 1, 2, 0, 2, 3
    };

    stb::ModelData::AttributeData attribute(
        (const char *)attributes,
        sizeof(attributes),
        { 3, 2 },
        sizeof(float)* 5,
        stb::ModelData::FLOAT
        );

    stb::ModelData planeModel(
    { stb::ModelData::AttributeElement(&attribute, stb::emptyDeleter<stb::ModelData::AttributeData>) },
    (const char *)indices,
    sizeof(indices),
    sizeof(indices[0]),
    stb::ModelData::TRIANGLE
    );

    stb::initVao(hud.vao, planeModel, { shaderVertexInputLayout, shaderTextureInputLayout });
}

static bool getUniformLocation(const char * variableName, GL_U shader, GL_I & location) {
    if ((location = glGetUniformLocation(shader, variableName)) == -1) {
        stb::setError("%s: Unable to find shader uniform \"%s\"", __FUNCTION__, variableName);
        return false;
    }
    return true;
}

static bool getAttributeLocation(const char * variableName, GL_U shader, GL_I & location) {
    if ((location = glGetAttribLocation(shader, variableName)) == -1) {
        stb::setError("%s: Unable to find shader attribute variable \"%s\"", __FUNCTION__, variableName);
        return false;
    }
    return true;
}

void stb::initTextHud(
    stb::TextHud & hud,
    const char * ttfFontData,
    const size_t sizeOfTtfFontData,
    const GL_U openglTextureUnit,
    const glm::vec3 & textColor,
    const float aspectRatio,
    const size_t charHeight,
    const size_t dpiX,
    const size_t dpiY
    )
{
    stb::CharacterAtlas atlas;

    // Generate list of characters that are available
    // start from space, ASCII code hex 0x20, and continue until ")"
    const size_t numberOfCharacters = 93;
    char characters[numberOfCharacters] = { 0 };

    characters[0] = ' ';
    for (size_t i = 1; i < numberOfCharacters; ++i) {
        characters[i] = characters[i - 1] + 1;
    }

    createCharacterAtlas(
        atlas,
        ttfFontData,
        sizeOfTtfFontData,
        characters,
        numberOfCharacters,
        charHeight,
        dpiX,
        dpiY
        );

    if (stb::isError()) {
        return;
    }

    hud.characters = atlas.characters;
    hud.textureUnit = openglTextureUnit;

    {
        stb::buffer::StaticMemory vs(vertexShader, strlen(vertexShader));
        stb::buffer::StaticMemory fs(fragmentShader, strlen(fragmentShader));
        stb::compileShader({ stb::ShaderSource(&vs, stb::ShaderType::Vertex),
            stb::ShaderSource(&fs, stb::ShaderType::Fragment) },
            hud.shader);

        if (stb::isError()) {
            return;
        }

        stb::activateShader(hud.shader);

        GL_I samplerLocation = -1;
        GL_I projectionLocation = -1;
        GL_I fontsPerRowLocation = -1;
        GL_I fontScaleLocation = -1;
        GL_I fontColorLocation = -1;
        GL_I textureSizeLocation = -1;
        GL_I texelSizeLocation = -1;
        if (!getUniformLocation("sampler", stb::glRef(hud.shader), samplerLocation)) { return; }
        if (!getUniformLocation("projection", stb::glRef(hud.shader), projectionLocation)) { return; }
        if (!getUniformLocation("fontIndex", stb::glRef(hud.shader), hud.shaderVariableFontIndex)) { return; }
        if (!getUniformLocation("cursorPostion", stb::glRef(hud.shader), hud.shaderVariableCursorPostion)) { return; }
        if (!getUniformLocation("cursorScale", stb::glRef(hud.shader), hud.shaderVariableCursorScale)) { return; }
        if (!getUniformLocation("fontsPerRow", stb::glRef(hud.shader), fontsPerRowLocation)) { return; }
        if (!getUniformLocation("fontScale", stb::glRef(hud.shader), fontScaleLocation)) { return; }
        if (!getUniformLocation("fontColor", stb::glRef(hud.shader), fontColorLocation)) { return; }
        if (!getUniformLocation("textureSize", stb::glRef(hud.shader), textureSizeLocation)) { return; }
        if (!getUniformLocation("texelSize", stb::glRef(hud.shader), texelSizeLocation)) { return; }

        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
            glm::value_ptr(glm::ortho(0.0f, aspectRatio * 1.0f, 0.0f, 1.0f, 0.0f, 10.f)));
        glUniform1i(samplerLocation, hud.textureUnit);
        glUniform1ui(fontsPerRowLocation, atlas.charactersPerRow);
        glUniform3fv(fontColorLocation, 1, glm::value_ptr(textColor));
        glUniform2fv(textureSizeLocation, 1, glm::value_ptr(glm::vec2((float)atlas.atlasWidth, (float)atlas.atlasHeight)));
        glUniform2fv(texelSizeLocation, 1, glm::value_ptr(glm::vec2(1.0f / (float)atlas.atlasWidth, 1.0f / (float)atlas.atlasHeight)));
        glUniform2fv(fontScaleLocation, 1,
            glm::value_ptr(glm::vec2(1.0f / (float)atlas.charactersPerRow, 1.0f / (float)atlas.characterRows)));
    }
    {
        GL_I layoutPosition = -1;
        GL_I layoutTexture = -1;

        if (!getAttributeLocation("position", stb::glRef(hud.shader), layoutPosition)) { return; }
        if (!getAttributeLocation("texturePosition", stb::glRef(hud.shader), layoutTexture)) { return; }

        initVertexArrayObject(hud, layoutPosition, layoutTexture);
        if (stb::isError()) {
            return;
        }
    }

    glGenTextures(1, &hud.textureTarget);
    glBindTexture(GL_TEXTURE_2D, hud.textureTarget);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        atlas.atlasWidth, atlas.atlasHeight,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        (const GLvoid *)atlas.buffer.data()
        );

    STB_GL_DEBUG_ERROR;

    glBindTexture(GL_TEXTURE_2D, 0);
}

void stb::renderText(
    stb::TextHud & hud,
    const char * text,
    const size_t textLength,
    const glm::vec2 & cursorScale,
    const glm::vec2 & cursorStartPosition
    )
{
    glActiveTexture(GL_TEXTURE0 + hud.textureUnit);
    glBindTexture(GL_TEXTURE_2D, hud.textureTarget);
    stb::activateShader(hud.shader);

    glm::vec2 cursorPostion(cursorStartPosition);
    glUniform2fv(hud.shaderVariableCursorPostion, 1, glm::value_ptr(cursorPostion));
    glUniform2fv(hud.shaderVariableCursorScale, 1, glm::value_ptr(cursorScale));
    for (size_t i = 0; i < textLength; ++i) {
        assert(hud.characters.count(text[i]) == 1);
        const stb::Character & c = hud.characters[text[i]];
        glUniform1ui(hud.shaderVariableFontIndex, c.atlasIndex);
        glUniform2fv(hud.shaderVariableCursorPostion, 1,
            glm::value_ptr(cursorPostion + glm::vec2(cursorScale.x * c.horizontalOffsetAsPercentageOfWidth, 0.0f)));
        stb::bindAndDraw(hud.vao);
        cursorPostion += glm::vec2(cursorScale.x * c.advanceAsPercentageOfWidth, 0.0f);
    }
}