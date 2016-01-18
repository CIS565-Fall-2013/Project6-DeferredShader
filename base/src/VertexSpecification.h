#pragma once

#include "Common.h"
#include <vector>

struct VertexAttribute
{
    VertexAttribute()
        : numElements(),
        dataType(-1),
        normalizeTo01Range(false),
        bytesFromStartOfVertexData(),
        bufferIndex()
    {}
#ifdef _DEBUG
    std::string name;
#endif
    uint32_t numElements;   // Number of dataType elements of this VertexAttribute element starting at this location. (e.g. "Position" VertexAttribute element has 3 float elements).
    GLType_int dataType;
    bool normalizeTo01Range; // If int type data is fed into a float attribute in the shader, should it be normalized to the [0, 1] range, or just static_casted?
    uint32_t bytesFromStartOfVertexData; // This VertexAttribute element occurs this many bytes from the start of a Vertex's data in the VB.
    uint32_t bufferIndex;   // 0 -> regular VB; 1 -> Instance VB, etc.
};

class VertexSpecification
{
    std::vector<VertexAttribute> m_vertexBufferDataFormat;
    uint32_t m_vertexStride;
    GLType_uint m_glVertexArrayName;

public:
    VertexSpecification(const std::vector<VertexAttribute>& attributeArray, uint32_t vertexStride);
    ~VertexSpecification();

    void SetActive();
    uint32_t GetVertexStride() const { return m_vertexStride; }
};