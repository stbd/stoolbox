#include "stb_generator.hh"

#include "stb_model.hh"
#include "stb_math.hh"
#include <cassert>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

typedef glm::vec3 Vertex;
typedef glm::vec3 Normal;
typedef glm::vec2 Uv;

struct Attribute
{
    Attribute()
    : vertexX(0.0f), vertexY(0.0f), vertexZ(0.0f),
    normalX(0.0f), normalY(0.0f), normalZ(0.0f),
    u(0.0f), v(0.0f)
    {}

    Attribute(const Vertex & vertex)
    : vertexX(vertex.x), vertexY(vertex.y), vertexZ(vertex.z),
    normalX(0.0f), normalY(0.0f), normalZ(0.0f),
    u(0.0f), v(0.0f)
    {}

    Attribute(const Vertex & vertex, const Normal & normal)
        : vertexX(vertex.x), vertexY(vertex.y), vertexZ(vertex.z),
        normalX(normal.x), normalY(normal.y), normalZ(normal.z),
        u(0.0f), v(0.0f)
    {}

    Attribute(const Vertex & vertex, const Normal & normal, const Uv & uv)
        : vertexX(vertex.x), vertexY(vertex.y), vertexZ(vertex.z),
        normalX(normal.x), normalY(normal.y), normalZ(normal.z),
        u(uv.s), v(uv.t)
    {}

    bool operator == (const Attribute & other) const
    {
        // This is safe only for objects in scale of unit cube,
        // so do not copy this comparison
        const float diff = 0.00001f;
        if (
            (std::abs(other.vertexX - vertexX) < diff)
            && (std::abs(other.vertexY - vertexY) < diff)
            && (std::abs(other.vertexZ - vertexZ) < diff)
            && (std::abs(other.normalX - normalX) < diff)
            && (std::abs(other.normalY - normalY) < diff)
            && (std::abs(other.normalZ - normalZ) < diff)
            && (std::abs(other.u - u) < diff)
            && (std::abs(other.v - v) < diff)
            )
        {
            return true;
        }
        return false;
    }

    Vertex getVertex() const { return Vertex(vertexX, vertexY, vertexZ); }
    Normal getNormal() const { return Normal(normalX, normalY, normalZ); }
    void setVertex(const Vertex & v){ vertexX = v.x; vertexY = v.y; vertexZ = v.z; }
    void setNormal(const Normal & n){ normalX = n.x; normalY = n.y; normalZ = n.z; }
    Uv getUv() const { return Uv(u, v); }
    void reset(){ vertexX = 0.0; vertexY = 0.0f; vertexZ = 0.0f; normalX = 0.0f; normalY = 0.0f; normalZ = 0.0f; u = 0.0f; v = 0.0f; }

    static size_t elementSizeInBytes() { return sizeof(float)* 8; }

    float vertexX, vertexY, vertexZ;
    float normalX, normalY, normalZ;
    float u, v;
};
typedef std::vector<Attribute> AttributeContainer;
typedef std::vector<U32> IndiceContainter;

static void discreteInterpolate1(const float start, const float end, const U subdivides, const U step, float & result)
{
    result = start + (((end - start) / static_cast<float>(subdivides + 1)) * static_cast<float>(step));
}

static void insertVertex(AttributeContainer & attributeContainer, IndiceContainter & indiceContainer,
    const Attribute & attribute)
{
    size_t index = attributeContainer.size();
    attributeContainer.push_back(attribute);
    indiceContainer.push_back(index);
}

static void reuseVertex(IndiceContainter & indiceContainer, const U32 reusedIndiceIndex)
{
    indiceContainer.push_back(indiceContainer[reusedIndiceIndex]);
}

static void mergeAndWeldEqualVertices(AttributeContainer & targetAttributes, IndiceContainter & targetIndices,
    const AttributeContainer & sourceAttributes, const IndiceContainter & sourceIndices)
{
    for (U i = 0; i < sourceIndices.size(); ++i) {
        const Attribute & attribute = sourceAttributes[sourceIndices[i]];
        AttributeContainer::const_iterator it = std::find(targetAttributes.begin(), targetAttributes.end(), attribute);
        if (it != targetAttributes.end()) {
            const size_t existingIndex = std::distance(AttributeContainer::const_iterator(targetAttributes.begin()), it);
            targetIndices.push_back(existingIndex);
        } else {
            targetIndices.push_back(targetAttributes.size());
            targetAttributes.push_back(attribute);
        }
    }
}

void generateAttributesFor2dPlane(
    const float x0, const float y0,
    const float x1, const float y1,
    const Uv & startUv, const Uv & endUv,
    const U subdivides,
    const U currentSquareRow,
    const U currentSquareColumn,
    const U tringleIndex,
    const U tringlePointIndex,
    float & resultX,
    float & resultY,
    float & resultU,
    float & resultV)
{
    U verticeRow = 0;
    U verticeColumn = 0;

    if (tringleIndex % 2 == 0) {
        //Left side triangle
        if (tringlePointIndex == 0) {
            verticeRow = currentSquareRow;
            verticeColumn = currentSquareColumn;
        } else if (tringlePointIndex == 1) {
            verticeRow = currentSquareRow + 1;
            verticeColumn = currentSquareColumn;
        } else if (tringlePointIndex == 2) {
            verticeRow = currentSquareRow;
            verticeColumn = currentSquareColumn + 1;
        } else {
            assert(!"Invalid triangle index");
        }

    } else {
        //Right side triangle
        if (tringlePointIndex == 0) {
            verticeRow = currentSquareRow;
            verticeColumn = currentSquareColumn + 1;
        } else if (tringlePointIndex == 1) {
            verticeRow = currentSquareRow + 1;
            verticeColumn = currentSquareColumn;
        } else if (tringlePointIndex == 2) {
            verticeRow = currentSquareRow + 1;
            verticeColumn = currentSquareColumn + 1;
        } else {
            assert(!"Invalid triangle index");
        }
    }

    discreteInterpolate1(x0, x1, subdivides, verticeColumn, resultX);
    discreteInterpolate1(y0, y1, subdivides, verticeRow, resultY);

    discreteInterpolate1(startUv.s, endUv.s, subdivides, verticeColumn, resultU);
    discreteInterpolate1(startUv.t, endUv.t, subdivides, verticeRow, resultV);
}

static void generatePlaneVertices(
    const U32 squaresPerface,
    const U32 squaresPerRow,
    const U32 subdivides,
    const float x0, const float x1, const float y0, const float y1,
    const Uv & startUv, const Uv & endUv,
    AttributeContainer & attributeContainer, IndiceContainter & indiceContainer,

    Attribute & v0, Attribute & v1, Attribute & v2, Attribute & v3,

    float & v0d0, float & v0d1,
    float & v1d0, float & v1d1,
    float & v2d0, float & v2d1,
    float & v3d0, float & v3d1
    )
{
    for (U s = 0; s < squaresPerface; ++s) {

        const U currentSquareRow = s / squaresPerRow;
        const U currentSquareColumn = s % squaresPerRow;

        if (currentSquareColumn == 0) {
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 0, 0, v0d0, v0d1, v0.u, v0.v);
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 0, 1, v1d0, v1d1, v1.u, v1.v);
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 0, 2, v2d0, v2d1, v2.u, v2.v);
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 1, 2, v3d0, v3d1, v3.u, v3.v);

            insertVertex(attributeContainer, indiceContainer, v0);
            insertVertex(attributeContainer, indiceContainer, v1);
            insertVertex(attributeContainer, indiceContainer, v2);
            reuseVertex(indiceContainer, indiceContainer.size() - 1);
            reuseVertex(indiceContainer, indiceContainer.size() - 3);
            insertVertex(attributeContainer, indiceContainer, v3);
        } else {
            const U startOfPreviousSquare = indiceContainer.size() - 6;
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 0, 2, v1d0, v1d1, v1.u, v1.v);
            generateAttributesFor2dPlane(x0, y0, x1, y1, startUv, endUv, subdivides, currentSquareRow, currentSquareColumn, 1, 2, v3d0, v3d1, v3.u, v3.v);
            reuseVertex(indiceContainer, startOfPreviousSquare + 2);
            reuseVertex(indiceContainer, startOfPreviousSquare + 5);
            insertVertex(attributeContainer, indiceContainer, v1);
            reuseVertex(indiceContainer, indiceContainer.size() - 1);
            reuseVertex(indiceContainer, indiceContainer.size() - 3);
            insertVertex(attributeContainer, indiceContainer, v3);
        }
    }
}

stb::ModelData stb::generateCube(const U subdivides)
{
    const U numberOfBaseVertices = 8;
    const float baseVertices[numberOfBaseVertices][3] = {
        //Top
        { -0.5, 0.5, 0.5 },
        { 0.5, 0.5, 0.5 },
        { 0.5, 0.5, -0.5 },
        { -0.5, 0.5, -0.5 },
        //Bottom
        { -0.5, -0.5, 0.5 },
        { 0.5, -0.5, 0.5 },
        { 0.5, -0.5, -0.5 },
        { -0.5, -0.5, -0.5 }
    };

    const U squaresPerface = (U)std::pow(1 + subdivides, 2);
    const U triangles = squaresPerface * 2 * 6;
    const U vertices = triangles * 3;
    const U verticesPerFace = squaresPerface * 2 * 3;

    AttributeContainer vertexContainer(vertices);
    IndiceContainter indiceContainer(vertices, 0);
    const U squaresPerRow = subdivides + 1;

    Normal normal(0.0f, 0.0f, 1.0f);
    Vertex vertex(0.0f, 0.0f, 0.5f);
    Attribute v0(vertex, normal);
    Attribute v1(vertex, normal);
    Attribute v2(vertex, normal);
    Attribute v3(vertex, normal);
    const float uvStep = 1.0f / 6.0f;

    vertexContainer.clear();
    indiceContainer.clear();

    generatePlaneVertices(
        squaresPerface,
        squaresPerRow,
        subdivides,
        baseVertices[0][0], baseVertices[1][0],
        baseVertices[0][1], baseVertices[4][1],
        Uv(0.0f * uvStep, 0.0f), Uv(1.0f * uvStep, 1.0),
        vertexContainer, indiceContainer,
        v0, v1, v2, v3,
        v0.vertexX, v0.vertexY,
        v1.vertexX, v1.vertexY,
        v2.vertexX, v2.vertexY,
        v3.vertexX, v3.vertexY
        );

    normal = Normal(1.0f, 0.0f, 0.0f);
    vertex = Vertex(0.5f, 0.0f, 0.0f);
    v0.setNormal(normal); v1.setNormal(normal); v2.setNormal(normal); v3.setNormal(normal);
    v0.setVertex(vertex); v1.setVertex(vertex); v2.setVertex(vertex); v3.setVertex(vertex);

    generatePlaneVertices(
        squaresPerface,
        squaresPerRow,
        subdivides,
        baseVertices[1][2], baseVertices[2][2],
        baseVertices[1][1], baseVertices[5][1],
        Uv(1.0f * uvStep, 0.0f), Uv(2.0f * uvStep, 1.0),
        vertexContainer, indiceContainer,
        v0, v1, v2, v3,
        v0.vertexZ, v0.vertexY,
        v1.vertexZ, v1.vertexY,
        v2.vertexZ, v2.vertexY,
        v3.vertexZ, v3.vertexY
        );

    normal = Normal(0.0f, 0.0f, -1.0f);
    vertex = Vertex(0.5f, 0.0f, -0.5f);
    v0.setNormal(normal); v1.setNormal(normal); v2.setNormal(normal); v3.setNormal(normal);
    v0.setVertex(vertex); v1.setVertex(vertex); v2.setVertex(vertex); v3.setVertex(vertex);

    generatePlaneVertices(
        squaresPerface,
        squaresPerRow,
        subdivides,
        baseVertices[2][0], baseVertices[3][0],
        baseVertices[2][1], baseVertices[6][1],
        Uv(2.0f * uvStep, 0.0f), Uv(3.0f * uvStep, 1.0),
        vertexContainer, indiceContainer,
        v0, v1, v2, v3,
        v0.vertexX, v0.vertexY,
        v1.vertexX, v1.vertexY,
        v2.vertexX, v2.vertexY,
        v3.vertexX, v3.vertexY
        );


    AttributeContainer planeAttributeContainer(verticesPerFace);
    IndiceContainter planeIndiceContainer(verticesPerFace);

    {
        planeAttributeContainer.clear();
        planeIndiceContainer.clear();

        normal = Normal(-1.0f, 0.0f, 0.0f);
        vertex = Vertex(-0.5f, 0.0f, 0.0f);
        v0.setNormal(normal); v1.setNormal(normal); v2.setNormal(normal); v3.setNormal(normal);
        v0.setVertex(vertex); v1.setVertex(vertex); v2.setVertex(vertex); v3.setVertex(vertex);

        generatePlaneVertices(
            squaresPerface,
            squaresPerRow,
            subdivides,
            baseVertices[3][2], baseVertices[0][2],
            baseVertices[3][1], baseVertices[7][1],
            Uv(3.0f * uvStep, 0.0f), Uv(4.0f * uvStep, 1.0),
            planeAttributeContainer, planeIndiceContainer,
            v0, v1, v2, v3,
            v0.vertexZ, v0.vertexY,
            v1.vertexZ, v1.vertexY,
            v2.vertexZ, v2.vertexY,
            v3.vertexZ, v3.vertexY
            );

        mergeAndWeldEqualVertices(vertexContainer, indiceContainer, planeAttributeContainer, planeIndiceContainer);
    }

    {
        planeAttributeContainer.clear();
        planeIndiceContainer.clear();

        normal = Normal(0.0f, -1.0f, 0.0f);
        vertex = Vertex(0.0f, -0.5f, 0.0f);
        v0.setNormal(normal); v1.setNormal(normal); v2.setNormal(normal); v3.setNormal(normal);
        v0.setVertex(vertex); v1.setVertex(vertex); v2.setVertex(vertex); v3.setVertex(vertex);

        generatePlaneVertices(
            squaresPerface,
            squaresPerRow,
            subdivides,
            baseVertices[7][2], baseVertices[4][2],
            baseVertices[4][0], baseVertices[5][0],
            Uv(4.0f * uvStep, 0.0f), Uv(5.0f * uvStep, 1.0),
            planeAttributeContainer, planeIndiceContainer,
            v0, v1, v2, v3,
            v0.vertexZ, v0.vertexX,
            v1.vertexZ, v1.vertexX,
            v2.vertexZ, v2.vertexX,
            v3.vertexZ, v3.vertexX
            );

        mergeAndWeldEqualVertices(vertexContainer, indiceContainer, planeAttributeContainer, planeIndiceContainer);
    }

    {
        planeAttributeContainer.clear();
        planeIndiceContainer.clear();

        normal = Normal(0.0f, 1.0f, 0.0f);
        vertex = Vertex(0.0f, 0.5f, 0.0f);
        v0.setNormal(normal); v1.setNormal(normal); v2.setNormal(normal); v3.setNormal(normal);
        v0.setVertex(vertex); v1.setVertex(vertex); v2.setVertex(vertex); v3.setVertex(vertex);

        generatePlaneVertices(
            squaresPerface,
            squaresPerRow,
            subdivides,
            baseVertices[0][2], baseVertices[3][2],
            baseVertices[0][0], baseVertices[1][0],
            Uv(5.0f * uvStep, 0.0f), Uv(6.0f * uvStep, 1.0),
            planeAttributeContainer, planeIndiceContainer,
            v0, v1, v2, v3,
            v0.vertexZ, v0.vertexX,
            v1.vertexZ, v1.vertexX,
            v2.vertexZ, v2.vertexX,
            v3.vertexZ, v3.vertexX
            );

        mergeAndWeldEqualVertices(vertexContainer, indiceContainer, planeAttributeContainer, planeIndiceContainer);
    }

    stb::ModelData::AttributeData * element = new stb::ModelData::AttributeData(
        (const char *)&vertexContainer[0],
        Attribute::elementSizeInBytes() * vertexContainer.size(),
        { 3, 3, 2 },
        sizeof(float) * 8,
        stb::ModelData::FLOAT
        );

    return stb::ModelData(
        { stb::ModelData::AttributeElement(element) },
        (const char *)&indiceContainer[0],
        sizeof(IndiceContainter::value_type) * indiceContainer.size(),
        sizeof(indiceContainer[0]),
        stb::ModelData::TRIANGLE
    );
}

void insertAttributeToContainerCheckIfExists(AttributeContainer & attributeContainer,
    IndiceContainter & indiceContainer, const Attribute & attribute)
{
    AttributeContainer::const_iterator it = std::find(attributeContainer.begin(), attributeContainer.end(), attribute);
    if (it != attributeContainer.end()) {
        const size_t existingIndex = std::distance(AttributeContainer::const_iterator(attributeContainer.begin()), it);
        indiceContainer.push_back(existingIndex);
    } else {
        indiceContainer.push_back(attributeContainer.size());
        attributeContainer.push_back(attribute);
    }
}

static Uv calculateUvCoordinate(const Vertex & vertex, const float radius)
{
    const float pi = 3.14159265359f;
    const float v = (radius + vertex.y);

    float u = glm::angle(glm::vec2(0.0f, 1.0f), glm::normalize(glm::vec2(vertex.x, vertex.z)));
    assert(!isnan(u));
    if (vertex[0] < 0.0)
        u = pi + (pi - u);

    return Uv(u / (pi * 2), v);
}

void generateTriangleOnSphere(
    const Attribute & top, const Attribute & bottomLeft, const Attribute & bottomRight,
    const U subdivides,
    AttributeContainer & attributeContainer, IndiceContainter & indiceContainer, bool direction,
    const float radius
    )
{
    if (subdivides == 0) {
        if (direction) {
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, top);
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, bottomLeft);
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, bottomRight);
        } else {
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, bottomRight);
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, bottomLeft);
            insertAttributeToContainerCheckIfExists(attributeContainer, indiceContainer, top);
        }
        return ;
    }

    const Vertex v0 = glm::normalize((top.getVertex() + bottomLeft.getVertex()) / 2.0f);
    const Vertex v1 = glm::normalize((top.getVertex() + bottomRight.getVertex()) / 2.0f);
    const Vertex v2 = glm::normalize((bottomRight.getVertex() + bottomLeft.getVertex()) / 2.0f);
    const Attribute a0(radius * v0, v0, calculateUvCoordinate(v0, radius));
    const Attribute a1(radius * v1, v1, calculateUvCoordinate(v1, radius));
    const Attribute a2(radius * v2, v2, calculateUvCoordinate(v2, radius));

    generateTriangleOnSphere(top, a0, a1, subdivides - 1, attributeContainer, indiceContainer, direction, radius);
    generateTriangleOnSphere(a0, bottomLeft, a2, subdivides - 1, attributeContainer, indiceContainer, direction, radius);
    generateTriangleOnSphere(a1, a2, bottomRight, subdivides - 1, attributeContainer, indiceContainer, direction, radius);
    generateTriangleOnSphere(a0, a2, a1, subdivides - 1, attributeContainer, indiceContainer, direction, radius);
}

stb::ModelData stb::generateSphere(const U subdivides, const float radius)
{
    const U numberOfBaseVertices = 6;
    const Attribute baseVertices[numberOfBaseVertices] = {
        Attribute(Vertex(0.0f, radius, 0.0f), Normal(0.0f, 1.0f, 0.0f), Uv(0.0f, 1.0f)), // Top
        Attribute(Vertex(0.0f, 0.0f, radius), Normal(0.0f, 0.0f, 1.0f), calculateUvCoordinate(Vertex(0.0f, 0.0f, radius), radius)),// Below previous
        Attribute(Vertex(-radius, 0.0f, 0.0f), Normal(-1.0f, 0.0f, 0.0f), calculateUvCoordinate(Vertex(-radius, 0.0f, 0.0f), radius)),// Right of previous
        Attribute(Vertex(0.0f, 0.0f, -radius), Normal(0.0f, 0.0f, -1.0f), calculateUvCoordinate(Vertex(0.0f, 0.0f, -radius), radius)),
        Attribute(Vertex(radius, 0.0f, 0.0f), Normal(1.0f, 0.0f, 0.0f), calculateUvCoordinate(Vertex(radius, 0.0f, 0.0f), radius)),
        Attribute(Vertex(0.0f, -radius, 0.0f), Normal(0.0f, -1.0f, 0.0f), Uv(0.0f, 0.0f)) // Bottom
    };

    const U numberOfVertice = 4098; // Number of vertices for 4 subdivides
    AttributeContainer attributeContainer(numberOfVertice);
    IndiceContainter indiceContainer(numberOfVertice);

    attributeContainer.clear();
    indiceContainer.clear();

    //Top hemisphere
    generateTriangleOnSphere(
        baseVertices[0], baseVertices[1], baseVertices[2],
        subdivides,
        attributeContainer, indiceContainer, false, radius
        );

    generateTriangleOnSphere(
        baseVertices[0], baseVertices[2], baseVertices[3],
        subdivides,
        attributeContainer, indiceContainer, false, radius
        );

    generateTriangleOnSphere(
        baseVertices[0], baseVertices[3], baseVertices[4],
        subdivides,
        attributeContainer, indiceContainer, false, radius
        );

    generateTriangleOnSphere(
        baseVertices[0], baseVertices[4], baseVertices[1],
        subdivides,
        attributeContainer, indiceContainer, false, radius
        );

    //Bottom hemisphere
    generateTriangleOnSphere(
        baseVertices[5], baseVertices[1], baseVertices[2],
        subdivides,
        attributeContainer, indiceContainer, true, radius
        );

    generateTriangleOnSphere(
        baseVertices[5], baseVertices[2], baseVertices[3],
        subdivides,
        attributeContainer, indiceContainer, true, radius
        );

    generateTriangleOnSphere(
        baseVertices[5], baseVertices[3], baseVertices[4],
        subdivides,
        attributeContainer, indiceContainer, true, radius
        );

    generateTriangleOnSphere(
        baseVertices[5], baseVertices[4], baseVertices[1],
        subdivides,
        attributeContainer, indiceContainer, true, radius
        );

    stb::ModelData::AttributeData * element = new stb::ModelData::AttributeData(
        (const char *)&attributeContainer[0],
        Attribute::elementSizeInBytes() * attributeContainer.size(),
        { 3, 3, 2 },
        Attribute::elementSizeInBytes(),
        stb::ModelData::FLOAT
        );

    return stb::ModelData(
    { stb::ModelData::AttributeElement(element) },
    (const char *)&indiceContainer[0],
    sizeof(IndiceContainter::value_type) * indiceContainer.size(),
    sizeof(indiceContainer[0]),
    stb::ModelData::TRIANGLE
    );
}
