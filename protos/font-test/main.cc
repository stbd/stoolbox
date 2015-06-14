#define STB_DEFAULT_TIMER_TYPES
#include "stb_text_hud.hh"
#include "stb_gl.hh"
#include "stb_log_boost.hh"
#include "stb_util.hh"
#include "stb_game.hh"
#include "stb_gl_object.hh"
#include "stb_buffer.hh"
#include "stb_gl_shader.hh"
#include "stb_error.hh"
#include "stb_model.hh"

#include <SDL2/SDL.h>
#include <glm/vec2.hpp>
#include <assert.h>
#include <string>
#include <fstream>

class Impl
{
public:
    bool init(const std::string & fontPath)
    {
        stb::initFileConsoleLogger("font-proto.log", 0);

        const size_t w = 512*2;
        const size_t h = 512*2;

        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        if ((m_window = SDL_CreateWindow("SToolbox - Font proto",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       w, h,
                                       SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                                       )) == 0
            ) {
            LogFatal(m_log) << "CreateWindow failed, SDL error: " << SDL_GetError();
            return false;
        }

        if ((m_glContext = SDL_GL_CreateContext(m_window)) == 0) {
            LogFatal(m_log) << "CreateContext failed, SDL error: " << SDL_GetError();
            return false;
        }

        if (!stb::initGl()) {
            LogFatal(m_log) << "Initializing OpenGL wrapper failed";
            return false;
        }

        std::string fontData;
        {
            // Read font to memory
            std::ifstream f(fontPath.c_str(), std::ios_base::in | std::ios_base::binary);
            f.seekg(0, f.end);
            const size_t fontDataSize = f.tellg();
            f.seekg(0, f.beg);

            fontData.assign(fontDataSize, 0);
            f.read(&fontData[0], fontDataSize);
        }

        stb::initTextHud(m_hud, fontData.c_str(), fontData.size(), 0, glm::vec3(0.0f, 0.5f, 0.5f), (float)w / (float)h);
        if (stb::isError()) {
            LogFatal(m_log) << "Failed to load font: " << stb::getErrorDescription();
            return false;
        }

        {
            // Just to show the interface
            stb::CharacterAtlas atlas;
            createCharacterAtlas(atlas, fontData.c_str(), fontData.size(), "abc", 3, 40);
            std::string buf;
            size_t w = 0;
            size_t h = 0;
            size_t s = 0;
            renderText(atlas, "ab", buf, w, h, s);
            // Buffer would now have texture with text "ab" of size "w" * "h"
            // with "s" number of extra texels
        }

        return true;
    }

    U handleInput()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) {
                return 1;
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    return 1;
                }
            }
        }

        if (stb::isError()) {
            return 1;
        }
        return 0;
    }

    void update(const TimeDurationNano /*elapsed*/)
    {}

    void render()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const std::string str("Text is meant to be read");
        const std::string specialChars("!\"#%&/()=");
        stb::renderText(m_hud, str.c_str(), str.length(), glm::vec2(.1f, .1f), glm::vec2(.1f, .5f));
        stb::renderText(m_hud, str.c_str(), str.length(), glm::vec2(.05f, .05f), glm::vec2(.0f, .25f));
        stb::renderText(m_hud, str.c_str(), str.length(), glm::vec2(.2f, .2f), glm::vec2(.1f, .0f));
        stb::renderText(m_hud, str.c_str(), str.length(), glm::vec2(.01f, .01f), glm::vec2(.5f, .62f));
        stb::renderText(m_hud, str.c_str(), str.length(), glm::vec2(.4f, .4f), glm::vec2(.1f, .6f));
        stb::renderText(m_hud, specialChars.c_str(), specialChars.length(), glm::vec2(.1f, .1f), glm::vec2(.3f, .35f));

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        SDL_GL_SwapWindow(m_window);
    }

private:
    SDL_GLContext m_glContext = 0;
    SDL_Window * m_window = 0;
    stb::TextHud m_hud;
    stb::Log m_log;
};

// For faster debugging
//int main(int /*argc*/, char ** /*argv*/)
/*
{
#ifdef STB_LINUX
    std::string fontPath("/usr/share/fonts/TTF/UbuntuMono-BI.ttf");
#elif STB_WINDOWS
    std::string fontPath("C:/Windows/Fonts/arial.ttf");
#endif
*/
    
int main(int argc, char * argv[]) {

    if (argc != 2) {
        printf("Usage: %s [path-to-TTF-font-file]\n", argv[0]);
        return 0;
    }
    std::string fontPath(argv[1]);
    

    Impl impl;
    assert(impl.init(fontPath) == true && "Initialization failed");
    stb::Main<Impl, TimeDurationNano, TimePoint>
        main(impl, TimeDurationNano(10000000), getTime);
    main.run();
    return 0;
}
