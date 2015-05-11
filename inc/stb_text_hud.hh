#ifndef STB_TEXT_HUD_HH_
#define STB_TEXT_HUD_HH_

#include "stb_gl_shader.hh"
#include "stb_gl_object.hh"
#include "stb_types.hh"
#include "stb_font.hh"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace stb
{
    struct TextHud
    {
        stb::Shader shader;
        stb::VertexArrayObject vao;
        GL_U textureUnit;
        GL_U textureTarget;
        GL_I shaderVariableFontIndex;
        GL_I shaderVariableCursorPostion;
        GL_I shaderVariableCursorScale;
        AtlasCharacters characters;
    };

    void initTextHud(
        TextHud & hud,
        const char * pathToTtf,
        const GL_U openglTextureUnit,
        const glm::vec3 & textColor,
        const float aspectRatio,
        const size_t charHeight = 512,
        const size_t dpiX = 72,
        const size_t dpiY = 72
        );

    void renderText(
        TextHud & hud,
        const char * text,
        const size_t textLength,
        const glm::vec2 & cursorScale,
        const glm::vec2 & cursorStartPosition
        );
}

#endif