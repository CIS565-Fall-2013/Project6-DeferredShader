#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"
#include "EventHandlers.h"
#include "TextureManager.h"

#include "GLFW/glfw3.h"
#include "tiny_obj_loader.h"
#include <glm/gtc/matrix_transform.hpp>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;

GLApp* GLApp::m_singleton = nullptr;

GLApp::GLApp(uint32_t width, uint32_t height, std::string windowTitle, const std::string& modelBasePath)
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
    m_windowTitle(windowTitle), 
    m_modelBasePath(modelBasePath)
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

    TextureManager::Create();
}

GLApp::~GLApp()
{
    for (uint32_t i = 0; i < m_drawableModels.size(); ++i)
    {
        m_drawableModels[i] = nullptr;
    }
    delete m_cam;
    delete m_renderer;
    TextureManager::Destroy();
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
            Vertex v0, v1, v2;

            uint32_t idx0 = shape.mesh.indices[i];
            uint32_t idx1 = shape.mesh.indices[i + 1];
            uint32_t idx2 = shape.mesh.indices[i + 2];

            v0.position = vec3(shape.mesh.positions[3 * idx0], shape.mesh.positions[3 * idx0 + 1], shape.mesh.positions[3 * idx0 + 2]);
            v1.position = vec3(shape.mesh.positions[3 * idx1], shape.mesh.positions[3 * idx1 + 1], shape.mesh.positions[3 * idx1 + 2]);
            v2.position = vec3(shape.mesh.positions[3 * idx2], shape.mesh.positions[3 * idx2 + 1], shape.mesh.positions[3 * idx2 + 2]);

            if (shape.mesh.normals.size() > 0)
            {
                v0.normal = vec3(shape.mesh.normals[3 * idx0], shape.mesh.normals[3 * idx0 + 1], shape.mesh.normals[3 * idx0 + 2]);
                v1.normal = vec3(shape.mesh.normals[3 * idx1], shape.mesh.normals[3 * idx1 + 1], shape.mesh.normals[3 * idx1 + 2]);
                v2.normal = vec3(shape.mesh.normals[3 * idx2], shape.mesh.normals[3 * idx2 + 1], shape.mesh.normals[3 * idx2 + 2]);
            }
            else
            {
                vec3 norm = glm::normalize(glm::cross(glm::normalize(v1.position - v0.position), glm::normalize(v2.position - v0.position)));
                v0.normal = norm;
                v1.normal = norm;
                v2.normal = norm;
            }

            if (shape.mesh.texcoords.size() > 0)
            {
                v0.texcoord = vec2(shape.mesh.positions[2 * idx0], shape.mesh.positions[2 * idx0 + 1]);
                v1.texcoord = vec2(shape.mesh.positions[2 * idx1], shape.mesh.positions[2 * idx1 + 1]);
                v2.texcoord = vec2(shape.mesh.positions[2 * idx2], shape.mesh.positions[2 * idx2 + 1]);
            }
            else
            {
                vec2 tex(0.0);
                v0.texcoord = tex;
                v1.texcoord = tex;
                v2.texcoord = tex;
            }

            model.vertices.push_back(v0);
            model.vertices.push_back(v1);
            model.vertices.push_back(v2);
            
            model.indices.push_back(i);
            model.indices.push_back(i + 1);
            model.indices.push_back(i + 2);
        }

        model.color = vec3(shape.material.diffuse[0], shape.material.diffuse[1], shape.material.diffuse[2]);
        model.diffuse_texpath = m_modelBasePath;
        model.diffuse_texpath.append(shape.material.diffuse_texname);
        model.normal_texpath = m_modelBasePath;
        model.normal_texpath.append(shape.material.normal_texname);
        model.specular_texpath = m_modelBasePath;
        model.specular_texpath.append(shape.material.specular_texname);

        std::unique_ptr<DrawableGeometry> drawableModel = std::make_unique<DrawableGeometry>();
        m_renderer->MakeDrawableModel(model, *drawableModel, m_world);
        m_drawableModels.push_back(std::move(drawableModel));
    }
}

void GLApp::display()
{
    m_cam->CalculateViewProjection(45.0f, m_width, m_height, RENDERER->GetNearPlaneDistance(), RENDERER->GetFarPlaneDistance());

    m_renderer->ClearLists();
    for (uint32_t i = 0; i < m_drawableModels.size(); ++i)
    {
        m_renderer->AddDrawableGeometryToList(m_drawableModels[i].get(), RenderEnums::OPAQUE_LIST);
    }

    m_renderer->Render();
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
//    m_renderer->InitShaders();
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

    m_renderer->Initialize(m_cam);
    ProcessScene(scene);

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