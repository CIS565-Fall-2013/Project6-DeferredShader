#include "GLRenderer.h"
#include "GLProgram.h"
#include "Utility.h"
#include "gl/glew.h"
#include "Camera.h"
#include "ShaderConstantManager.h"
#include "TextureManager.h"

namespace Colours
{
    glm::vec3 yellow = glm::vec3(1, 1, 0);
    glm::vec3 orange = glm::vec3(0.89, 0.44, 0.1);
    glm::vec3 red = glm::vec3(1, 0, 0);
    glm::vec3 blue = glm::vec3(0, 0, 1);
}

GLRenderer::GLRenderer(uint32_t width, uint32_t height)
    : m_width(width),
    m_height(height),
    m_farPlane(100.0f),
    m_nearPlane(0.1f),
    m_randomNormalTexture(0),
    m_randomScalarTexture(0),
    m_depthTexture(0),
    m_normalTexture(0),
    m_positionTexture(0),
    m_colorTexture(0),
    m_postTexture(0),
    m_passProg(0),
    m_pointProg(0),
    m_ambientProg(0),
    m_diagnosticProg(0),
    m_postProg(0),
    m_currentProgram(0),
    m_pRenderCam(nullptr)
{
    m_invWidth = 1.0f / m_width;
    m_invHeight = 1.0f / m_height;

    ShaderConstantManager::Create();
}

GLRenderer::~GLRenderer()
{
    ShaderConstantManager::Destroy();
}

DrawableGeometry::DrawableGeometry()
    : vertex_array(),
    vertex_buffer(),
    index_buffer(),
    num_indices(),
    diffuse_tex(),
    normal_tex(),
    specular_tex()
{}

DrawableGeometry::~DrawableGeometry()
{
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);

    if (diffuse_tex != 0)
        TextureManager::GetSingleton()->Release(diffuse_tex);
    if (normal_tex != 0)
        TextureManager::GetSingleton()->Release(normal_tex);
    if (specular_tex != 0)
        TextureManager::GetSingleton()->Release(specular_tex);

    num_indices = 0;
    color = glm::vec3(0);
}

void GLRenderer::AddDrawableGeometryToList(const DrawableGeometry* geometry, RenderEnums::DrawListType listType)
{
    switch (listType)
    {
    case RenderEnums::OPAQUE_LIST:
        m_opaqueList.push_back(geometry);
        break;
    case RenderEnums::ALPHA_MASKED_LIST:
        m_alphaMaskedList.push_back(geometry);
        break;
    case RenderEnums::TRANSPARENT_LIST:
        m_transparentList.push_back(geometry);
        break;
    case RenderEnums::LIGHT_LIST:
        m_lightList.push_back(geometry);
        break;
    default:
        assert(true);   // Unknown list type!
    }
}

void GLRenderer::ApplyPerFrameShaderConstants()
{
    ShaderConstantManager* shaderConstantManager = ShaderConstantManager::GetSingleton();

    std::string perFrameConstantBuffer("PerFrame");
    shaderConstantManager->SetShaderConstant("ufFar", perFrameConstantBuffer, &m_farPlane);
    shaderConstantManager->SetShaderConstant("ufNear", perFrameConstantBuffer, &m_nearPlane);
    shaderConstantManager->SetShaderConstant("uiScreenHeight", perFrameConstantBuffer, &m_height);
    shaderConstantManager->SetShaderConstant("uiScreenWidth", perFrameConstantBuffer, &m_width);
    shaderConstantManager->SetShaderConstant("ufInvScrHeight", perFrameConstantBuffer, &m_invHeight);
    shaderConstantManager->SetShaderConstant("ufInvScrWidth", perFrameConstantBuffer, &m_invWidth);
    //glUniform1f(glGetUniformLocation(m_postProg, "ufMouseTexX"), mouse_dof_x*m_invWidth);
    //glUniform1f(glGetUniformLocation(m_postProg, "ufMouseTexY"), abs(static_cast<int32_t>(m_height)-mouse_dof_y)*m_invHeight);

    glm::mat4 view = m_pRenderCam->GetView();
    glm::mat4 persp = m_pRenderCam->GetPerspective();
    shaderConstantManager->SetShaderConstant("um4View", perFrameConstantBuffer, &view); 
    shaderConstantManager->SetShaderConstant("um4Persp", perFrameConstantBuffer, &persp);

    float zero = 0.0f;
    shaderConstantManager->SetShaderConstant("ufGlowmask", perFrameConstantBuffer, &zero);

    int32_t value = 0;
    shaderConstantManager->SetShaderConstant("ubBloomOn", perFrameConstantBuffer, &value/*m_bloomEnabled*/);
    shaderConstantManager->SetShaderConstant("ubToonOn", perFrameConstantBuffer, &value/*m_toonEnabled*/);
    shaderConstantManager->SetShaderConstant("ubDOFOn", perFrameConstantBuffer, &value/*m_DOFEnabled*/);
    shaderConstantManager->SetShaderConstant("ubDOFDebug", perFrameConstantBuffer, &value/*m_DOFDebug*/);

    value = m_displayType;
    shaderConstantManager->SetShaderConstant("uiDisplayType", perFrameConstantBuffer, &value);
}

void GLRenderer::ClearFramebuffer(RenderEnums::ClearType clearFlags)
{
    GLenum flags = 0;

    if (clearFlags & RenderEnums::CLEAR_COLOUR)
        flags |= GL_COLOR_BUFFER_BIT;
    if (clearFlags & RenderEnums::CLEAR_DEPTH)
        flags |= GL_DEPTH_BUFFER_BIT;
    if (clearFlags & RenderEnums::CLEAR_STENCIL)
        flags |= GL_STENCIL_BUFFER_BIT;

    glClear(flags);
}

void GLRenderer::ClearLists()
{
    m_opaqueList.clear();
    m_alphaMaskedList.clear();
    m_transparentList.clear();
    m_lightList.clear();
}

void GLRenderer::CreateBuffersAndUploadData(const Geometry& model, DrawableGeometry& out)
{
    // Create Vertex/Index buffers
    glGenBuffers(1, &(out.vertex_buffer));
    glGenBuffers(1, &(out.index_buffer));

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, out.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

    // Upload Indices
    out.num_indices = model.indices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, out.num_indices * sizeof(GLuint), &model.indices[0], GL_STATIC_DRAW);
}

void GLRenderer::DrawAlphaMaskedList()
{
    glDepthMask(GL_FALSE);
    glDepthMask(GL_TRUE);
}

void GLRenderer::DrawGeometry(const DrawableGeometry* geom)
{
    assert(m_currentProgram != nullptr);
    m_currentProgram->CommitTextureBindings();
    m_currentProgram->CommitConstantBufferBindings();

    glBindVertexArray(geom->vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->index_buffer);
    glDrawElements(GL_TRIANGLES, geom->num_indices, GL_UNSIGNED_INT, 0);
}

void GLRenderer::drawLight(glm::vec3 pos, float strength)
{
    float radius = strength;
    glm::vec4 light = m_pRenderCam->GetView() * glm::vec4(pos, 1.0);
    if (light.z > m_nearPlane)
    {
        return;
    }
    light.w = radius;
    m_currentProgram->SetShaderConstant("uf4Light", light);
    m_currentProgram->SetShaderConstant("ufLightIl", strength);

    //glm::vec4 left = vp * glm::vec4(pos + radius*m_pRenderCam->start_left, 1.0);
    //glm::vec4 up = vp * glm::vec4(pos + radius*m_pRenderCam->up, 1.0);
    //glm::vec4 center = vp * glm::vec4(pos, 1.0);

    //left = sc * left;
    //up = sc * up;
    //center = sc * center;

    //left /= left.w;
    //up /= up.w;
    //center /= center.w;

    //float hw = glm::distance(left, center);
    //float hh = glm::distance(up, center);

    //float r = (hh > hw) ? hh : hw;

    //float x = center.x - r;
    //float y = center.y - r;

    //glScissor(x, y, 2 * r, 2 * r);
    RenderQuad();
}

void GLRenderer::DrawLightList()
{
    SetShaderProgram(m_pointProg);
    SetTexturesForFullScreenPass();

    m_pointProg->SetTexture("u_Colortex", m_colorTexture);
    m_pointProg->SetShaderConstant("uf3LightCol", Colours::yellow);
    glDepthMask(GL_FALSE);
    drawLight(glm::vec3(5.4, -0.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -0.5, 3.0), 1.0);
    m_pointProg->SetShaderConstant("uf3LightCol", Colours::orange);
    drawLight(glm::vec3(5.4, -2.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -2.5, 3.0), 1.0);
    m_pointProg->SetShaderConstant("uf3LightCol", Colours::yellow);
    drawLight(glm::vec3(5.4, -4.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -4.5, 3.0), 1.0);

    m_pointProg->SetShaderConstant("uf3LightCol", Colours::red);
    drawLight(glm::vec3(2.5, -1.2, 0.5), 2.5);

    m_pointProg->SetShaderConstant("uf3LightCol", Colours::blue);
    drawLight(glm::vec3(2.5, -5.0, 4.2), 2.5);
    glDepthMask(GL_TRUE);
}

void GLRenderer::DrawOpaqueList()
{
    glm::mat4 inverseView = m_pRenderCam->GetInverseView();
    SetShaderProgram(m_passProg);

    for (uint32_t i = 0; i < m_opaqueList.size(); ++i)
    {
        glm::mat4 inverse_transposed = glm::transpose(m_opaqueList[i]->inverseModelMat * inverseView);
        m_passProg->SetShaderConstant("um4Model", m_opaqueList[i]->modelMat);
        m_passProg->SetShaderConstant("um4InvTrans", inverse_transposed);
        m_passProg->SetShaderConstant("uf3Color", m_opaqueList[i]->color);

        m_passProg->SetTexture("t2DDiffuse", m_opaqueList[i]->diffuse_tex);
        m_passProg->SetTexture("t2DNormal", m_opaqueList[i]->normal_tex);
        m_passProg->SetTexture("t2DSpecular", m_opaqueList[i]->specular_tex);

        DrawGeometry(m_opaqueList[i]);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GLRenderer::DrawTransparentList()
{

}

void GLRenderer::EndActiveFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLRenderer::InitFramebuffers()
{
    GLenum FBOstatus;
    glActiveTexture(GL_TEXTURE9);

    glGenTextures(1, &m_depthTexture);
    glGenTextures(1, &m_normalTexture);
    glGenTextures(1, &m_positionTexture);
    glGenTextures(1, &m_colorTexture);

    glEnable(GL_FRAMEBUFFER_SRGB);

    //Set up depth texture
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    //Set up normal texture
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up position texture
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up color texture
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    GLType_uint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLType_int normal_loc;
    if (!m_passProg->GetOutputBindLocation("out_f4Normal", reinterpret_cast<GLType_uint&>(normal_loc)))
        assert(false);
    GLType_int position_loc;
    if (!m_passProg->GetOutputBindLocation("out_f4Position", reinterpret_cast<GLType_uint&>(position_loc)))
        assert(false);
    GLType_int color_loc;
    if (!m_passProg->GetOutputBindLocation("out_f4Colour", reinterpret_cast<GLType_uint&>(color_loc)))
        assert(false);
    GLType_int glowmask_loc;
    if (!m_passProg->GetOutputBindLocation("out_f4GlowMask", reinterpret_cast<GLType_uint&>(glowmask_loc)))
        assert(false);

    GLenum draws[4];
    draws[normal_loc] = GL_COLOR_ATTACHMENT0;
    draws[position_loc] = GL_COLOR_ATTACHMENT1;
    draws[color_loc] = GL_COLOR_ATTACHMENT2;
    draws[glowmask_loc] = GL_COLOR_ATTACHMENT3;
    glDrawBuffers(4, draws);

    // attach the texture to FBO depth attachment point
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[normal_loc], m_normalTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[position_loc], m_positionTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[color_loc], m_colorTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);
    else
        assert(false);

    //Post Processing buffer!
    //Set up post texture
    glGenTextures(1, &m_postTexture);
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, 0);

    // create a framebuffer object
    fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (!m_ambientProg->GetOutputBindLocation("out_f4Colour", reinterpret_cast<GLType_uint&>(color_loc)))
        assert(false);
    GLenum draw[1];
    draw[color_loc] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, draw);

    // attach the texture to FBO depth attachment point
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draw[color_loc], m_postTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);
    else
        assert(false);

    // switch back to window-system-provided framebuffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::Initialize(const Camera* renderCamera)
{
    InitNoise();
    InitShaders();
    InitFramebuffers();
    InitQuad();
    InitSphere();

    m_pRenderCam = const_cast<Camera*>(renderCamera);
    glDepthFunc(GL_LEQUAL);
}

void GLRenderer::InitNoise()
{
    const char * rand_norm_png = "../res/random_normal.png";
    const char * rand_png = "../res/random.png";

    m_randomNormalTexture = TextureManager::GetSingleton()->Acquire(rand_norm_png);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_randomScalarTexture = TextureManager::GetSingleton()->Acquire(rand_png);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::InitQuad()
{
    Geometry quad;

    quad.vertices.push_back(Vertex(glm::vec3(-1, 1, 0), glm::vec3(-1, 1, 0), glm::vec2(0, 1)));
    quad.vertices.push_back(Vertex(glm::vec3(-1, -1, 0), glm::vec3(-1, 1, 0), glm::vec2(0, 0)));
    quad.vertices.push_back(Vertex(glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec2(1, 0)));
    quad.vertices.push_back(Vertex(glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec2(1, 1)));

    quad.indices.push_back(0);
    quad.indices.push_back(1);
    quad.indices.push_back(2);
    quad.indices.push_back(0);
    quad.indices.push_back(2);
    quad.indices.push_back(3);

    CreateBuffersAndUploadData(quad, m_QuadGeometry);

    // Quad vertex specification
    glGenVertexArrays(1, &(m_QuadGeometry.vertex_array));
    glBindVertexArray(m_QuadGeometry.vertex_array);
    glVertexAttribPointer(quad_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(quad_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texcoord)));
    glEnableVertexAttribArray(quad_attributes::POSITION);
    glEnableVertexAttribArray(quad_attributes::TEXCOORD);

    glBindVertexArray(0);
}

void GLRenderer::InitShaders()
{
    const char * pass_vert = "../res/shaders/pass.vert";
    const char * shade_vert = "../res/shaders/shade.vert";
    const char * post_vert = "../res/shaders/post.vert";

    const char * pass_frag = "../res/shaders/pass.frag";
    const char * diagnostic_frag = "../res/shaders/diagnostic.frag";
    const char * ambient_frag = "../res/shaders/ambient.frag";
    const char * point_frag = "../res/shaders/point.frag";
    const char * post_frag = "../res/shaders/post.frag";

    std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>> shaderSourceAndStagePair;
    std::map<std::string, GLType_uint> meshAttributeBindIndices, quadAttributeBindIndices, outputBindIndices;

    meshAttributeBindIndices["in_f3Position"] = mesh_attributes::POSITION;
    meshAttributeBindIndices["in_f3Normal"] = mesh_attributes::NORMAL;
    meshAttributeBindIndices["in_f2Texcoord"] = mesh_attributes::TEXCOORD;

    quadAttributeBindIndices["in_f3Position"] = quad_attributes::POSITION;
    quadAttributeBindIndices["in_f2Texcoord"] = quad_attributes::TEXCOORD;

    outputBindIndices["out_f4Colour"] = 0;
    outputBindIndices["out_f4Normal"] = 1;
    outputBindIndices["out_f4Position"] = 2;
    outputBindIndices["out_f4GlowMask"] = 3;

    shaderSourceAndStagePair.clear();
    shaderSourceAndStagePair.push_back(std::make_pair(pass_vert, RenderEnums::VERT));
    shaderSourceAndStagePair.push_back(std::make_pair(pass_frag, RenderEnums::FRAG));
    m_passProg = new GLProgram(RenderEnums::RENDER_PROGRAM, shaderSourceAndStagePair, meshAttributeBindIndices, outputBindIndices);

    shaderSourceAndStagePair.clear();
    shaderSourceAndStagePair.push_back(std::make_pair(shade_vert, RenderEnums::VERT));
    shaderSourceAndStagePair.push_back(std::make_pair(diagnostic_frag, RenderEnums::FRAG));
    m_diagnosticProg = new GLProgram(RenderEnums::RENDER_PROGRAM, shaderSourceAndStagePair, quadAttributeBindIndices, outputBindIndices);

    shaderSourceAndStagePair[1] = std::make_pair(ambient_frag, RenderEnums::FRAG);
    m_ambientProg = new GLProgram(RenderEnums::RENDER_PROGRAM, shaderSourceAndStagePair, quadAttributeBindIndices, outputBindIndices);

    shaderSourceAndStagePair[1] = std::make_pair(point_frag, RenderEnums::FRAG);
    m_pointProg = new GLProgram(RenderEnums::RENDER_PROGRAM, shaderSourceAndStagePair, quadAttributeBindIndices, outputBindIndices);

    shaderSourceAndStagePair.clear();
    shaderSourceAndStagePair.push_back(std::make_pair(post_vert, RenderEnums::VERT));
    shaderSourceAndStagePair.push_back(std::make_pair(post_frag, RenderEnums::FRAG));
    m_postProg = new GLProgram(RenderEnums::RENDER_PROGRAM, shaderSourceAndStagePair, quadAttributeBindIndices, outputBindIndices);
}

void GLRenderer::InitSphere()
{
    Geometry sphere;

    const uint32_t divisor = 10;
    const float inverseDivisor = 1.0f / divisor;
    const float pi = 3.1415926f;
    float thetaAdvance = 2 * pi * inverseDivisor;
    float phiAdvance = pi * inverseDivisor;

    for (uint32_t i = 0; i <= divisor; ++i)   // theta
    {
        for (uint32_t j = 0; j <= divisor; ++j)  // phi
        {
            glm::vec3 positionNormal(sin(i * thetaAdvance) * sin(j * phiAdvance), cos(j * phiAdvance), cos(i * thetaAdvance) * sin(j * phiAdvance));
            sphere.vertices.push_back(Vertex(positionNormal, positionNormal, glm::vec2(i * inverseDivisor, j * inverseDivisor)));

            if ((i < divisor) && (j < divisor))
            {
                sphere.indices.push_back(j);
                sphere.indices.push_back(((i + 1) * divisor + j));
                sphere.indices.push_back(((i + 1) * divisor + (j + 1)));
                sphere.indices.push_back(j);
                sphere.indices.push_back(((i + 1) * divisor + (j + 1)));
                sphere.indices.push_back(j + 1);
            }
        }
    }

    CreateBuffersAndUploadData(sphere, m_SphereGeometry);

    // Sphere vertex specification
    glGenVertexArrays(1, &(m_SphereGeometry.vertex_array));
    glBindVertexArray(m_SphereGeometry.vertex_array);
    glVertexAttribPointer(mesh_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(mesh_attributes::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glVertexAttribPointer(mesh_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, texcoord)));
    glEnableVertexAttribArray(mesh_attributes::POSITION);
    glEnableVertexAttribArray(mesh_attributes::NORMAL);
    glEnableVertexAttribArray(mesh_attributes::TEXCOORD);

    glBindVertexArray(0);
}

void GLRenderer::MakeDrawableModel(const Geometry& model, DrawableGeometry& out, const glm::mat4& modelMatrix)
{
    CreateBuffersAndUploadData(model, out);

    // Vertex specification
    glGenVertexArrays(1, &(out.vertex_array));
    glBindVertexArray(out.vertex_array);
    glVertexAttribPointer(mesh_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(mesh_attributes::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glVertexAttribPointer(mesh_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, texcoord)));
    glEnableVertexAttribArray(mesh_attributes::POSITION);
    glEnableVertexAttribArray(mesh_attributes::NORMAL);
    glEnableVertexAttribArray(mesh_attributes::TEXCOORD);

    // Unplug Vertex Array
    glBindVertexArray(0);

    out.diffuse_tex = TextureManager::GetSingleton()->Acquire(model.diffuse_texpath);
    out.normal_tex = TextureManager::GetSingleton()->Acquire(model.normal_texpath);
    out.specular_tex = TextureManager::GetSingleton()->Acquire(model.specular_texpath);

    out.modelMat = modelMatrix;
    out.inverseModelMat = glm::inverse(out.modelMat);
    out.color = model.color;
}

void GLRenderer::Render()
{
    ApplyPerFrameShaderConstants();

    // GBuffer Pass
    SetFramebufferActive(RenderEnums::GBUFFER_FRAMEBUFFER);
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    DrawOpaqueList();
    DrawAlphaMaskedList();

    // Lighting Pass
    SetFramebufferActive(RenderEnums::LIGHTING_FRAMEBUFFER);
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    DrawLightList();
    glDisable(GL_BLEND);
    RenderAmbientLighting();
    EndActiveFramebuffer();

    // Post Process Pass
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    glDisable(GL_DEPTH_TEST);
    if (m_displayType != RenderEnums::DISPLAY_TOTAL)
        RenderFramebuffers();
    else
        RenderPostProcessEffects();
    glEnable(GL_DEPTH_TEST);
}

void GLRenderer::RenderAmbientLighting()
{
    glm::vec4 dir_light(0.0, 1.0, 1.0, 0.0);
    dir_light = glm::normalize(dir_light);
    dir_light = m_pRenderCam->GetView() * dir_light;
    dir_light = glm::normalize(dir_light);
    dir_light.w = 1.0f; // strength
    float ambient = 0.04f;

    SetShaderProgram(m_ambientProg);
    SetTexturesForFullScreenPass();
    m_ambientProg->SetTexture("u_Colortex", m_colorTexture);
    m_ambientProg->SetShaderConstant("uf4Light", dir_light);
    m_ambientProg->SetShaderConstant("ufLightIl", ambient);

    glDepthMask(GL_FALSE);
    RenderQuad();
    glDepthMask(GL_TRUE);
}

void GLRenderer::RenderFramebuffers()
{
    SetShaderProgram(m_diagnosticProg);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SetTexturesForFullScreenPass();
    m_diagnosticProg->SetTexture("u_Colortex", m_colorTexture);

    glDepthMask(GL_FALSE);
    RenderQuad();
    glDepthMask(GL_TRUE);
}

void GLRenderer::RenderPostProcessEffects()
{
    SetShaderProgram(m_postProg);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SetTexturesForFullScreenPass();
    m_postProg->SetTexture("u_Posttex", m_postTexture);

    glDepthMask(GL_FALSE);
    RenderQuad();
    glDepthMask(GL_TRUE);
}

void GLRenderer::RenderQuad()
{
    DrawGeometry(&m_QuadGeometry);
}

void GLRenderer::SetFramebufferActive(GLType_uint fbID)
{
    assert(fbID < m_FBO.size());
    glBindTexture(GL_TEXTURE_2D, 0); //Bad mojo to unbind the framebuffer using the texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[fbID]);
}

void GLRenderer::SetShaderProgram(GLProgram* currentlyUsedProgram)
{ 
    m_currentProgram = currentlyUsedProgram; 
    m_currentProgram->SetActive();
}

void GLRenderer::SetTexturesForFullScreenPass()
{
    m_currentProgram->SetTexture("u_Depthtex", m_depthTexture);
    m_currentProgram->SetTexture("u_Normaltex", m_normalTexture);
    m_currentProgram->SetTexture("u_Positiontex", m_positionTexture);
    m_currentProgram->SetTexture("u_RandomNormaltex", m_randomNormalTexture);
    m_currentProgram->SetTexture("u_RandomScalartex", m_randomScalarTexture);
}