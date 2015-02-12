#include <Windows.h>
#include "GLApp.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
//#include <GL/glut.h>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform2.hpp>

#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace glm;

//#define EPSILON 1e-6;

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
        glm::clamp(ry, -70.0f, 70.0f);
    }

    vec4 translation(tx, ty, tz, 0.0f);
    translation = m_transform * translation;
    pos += vec3(translation.x, translation.y, translation.z);
}

void Camera::CalculateView()
{
    m_transform = glm::translate(glm::mat4(), pos);
    m_transform = glm::rotate(m_transform, rx, glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform = glm::rotate(m_transform, ry, glm::vec3(1.0f, 0.0f, 0.0f));
    m_view = glm::inverse(m_transform);
}

mat4 Camera::get_view() 
{
    return m_view;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool loadedScene = false;
    string header; string data;
    istringstream liness(lpCmdLine);
    getline(liness, header, '='); 
    getline(liness, data, '=');

    std::vector<tinyobj::shape_t> shapes;
    if(strcmp(header.c_str(), "mesh") == 0)
    {
        int found = data.find_last_of("/\\");
        string path = data.substr(0,found+1);
        cerr << "Loading: " << data << endl;
        string err = tinyobj::LoadObj(shapes, data.c_str(), path.c_str());
        if(!err.empty())
        {
            cerr << err << endl;
            return -1;
        }
        loadedScene = true;
    }

    if(!loadedScene)
    {
        cerr << "Usage: mesh=[obj file]" << endl; 
        return EXIT_SUCCESS;
    }

    GLApp* app = GLApp::Create(1280, 720, "CIS 565 OpenGL Frame");
    if (!app || !app->init(shapes))
        return EXIT_FAILURE;

    return app->Run();
}
