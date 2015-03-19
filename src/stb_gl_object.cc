#include "stb_gl_object.hh"

#include "stb_model.hh"
#include "stb_gl.hh"
#include "stb_math.hh"
#include <cstring>
#include <cassert>

namespace stb {
    class VaoAccess
    {
    public:
        VaoAccess(VertexArrayObject & vao)
        : m_vao(vao)
        {}

        GL_U & vao() { return m_vao.vao; }
        const GL_U & vao() const { return m_vao.vao; }
        GL_U & indiceBuffer() { return m_vao.indiceBuffer; }
        const GL_U & indiceBuffer() const { return m_vao.indiceBuffer; }
        GL_I & typeOfData() { return m_vao.typeOfData; }
        const GL_I & typeOfData() const { return m_vao.typeOfData; }
        GL_I & numberOfIndicesElements() { return m_vao.numberOfIndicesElements; }
        const GL_I & numberOfIndicesElements() const { return m_vao.numberOfIndicesElements; }
        GL_I & indiceElementSizeGlEnum() { return m_vao.indiceElementSizeGlEnum; }
        const GL_I & indiceElementSizeGlEnum() const { return m_vao.indiceElementSizeGlEnum; }
        GL_U * vbo() { return m_vao.vbo; }

    private:
        VertexArrayObject & m_vao;
    };

}

stb::VertexArrayObject::VertexArrayObject()
    : vao(0),
      indiceBuffer(0),
      typeOfData(0),
      indiceElementSizeGlEnum(0)
{
    memset(vbo, 0, sizeof(vbo[0]) * STB_GL_OBJECT_MAX_NUMBER_OF_BUFFERS_PER_OBJECT);
}

void stb::initVao(VertexArrayObject & v,
                  const stb::ModelData & model,
                  const ShaderAttributeLayoutInfo & shaderAttributeIndexInfo
                  )
{
    VaoAccess vao(v);

    glGenVertexArrays(1, &vao.vao());
    glBindVertexArray(vao.vao());

    ShaderAttributeLayoutInfo::const_iterator it = shaderAttributeIndexInfo.begin();
    for (size_t attributeBufferIndex = 0;
         attributeBufferIndex < model.numberOfAttrBuffers();
         ++attributeBufferIndex) {

        bool outOfData = false;

        glGenBuffers(1, &vao.vbo()[attributeBufferIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, vao.vbo()[attributeBufferIndex]);
        glBufferData(GL_ARRAY_BUFFER,
                     model.attrBufferSize(attributeBufferIndex),
                     model.attrBuffer(attributeBufferIndex),
                     GL_STATIC_DRAW); // TODO: parametritize draw type

        for (size_t bufferIndex = 0;
             bufferIndex < model.numberOfAttrInBuffer(attributeBufferIndex);
             ++bufferIndex) {

            if ((*it) == -1) continue;

            const size_t numberOfValues = model.valuesPerAttribute(attributeBufferIndex, bufferIndex);
            glEnableVertexAttribArray(*it);
            switch (model.attrBufferDataType(attributeBufferIndex)) {
            case stb::ModelData::FLOAT:
                glVertexAttribPointer(*it,
                                      numberOfValues,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      model.attrBufferSizeOfElement(attributeBufferIndex),
                                      (const void *)model.pointerToDataInBuffer(
                                                                 attributeBufferIndex, bufferIndex));
                break;
            case stb::ModelData::UINT32:
                glVertexAttribIPointer(*it,
                                       numberOfValues,
                                       GL_UNSIGNED_INT,
                                       model.attrBufferSizeOfElement(attributeBufferIndex),
                                       (const void *)model.pointerToDataInBuffer(
                                                                attributeBufferIndex, bufferIndex));
                break;
            }

            ++it;
            if (it == shaderAttributeIndexInfo.end()) {
                outOfData = true;
                break;
            }
        }
        if (outOfData) {
            break;
        }
    }

    glGenBuffers(1, &vao.indiceBuffer());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.indiceBuffer());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 model.indicesDataSize(),
                 model.indicesData(),
                 GL_STATIC_DRAW); // TODO: parametritize draw type

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    switch (model.attributeDataMode()) {
    case ModelData::TRIANGLE:
        vao.typeOfData() = GL_TRIANGLES;
        break;
    case ModelData::TRIANGE_STRIP:
        vao.typeOfData() = GL_TRIANGLE_STRIP;
        break;
    }

    switch (model.sizeOfIndiceElement()) {
    case 2:
        vao.indiceElementSizeGlEnum() = GL_UNSIGNED_SHORT;
        break;
    case 4:
        vao.indiceElementSizeGlEnum() = GL_UNSIGNED_INT;
        break;
    default:
        assert(!"Unhandled indice element size");
    }

    vao.numberOfIndicesElements() = model.indicesDataSize() / model.sizeOfIndiceElement();
}

void stb::bindAndDraw(VertexArrayObject & v)
{
    stb::VaoAccess vao(v);
    glBindVertexArray(vao.vao());
    glDrawElements(vao.typeOfData(), vao.numberOfIndicesElements(), vao.indiceElementSizeGlEnum(), 0);
}

void stb::draw(VertexArrayObject & v)
{
    const stb::VaoAccess vao(v);
    glDrawElements(vao.typeOfData(), vao.numberOfIndicesElements(), vao.indiceElementSizeGlEnum(), 0);
}

void stb::bindAndDraw(VertexArrayObject & v, const GL_I customDataType)
{
    stb::VaoAccess vao(v);
    glBindVertexArray(vao.vao());
    glDrawElements(customDataType, vao.numberOfIndicesElements(), vao.indiceElementSizeGlEnum(), 0);
}

void stb::draw(VertexArrayObject & v, const GL_I customDataType)
{
    const stb::VaoAccess vao(v);
    glDrawElements(customDataType, vao.numberOfIndicesElements(), vao.indiceElementSizeGlEnum(), 0);
}

void stb::releaseVao(VertexArrayObject & v)
{
    stb::VaoAccess vao(v);
    glDeleteBuffers(STB_GL_OBJECT_MAX_NUMBER_OF_BUFFERS_PER_OBJECT, vao.vbo());
    glDeleteBuffers(1, &vao.indiceBuffer());
    glDeleteVertexArrays(1, &vao.vao());
}
