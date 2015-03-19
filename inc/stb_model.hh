#ifndef STB_GL_MODEL_HH_
#define STB_GL_MODEL_HH_

#include "stb_types.hh"

#include <string>
#include <vector>
#include <boost/ref.hpp>
#include <memory>
#include <cstdio>

namespace stb
{
    class ModelData
    {
    public:
        enum AttributeDataMode { TRIANGLE, TRIANGE_STRIP };
        enum AttributeBufferDataType { FLOAT, UINT32 };
        typedef std::vector<size_t> ValuesPerAttributeContainer;

        struct AttributeData
        {
            AttributeData(const char * attrBufferData,
                const size_t attrBufferDataSize,
                const ValuesPerAttributeContainer & valuesPerAttr,
                const size_t sizeOfAttrElement,
                const AttributeBufferDataType attrDataType)
            : m_attributeData(attrBufferData, attrBufferDataSize),
            m_valuesPerAttribute(valuesPerAttr),
            m_sizeOfAttributeElement(sizeOfAttrElement),
            m_dataType(attrDataType)
            {}

            std::string m_attributeData;
            ValuesPerAttributeContainer m_valuesPerAttribute;
            size_t m_sizeOfAttributeElement;
            AttributeBufferDataType m_dataType;
        };

        typedef std::shared_ptr<const AttributeData> AttributeElement;
        typedef std::vector<AttributeElement> AttributeElementContainer;

        ModelData(AttributeElementContainer attrDataBuffers,
            const char * indicesBuffer,
            const size_t indicesBufferSize,
            const size_t indiceElementSize,
            const AttributeDataMode modeOfAttrData
            );

        ModelData(const ModelData & other);
        ModelData();

        size_t numberOfAttrBuffers() const { return m_attrDataBuffers.size(); }

        const char * attrBuffer(const size_t attributeBufferIndex) const;
        size_t attrBufferSize(const size_t attributeBufferIndex) const;
        size_t attrBufferSizeOfElement(const size_t attributeBufferIndex) const;
        size_t numberOfAttrInBuffer(const size_t attributeBufferIndex) const;
        AttributeBufferDataType attrBufferDataType(const size_t attributeBufferIndex) const;
        size_t valuesPerAttribute(const size_t attributeBufferIndex, const size_t attrIndex) const;
        size_t numberOfAttributes() const { return m_numberAttributes; }
        size_t pointerToDataInBuffer(const size_t attributeBufferIndex, const size_t attrIndex) const;

        size_t indicesDataSize(void) const { return m_indicesBuffer.size(); }
        const char * indicesData(void) const { return m_indicesBuffer.c_str(); }
        size_t sizeOfIndiceElement(void) const { return m_indiceElementSize; }
        AttributeDataMode attributeDataMode(void) const { return m_modeOfAttributeData; }

        bool valid() const {
            return (m_numberAttributes != 0)
                && (m_indiceElementSize != 0)
                ;
        }

    private:
        AttributeElementContainer m_attrDataBuffers;
        U32 m_numberAttributes;

        const std::string m_indicesBuffer;
        const size_t m_indiceElementSize;
        const AttributeDataMode m_modeOfAttributeData;

        // Prevent heap allocation
        void * operator new   (size_t);
        void * operator new[](size_t);
        void  operator delete   (void *);
        void  operator delete[](void*);
        void  operator = (const ModelData & /*other*/) {}
    };

    ModelData readModel(const char * buffer, const size_t size);
    void dumpModel(const stb::ModelData & model, FILE * stream);

}

#endif
