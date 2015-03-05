#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"
#include "EventHandlers.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "SOIL/SOIL.h"
#include "tiny_obj_loader.h"
#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;

GLApp* GLApp::m_singleton = nullptr;

GLApp::GLApp(uint32_t width, uint32_t height, std::string windowTitle)
    : m_startTime(0), 
    m_currentTime(0),
    m_currentFrame(0),
    m_displayType(DISPLAY_TOTAL),
    m_width(width),
    m_height(height),
    m_bloomEnabled(true),
    m_toonEnabled(false),
    m_DOFEnabled(false),
    m_DOFDebug(false),
    m_scissorEnabled(true),
    m_mouseCaptured(true),
    mouse_dof_x(0),
    mouse_dof_y(0),
    m_farPlane(5000.0f),
    m_nearPlane(0.1f),
    m_windowTitle(windowTitle)
{
    vec3 tilt(1.0f, 0.0f, 0.0f);
    mat4 tilt_mat = mat4();
    mat4 scale_mat = glm::scale(mat4(), vec3(0.01));
    m_world = tilt_mat * scale_mat;

    m_cam = new Camera(vec3(0, 2, 0), glm::normalize(vec3(0, 0, -1)), glm::normalize(vec3(0, 1, 0)));

    m_invHeight = 1.0f / (m_height - 1);
    m_invWidth = 1.0f / (m_width - 1);

    m_lastX = width / 2.0;
    m_lastY = height / 2.0;
}

GLApp::~GLApp()
{
    delete m_cam;
}

void GLApp::ProcessScene(std::vector<tinyobj::shape_t>& scene)
{
    for (auto it = scene.begin(); it != scene.end(); ++it)
    {
        tinyobj::shape_t shape = *it;
        uint32_t nVertices = shape.mesh.indices.size();

        Geometry model;
        for (uint32_t i = 0; i < nVertices; i+=3)
        {
            uint32_t idx0 = shape.mesh.indices[i];
            uint32_t idx1 = shape.mesh.indices[i + 1];
            uint32_t idx2 = shape.mesh.indices[i + 2];

            vec3 p0 = vec3(shape.mesh.positions[3 * idx0], shape.mesh.positions[3 * idx0 + 1], shape.mesh.positions[3 * idx0 + 2]);
            vec3 p1 = vec3(shape.mesh.positions[3 * idx1], shape.mesh.positions[3 * idx1 + 1], shape.mesh.positions[3 * idx1 + 2]);
            vec3 p2 = vec3(shape.mesh.positions[3 * idx2], shape.mesh.positions[3 * idx2 + 1], shape.mesh.positions[3 * idx2 + 2]);
            model.vertices.push_back(p0);
            model.vertices.push_back(p1);
            model.vertices.push_back(p2);

            if (shape.mesh.normals.size() > 0)
            {
                model.normals.push_back(vec3(shape.mesh.normals[3 * idx0], shape.mesh.normals[3 * idx0 + 1], shape.mesh.normals[3 * idx0 + 2]));
                model.normals.push_back(vec3(shape.mesh.normals[3 * idx1], shape.mesh.normals[3 * idx1 + 1], shape.mesh.normals[3 * idx1 + 2]));
                model.normals.push_back(vec3(shape.mesh.normals[3 * idx2], shape.mesh.normals[3 * idx2 + 1], shape.mesh.normals[3 * idx2 + 2]));
            }
            else
            {
                vec3 norm = glm::normalize(glm::cross(glm::normalize(p1 - p0), glm::normalize(p2 - p0)));
                model.normals.push_back(norm);
                model.normals.push_back(norm);
                model.normals.push_back(norm);
            }

            if (shape.mesh.texcoords.size() > 0)
            {
                model.texcoords.push_back(vec2(shape.mesh.positions[2 * idx0], shape.mesh.positions[2 * idx0 + 1]));
                model.texcoords.push_back(vec2(shape.mesh.positions[2 * idx1], shape.mesh.positions[2 * idx1 + 1]));
                model.texcoords.push_back(vec2(shape.mesh.positions[2 * idx2], shape.mesh.positions[2 * idx2 + 1]));
            }
            else
            {
                vec2 tex(0.0);
                model.texcoords.push_back(tex);
                model.texcoords.push_back(tex);
                model.texcoords.push_back(tex);
            }

            model.indices.push_back(i);
            model.indices.push_back(i + 1);
            model.indices.push_back(i + 2);
        }

        model.color = vec3(shape.material.diffuse[0], shape.material.diffuse[1], shape.material.diffuse[2]);
        model.texname = shape.material.name;//diffuse_texname;

        DrawableGeometry drawableModel;
        m_renderer->MakeDrawableGeometry(model, drawableModel);
        m_drawableModels.push_back(drawableModel);
    }
}

void GLApp::bindFBO(uint32_t buf) 
{
    assert(buf < RENDERER->m_FBO.size());
    glBindTexture(GL_TEXTURE_2D, 0); //Bad mojo to unbind the framebuffer using the texture
    glBindFramebuffer(GL_FRAMEBUFFER, RENDERER->m_FBO[buf]);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glColorMask(false,false,false,false);
    glEnable(GL_DEPTH_TEST);
}

void GLApp::setTextures() 
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glColorMask(true,true,true,true);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLApp::setupQuad(GLuint prog)
{
    glUseProgram(prog);

    mat4 persp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane);
    vec4 test(-2, 0, 10, 1);
    vec4 testp = persp * test;
    vec4 testh = testp / testp.w;
    vec2 coords = vec2(testh.x, testh.y) / 2.0f + 0.5f;
    glUniform1i(glGetUniformLocation(prog, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(prog, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(prog, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(prog, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(prog, "u_DisplayType"), m_displayType);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Persp"), 1, GL_FALSE, &persp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_depthTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Depthtex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_normalTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Normaltex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_positionTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Positiontex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_colorTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Colortex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(prog, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(prog, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(prog, "u_GlowMask"), 6);
}


void GLApp::drawMeshes()
{
    m_farPlane = 100.0f;
    m_nearPlane = 0.1f;

    glUseProgram(RENDERER->m_passProg);

    mat4 view = m_cam->get_view();
    mat4 persp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane);
    mat4 inverse_transposed = glm::transpose(glm::inverse(view*m_world));

    glUniform1f(glGetUniformLocation(RENDERER->m_passProg, "u_Far"), m_farPlane);
    glUniformMatrix4fv(glGetUniformLocation(RENDERER->m_passProg, "u_Model"), 1, GL_FALSE, &m_world[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(RENDERER->m_passProg, "u_View"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(RENDERER->m_passProg, "u_Persp"), 1, GL_FALSE, &persp[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(RENDERER->m_passProg, "u_InvTrans"), 1, GL_FALSE, &inverse_transposed[0][0]);

    for (uint32_t i = 0; i < m_drawableModels.size(); i++)
    {
        glUniform3fv(glGetUniformLocation(RENDERER->m_passProg, "u_Color"), 1, &(m_drawableModels[i].color[0]));
        glBindVertexArray(m_drawableModels[i].vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_drawableModels[i].index_buffer);

        float glowmask = 0.0f;
        if (m_drawableModels[i].texname == "light")
            glowmask = 1.0f;
        glUniform1f(glGetUniformLocation(RENDERER->m_passProg, "glowmask"), glowmask);
        glDrawElements(GL_TRIANGLES, m_drawableModels[i].num_indices, GL_UNSIGNED_SHORT, 0);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLApp::drawLight(vec3 pos, float strength, mat4 sc, mat4 vp)
{
    float radius = strength;
    vec4 light = m_cam->get_view() * vec4(pos, 1.0);
    if (light.z > m_nearPlane)
    {
        return;
    }
    light.w = radius;
    glUniform4fv(glGetUniformLocation(RENDERER->m_pointProg, "u_Light"), 1, &(light[0]));
    glUniform1f(glGetUniformLocation(RENDERER->m_pointProg, "u_LightIl"), strength);

    vec4 left = vp * vec4(pos + radius*m_cam->start_left, 1.0);
    vec4 up = vp * vec4(pos + radius*m_cam->up, 1.0);
    vec4 center = vp * vec4(pos, 1.0);

    left = sc * left;
    up = sc * up;
    center = sc * center;

    left /= left.w;
    up /= up.w;
    center /= center.w;

    float hw = glm::distance(left, center);
    float hh = glm::distance(up, center);

    float r = (hh > hw) ? hh : hw;

    float x = center.x - r;
    float y = center.y - r;

    glScissor(x, y, 2 * r, 2 * r);
    m_renderer->RenderQuad();
}

void GLApp::display()
{
    m_cam->CalculateView();

    // Stage 1 -- RENDER TO G-BUFFER
    bindFBO(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMeshes();
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_glowmaskTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Stage 2 -- RENDER TO P-BUFFER
    setTextures();
    bindFBO(1);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT);
    if (m_displayType == DISPLAY_LIGHTS || m_displayType == DISPLAY_TOTAL)
    {
        setupQuad(RENDERER->m_pointProg);
        if (m_scissorEnabled)
            glEnable(GL_SCISSOR_TEST);
        mat4 vp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane) * m_cam->get_view();
        mat4 sc = mat4(m_width, 0.0, 0.0, 0.0,
            0.0, m_height, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0) *
            mat4(0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.5, 0.5, 0.0, 1.0);

        glm::vec3 yellow = glm::vec3(1, 1, 0);
        glm::vec3 orange = glm::vec3(0.89, 0.44, 0.1);
        glm::vec3 red = glm::vec3(1, 0, 0);
        glm::vec3 blue = glm::vec3(0, 0, 1);

        glUniform1i(glGetUniformLocation(RENDERER->m_pointProg, "u_toonOn"), m_toonEnabled);
        glUniform3fv(glGetUniformLocation(RENDERER->m_pointProg, "u_LightCol"), 1, &(yellow[0]));
        drawLight(vec3(5.4, -0.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -0.5, 3.0), 1.0, sc, vp);
        glUniform3fv(glGetUniformLocation(RENDERER->m_pointProg, "u_LightCol"), 1, &(orange[0]));
        drawLight(vec3(5.4, -2.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -2.5, 3.0), 1.0, sc, vp);
        glUniform3fv(glGetUniformLocation(RENDERER->m_pointProg, "u_LightCol"), 1, &(yellow[0]));
        drawLight(vec3(5.4, -4.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -4.5, 3.0), 1.0, sc, vp);

        glUniform3fv(glGetUniformLocation(RENDERER->m_pointProg, "u_LightCol"), 1, &(red[0]));
        drawLight(vec3(2.5, -1.2, 0.5), 2.5, sc, vp);

        glUniform3fv(glGetUniformLocation(RENDERER->m_pointProg, "u_LightCol"), 1, &(blue[0]));
        drawLight(vec3(2.5, -5.0, 4.2), 2.5, sc, vp);

        glDisable(GL_SCISSOR_TEST);
        vec4 dir_light(0.1, 1.0, 1.0, 0.0);
        dir_light = m_cam->get_view() * dir_light;
        dir_light = glm::normalize(dir_light);
        dir_light.w = 0.3f;
        float strength = 0.09f;

        setupQuad(RENDERER->m_ambientProg);
        glUniform1i(glGetUniformLocation(RENDERER->m_ambientProg, "u_toonOn"), m_toonEnabled);
        glUniform4fv(glGetUniformLocation(RENDERER->m_ambientProg, "u_Light"), 1, &(dir_light[0]));
        glUniform1f(glGetUniformLocation(RENDERER->m_ambientProg, "u_LightIl"), strength);
        m_renderer->RenderQuad();
    }
    else
    {
        setupQuad(RENDERER->m_diagnosticProg);
        m_renderer->RenderQuad();
    }
    glDisable(GL_BLEND);

    //Stage 3 -- RENDER TO SCREEN
    setTextures();
    glUseProgram(RENDERER->m_postProg);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_postTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_Posttex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_GlowMask"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_normalTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_normalTex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_positionTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_positionTex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, RENDERER->m_depthTexture);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_depthTex"), 6);

    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_InvScrHeight"), m_invHeight);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_InvScrWidth"), m_invWidth);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_mouseTexX"), mouse_dof_x*m_invWidth);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_mouseTexY"), abs(static_cast<int32_t>(m_height)-mouse_dof_y)*m_invHeight);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_lenQuant"), 0.0025f);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(RENDERER->m_postProg, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_BloomOn"), m_bloomEnabled);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_toonOn"), m_toonEnabled);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_DOFOn"), m_DOFEnabled);
    glUniform1i(glGetUniformLocation(RENDERER->m_postProg, "u_DOFDebug"), m_DOFDebug);
    m_renderer->RenderQuad();

    glEnable(GL_DEPTH_TEST);
    //    updateTitle();
}

void GLApp::reshape(int w, int h)
{
    m_width = w;
    m_height = h;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    //if (m_depthTexture)
    //    glDeleteTextures(1, &m_depthTexture);
    //if (m_normalTexture)
    //    glDeleteTextures(1, &m_normalTexture);
    //if (m_positionTexture)
    //    glDeleteTextures(1, &m_positionTexture);
    //if (m_colorTexture)
    //    glDeleteTextures(1, &m_colorTexture);
    //if (m_postTexture)
    //    glDeleteTextures(1, &m_postTexture);

    //for (GLuint& fbo : m_FBO)
    //    glDeleteFramebuffers(1, &fbo);

    //initFBO();
}

void GLApp::AdjustCamera(float xAdjustment, float yAdjustment, float zAdjustment)
{ 
    if (m_cam)
        m_cam->adjust(0.0f, 0.0f, 0.0f, xAdjustment, yAdjustment, zAdjustment); 
}

void GLApp::RotateCamera(float xAngle, float yAngle)
{
    if (m_cam)
        m_cam->adjust(xAngle, yAngle, 0.0f, 0.0f, 0.0f, 0.0f);
}

void GLApp::ReloadShaders()
{
    assert(m_renderer != NULL);
    m_renderer->InitShaders();
}

int32_t GLApp::Initialize(std::vector<tinyobj::shape_t>& scene)
{
    if (!glfwInit())
        return 0;

    /* Create a windowed mode window and its OpenGL context */
    m_glfwWindow = glfwCreateWindow(m_width, m_height, m_windowTitle.c_str(), NULL, NULL);
    if (!m_glfwWindow)
    {
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(m_glfwWindow);

    glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(m_glfwWindow, EventHandler::OnKeyPress);
    glfwSetMouseButtonCallback(m_glfwWindow, EventHandler::OnMouseClick);
    glfwSetCursorPosCallback(m_glfwWindow, EventHandler::OnMouseMove);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        return 0;
    }

    m_renderer = new GLRenderer(m_width, m_height);
    if (m_renderer == nullptr)
        return 0;

    m_renderer->InitNoise();
    m_renderer->InitShaders();
    m_renderer->InitFBO();
    m_renderer->InitQuad();
    ProcessScene(scene);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return 1;
}

int32_t GLApp::Run()
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(m_glfwWindow))
    {
        /* Render here */
        display();

        /* Swap front and back buffers */
        glfwSwapBuffers(m_glfwWindow);

        /* Wait for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}