#pragma once

#include "GLRenderer.h"
#include <memory>
#include <map>

class Camera;
class TextureManager;
struct GLFWwindow;
class GLApp
{
    uint32_t m_startTime;
    uint32_t m_currentTime;
    uint32_t m_currentFrame;
    RenderEnums::DisplayType m_displayType;
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
    std::shared_ptr<Camera> m_spViewCamera;

    std::unique_ptr<GLRenderer> m_spRenderer;
    std::shared_ptr<TextureManager> m_spTextureManager;

    GLFWwindow* m_glfwWindow;
    static std::weak_ptr<GLApp> g_spSingleton;

    std::vector<std::unique_ptr<DrawableGeometry>> m_drawableModels;

    std::string m_windowTitle;

    // Loops through each model in the scene and creates Vertex/Index buffers for each.
    // Also uploads data to GPU.
    bool ProcessScene(const std::string& sceneFile);

    void display();
    void reshape(int, int);

    GLApp(uint32_t width, uint32_t height, std::string windowTitle);
public:
    ~GLApp();

    bool Initialize(const std::map<std::string, std::string>& argumentList);
    int32_t Run();

    static std::shared_ptr<GLApp> Create(uint32_t width, uint32_t height, std::string windowTitle); // Whomever calls this gets the owning reference. Only the very first call is effective. Subsequent calls are ignored.
    static std::weak_ptr<GLApp>& Get() { return g_spSingleton; }

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

    void SetDisplayType(RenderEnums::DisplayType newDisplayType) { m_displayType = newDisplayType; }
    void SetLastX(double lastX) { m_lastX = lastX; }
    void SetLastY(double lastY) { m_lastY = lastY; }

    void AdjustCamera(float xAdjustment, float yAdjustment, float zAdjustment);
    void RotateCamera(float xAngle, float yAngle);

    void ReloadShaders();

    static const std::string c_meshArgumentString;
};

#define RENDERER m_spRenderer