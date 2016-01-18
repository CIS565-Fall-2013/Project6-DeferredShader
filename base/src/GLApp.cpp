#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"
#include "EventHandlers.h"
#include "TextureManager.h"
#include "VertexSpecification.h"
#include <sstream>

#include "gl/glew.h"
#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;

const std::string GLApp::c_meshArgumentString = "mesh";

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
        Utility::LogMessageAndEndLine(outputMsg.str().c_str());
        assert(!shouldAssert);
    }
}

GLApp* GLApp::m_singleton = nullptr;

GLApp::GLApp(uint32_t width, uint32_t height, std::string windowTitle)
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
    m_windowTitle(windowTitle) 
{
    m_world = glm::mat4();

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

bool GLApp::ProcessScene(const std::string& sceneFile)
{
    // Create vertex specification for scene models:
    std::string sceneModelVertSpecName = "SceneModel";
    std::vector<VertexAttribute> sceneModelVertexAtribList;
    {
        VertexAttribute positionAttribute;
        positionAttribute.numElements = 3;
        positionAttribute.dataType = GL_FLOAT;
        positionAttribute.normalizeTo01Range = false;
        positionAttribute.bytesFromStartOfVertexData = 0;
        sceneModelVertexAtribList.push_back(positionAttribute);

        VertexAttribute normalAttribute;
        normalAttribute.numElements = 3;
        normalAttribute.dataType = GL_FLOAT;
        normalAttribute.normalizeTo01Range = false;
        normalAttribute.bytesFromStartOfVertexData = offsetof(Vertex, normal);
        sceneModelVertexAtribList.push_back(normalAttribute);

        VertexAttribute texCoordAttribute;
        texCoordAttribute.numElements = 2;
        texCoordAttribute.dataType = GL_FLOAT;
        texCoordAttribute.normalizeTo01Range = false;
        texCoordAttribute.bytesFromStartOfVertexData = offsetof(Vertex, texcoord);
        sceneModelVertexAtribList.push_back(texCoordAttribute);

        VertexAttribute tangentAttribute;
        tangentAttribute.numElements = 3;
        tangentAttribute.dataType = GL_FLOAT;
        tangentAttribute.normalizeTo01Range = false;
        tangentAttribute.bytesFromStartOfVertexData = offsetof(Vertex, tangent);
        sceneModelVertexAtribList.push_back(tangentAttribute);
    }
    m_renderer->CreateVertexSpecification(sceneModelVertSpecName, sceneModelVertexAtribList, sizeof(Vertex));

    std::vector<tinyobj::shape_t> sceneObjects;
    std::vector<tinyobj::material_t> materialList;
    
    std::string sceneFileDir = sceneFile.substr(0, sceneFile.find_last_of("/\\") + 1);
    Utility::LogMessage("Loading: ");
    Utility::LogMessageAndEndLine(sceneFile.c_str());
    std::string loadError;
    if (!tinyobj::LoadObj(sceneObjects, materialList, loadError, sceneFile.c_str(), sceneFileDir.c_str()))
    {
        Utility::LogMessageAndEndLine(loadError.c_str());
        return false;
    }

    float maxExtent = -1e6, minExtent = 1e6;
    for (auto it = sceneObjects.begin(); it != sceneObjects.end(); ++it)
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
                    quotient = (abs(quotient) > 1e-6) ? 1.0f / quotient : 0.0f;
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
                v.tangent = calculatedTangents[i] - (v.normal * glm::dot(v.normal, calculatedTangents[i]));
            }

            model.vertices.push_back(v);

            float toCompareGreater = (v.position.x > v.position.y) ? ((v.position.x > v.position.z) ? v.position.x : v.position.z) : ((v.position.y > v.position.z) ? v.position.y : v.position.z);
            float toCompareLesser = (v.position.x < v.position.y) ? ((v.position.x < v.position.z) ? v.position.x : v.position.z) : ((v.position.y < v.position.z) ? v.position.y : v.position.z);
            maxExtent = (maxExtent > toCompareGreater) ? maxExtent : toCompareGreater;
            minExtent = (minExtent < toCompareLesser) ? minExtent : toCompareLesser;
        }

        if (shape.mesh.material_ids.size() > 0)
        {
            tinyobj::material_t& modelMaterial = materialList[shape.mesh.material_ids[0]];
            if (modelMaterial.diffuse_texname.length() > 0)
            {
                model.diffuse_texpath = sceneFileDir;
                model.diffuse_texpath.append(modelMaterial.diffuse_texname);
            }
            if (modelMaterial.bump_texname.length() > 0)
            {
                model.normal_texpath = sceneFileDir;
                model.normal_texpath.append(modelMaterial.bump_texname);
            }
            if (modelMaterial.specular_texname.length() > 0)
            {
                model.specular_texpath = sceneFileDir;
                model.specular_texpath.append(modelMaterial.specular_texname);
            }
        }
        model.vertex_specification = sceneModelVertSpecName;

        std::unique_ptr<DrawableGeometry> drawableModel = std::make_unique<DrawableGeometry>();
        m_renderer->MakeDrawableModel(model, *drawableModel, m_world);
        m_drawableModels.push_back(std::move(drawableModel));
    }

    // Apply scene adaptive scaling. This ensures that our vertices will always be in the range [-100, 100] in all axes.
    float scale = (maxExtent < std::abs(minExtent) ? std::abs(minExtent) : maxExtent) / 100.0f;
    glm::mat4 sceneAdaptiveScaleInverse = glm::scale(glm::mat4(), glm::vec3(scale));
    scale = 1.0f / scale;
    glm::mat4 sceneAdaptiveScale = glm::scale(glm::mat4(), glm::vec3(scale));
    for (std::unique_ptr<DrawableGeometry>& i : m_drawableModels)
    {
        i->modelMat *= sceneAdaptiveScale;
        i->inverseModelMat = sceneAdaptiveScaleInverse * i->inverseModelMat;
    }

    Utility::LogMessageAndEndLine("Scene loading complete.");
    return true;
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

bool GLApp::Initialize(const std::map<std::string, std::string>& argumentList)
{
    if (!glfwInit())
    {
        Utility::LogMessageAndEndLine("Failed to initialize GLFW.");
        return false;
    }

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
        Utility::LogMessageAndEndLine("Failed to create GLFW window.");
        glfwTerminate();
        return false;
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
        Utility::LogMessageAndEndLine("Failed to initialize glew.");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OnGLError, nullptr);

    float nearPlane = 0.1f;
    float farPlane = 2000.0f;
    m_renderer = new GLRenderer(m_width, m_height, nearPlane, farPlane);
    if (m_renderer == nullptr)
    {
        Utility::LogMessageAndEndLine("Failed to create the renderer.");
        return false;
    }

    m_renderer->Initialize(m_cam);

    return ProcessScene(argumentList.at(c_meshArgumentString));
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