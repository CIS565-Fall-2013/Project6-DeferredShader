#include <Windows.h>
#include "GLApp.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include "Utility.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_projection.hpp>

#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace glm;

Camera::Camera(glm::vec3 start_pos, glm::vec3 start_dir, glm::vec3 up) 
    : pos(start_pos), 
    up(up),
    start_dir(start_dir), 
    start_left(glm::cross(start_dir, up)),
    rx(0), 
    ry(0) 
{
    m_view = glm::translate(glm::mat4(), start_pos);
    m_view = glm::rotate(m_view, rx, glm::vec3(1.0f, 0.0f, 0.0f));
    m_view = glm::rotate(m_view, ry, glm::vec3(0.0f, 1.0f, 0.0f));

    m_transform = m_view;
}

void Camera::adjust(float dx, // look left right
        float dy, //look up down
        float dz,
        float tx, //strafe left right
        float ty,
        float tz)//go forward) //strafe up down
{
    if (abs(dx) > FLT_EPSILON)
    {
        rx -= dx;
        rx = fmod(rx,360.0f);
    }

    if (abs(dy) > FLT_EPSILON)
    {
        ry -= dy;
        ry = glm::clamp(ry, -70.0f, 70.0f);
    }

    vec4 translation(tx, ty, tz, 0.0f);
    translation = m_transform * translation;
    pos += vec3(translation.x, translation.y, translation.z);
}

void Camera::CalculateViewProjection(float fov, float width, float height, float nearPlane, float farPlane)
{
    m_transform = glm::translate(glm::mat4(), pos);
    m_transform = glm::rotate(m_transform, rx, glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform = glm::rotate(m_transform, ry, glm::vec3(1.0f, 0.0f, 0.0f));
    m_view = glm::inverse(m_transform);

    m_perspective = glm::perspective(fov, width/height, nearPlane, farPlane);
}

mat4 Camera::get_view() const
{
    return m_view;
}

mat4 Camera::GetPerspective() const
{
    return m_perspective;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool loadedScene = false;
    string header; string data;
    istringstream liness(lpCmdLine);
    getline(liness, header, '='); 
    getline(liness, data, '=');

    std::vector<tinyobj::shape_t> scene;
    if(strcmp(header.c_str(), "mesh") == 0)
    {
        int found = data.find_last_of("/\\");
        string path = data.substr(0,found+1);
        Utility::LogOutput("Loading: ");
        Utility::LogOutput(data.c_str());
        Utility::LogOutput("\n");
        string err = tinyobj::LoadObj(scene, data.c_str(), path.c_str());
        if(!err.empty())
        {
            Utility::LogOutput(err.c_str());
            Utility::LogOutput("\n"); 
            return -1;
        }
        loadedScene = true;
    }

    if(!loadedScene)
    {
        Utility::LogOutput("Usage: mesh=[obj file]\n");
        return EXIT_SUCCESS;
    }

    GLApp* app = GLApp::Create(1280, 720, "CIS 565 OpenGL Frame");
    if (!app || !app->Initialize(scene))
    {
        delete app;
        return EXIT_FAILURE;
    }

    int32_t returnCode = app->Run();
    delete app;
    return returnCode;
}
