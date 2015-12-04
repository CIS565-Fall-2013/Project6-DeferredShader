#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"
#include "EventHandlers.h"
#include "TextureManager.h"
#include <sstream>

#include "gl/glew.h"
#include "GLFW/glfw3.h"
#include "tiny_obj_loader.h"
#include <glm/gtc/matrix_transform.hpp>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;

namespace
{
    inline char* DebugEnumToString(GLenum debugEnum)
    {
        switch (debugEnum)
        {
        case GL_DEBUG_SOURCE_API:
            return "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "Window-system API";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "GLSL Shader compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "third-party application";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            return "this application";
            break;
        case GL_DEBUG_TYPE_ERROR:
            return "error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        case GL_DEBUG_TYPE_PORTABILITY:
            return "warning";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "performance advisory";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            return "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            return "Low";
            break;
        default:
            return "Unknown";
        }
    }

    void APIENTRY OnGLError(GLenum msgSource, GLenum msgType, GLuint id, GLenum msgSeverity, GLsizei length, const GLchar* message, const void* userParam)
    {
        if (msgType == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        bool shouldAssert = (msgType == GL_DEBUG_TYPE_ERROR);
        std::ostringstream outputMsg;

        outputMsg << DebugEnumToString(msgSeverity) << " severity " << DebugEnumToString(msgType) << " issued by " << DebugEnumToString(msgSource) << ": " << message << "\n";
        Utility::LogOutput(outputMsg.str().c_str());
        assert(!shouldAssert);
    }
}

GLApp* GLApp::m_singleton = nullptr;

GLApp::GLApp(uint32_t width, uint32_t height, std::string windowTitle, const std::string& modelBasePath)
    : m_startTime(0), 
    m_currentTime(0),
    m_currentFrame(0),
    m_displayType(RenderEnums::DISPLAY_TOTAL),
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
    vec3 scale(1.0f);
    mat4 tilt_mat = mat4();
    mat4 scale_mat = glm::scale(mat4(), scale);
    m_world = tilt_mat * scale_mat;

    m_cam = new Camera(vec3(0, 0, 0), glm::normalize(vec3(0, 0, -1)), glm::normalize(vec3(0, 1, 0)));

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
        uint32_t nIndices = shape.mesh.indices.size();
        uint32_t nVertices = shape.mesh.positions.size() / 3;
        uint32_t nNormals = shape.mesh.normals.size();
        uint32_t nTexCoords = shape.mesh.texcoords.size();
        assert(nVertices != 0);
        assert((nIndices % 3) == 0);

        std::vector<vec3> calculatedNormals, calculatedTangents(nVertices, vec3());
        if (nNormals == 0)
            calculatedNormals.resize(nVertices, vec3());

        Geometry model;
        for (uint32_t i = 0; i < nIndices; ++i)
        {
            model.indices.push_back(shape.mesh.indices[i]);

            if (i % 3 == 0)
            {
                // Normal and Tangent calculations.
                uint32_t thisIndex = shape.mesh.indices[i], nextIndex = shape.mesh.indices[i + 1], nextToNextIndex = shape.mesh.indices[i + 2];
                vec3 v1 = vec3(shape.mesh.positions[3 * nextIndex] - shape.mesh.positions[3 * thisIndex],
                                shape.mesh.positions[3 * nextIndex + 1] - shape.mesh.positions[3 * thisIndex + 1],
                                shape.mesh.positions[3 * nextIndex + 2] - shape.mesh.positions[3 * thisIndex + 2]);
                vec3 v2 = vec3(shape.mesh.positions[3 * nextToNextIndex] - shape.mesh.positions[3 * thisIndex],
                                shape.mesh.positions[3 * nextToNextIndex + 1] - shape.mesh.positions[3 * thisIndex + 1],
                                shape.mesh.positions[3 * nextToNextIndex + 2] - shape.mesh.positions[3 * thisIndex + 2]);

                if (nNormals == 0)
                {
                    // Compute normals
                    vec3 n1 = glm::cross(v1, v2);
                    calculatedNormals[thisIndex] += n1;
                    calculatedNormals[nextIndex] += n1;
                    calculatedNormals[nextToNextIndex] += n1;
                }
                if (nTexCoords != 0)
                {
                    // Compute tangents
                    vec2 s1t1 = vec2(shape.mesh.texcoords[2 * nextIndex] - shape.mesh.texcoords[2 * thisIndex],
                                     shape.mesh.texcoords[2 * nextIndex + 1] - shape.mesh.texcoords[2 * thisIndex + 1]);
                    vec2 s2t2 = vec2(shape.mesh.texcoords[2 * nextToNextIndex] - shape.mesh.texcoords[2 * thisIndex],
                                     shape.mesh.texcoords[2 * nextToNextIndex + 1] - shape.mesh.texcoords[2 * thisIndex + 1]);

                    float quotient = (s1t1.x * s2t2.y - s2t2.x * s1t1.y);   // This is the denominator of the actual quotient. We'll set the actual quotient to zero if the denominator is zero. 
                    quotient = (quotient > 1e-6) ? 1.0f / quotient : 0.0f;
                    vec3 tangent = vec3(quotient * (s2t2.y * v1.x - s1t1.y * v2.x),
                                        quotient * (s2t2.y * v1.y - s1t1.y * v2.y),
                                        quotient * (s2t2.y * v1.z - s1t1.y * v2.z));

                    calculatedTangents[thisIndex] += tangent;
                    calculatedTangents[nextIndex] += tangent;
                    calculatedTangents[nextToNextIndex] += tangent;
                }
            }
        }
        for (uint32_t i = 0; i < nVertices; ++i)
        {
            Vertex v;
            v.position = vec3(shape.mesh.positions[3 * i], shape.mesh.positions[3 * i + 1], shape.mesh.positions[3 * i + 2]);
            if (nNormals != 0)
                v.normal = vec3(shape.mesh.normals[3 * i], shape.mesh.normals[3 * i + 1], shape.mesh.normals[3 * i + 2]);
            else
                v.normal = calculatedNormals[i];
            if (nTexCoords != 0)
            {
                v.texcoord = vec2(shape.mesh.texcoords[2 * i], shape.mesh.texcoords[2 * i + 1]);
                v.tangent = (glm::dot(calculatedTangents[i], calculatedTangents[i]) > 1e-6) ? calculatedTangents[i] - (v.normal * glm::dot(v.normal, calculatedTangents[i])) : glm::vec3();
            }

            model.vertices.push_back(v);
        }

        model.color = vec3(shape.material.diffuse[0], shape.material.diffuse[1], shape.material.diffuse[2]);
        model.diffuse_texpath = m_modelBasePath;
        model.diffuse_texpath.append(shape.material.diffuse_texname);
        model.normal_texpath = m_modelBasePath;
        model.normal_texpath.append(shape.material.unknown_parameter["map_bump"]);
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
    m_renderer->SetDisplayType(m_displayType);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
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

    glEnable(GL_DEBUG_OUTPUT);
#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    glDebugMessageCallback(OnGLError, nullptr);

    float nearPlane = 0.1f;
    float farPlane = 2000.0f;
    m_renderer = new GLRenderer(m_width, m_height, nearPlane, farPlane);
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