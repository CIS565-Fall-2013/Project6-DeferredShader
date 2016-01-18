#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <memory>
#include <map>

#include "Common.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texcoord;

    Vertex() : position(0, 0, 0), normal(0, 0, 0), tangent(0, 0, 0), texcoord(0, 0) {}
    Vertex(glm::vec3 inPosition, glm::vec3 inNormal, glm::vec2 inTexCoord, glm::vec3 inTangent = glm::vec3(0)) : position(inPosition), normal(inNormal), tangent(inTangent), texcoord(inTexCoord) {}
    Vertex(const Vertex& inVertex) : position(inVertex.position), normal(inVertex.normal), tangent(inVertex.tangent), texcoord(inVertex.texcoord) {}
};

struct Geometry
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string vertex_specification;
    std::string diffuse_texpath;
    std::string normal_texpath;
    std::string specular_texpath;
    glm::vec3 color;
};

class VertexSpecification;
class DrawableGeometry
{
public:
    DrawableGeometry();
    ~DrawableGeometry();

    GLType_uint vertex_buffer;
    GLType_uint index_buffer;
    uint32_t num_indices;

    GLType_uint diffuse_tex;
    GLType_uint normal_tex;
    GLType_uint specular_tex;

    glm::vec3 color;
    glm::mat4 modelMat;
    glm::mat4 inverseModelMat;

    std::weak_ptr<VertexSpecification> vertexSpecification;
};

class Camera;
class GLProgram;
struct VertexAttribute;
class GLRenderer
{
    uint32_t m_height;
    uint32_t m_width;

    float m_invWidth;
    float m_invHeight;

    float m_farPlane;
    float m_nearPlane;

    RenderEnums::DisplayType m_displayType;

    // Textures
    GLType_uint m_randomNormalTexture;
    GLType_uint m_randomScalarTexture;
    GLType_uint m_depthTexture;
    GLType_uint m_normalTexture;
    GLType_uint m_positionTexture;
    GLType_uint m_colorTexture;
    GLType_uint m_postTexture;

    // Techniques
    GLProgram* m_passProg;
    GLProgram* m_pointProg;
    GLProgram* m_directionalProg;
    GLProgram* m_diagnosticProg;
    GLProgram* m_postProg;
    GLProgram* m_currentProgram;

    DrawableGeometry m_QuadGeometry;
    DrawableGeometry m_SphereGeometry;

    const Camera* m_pRenderCam;

    // FBOs
    std::vector<GLType_uint> m_FBO;

    std::vector<const DrawableGeometry*> m_opaqueList;
    std::vector<const DrawableGeometry*> m_alphaMaskedList;
    std::vector<const DrawableGeometry*> m_transparentList;
    std::vector<const DrawableGeometry*> m_lightList;

    std::map<uint32_t, std::shared_ptr<VertexSpecification>> m_vertexSpecifications;
    std::shared_ptr<VertexSpecification> m_activeVertexSpecification;
    GLType_uint m_activeVertexBuffer;
    GLType_uint m_activeIndexBuffer;

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
    void drawLight(glm::vec3 pos, float strength); //TODORC

    void RenderDirectionalAndAmbientLighting();
    void RenderFramebuffers();
    void RenderPostProcessEffects();
    void ApplyPerFrameShaderConstants();

    void SetTexturesForFullScreenPass();
    void SetShaderProgram(GLProgram* currentlyUsedProgram);
    void SetVertexSpecification(const std::weak_ptr<VertexSpecification>& vertexSpec);
    void BindVertexBuffer(GLType_uint vertexBuffer);
    void BindIndexBuffer(GLType_uint indexBuffer);

public:
    GLRenderer(uint32_t width, uint32_t height, float nearPlaneDistance, float farPlaneDistance);
    ~GLRenderer();

    void Initialize(const Camera* renderCamera);

    void MakeDrawableModel(const Geometry& model, DrawableGeometry& out, const glm::mat4& modelMatrix = glm::mat4());
    void CreateVertexSpecification(const std::string& vertSpecName, const std::vector<VertexAttribute>& vertexAttributeList, uint32_t vertexStride);

    const DrawableGeometry& GetQuadGeometry() const { return m_QuadGeometry; }
    const DrawableGeometry& GetSphereGeometry() const { return m_SphereGeometry; }
    const float GetNearPlaneDistance() const { return m_nearPlane; }
    const float GetFarPlaneDistance() const { return m_farPlane; }
    void SetDisplayType(RenderEnums::DisplayType displayType) { m_displayType = displayType; }

    void AddDrawableGeometryToList(const DrawableGeometry* geometry, RenderEnums::DrawListType listType);
    void ClearLists();

    void RenderQuad();
    void Render();

    void SetFramebufferActive(GLType_uint fbID);
    void EndActiveFramebuffer();
};