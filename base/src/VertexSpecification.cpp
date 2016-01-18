#include "VertexSpecification.h"
#include "GLRenderer.h"
#include "gl/glew.h"

VertexSpecification::VertexSpecification(const std::vector<VertexAttribute>& attributeArray, uint32_t vertexStride)
    : m_glVertexArrayName(0),
    m_vertexStride(vertexStride)
{
    assert(attributeArray.size() > 0);
    m_vertexBufferDataFormat = attributeArray;

    glCreateVertexArrays(1, &m_glVertexArrayName);
    assert(m_glVertexArrayName != 0);

    for (uint32_t i = 0; i < m_vertexBufferDataFormat.size(); ++i)
    {
        const VertexAttribute& thisAttribute = m_vertexBufferDataFormat[i];

        switch (thisAttribute.dataType)
        {
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            if (!thisAttribute.normalizeTo01Range)
            {
                glVertexArrayAttribIFormat(m_glVertexArrayName, i, thisAttribute.numElements, thisAttribute.dataType, thisAttribute.bytesFromStartOfVertexData);
                break;
            }
        case GL_FLOAT:
            glVertexArrayAttribFormat(m_glVertexArrayName, i, thisAttribute.numElements, GL_FLOAT, false, thisAttribute.bytesFromStartOfVertexData);
            break;
        case GL_DOUBLE:
            glVertexArrayAttribLFormat(m_glVertexArrayName, i, thisAttribute.numElements, GL_DOUBLE, thisAttribute.bytesFromStartOfVertexData);
            break;
        }

        glVertexArrayAttribBinding(m_glVertexArrayName, i, thisAttribute.bufferIndex);
        glEnableVertexArrayAttrib(m_glVertexArrayName, i);
    }
}

VertexSpecification::~VertexSpecification()
{
    glDeleteVertexArrays(1, &m_glVertexArrayName);
}

void VertexSpecification::SetActive()
{
    glBindVertexArray(m_glVertexArrayName);
}