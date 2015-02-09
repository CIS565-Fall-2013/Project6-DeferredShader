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

    int32_t mouse_buttons;
    int32_t mouse_old_x;
    int32_t mouse_dof_x;
    int32_t mouse_old_y;
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

    // FBOs
    std::vector<uint32_t> m_FBO;

    device_mesh2_t m_Quad;
    std::vector<device_mesh_t> m_meshes;

    std::string m_windowTitle;

    void initNoise();
    void initShader();
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
    void keyboard(unsigned char, int, int);
    void reshape(int, int);
    void mouse(int button, int state, int x, int y);
    void motion(int x, int y);

public:
    GLApp(uint32_t width, uint32_t height, std::string windowTitle);
    ~GLApp();
    int32_t init(std::vector<tinyobj::shape_t>& shapes);
    int32_t Run();
};