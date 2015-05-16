#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Common.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;

    Vertex() : position(0, 0, 0), normal(0, 0, 0), texcoord(0, 0) {}
    Vertex(glm::vec3 inPosition, glm::vec3 inNormal, glm::vec2 inTexCoord) : position(inPosition), normal(inNormal), texcoord(inTexCoord) {}
    Vertex(const Vertex& inVertex) : position(inVertex.position), normal(inVertex.normal), texcoord(inVertex.texcoord) {}
};

struct Geometry
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::string texname;
    glm::vec3 color;
};

class DrawableGeometry
{
public:
    ~DrawableGeometry();
    uint32_t vertex_array;
    uint32_t vertex_buffer;
    uint32_t index_buffer;
    uint32_t num_indices;
    glm::vec3 color;
    glm::mat4 modelMat;
    std::string texname;
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

class Camera;
class GLProgram;
class GLRenderer
{
    uint32_t m_height;
    uint32_t m_width;

    float m_invWidth;
    float m_invHeight;

    float m_farPlane;
    float m_nearPlane;

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
    GLProgram* m_passProg;
    GLProgram* m_pointProg;
    GLProgram* m_ambientProg;
    GLProgram* m_diagnosticProg;
    GLProgram* m_postProg;
    GLProgram* m_currentProgram;

    DrawableGeometry m_QuadGeometry;
    DrawableGeometry m_SphereGeometry;

    const Camera* m_pRenderCam;

    // FBOs
    std::vector<uint32_t> m_FBO;

    std::vector<const DrawableGeometry*> m_opaqueList;
    std::vector<const DrawableGeometry*> m_alphaMaskedList;
    std::vector<const DrawableGeometry*> m_transparentList;
    std::vector<const DrawableGeometry*> m_lightList;

    void InitShaders();
    void InitNoise();
    void InitFramebuffers();
    void InitQuad();
    void InitSphere();

    void CreateBuffersAndUploadData(const Geometry& model, DrawableGeometry& out);

    void ClearFramebuffer(RenderEnums::ClearType clearFlags);

    void DrawGeometry(const DrawableGeometry* geom);

    void DrawOpaqueList();
    void DrawAlphaMaskedList();
    void DrawTransparentList();
    void DrawLightList();

    void RenderAmbientLighting();
    void RenderPostProcessEffects();

    void ApplyShaderConstantsForFullScreenPass();

    void drawLight(glm::vec3 pos, float strength); //TODORC
    void SetShaderProgram(GLProgram* currentlyUsedProgram);

public:
    GLRenderer(uint32_t width, uint32_t height);
    ~GLRenderer();

    void Initialize(const Camera* renderCamera);

    void MakeDrawableModel(const Geometry& model, DrawableGeometry& out, const glm::mat4& modelMatrix = glm::mat4());

    const DrawableGeometry& GetQuadGeometry() const { return m_QuadGeometry; }
    const DrawableGeometry& GetSphereGeometry() const { return m_SphereGeometry; }
    const float GetNearPlaneDistance() const { return m_nearPlane; }
    const float GetFarPlaneDistance() const { return m_farPlane; }

    void AddDrawableGeometryToList(const DrawableGeometry* geometry, RenderEnums::DrawListType listType);
    void ClearLists();

    void RenderQuad();
    void Render();

    void SetFramebufferActive(uint32_t fbID);
    void EndActiveFramebuffer();
};