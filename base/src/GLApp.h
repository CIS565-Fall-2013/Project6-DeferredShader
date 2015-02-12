#pragma once

#include <glm/glm.hpp>
#include "tiny_obj_loader.h"

#include <vector>
#include <cstdint>

struct mesh_t
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned short> indices;
    std::string texname;
    glm::vec3 color;
};

struct device_mesh_t
{
    unsigned int vertex_array;
    unsigned int vbo_indices;
    unsigned int num_indices;
    unsigned int vbo_vertices;
    unsigned int vbo_normals;
    unsigned int vbo_texcoords;
    glm::vec3 color;
    std::string texname;
};

struct device_mesh2_t
{
    unsigned int vertex_array;
    unsigned int vbo_indices;
    unsigned int num_indices;
    //Don't need these to get it working, but needed for deallocation
    unsigned int vbo_data;
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

enum Display
{
    DISPLAY_DEPTH = 0,
    DISPLAY_NORMAL = 1,
    DISPLAY_POSITION = 2,
    DISPLAY_COLOR = 3,
    DISPLAY_TOTAL = 4,
    DISPLAY_LIGHTS = 5,
    DISPLAY_GLOWMASK = 6
};

class Camera;
class GLFWwindow;
class GLApp
{
    uint32_t m_startTime;
    uint32_t m_currentTime;
    uint32_t m_currentFrame;
    Display m_displayType;
    uint32_t m_width;
    uint32_t m_height;
    float m_invWidth;
    float m_invHeight;
    bool m_bloomEnabled;
    bool m_toonEnabled;
    bool m_DOFEnabled;
    bool m_DOFDebug;
    bool m_scissorEnabled;
    bool m_mouseCaptured;

    double m_lastX;
    double m_lastY;
    int32_t mouse_dof_x;
    int32_t mouse_dof_y;

    float m_farPlane;
    float m_nearPlane;

    glm::mat4 m_world;
    Camera* m_cam;

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

    GLFWwindow* m_glfwWindow;
    static GLApp* m_singleton;

    // FBOs
    std::vector<uint32_t> m_FBO;

    device_mesh2_t m_Quad;
    std::vector<device_mesh_t> m_meshes;

    std::string m_windowTitle;

    void initNoise();
    void initFBO();
    void initMesh(std::vector<tinyobj::shape_t>& shapes);
    void initQuad();
    void bindFBO(uint32_t buf);
    void setTextures();

    void setupQuad(uint32_t prog);
    void drawMeshes();
    void drawQuad();
    void drawLight(glm::vec3 pos, float strength, glm::mat4 sc, glm::mat4 vp);

    void display();
    void reshape(int, int);

    static void OnKeyPress(GLFWwindow* windowHandle, int32_t pressedKey, int32_t pressedKeyScancode, int32_t action, int32_t modifiers);
    static void OnMouseClick(GLFWwindow* windowHandle, int32_t pressedButton, int32_t action, int32_t modifiers);
    static void OnMouseMove(GLFWwindow* windowHandle, double xPos, double yPos);

    GLApp(uint32_t width, uint32_t height, std::string windowTitle);
public:
    ~GLApp();
    int32_t init(std::vector<tinyobj::shape_t>& shapes);
    int32_t Run();
    static GLApp* Create(uint32_t width, uint32_t height, std::string windowTitle)
    { 
        if (m_singleton)
            delete m_singleton;
        
        m_singleton = new GLApp(width, height, windowTitle);
        return m_singleton;
    }
    static GLApp* Get() { return m_singleton; }

    double GetLastX() { return m_lastX; }
    double GetLastY() { return m_lastY; }
    int32_t GetHeight() { return m_height; }
    int32_t GetWidth() { return m_width; }

    bool IsScissorEnabled() { return m_scissorEnabled; }
    bool IsBloomEnabled() { return m_bloomEnabled; }
    bool IsDOFEnabled() { return m_DOFEnabled; }
    bool IsToonEnabled() { return m_toonEnabled; }
    bool IsDOFDebug() { return m_DOFDebug; }
    bool IsMouseCaptured() { return m_mouseCaptured; }

    void SetScissorEnabled(bool isScissorEnabled) { m_scissorEnabled = isScissorEnabled; }
    void SetBloomEnabled(bool isBloomEnabled) { m_bloomEnabled = isBloomEnabled; }
    void SetDOFEnabled(bool isDOFEnabled) { m_DOFEnabled = isDOFEnabled; }
    void SetToonEnabled(bool isToonEnabled) { m_toonEnabled = isToonEnabled; }
    void SetDOFDebug(bool isDOFDebug) { m_DOFDebug = isDOFDebug; }
    void SetMouseCaptured(bool isMouseCaptured) { m_mouseCaptured = isMouseCaptured; }

    void ToggleScissor() { m_scissorEnabled = !m_scissorEnabled; }
    void ToggleBloom() { m_bloomEnabled = !m_bloomEnabled; }
    void ToggleDOF() { m_DOFEnabled = !m_DOFEnabled; }
    void ToggleToon() { m_toonEnabled = !m_toonEnabled; }
    void ToggleDOFDebug() { m_DOFDebug = !m_DOFDebug; }
    void ToggleMouseCaptured() { m_mouseCaptured = !m_mouseCaptured; }

    void SetDisplayType(Display newDisplayType) { m_displayType = newDisplayType; }
    void SetLastX(double lastX) { m_lastX = lastX; }
    void SetLastY(double lastY) { m_lastY = lastY; }

    void InitShader();

    void AdjustCamera(float xAdjustment, float yAdjustment, float zAdjustment);
    void RotateCamera(float xAngle, float yAngle);
};