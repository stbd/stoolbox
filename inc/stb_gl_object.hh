#ifndef STB_GL_OBJECT_HH_
#define STB_GL_OBJECT_HH_

#include "stb_types.hh"
#include <vector>

namespace stb
{
    class ModelData;
    typedef std::vector<I> ShaderAttributeLayoutInfo;
    #define STB_GL_OBJECT_MAX_NUMBER_OF_BUFFERS_PER_OBJECT 5

    struct VertexArrayObject
    {
        VertexArrayObject();
    private:
        GL_U vao;
        GL_U indiceBuffer;
        GL_I typeOfData;
        GL_I numberOfIndicesElements;
        GL_I indiceElementSizeGlEnum;
        GL_U vbo[STB_GL_OBJECT_MAX_NUMBER_OF_BUFFERS_PER_OBJECT];

        friend class VaoAccess;
    };

    void initVao(VertexArrayObject & vao,
                 const stb::ModelData & model,
                 const ShaderAttributeLayoutInfo & shaderAttributeIndexInfo
                 );
    void bindAndDraw(VertexArrayObject & vao);
    void bindAndDraw(VertexArrayObject & vao, const GL_I customDataType);
    void draw(VertexArrayObject & vao);
    void draw(VertexArrayObject & vao, const GL_I customDataType);
    void releaseVao(VertexArrayObject & vao);
}

#endif
