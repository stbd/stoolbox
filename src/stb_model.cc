#include "stb_model.hh"

#include "stb_util.hh"
#include "stb_types.hh"
#include "stb_error.hh"

#include <cassert>
#include <iterator>
#include <cstring>

using namespace stb;

namespace stb
{
    extern void setError(const char * format, ...);
}

static const size_t SIZE_OF_MODEL_HEADER = 5;
static const size_t INDEX_VERSION = 0;
static const size_t INDEX_FORMAT = 1;

ModelData::ModelData(const ModelData & other)
    : m_attrDataBuffers(other.m_attrDataBuffers),
    m_numberAttributes(other.m_numberAttributes),
    m_indicesBuffer(other.m_indicesBuffer),
    m_indiceElementSize(other.m_indiceElementSize),
    m_modeOfAttributeData(other.m_modeOfAttributeData)
{}

ModelData::ModelData()
:
m_numberAttributes(0),
m_indiceElementSize(0),
m_modeOfAttributeData(TRIANGLE)
{}

ModelData::ModelData(AttributeElementContainer attrDataBuffers,
    const char * indicesBuffer,
    const size_t indicesBufferSize,
    const size_t indiceElementSize,
    const ModelData::AttributeDataMode modeOfAttrData
    )
    : m_attrDataBuffers(attrDataBuffers),
    m_numberAttributes(0),
    m_indicesBuffer(indicesBuffer, indicesBufferSize),
    m_indiceElementSize(indiceElementSize),
    m_modeOfAttributeData(modeOfAttrData)

{
    for (AttributeElementContainer::const_iterator it = m_attrDataBuffers.begin(); it != m_attrDataBuffers.end(); ++it) {
        m_numberAttributes += (*it)->m_valuesPerAttribute.size();
    }
}

const char * ModelData::attrBuffer(const size_t attributeBufferIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_attributeData.c_str();
}

size_t ModelData::attrBufferSize(const size_t attributeBufferIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_attributeData.size();
}

size_t ModelData::attrBufferSizeOfElement(const size_t attributeBufferIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_sizeOfAttributeElement;
}

size_t ModelData::numberOfAttrInBuffer(const size_t attributeBufferIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_valuesPerAttribute.size();
}

ModelData::AttributeBufferDataType ModelData::attrBufferDataType(const size_t attributeBufferIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_dataType;
}

size_t ModelData::valuesPerAttribute(const size_t attributeBufferIndex, const size_t attrIndex) const
{
    return m_attrDataBuffers[attributeBufferIndex]->m_valuesPerAttribute[attrIndex];
}

size_t ModelData::pointerToDataInBuffer(const size_t attributeBufferIndex, const size_t attrIndex) const
{
    size_t attrPointer = 0;
    size_t i = 0;
    for (ValuesPerAttributeContainer::const_iterator it = m_attrDataBuffers[attributeBufferIndex]->m_valuesPerAttribute.begin();
        it != m_attrDataBuffers[attributeBufferIndex]->m_valuesPerAttribute.end(); ++it) {
        if (i == attrIndex) {
            break;
        }
        attrPointer += (*it) * sizeof(float); //If other than floats or uints are required some day, this will break
        ++i;
    }
    return attrPointer;
}

static stb::ModelData parseVn(const char * buffer, const size_t size)
{
    const char * end = buffer + size;
    if ((size_t)(end - buffer) < 6) {
        stb::setError("%s:%u", __FUNCTION__, __LINE__);
        return stb::ModelData();
    }

    size_t sizeOfIndice = 0;
    memcpy(&sizeOfIndice, buffer, 1);
    buffer += 1;

    size_t numberOfIndices = 0;
    {
        memcpy(&numberOfIndices, buffer, 4);
        buffer += 4;
        if ((size_t)(end - buffer) < (numberOfIndices * sizeOfIndice)) {
            stb::setError("%s:%u: %zu - %zu", __FUNCTION__, __LINE__,
                (end - buffer), (numberOfIndices * sizeOfIndice));
            return stb::ModelData();
        }
    }

    std::string indices(buffer, numberOfIndices * sizeOfIndice);
    buffer += numberOfIndices * sizeOfIndice;

    const size_t sizeOfAttrElement = sizeof(float) * 7;
    size_t numberOfAttributes = 0;
    {
        memcpy(&numberOfAttributes, buffer, 4);
        buffer += 4;
        if ((size_t)(end - buffer) < (numberOfAttributes * sizeOfAttrElement)) {
            stb::setError("%s:%u: %zu - %zu", __FUNCTION__, __LINE__,
                (end - buffer), (numberOfAttributes * sizeOfAttrElement));
            return stb::ModelData();
        }
    }

    stb::ModelData::AttributeData * element
        = new stb::ModelData::AttributeData(
        (const char *)buffer,
        numberOfAttributes * sizeOfAttrElement,
        { 4, 3 },
        sizeOfAttrElement,
        stb::ModelData::FLOAT
        );

    return stb::ModelData(
        { stb::ModelData::AttributeElement(element) },
        (const char *)indices.c_str(),
        indices.size(),
        sizeOfIndice,
        stb::ModelData::TRIANGLE
    );
}

stb::ModelData stb::readModel(const char * buffer, const size_t size)
{
    std::string format(4, ' ');

    if (size < 5) {
        stb::setError("%s:%u", __FUNCTION__, __LINE__);
        return stb::ModelData();
    }

    if ((U8)*(buffer + INDEX_VERSION) != 1) {
        stb::setError("%s: Unsupported version %u",
            __FUNCTION__, (U8)*(buffer + INDEX_VERSION));
        return stb::ModelData();
    }

    memcpy(&format[0], &buffer[INDEX_FORMAT], 4);

    const size_t parsedSoFar = 5;
    if (format == "vn  ") {
        return parseVn(buffer + parsedSoFar, size - parsedSoFar);
    } else {
        stb::setError("%s:%u", __FUNCTION__, __LINE__);
        return stb::ModelData();
    }
}


void stb::dumpModel(const ModelData & model, FILE * stream)
{
    const size_t numberOfAttrBuffers = model.numberOfAttrBuffers();
    fprintf(stream, "Dumping model at %p\n", (void *)&model);
    fprintf(stream, "Valid: %u\n", (U32)model.valid());

    switch (model.attributeDataMode()) {
    case ModelData::TRIANGLE:
        fprintf(stream, "Type of data: triangle\n");
        break;
    case ModelData::TRIANGE_STRIP:
        fprintf(stream, "Type of data: triangle strip\n");
        break;
    }

    fprintf(stream, "Size of indice element %zuB, size of indice buffer: %zuB\n",
        model.sizeOfIndiceElement(), model.indicesDataSize());

    fprintf(stream, "Number of attribute buffers: %zu\n", numberOfAttrBuffers);

    for (size_t b = 0; b < numberOfAttrBuffers; ++b) {
        const size_t values = model.numberOfAttrInBuffer(b);
        fprintf(stream, "Buffer %zu has %zu attributes that consist of values:", b, values);
        for (size_t v = 0; v < values; ++v) {
            fprintf(stream, " %zu (starting at %zu)",
                model.valuesPerAttribute(b, v), model.pointerToDataInBuffer(b, v));
        }
        fprintf(stream, "\n");

        switch (model.attrBufferDataType(b)) {
        case stb::ModelData::FLOAT:
            fprintf(stream, "Buffer %zu type of data FLOAT\n", b);
            break;
        case stb::ModelData::UINT32:
            fprintf(stream, "Buffer %zu type of data UINT32\n", b);
            break;
        }

        fprintf(stream, "Buffer %zu: size of an element %zu\n", b, model.attrBufferSizeOfElement(b));
        fprintf(stream, "Buffer %zu: total buffer size %zu\n", b, model.attrBufferSize(b));
    }

    for (size_t b = 0; b < numberOfAttrBuffers; ++b) {
        fprintf(stream, "Dumping buffer %zu:\n", b);
        const char * p = model.attrBuffer(b);
        const char * end = model.attrBuffer(b) + model.attrBufferSize(b);
        while (p < end) {
            fprintf(stream, "%zu:", (p - model.attrBuffer(b)));

            switch (model.attrBufferDataType(b)) {
            case stb::ModelData::FLOAT:
                for (const char * v = p; v < (p + model.attrBufferSizeOfElement(b)); v += 4) {
                    fprintf(stream, " %f", *(float *)v);
                }
                break;
            case stb::ModelData::UINT32:
                for (const char * v = p; v < (p + model.attrBufferSizeOfElement(b)); v += 4) {
                    fprintf(stream, "%u", *(U32 *)v);
                }
                break;
            }
            fprintf(stream, "\n");
            p += model.attrBufferSizeOfElement(b);
        }
    }

    {
        fprintf(stream, "Dumping indices\n");
        const char * i = model.indicesData();
        const char * end = model.indicesData() + model.indicesDataSize();
        while (i < end) {
            fprintf(stream, "%zu:", (i - model.indicesData()));
            switch (model.sizeOfIndiceElement()) {
            case 2:
                fprintf(stream, " %u", *(U16 *)i);
                i += 2;
                break;
            case 4:
                fprintf(stream, " %u", *(U32 *)i);
                i += 4;
                break;
            }
            fprintf(stream, "\n");
        }
    }
}
