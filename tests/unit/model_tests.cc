#define BOOST_TEST_MODULE unit_test_model
#include <boost/test/unit_test.hpp>

#include "stb_model.hh"
#include "stb_types.hh"
#include "stb_util.hh"

using namespace stb;

BOOST_AUTO_TEST_CASE(test_single_buffer_single_attribute)
{
    const float attrData[] = {
        //Vertice x 3, rest are not used
        -0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,

        -0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    const U32 indicesData[] = {
        0, 1, 2, 0, 2, 3
    };

    const ModelData::AttributeData attr1(
        (const char *)attrData,
        sizeof(attrData),
        { 3 },
        sizeof(float) * 9,
        ModelData::FLOAT
        );

    const ModelData model(
        {
            stb::ModelData::AttributeElement(&attr1, stb::emptyDeleter<stb::ModelData::AttributeData>)
        },
        (const char *)indicesData,
        sizeof(indicesData),
        sizeof(indicesData[0]),
        ModelData::TRIANGLE
        );


    //Attribute metadata
    BOOST_CHECK_EQUAL(model.numberOfAttributes(), (size_t)1);
    BOOST_CHECK_EQUAL(model.numberOfAttrBuffers(), (size_t)1);
    BOOST_CHECK_EQUAL(model.numberOfAttrInBuffer(0), (size_t)1);
    BOOST_CHECK_EQUAL(model.attrBufferDataType(0), ModelData::FLOAT);
    BOOST_CHECK_EQUAL(model.attrBufferSize(0), sizeof(attrData));
    BOOST_CHECK_EQUAL(model.attrBufferSizeOfElement(0), 9 * sizeof(float));
    BOOST_CHECK_EQUAL(model.valuesPerAttribute(0, 0), (size_t)3);
    BOOST_CHECK_EQUAL(model.pointerToDataInBuffer(0, 0), (size_t)0);

    //Attribute data
    const float * attrDataP = (const float *)model.attrBuffer(0);
    BOOST_CHECK_EQUAL(attrDataP[0], -0.5f);
    BOOST_CHECK_EQUAL(attrDataP[1], 0.5f);
    BOOST_CHECK_EQUAL(attrDataP[2], -1.0f);
    BOOST_CHECK_EQUAL(attrDataP[3], 0.0f); //First index not used for anything

    //Indices metadata and data
    BOOST_CHECK_EQUAL(model.indicesDataSize(), 6 * sizeof(U32));
    BOOST_CHECK_EQUAL(model.sizeOfIndiceElement(), (size_t)4);
    BOOST_CHECK_EQUAL(model.indicesDataSize(), sizeof(indicesData));
    BOOST_CHECK_EQUAL(model.attributeDataMode(), ModelData::TRIANGLE);
    const U32 * indicesDataP = (const U32 *)model.indicesData();
    BOOST_CHECK_EQUAL(indicesDataP[0], (size_t)0);
    BOOST_CHECK_EQUAL(indicesDataP[1], (size_t)1);
    BOOST_CHECK_EQUAL(indicesDataP[2], (size_t)2);
    BOOST_CHECK_EQUAL(indicesDataP[3], (size_t)0);

}

BOOST_AUTO_TEST_CASE(test_multiple_buffers_multiple_attribute)
{
    const float attrData1[] = {
        //3 x vertice, 6 x unused
        -0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        -0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    const U32 attrData2[] = {
        //3 x vertice, 2 x something else
        0, 1, 1, 2, 3,
        1, 2, 2, 3, 4,
        2, 3, 4, 4, 5,
        3, 4, 5, 5, 6,
    };
    const U32 indicesData[] = {
        0, 1, 2, 0, 2, 3
    };

    const ModelData::AttributeData attr1(
        (const char *)attrData1,
        sizeof(attrData1),
        { 3 },
        sizeof(float)* 9,
        ModelData::FLOAT
        );

    const ModelData::AttributeData attr2(
        (const char *)attrData2,
        sizeof(attrData2),
        {3, 2},
        sizeof(U32)* 5,
        ModelData::UINT32
        );

    const ModelData model(
        {
            stb::ModelData::AttributeElement(&attr1, stb::emptyDeleter<stb::ModelData::AttributeData>),
            stb::ModelData::AttributeElement(&attr2, stb::emptyDeleter<stb::ModelData::AttributeData>)
        },
        (const char *)indicesData,
        sizeof(indicesData),
        sizeof(indicesData[0]),
        ModelData::TRIANGLE
        );


    //Attribute metadata
    BOOST_CHECK_EQUAL(model.numberOfAttributes(), (size_t)3);
    BOOST_CHECK_EQUAL(model.numberOfAttrBuffers(), (size_t)2);
    BOOST_CHECK_EQUAL(model.numberOfAttrInBuffer(0), (size_t)1);
    BOOST_CHECK_EQUAL(model.numberOfAttrInBuffer(1), (size_t)2);
    BOOST_CHECK_EQUAL(model.attrBufferDataType(0), ModelData::FLOAT);
    BOOST_CHECK_EQUAL(model.attrBufferDataType(1), ModelData::UINT32);
    BOOST_CHECK_EQUAL(model.attrBufferSize(0), sizeof(attrData1));
    BOOST_CHECK_EQUAL(model.attrBufferSize(1), sizeof(attrData2));
    BOOST_CHECK_EQUAL(model.attrBufferSizeOfElement(0), 9 * sizeof(float));
    BOOST_CHECK_EQUAL(model.attrBufferSizeOfElement(1), 5 * sizeof(U32));
    BOOST_CHECK_EQUAL(model.valuesPerAttribute(0, 0), (size_t)3);
    BOOST_CHECK_EQUAL(model.valuesPerAttribute(1, 0), (size_t)3);
    BOOST_CHECK_EQUAL(model.valuesPerAttribute(1, 1), (size_t)2);
    BOOST_CHECK_EQUAL(model.pointerToDataInBuffer(0, 0), (size_t)0);
    BOOST_CHECK_EQUAL(model.pointerToDataInBuffer(1, 0), (size_t)0);
    BOOST_CHECK_EQUAL(model.pointerToDataInBuffer(1, 1), sizeof(U32) * 3);

    //Attribute data
    const float * attrDataP1 = (const float *)model.attrBuffer(0);
    BOOST_CHECK_EQUAL(attrDataP1[0], attrData1[0]);
    BOOST_CHECK_EQUAL(attrDataP1[1], attrData1[1]);
    BOOST_CHECK_EQUAL(attrDataP1[2], attrData1[2]);
    BOOST_CHECK_EQUAL(attrDataP1[3], attrData1[3]); //First index not used for anything

    const U32 * attrDataP2 = (const U32 *)model.attrBuffer(1);
    BOOST_CHECK_EQUAL(attrDataP2[0], attrData2[0]);
    BOOST_CHECK_EQUAL(attrDataP2[1], attrData2[1]);
    BOOST_CHECK_EQUAL(attrDataP2[2], attrData2[2]);
    BOOST_CHECK_EQUAL(attrDataP2[3], attrData2[3]); //First index not used for anything

    //Indices metadata and data
    BOOST_CHECK_EQUAL(model.indicesDataSize(), 6 * sizeof(U32));
    BOOST_CHECK_EQUAL(model.sizeOfIndiceElement(), (size_t)4);
    BOOST_CHECK_EQUAL(model.indicesDataSize(), sizeof(indicesData));
    BOOST_CHECK_EQUAL(model.attributeDataMode(), ModelData::TRIANGLE);
    const U32 * indicesDataP = (const U32 *)model.indicesData();
    BOOST_CHECK_EQUAL(indicesDataP[0], (size_t)0);
    BOOST_CHECK_EQUAL(indicesDataP[1], (size_t)1);
    BOOST_CHECK_EQUAL(indicesDataP[2], (size_t)2);
    BOOST_CHECK_EQUAL(indicesDataP[3], (size_t)0);
}

BOOST_AUTO_TEST_CASE(test_allocating_invalid_model_and_copyconstructor_for_it)
{
    const ModelData model;
    BOOST_CHECK_EQUAL(model.valid(), false);
    
    const ModelData model2(model);
    BOOST_CHECK_EQUAL(model2.valid(), false);
}

/*
* This will cause a compile time error, uncomment to test
*/
/*
BOOST_AUTO_TEST_CASE(test_allocating_from_heap)
{
    const float attrData[] = {
        //3 x vertice, 6 x unused
        -0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, 0.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        -0.5, -.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    const U32 indicesData[] = {
        0, 1, 2, 0, 2, 3
    };

    const ModelData::AttributeData attr1(
        (const char *)attrData,
        sizeof(attrData),
        { 3 },
        sizeof(float) * 9,
        ModelData::FLOAT
        );

    const ModelData * model = new ModelData(
    {
        stb::ModelData::AttributeElement(&attr1, stb::emptyDeleter<stb::ModelData::AttributeData>)
    },
    (const char *)indicesData,
    sizeof(indicesData),
    sizeof(indicesData[0]),
    ModelData::TRIANGLE
    );
}
*/