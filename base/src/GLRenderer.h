#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

struct Geometry
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<uint16_t> indices;
    std::string texname;
    glm::vec3 color;
};

struct DrawableGeometry
{
    uint32_t vertex_array;
    uint32_t vertex_buffer;
    uint32_t index_buffer;
    uint32_t num_indices;
    glm::vec3 color;
    std::string texname;
};

struct vertex2_t
{
    glm::vec3 pt;
    glm::vec2 texcoord;
};

namespace mesh_attributes
{
    enum
    {
        POSITION,
        NORMAL,
        TEXCOORD
    };
}
namespace quad_attributes
{
    enum
    {
        POSITION,
        TEXCOORD
    };
}

class GLRenderer
{
    uint32_t m_height;
    uint32_t m_width;

    float m_invWidth;
    float m_invHeight;

    // Textures
    uint32_t m_randomNormalTexture;
    uint32_t m_randomScalarTexture;
    uint32_t m_depthTexture;
    uint32_t m_normalTexture;
    uint32_t m_positionTexture;
    uint32_t m_colorTexture;
    uint32_t m_postTexture;
    uint32_t m_glowmaskTexture;

    // Techniques
    uint32_t m_passProg;
    uint32_t m_pointProg;
    uint32_t m_ambientProg;
    uint32_t m_diagnosticProg;
    uint32_t m_postProg;

    DrawableGeometry m_QuadGeometry;

    // FBOs
    std::vector<uint32_t> m_FBO;

public:
    GLRenderer(uint32_t width, uint32_t height);
    void InitShaders();
    void InitNoise();
    void InitFBO();
    void InitQuad();

    void MakeDrawableGeometry(const Geometry& model, DrawableGeometry& out);
    const DrawableGeometry& GetQuadGeometry() { return m_QuadGeometry; }

    void RenderQuad();
    void Render();

    friend class GLApp; // TODORC
};