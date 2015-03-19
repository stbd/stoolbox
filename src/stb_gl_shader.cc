#include "stb_gl_shader.hh"

#include "stb_gl.hh"
#include "stb_error.hh"
#include "stb_buffer.hh"

#include <algorithm>
#include <assert.h>

static const char * ERROR_MSG_BASE = "Shader compilation failed";
static const size_t DEFAULT_BUFFER_SIZE = 1024 * 5;
static const size_t MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT = 5;

namespace stb {
    extern void setError(const char * format, ...);
}

namespace stb {
    class ShaderAccess
    {
    public:
        ShaderAccess(Shader & shader)
            : m_shader(shader)
        {}

        GL_U & glApp() { return m_shader.glApp;  }
        const GL_U & glApp() const { return m_shader.glApp; }
        GL_U & vShader() { return m_shader.vShader; }
        const GL_U & vShader() const { return m_shader.vShader; }
        GL_U & fShader() { return m_shader.fShader; }
        const GL_U & fShader() const { return m_shader.fShader; }
    private:
        Shader & m_shader;
    };

}

stb::ShaderSource::ShaderSource(buffer::Buffer * buffer, ShaderType::Type type)
    : m_buffer(buffer), m_type(type)
{}
stb::ShaderSource::ShaderSource(const ShaderSource & other)
    : m_buffer(other.m_buffer), m_type(other.m_type)
{}
stb::ShaderSource & stb::ShaderSource::operator = (const ShaderSource & other)
{
    m_buffer = other.m_buffer;
    m_type = other.m_type;
    return *this;
}
bool stb::ShaderSource::operator < (const stb::ShaderSource & other) const
{
    if (m_type != other.m_type) {
        if (m_type == stb::ShaderType::Vertex) {
            return true;
        } else {
            return false;
        }
    } else {
        return m_buffer < other.m_buffer;
    }
}

stb::Shader::Shader()
    : glApp(0),
      vShader(0),
      fShader(0)
{}

static const char * shaderTypeToStr(GLint type)
{
    switch (type) {
        case GL_VERTEX_SHADER:
            return "Vertex shader";
        case GL_FRAGMENT_SHADER:
            return "Fragment shader";
        default:
            return "Unknown";
    }
}

static void logCompilationError(const GLuint shader, const std::string description)
{
    GLint logLength = 0;
    GLsizei length = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
        return ;
    }

    std::string logMessage(logLength, ' ');
    glGetShaderInfoLog(shader, logLength, &length, &logMessage[0]);
    stb::setError("%s\n\n\n%s\nFile: %s\n\n", ERROR_MSG_BASE, logMessage.c_str(), description.c_str());
}

static bool compile(const GLuint glProg,
                    stb::ShaderSources::const_iterator & bufferIt,
                    const size_t numberOfFiles,
                    const size_t sizeOfShader,
                    const int typeOfShader,
                    GLuint & shader
                    )
{
    std::string buffer(sizeOfShader, ' ');
    size_t bufferIndex = 0;
    GLchar * buffers[MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT] = { 0 };
    GLint bufferSizes[MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT] = { 0 };
    GLint status = 0;
    GLenum glError = GL_NO_ERROR;

    for (size_t i = 0; i < numberOfFiles; ++i) {

        bufferIt->m_buffer->read(&buffer[bufferIndex]);

        // Remove the first line which holds version
        if (i > 0) {
            for (char * c = &buffer[bufferIndex]; *c != '\n'; ++c) {
                *c = ' ';
            }
        }

        buffers[i] = &buffer[bufferIndex];
        bufferSizes[i] = bufferIt->m_buffer->size();
        bufferIndex += bufferIt->m_buffer->size();
        ++bufferIt;
    }

    if ((shader = glCreateShader(typeOfShader)) == 0) {
        stb::setError("%s: Unable to create shader, GL error: %d", ERROR_MSG_BASE, glGetError());
        goto exit_failure;
    }

    glShaderSource(shader,
                   static_cast<GLsizei>(numberOfFiles),
                   (const GLchar **)buffers,
                   (const GLint *)bufferSizes);

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        logCompilationError(shader, shaderTypeToStr(typeOfShader));
        goto exit_failure;
    }

    glAttachShader(glProg, shader);

    if ((glError = glGetError()) != GL_NO_ERROR) {
        stb::setError("%s: glGetError nonzero after attach: %d", ERROR_MSG_BASE, glError);
        goto exit_failure;
    }

    return true;

exit_failure:
    if (shader != 0) {
        glDeleteShader(shader);
        shader = 0;
    }
    return false;
}

void stb::releaseShader(stb::Shader & s)
{
    stb::ShaderAccess shader(s);
    glDetachShader(shader.glApp(), shader.vShader());
    glDeleteShader(shader.vShader());
    glDetachShader(shader.glApp(), shader.fShader());
    glDeleteShader(shader.fShader());
    glDeleteProgram(shader.glApp());
    shader.glApp() = 0;
    shader.vShader() = 0;
    shader.fShader() = 0;
}

void stb::compileShader(const stb::ShaderSources & sources,
                        stb::Shader & s)
{
    size_t numberOfFragmentShaders = 0;
    size_t numberOfVertexShaders = 0;
    size_t sizeOfFragmentShader = 0;
    size_t sizeOfVertexShader = 0;
    stb::ShaderSources::const_iterator it = sources.begin();
    GLenum glError = GL_NO_ERROR;
    stb::ShaderAccess shader(s);

    for (const ShaderSource & source : sources) {
        if (!source.m_buffer->ready()) {
            stb::setError("%s: buffer was not ready", ERROR_MSG_BASE);
            goto exit_failure;
        }
        switch (source.m_type) {
        case ShaderType::Vertex:
            ++numberOfVertexShaders;
            sizeOfVertexShader += source.m_buffer->size();
            break;
        case ShaderType::Fragment:
            ++numberOfFragmentShaders;
            sizeOfFragmentShader += source.m_buffer->size();
            break;
        }
    }

    if ((numberOfVertexShaders > MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT) ||
        (numberOfFragmentShaders > MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT)) {
        stb::setError("%s: Too many source files for shader, max supported value is %u",
            ERROR_MSG_BASE, MAX_NUMBER_OF_SHADERS_PER_COMPILATION_UNIT);
        goto exit_failure;
    }

    if ((shader.glApp() = glCreateProgram()) == 0) {
        stb::setError("%s: Create program failed, GL error: %d", ERROR_MSG_BASE, glGetError());
        goto exit_failure;
    }

    if (!compile(shader.glApp(), it, numberOfVertexShaders,
        sizeOfVertexShader, GL_VERTEX_SHADER, shader.vShader())) {
        goto exit_failure;
    }

    if (!compile(shader.glApp(), it, numberOfFragmentShaders,
        sizeOfFragmentShader, GL_FRAGMENT_SHADER, shader.fShader())) {
        goto exit_failure;
    }

    glLinkProgram(shader.glApp());

    if ((glError = glGetError()) != GL_NO_ERROR) {
        stb::setError("%s: glGetError nonzero after linking: %d", ERROR_MSG_BASE, glError);
        goto exit_failure;
    }

    return ;

exit_failure:
    releaseShader(s);
    return ;
}

void stb::activateShader(stb::Shader & s)
{
    const stb::ShaderAccess shader(s);
    glUseProgram(shader.glApp());
}

GL_U stb::glRef(stb::Shader & s)
{
    const stb::ShaderAccess shader(s);
    return shader.glApp();
}

