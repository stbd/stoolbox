#define STB_DEFAULT_TIMER_TYPES
#include "stb_gl.hh"
#include "stb_log_boost.hh"
#include "stb_util.hh"
#include "stb_game.hh"
#include "stb_gl_object.hh"
#include "stb_buffer.hh"
#include "stb_gl_shader.hh"
#include "stb_error.hh"
#include "stb_model.hh"
#include "stb_generator.hh"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <assert.h>

static const char * vertexShader =
"#version 150\n \
in vec3 position;\n \
uniform mat4 view = mat4(1.0);\n \
uniform mat4 projection = mat4(1.0);\n \
uniform mat4 model = mat4(1.0);\n \
void main()\n \
{\n \
    gl_Position = projection * view * model * vec4(position, 1);\n \
}\n";

static const char * fragmentShader =
"#version 150\n \
out vec4 color; \
void main() \
{ \
    color = vec4(0.4,0.4,0.8,1.0); \
}";

class Impl
{
public:
    bool init()
    {
        stb::initFileConsoleLogger("demo.log", 0);

        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        if ((m_window = SDL_CreateWindow("SToolbox - demo",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       512, 512,
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

        {
            stb::buffer::StaticMemory vs(vertexShader, strlen(vertexShader));
            stb::buffer::StaticMemory fs(fragmentShader, strlen(fragmentShader));
            stb::compileShader({stb::ShaderSource(&vs, stb::ShaderType::Vertex),
                        stb::ShaderSource(&fs, stb::ShaderType::Fragment)},
                m_shader);

            if (stb::isError()) {
                LogFatal(m_log) << "Compiling shader failed, stb error: " << stb::getErrorDescription();
                return false;
            }
        }
        {
            const stb::ModelData cube = stb::generateCube(2);
            GL_I layoutPosition = -1;

            if ((layoutPosition = glGetAttribLocation(stb::glRef(m_shader), "position")) == -1) {
                LogFatal(m_log) << "Shader variable \"position\" not found";
                return false;
            }

            stb::initVao(m_vao, cube, {layoutPosition});
            if (stb::isError()) {
                LogFatal(m_log) << "Creating VAO failed, stb error: " << stb::getErrorDescription();
                return false;
            }
        }
        {
            GL_I c(0);
            GL_I v(0);
            if ((c = glGetUniformLocation(stb::glRef(m_shader), "projection")) == -1) {
                LogFatal(m_log) << "Shader variable \"projection\" not found";
                return false;
            }
            if ((v = glGetUniformLocation(stb::glRef(m_shader), "view")) == -1) {
                LogFatal(m_log) << "Shader variable \"view\" not found";
                return false;
            }
            if ((m_modelMatrix = glGetUniformLocation(stb::glRef(m_shader), "model")) == -1) {
                LogFatal(m_log) << "Shader variable \"model\" not found";
                return false;
            }

            const float fovy = glm::radians(50.f);
            const glm::mat4 projection =
                glm::perspective<float>(fovy,
                                        static_cast<float>(512) / static_cast<float>(512),
                                        0.1f,
                                        10.0f);
            const glm::mat4 view = glm::lookAt<float>(glm::vec3(0.0f, 0.0f, -3.0f),
                                                      glm::vec3(0.0f, 0.0f, 0.0f),
                                                      glm::vec3(0.0f, 1.0f, 0.0f));

            glUseProgram(stb::glRef(m_shader));
            glUniformMatrix4fv(c, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(v, 1, GL_FALSE, glm::value_ptr(view));
            glUseProgram(0);
        }
        {
            int b = 0, s = 0;
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &b);
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &s);
            LogInfo(m_log) << "Initialization done, OpenGL version: " << b << "." << s;
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
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    m_horizontalRotation += 0.1;
                }
            }
        }
        return 0;
    }

    void update(const TimeDurationNano /*elapsed*/)
    {}

    void render()
    {
        glm::mat4 model =
            glm::rotate(glm::mat4(1.0f), m_horizontalRotation, glm::vec3(0.0f, 1.0f, 0.0f));

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        stb::activateShader(m_shader);
        glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
        stb::bindAndDraw(m_vao);

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
    stb::Shader m_shader;
    stb::VertexArrayObject m_vao;
    GL_I m_modelMatrix = GL_I(0);
    float m_horizontalRotation = 0.0f;

    stb::Log m_log;
};

int main(int argc, char * argv[])
{
    Impl impl;
    assert(impl.init() == true && "Initialization failed");
    stb::Main<Impl, TimeDurationNano, TimePoint>
        main(impl, TimeDurationNano(10000000), getTime);
    main.run();
    return 0;
}
