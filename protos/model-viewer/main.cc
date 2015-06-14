#define STB_DEFAULT_TIMER_TYPES
#include "stb_game.hh"
#include "stb_gl.hh"
#include "stb_log_boost.hh"
#include "stb_gl_shader.hh"
#include "stb_gl_object.hh"
#include "stb_model.hh"
#include "stb_generator.hh"
#include "stb_buffer.hh"
#include "stb_error.hh"
#include "stb_util.hh"

#include <SDL2/SDL.h>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>

#include <vector>
#include <string>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char * vertexShader =
"#version 150\n \
in vec3 position;\n \
in vec3 normal;\n \
in vec2 uv;\n \
out vec3 fNormal;\n \
out vec2 fUv;\n \
\n \
uniform mat4 model = mat4(1.0);\n \
uniform mat4 view = mat4(1.0);\n \
uniform mat4 projection = mat4(1.0);\n \
uniform float pointSize;\n \
\n \
void main() \n \
{ \n \
    gl_Position = projection * view * model * vec4(position, 1); \n \
    gl_PointSize = pointSize; \n \
    fNormal = vec3(vec4(model * vec4(normal, 0)).xyz); \n\
    fUv = uv; \n \
}\n";

static const char * fragmentShader =
"#version 150\n \
in vec3 fNormal;\n \
in vec2 fUv;\n \
\n \
out vec4 fragmentColor;\n \
\n \
uniform vec3 lightDirection = vec3(0.0, 0.0, -1.0);\n \
uniform uint calculateLight = uint(1);\n \
uniform uint visualizeMappingU = uint(0);\n \
uniform uint visualizeMappingV = uint(0);\n \
\n \
void main()\n \
{\n \
    vec3 color = vec3(0.0);\n \
    if (bool(visualizeMappingU)) {\n \
        color.r = fUv.s;\n \
    }\n \
    if (bool(visualizeMappingV)) {\n \
        color.g = fUv.t;\n \
    }\n \
    if (!bool(visualizeMappingU) && !bool(visualizeMappingV)) {\n \
        color = vec3(0.5);\n \
    }\n \
\n \
    if (bool(calculateLight)) {\n \
        float diffuseFactor = dot(fNormal, -lightDirection);\n \
        if (diffuseFactor > 0.0) {\n \
            fragmentColor = vec4(color, 1.0) * diffuseFactor;\n \
        } else {\n \
            fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);\n \
        }\n \
    } else {\n \
        fragmentColor = vec4(color, 1.0);\n \
    }\n \
}\n";

class Parameters
{
public:
    Parameters(void) : w(1024), h(1024), pointSize(25.0f) {}

    U w;
    U h;
    float pointSize;
    std::string pathModelFile;
};

struct ShaderVars
{
    ShaderVars()
    : model(-1), view(-1), projection(-1),
    calculateLight(-1), visualizeMappingU(-1), visualizeMappingV(-1)
    {}

    I32 model;
    I32 view;
    I32 projection;

    I32 calculateLight;
    I32 visualizeMappingU;
    I32 visualizeMappingV;
};

bool parseParameters(int argc, char * argv[], Parameters & params)
{
    namespace po = boost::program_options;

    po::options_description options("Shader dev app");

    options.add_options()
        ("help", "Show help, this print")
        ("p", po::value<float>(&params.pointSize), "point size")
        ("f", po::value<std::string>(&params.pathModelFile)->required(), "Path to model file")
        ;

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
        po::notify(vm);
    } catch (std::exception & e) {
        std::cout << e.what() << "\n";
        return false;
    }

    if (vm.count("help")) {
        std::cout << options << "\n";
        return false;
    }

    return true;
}

bool initShader(stb::Shader & shader, ShaderVars & shaderVars, const Parameters & params, stb::Log & log)
{
    {
        stb::ShaderSources sources;
        stb::buffer::StaticMemory vs(vertexShader, strlen(vertexShader));
        stb::buffer::StaticMemory fs(fragmentShader, strlen(fragmentShader));
        stb::compileShader({ stb::ShaderSource(&vs, stb::ShaderType::Vertex),
            stb::ShaderSource(&fs, stb::ShaderType::Fragment) },
            shader);

        if (stb::isError()) {
            LogFatal(log) << "Compiling shader failed, stb error: " << stb::getErrorDescription();
            return false;
        }
    }


    GL_I pointSize = -1;

    if ((shaderVars.model = glGetUniformLocation(stb::glRef(shader), "model")) == -1) {
        LogWarn(log) << "Shader uniform model not found";
    }
    if ((shaderVars.view = glGetUniformLocation(stb::glRef(shader), "view")) == -1) {
        LogWarn(log) << "Shader uniform model not found";
    }
    if ((shaderVars.projection = glGetUniformLocation(stb::glRef(shader), "projection")) == -1) {
        LogWarn(log) << "Shader uniform model not found";
    }
    if ((pointSize = glGetUniformLocation(stb::glRef(shader), "pointSize")) == -1) {
        LogWarn(log) << "Shader uniform model not found";
    }
    if ((shaderVars.calculateLight = glGetUniformLocation(stb::glRef(shader), "calculateLight")) == -1) {
        LogWarn(log) << "Shader uniform calculateLight not found";
    }
    if ((shaderVars.visualizeMappingU = glGetUniformLocation(stb::glRef(shader), "visualizeMappingU")) == -1) {
        LogWarn(log) << "Shader uniform visualizeMappingU not found";
    }
    if ((shaderVars.visualizeMappingV = glGetUniformLocation(stb::glRef(shader), "visualizeMappingV")) == -1) {
        LogWarn(log) << "Shader uniform visualizeMappingV not found";
    }

    const float fovy = glm::radians(50.f);
    const glm::mat4 projectionMatrix =
        glm::perspective<float>(fovy, static_cast<float>(params.w) / static_cast<float>(params.h), 0.1f, 100.0f);

    glUseProgram(stb::glRef(shader));
    glUniformMatrix4fv(shaderVars.projection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform1f(pointSize, params.pointSize);
    glUseProgram(0);
    return true;
}

void logModelData(const stb::ModelData & model, stb::Log & log)
{
    LogInfo(log) << "\n\n---------------------------Generated model---------------------------\n\n"

        << "Number of indices: " << model.indicesDataSize() / model.sizeOfIndiceElement()
        << ", size of an indice (bytes): " << model.sizeOfIndiceElement()
        << ", total size (bytes): " << model.indicesDataSize()
        << " (kb: " << (static_cast<float>(model.indicesDataSize()) / 1024.0f) << " )"
        << "\nNumber of attribute buffers: " << model.numberOfAttrBuffers()
        << "\nNumber of attributes: " << model.numberOfAttributes()
        << "\n"
    ;

    for (size_t i = 0; i < model.numberOfAttrBuffers(); ++i) {
        LogInfo(log) << "\n\nAttribute Buffer: " << i << "\n"
            << "Attributes per element: " << model.numberOfAttrInBuffer(i)
            << ", number of elements: "
                << (model.attrBufferSize(i) / model.attrBufferSizeOfElement(i))
            << ", size of an element (bytes): " << model.attrBufferSizeOfElement(i)
            << ", total size (bytes): " << model.attrBufferSize(i)
            << " (kbytes: " << (static_cast<float>(model.attrBufferSize(i)) / 1024.0f) << " )"
            << "\n"
        ;
    }

}

class Impl
{
public:
    enum RenderType
    {
        NORMAL,
        LINES,
        POINTS
    };

    Impl()
        : m_glContext(0),
        m_window(0),
        m_renderType(NORMAL),
        m_viewMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))),
        m_cameraDistance(0.0f),
        m_angleX(0.0f),
        m_angleY(0.0f),
        m_windowWidth(0.0f),
        m_windowHeight(0.0f),
        m_subdivides(1),
        m_generatedEntity(0),
        m_visualizationType(0)
    {}

    ~Impl()
    {
        if (m_glContext) {
            SDL_GL_DeleteContext(m_glContext);
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
        }
    }

    bool init(const Parameters & params)
    {
        m_pathModelFile = params.pathModelFile;

        stb::initFileConsoleLogger("model-viewwer.log", 0);
        m_windowWidth = static_cast<float>(params.w);
        m_windowHeight = static_cast<float>(params.h);

        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        if ((m_window = SDL_CreateWindow("Model viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params.w, params.h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_GRABBED
            )) == 0) {
            LogFatal(m_log) << "CreateWindow failed, error: " << SDL_GetError();
            return false;
        }

        if ((m_glContext = SDL_GL_CreateContext(m_window)) == 0) {
            LogFatal(m_log) << "CreateContext failed, error: " << SDL_GetError();
            return false;
        }

        if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
            LogWarn(m_log) << "SetRelativeMouseMode failed: " << SDL_GetError();
        }

        if (!stb::initGl()) {
            LogFatal(m_log) << "initializing GL failed";
            return false;
        }

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        if (!initShader(m_shader, m_shaderVars, params, m_log)) {
            return false;
        }

        return loadModel();
    }

    bool loadModel()
    {
        stb::buffer::InputFile f(m_pathModelFile.c_str());
        if (!f.ready()) {
            return false;
        }
        std::string modelData(f.size(), ' ');
        f.read(&modelData[0]);

        stb::ModelData model = stb::readModel(modelData.c_str(), modelData.size());
        if (stb::isError()) {
            return false;
        }
        logModelData(model, m_log);

        GLint layoutPos = -1;
        GLint layoutNormal = -1;
        GLint layoutUv = -1;
        if ((layoutPos = glGetAttribLocation(stb::glRef(m_shader), "position")) == -1) {
            LogWarn(m_log) << "Failed to find shader attribute position";
            return false;
        }
        if ((layoutNormal = glGetAttribLocation(stb::glRef(m_shader), "normal")) == -1) {
            LogWarn(m_log) << "Failed to find shader attribute normal";
        }
        if ((layoutUv = glGetAttribLocation(stb::glRef(m_shader), "uv")) == -1) {
            LogWarn(m_log) << "Failed to find shader attribute uv";
        }

        stb::ShaderAttributeLayoutInfo layoutInfo = { layoutPos, layoutNormal, layoutUv };
        stb::initVao(m_vao, model, layoutInfo);
        return true;
    }

    void changeRenderType()
    {
        switch (m_renderType) {
        case NORMAL:
            m_renderType = LINES;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case LINES:
            m_renderType = POINTS;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case POINTS:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            m_renderType = NORMAL;
            break;
        }
    }

    void setRenderingType()
    {

        glUseProgram(stb::glRef(m_shader));
        switch (m_visualizationType)
        {
        default:
            m_visualizationType = 0;
        case 0:
            glUniform1ui(m_shaderVars.calculateLight, 1);
            glUniform1ui(m_shaderVars.visualizeMappingU, 1);
            glUniform1ui(m_shaderVars.visualizeMappingV, 1);
            break;
        case 1:
            glUniform1ui(m_shaderVars.calculateLight, 0);
            glUniform1ui(m_shaderVars.visualizeMappingU, 0);
            glUniform1ui(m_shaderVars.visualizeMappingV, 1);
            break;
        case 2:
            glUniform1ui(m_shaderVars.calculateLight, 0);
            glUniform1ui(m_shaderVars.visualizeMappingU, 1);
            glUniform1ui(m_shaderVars.visualizeMappingV, 0);
            break;
        case 3:
            glUniform1ui(m_shaderVars.calculateLight, 1);
            glUniform1ui(m_shaderVars.visualizeMappingU, 0);
            glUniform1ui(m_shaderVars.visualizeMappingV, 0);
            break;
        case 4:
            glUniform1ui(m_shaderVars.calculateLight, 0);
            glUniform1ui(m_shaderVars.visualizeMappingU, 0);
            glUniform1ui(m_shaderVars.visualizeMappingV, 0);
        }
        glUseProgram(0);
    }

    U32 handleInput()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 1;
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    return 1;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    changeRenderType();
                } else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    loadModel();
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                    ++m_visualizationType;
                    setRenderingType();
                }
            } else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y) {
                    m_cameraDistance += static_cast<float>(event.wheel.y) * 0.1f;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                const float normalizedX = static_cast<float>(event.motion.yrel) / m_windowWidth;
                m_angleX += normalizedX;
                m_angleX = std::min(m_angleX, 1.50f);
                m_angleX = std::max(m_angleX, -1.50f);

                const float normalizedY = static_cast<float>(event.motion.xrel) / m_windowHeight;
                m_angleY += normalizedY;
                m_angleY = std::min(m_angleY, 3.40f);
                m_angleY = std::max(m_angleY, -3.40f);
            }
        }

        return 0;
    }

    void update(const TimeDurationNano /*elapsed*/)
    {

    }

    void render()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(stb::glRef(m_shader));
        glm::mat4 viewMatrix(m_viewMatrix);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, m_cameraDistance));

        glm::mat4 modelMatrix;
        modelMatrix = glm::rotate(modelMatrix, m_angleX, glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, m_angleY, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(m_shaderVars.view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(m_shaderVars.model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        switch (m_renderType) {
        case NORMAL:
        case LINES:
            stb::bindAndDraw(m_vao);
            break;
        case POINTS:
            stb::bindAndDraw(m_vao, GL_POINTS);
            break;
        }

        glUseProgram(0);
        SDL_GL_SwapWindow(m_window);
    }

private:
    SDL_GLContext m_glContext = 0;
    SDL_Window * m_window = 0;
    stb::Shader m_shader;
    stb::VertexArrayObject m_vao;
    ShaderVars m_shaderVars;
    RenderType m_renderType;

    const glm::mat4 m_viewMatrix;
    float m_cameraDistance;
    float m_angleX;
    float m_angleY;
    float m_windowWidth;
    float m_windowHeight;
    U m_subdivides;
    U32 m_generatedEntity;
    U32 m_visualizationType;

    std::string m_pathModelFile;
    stb::Log m_log;

    Impl(const Impl & ){}
    Impl operator = (const Impl &){ return *this; }
};

int main(int argc, char * argv[])
{
    Parameters params;
    Impl impl;

    if (!parseParameters(argc, argv, params)) {
        return 1;
    }

    if (impl.init(params)) {
        stb::Main<Impl, TimeDurationNano, TimePoint> runner(impl, TimeDurationNano(10000000), getTime);
        runner.run();
    }

    return 0;
}
