#pragma once

#include "GLRenderer.h"
#include "tiny_obj_loader.h"
#include <memory>

enum Display
{
    DISPLAY_DEPTH = 0,
    DISPLAY_NORMAL = 1,
    DISPLAY_POSITION = 2,
    DISPLAY_COLOR = 3,
    DISPLAY_TOTAL = 4,
    DISPLAY_LIGHTS = 5,
};

class Camera;
struct GLFWwindow;
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

    glm::mat4 m_world;
    Camera* m_cam;

    GLRenderer* m_renderer;

    GLFWwindow* m_glfwWindow;
    static GLApp* m_singleton;

    std::vector<std::unique_ptr<DrawableGeometry>> m_drawableModels;

    std::string m_windowTitle;
    std::string m_modelBasePath;

    // Loops through each model in the scene and creates Vertex/Index buffers for each.
    // Also uploads data to GPU.
    void ProcessScene(std::vector<tinyobj::shape_t>& scene);

    void bindFBO(uint32_t buf);
    void setTextures();

    void setupQuad(uint32_t prog);
    void drawMeshes();
    void drawLight(glm::vec3 pos, float strength, glm::mat4 sc, glm::mat4 vp);

    void display();
    void reshape(int, int);

    GLApp(uint32_t width, uint32_t height, std::string windowTitle, const std::string& modelBasePath);
public:
    ~GLApp();
    int32_t Initialize(std::vector<tinyobj::shape_t>& shapes);
    int32_t Run();
    static GLApp* Create(uint32_t width, uint32_t height, std::string windowTitle, const std::string& modelBasePath)
    { 
        if (m_singleton)
            delete m_singleton;
        
        m_singleton = new GLApp(width, height, windowTitle, modelBasePath);
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

    void AdjustCamera(float xAdjustment, float yAdjustment, float zAdjustment);
    void RotateCamera(float xAngle, float yAngle);

    void ReloadShaders();
};

#define RENDERER m_renderer