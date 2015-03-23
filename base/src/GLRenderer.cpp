#include "GLRenderer.h"
#include "Utility.h"
#include "gl/glew.h"
#include "SOIL/SOIL.h"
#include "Camera.h"
#include <cmath>

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
    m_glowmaskTexture(0),
    m_passProg(0),
    m_pointProg(0),
    m_ambientProg(0),
    m_diagnosticProg(0),
    m_postProg(0),
    m_pRenderCam(nullptr)
{
    m_invWidth = 1.0f / m_width;
    m_invHeight = 1.0f / m_height;
}

GLRenderer::~GLRenderer()
{
}

DrawableGeometry::~DrawableGeometry()
{
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteVertexArrays(1, &vertex_array);
    
    num_indices = 0;
    color = glm::vec3(0);
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

    Utility::shaders_t shaders;

    shaders = Utility::loadShaders(pass_vert, pass_frag);
    m_passProg = glCreateProgram();
    glBindAttribLocation(m_passProg, mesh_attributes::POSITION, "Position");
    glBindAttribLocation(m_passProg, mesh_attributes::NORMAL, "Normal");
    glBindAttribLocation(m_passProg, mesh_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_passProg, shaders);

    shaders = Utility::loadShaders(shade_vert, diagnostic_frag);
    m_diagnosticProg = glCreateProgram();
    glBindAttribLocation(m_diagnosticProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_diagnosticProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_diagnosticProg, shaders);

    shaders = Utility::loadShaders(shade_vert, ambient_frag);
    m_ambientProg = glCreateProgram();
    glBindAttribLocation(m_ambientProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_ambientProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_ambientProg, shaders);

    shaders = Utility::loadShaders(shade_vert, point_frag);
    m_pointProg = glCreateProgram();
    glBindAttribLocation(m_pointProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_pointProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_pointProg, shaders);

    shaders = Utility::loadShaders(post_vert, post_frag);
    m_postProg = glCreateProgram();
    glBindAttribLocation(m_postProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_postProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_postProg, shaders);
}

void GLRenderer::InitNoise()
{
    const char * rand_norm_png = "../res/random_normal.png";
    const char * rand_png = "../res/random.png";

    m_randomNormalTexture = SOIL_load_OGL_texture(rand_norm_png, 0, 0, 0);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_randomScalarTexture = SOIL_load_OGL_texture(rand_png, 0, 0, 0);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::InitFramebuffers()
{
    GLenum FBOstatus;

    glActiveTexture(GL_TEXTURE9);

    glGenTextures(1, &m_depthTexture);
    glGenTextures(1, &m_normalTexture);
    glGenTextures(1, &m_positionTexture);
    glGenTextures(1, &m_colorTexture);
    glGenTextures(1, &m_glowmaskTexture);

    //Set up depth FBO
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    //Set up normal FBO
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up position FBO
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up color FBO
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up glowmap FBO
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    // creatwwe a framebuffer object
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Instruct openGL that we won't bind a color texture with the currently bound FBO
    glReadBuffer(GL_NONE);
    GLint normal_loc = glGetFragDataLocation(m_passProg, "out_Normal");
    GLint position_loc = glGetFragDataLocation(m_passProg, "out_Position");
    GLint color_loc = glGetFragDataLocation(m_passProg, "out_Color");
    GLint glowmask_loc = glGetFragDataLocation(m_passProg, "out_GlowMask");

    GLenum draws[4];
    draws[normal_loc] = GL_COLOR_ATTACHMENT0;
    draws[position_loc] = GL_COLOR_ATTACHMENT1;
    draws[color_loc] = GL_COLOR_ATTACHMENT2;
    draws[glowmask_loc] = GL_COLOR_ATTACHMENT3;
    glDrawBuffers(4, draws);

    // attach the texture to FBO depth attachment point
    int test = GL_COLOR_ATTACHMENT0;
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[normal_loc], m_normalTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[position_loc], m_positionTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[color_loc], m_colorTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[glowmask_loc], m_glowmaskTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);

    //Post Processing buffer!
    glActiveTexture(GL_TEXTURE9);

    glGenTextures(1, &m_postTexture);
    //Set up post FBO
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    // create a framebuffer object
    fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Instruct openGL that we won't bind a color texture with the currently bound FBO
    glReadBuffer(GL_BACK);
    color_loc = glGetFragDataLocation(m_ambientProg, "out_Color");
    GLenum draw[1];
    draw[color_loc] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, draw);

    // attach the texture to FBO depth attachment point
    test = GL_COLOR_ATTACHMENT0;
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draw[color_loc], m_postTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);

    // switch back to window-system-provided framebuffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::InitQuad()
{
    Geometry quad;

    quad.vertices.push_back(Vertex(glm::vec3(-1, 1, 0), glm::vec3(-1, 1, 0), glm::vec2(0, 1)));
    quad.vertices.push_back(Vertex(glm::vec3(-1, -1, 0), glm::vec3(-1, 1, 0), glm::vec2(0, 0)));
    quad.vertices.push_back(Vertex(glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec2(1, 0)));
    quad.vertices.push_back(Vertex(glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec2(1, 1)));
    
    quad.indices.push_back(uint16_t(0));
    quad.indices.push_back(uint16_t(1));
    quad.indices.push_back(uint16_t(2));
    quad.indices.push_back(uint16_t(0));
    quad.indices.push_back(uint16_t(2));
    quad.indices.push_back(uint16_t(3));

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

void GLRenderer::InitSphere()
{
    Geometry sphere;

    const uint32_t divisor = 10;
    const float inverseDivisor = 1.0f / divisor;
    const float pi = 3.1415926;
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
                sphere.indices.push_back(uint16_t(j));
                sphere.indices.push_back(uint16_t((i + 1) * divisor + j));
                sphere.indices.push_back(uint16_t((i + 1) * divisor + (j + 1)));
                sphere.indices.push_back(uint16_t(j));
                sphere.indices.push_back(uint16_t((i + 1) * divisor + (j + 1)));
                sphere.indices.push_back(uint16_t(j + 1));
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, out.num_indices * sizeof(GLushort), &model.indices[0], GL_STATIC_DRAW);
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

void GLRenderer::DrawGeometry(const DrawableGeometry* geom)
{
    glBindVertexArray(geom->vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->index_buffer);
    glDrawElements(GL_TRIANGLES, geom->num_indices, GL_UNSIGNED_SHORT, 0);
}

void GLRenderer::DrawOpaqueList()
{
    glm::mat4 view = m_pRenderCam->get_view();
    glm::mat4 persp = m_pRenderCam->GetPerspective();

    glUseProgram(m_passProg);
    glUniform1f(glGetUniformLocation(m_passProg, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(m_passProg, "glowmask"), 0);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_View"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_Persp"), 1, GL_FALSE, &persp[0][0]);

    uint32_t modelUnifLoc = glGetUniformLocation(m_passProg, "u_Model");
    uint32_t inverseTransposeUnifLoc = glGetUniformLocation(m_passProg, "u_InvTrans");
    uint32_t colourUnifLoc = glGetUniformLocation(m_passProg, "u_Color");

    for (uint32_t i = 0; i < m_opaqueList.size(); ++i)
    {
        glm::mat4 inverse_transposed = glm::transpose(glm::inverse(view*m_opaqueList[i]->modelMat));
        glUniformMatrix4fv(modelUnifLoc, 1, GL_FALSE, &m_opaqueList[i]->modelMat[0][0]);
        glUniformMatrix4fv(inverseTransposeUnifLoc, 1, GL_FALSE, &inverse_transposed[0][0]);
        glUniform3fv(colourUnifLoc, 1, &(m_opaqueList[i]->color[0]));

        DrawGeometry(m_opaqueList[i]);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GLRenderer::DrawAlphaMaskedList()
{
    glDepthMask(GL_FALSE);
    glDepthMask(GL_TRUE);
}

void GLRenderer::DrawTransparentList()
{

}

void GLRenderer::DrawLightList()
{
    ApplyShaderConstantsForFullScreenPass(m_pointProg);
    glUniform1i(glGetUniformLocation(m_pointProg, "u_toonOn"), 0);
    glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(Colours::yellow[0]));
    glDepthMask(GL_FALSE);
    drawLight(glm::vec3(5.4, -0.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -0.5, 3.0), 1.0);
    glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(Colours::orange[0]));
    drawLight(glm::vec3(5.4, -2.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -2.5, 3.0), 1.0);
    glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(Colours::yellow[0]));
    drawLight(glm::vec3(5.4, -4.5, 3.0), 1.0);
    drawLight(glm::vec3(0.2, -4.5, 3.0), 1.0);

    glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(Colours::red[0]));
    drawLight(glm::vec3(2.5, -1.2, 0.5), 2.5);

    glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(Colours::blue[0]));
    drawLight(glm::vec3(2.5, -5.0, 4.2), 2.5);
    glDepthMask(GL_TRUE);
}

void GLRenderer::drawLight(glm::vec3 pos, float strength)
{
    float radius = strength;
    glm::vec4 light = m_pRenderCam->get_view() * glm::vec4(pos, 1.0);
    if (light.z > m_nearPlane)
    {
        return;
    }
    light.w = radius;
    glUniform4fv(glGetUniformLocation(m_pointProg, "u_Light"), 1, &(light[0]));
    glUniform1f(glGetUniformLocation(m_pointProg, "u_LightIl"), strength);

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

void GLRenderer::RenderAmbientLighting()
{
    glm::vec4 dir_light(0.1, 1.0, 1.0, 0.0);
    dir_light = m_pRenderCam->get_view() * dir_light;
    dir_light = glm::normalize(dir_light);
    dir_light.w = 0.3f;
    float strength = 0.09f;

    ApplyShaderConstantsForFullScreenPass(m_ambientProg);
    glUniform4fv(glGetUniformLocation(m_ambientProg, "u_Light"), 1, &(dir_light[0]));
    glUniform1f(glGetUniformLocation(m_ambientProg, "u_LightIl"), strength);

    glDepthMask(GL_FALSE);
    RenderQuad();
    glDepthMask(GL_TRUE);
}

void GLRenderer::RenderPostProcessEffects()
{
    glUseProgram(m_postProg);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_Posttex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_GlowMask"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_normalTex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_positionTex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_depthTex"), 6);

    glUniform1i(glGetUniformLocation(m_postProg, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(m_postProg, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(m_postProg, "u_InvScrHeight"), m_invHeight);
    glUniform1f(glGetUniformLocation(m_postProg, "u_InvScrWidth"), m_invWidth);
    //glUniform1f(glGetUniformLocation(m_postProg, "u_mouseTexX"), mouse_dof_x*m_invWidth);
    //glUniform1f(glGetUniformLocation(m_postProg, "u_mouseTexY"), abs(static_cast<int32_t>(m_height)-mouse_dof_y)*m_invHeight);
    glUniform1f(glGetUniformLocation(m_postProg, "u_lenQuant"), 0.0025f);
    glUniform1f(glGetUniformLocation(m_postProg, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(m_postProg, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(m_postProg, "u_BloomOn"), 0/*m_bloomEnabled*/);
    glUniform1i(glGetUniformLocation(m_postProg, "u_toonOn"), 0/*m_toonEnabled*/);
    glUniform1i(glGetUniformLocation(m_postProg, "u_DOFOn"), 0/*m_DOFEnabled*/);
    glUniform1i(glGetUniformLocation(m_postProg, "u_DOFDebug"), 0/*m_DOFDebug*/);
    glDepthMask(GL_FALSE);
    RenderQuad();
    glDepthMask(GL_TRUE);
}

void GLRenderer::ApplyShaderConstantsForFullScreenPass(uint32_t glProgram)
{
    glUseProgram(glProgram);

    glm::mat4 persp = m_pRenderCam->GetPerspective();
    glUniform1i(glGetUniformLocation(glProgram, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(glProgram, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(glProgram, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(glProgram, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(glProgram, "u_DisplayType"), 1);
    glUniformMatrix4fv(glGetUniformLocation(glProgram, "u_Persp"), 1, GL_FALSE, &persp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_Depthtex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_Normaltex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_Positiontex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_Colortex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(glProgram, "u_GlowMask"), 6);
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

    out.texname = model.texname;
    out.modelMat = modelMatrix;
    out.color = model.color;
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

void GLRenderer::ClearLists()
{
    m_opaqueList.clear();
    m_alphaMaskedList.clear();
    m_transparentList.clear();
    m_lightList.clear();
}

void GLRenderer::RenderQuad()
{
    DrawGeometry(&m_QuadGeometry);
}

void GLRenderer::Render()
{
    // GBuffer Pass
    SetFramebufferActive(RenderEnums::GBUFFER_FRAMEBUFFER);
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    DrawOpaqueList();
    DrawAlphaMaskedList();
    EndActiveFramebuffer();

    // Lighting Pass
    SetFramebufferActive(RenderEnums::LIGHTING_FRAMEBUFFER);
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    DrawLightList();
    RenderAmbientLighting();
    glDisable(GL_BLEND);
    EndActiveFramebuffer();

    // Post Process Pass
    ClearFramebuffer(RenderEnums::CLEAR_ALL);
    glDisable(GL_DEPTH_TEST);
    RenderPostProcessEffects();
    glEnable(GL_DEPTH_TEST);
}

void GLRenderer::SetFramebufferActive(uint32_t fbID)
{
    assert(fbID < m_FBO.size());
    glBindTexture(GL_TEXTURE_2D, 0); //Bad mojo to unbind the framebuffer using the texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[fbID]);
}

void GLRenderer::EndActiveFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}