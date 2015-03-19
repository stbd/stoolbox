#ifndef STB_GL_SHADER_H_
#define STB_GL_SHADER_H_

#include "stb_types.hh"

#include <set>

namespace stb
{
    namespace buffer {
        class Buffer;
    }
    namespace ShaderType {
        enum Type
        {
            Fragment,
            Vertex
        };
    }
    struct ShaderSource
    {
        ShaderSource(buffer::Buffer * buffer, ShaderType::Type type);
        ShaderSource(const ShaderSource & other);
        ShaderSource & operator = (const ShaderSource & other);
        bool operator < (const ShaderSource & other) const;

        buffer::Buffer * m_buffer;
        ShaderType::Type m_type;
    };
    typedef std::set<ShaderSource> ShaderSources;

    struct Shader
    {
        Shader(void);
    private:
        GL_U glApp;
        GL_U vShader;
        GL_U fShader;

        friend class ShaderAccess;
    };

    void compileShader(const ShaderSources & sources, stb::Shader & shader);
    void activateShader(stb::Shader & shader);
    void releaseShader(Shader & shader);
    GL_U glRef(stb::Shader & shader);
}

#endif
