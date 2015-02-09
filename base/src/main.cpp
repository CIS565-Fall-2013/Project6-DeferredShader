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

void Camera::adjust(float dx, // look left right
        float dy, //look up down
        float dz,
        float tx, //strafe left right
        float ty,
        float tz)//go forward) //strafe up down
{

    if (abs(dx) > 0)
    {
        rx += dx;
        rx = fmod(rx,360.0f);
    }

    if (abs(dy) > 0)
    {
        ry += dy;
        ry = clamp(ry,-70.0f, 70.0f);
    }

    if (abs(tx) > 0)
    {
        vec3 dir = glm::gtx::rotate_vector::rotate(start_dir,rx + 90,up);
        vec2 dir2(dir.x,dir.y);
        vec2 mag = dir2 * tx;
        pos += mag;	
    }

    if (abs(ty) > 0)
    {
        z += ty;
    }

    if (abs(tz) > 0)
    {
        vec3 dir = glm::gtx::rotate_vector::rotate(start_dir,rx,up);
        vec2 dir2(dir.x,dir.y);
        vec2 mag = dir2 * tz;
        pos += mag;
    }
}

mat4 Camera::get_view() 
{
    vec3 inclin = glm::gtx::rotate_vector::rotate(start_dir,ry,start_left);
    vec3 spun = glm::gtx::rotate_vector::rotate(inclin,rx,up);
    vec3 cent(pos, z);
    return glm::lookAt(cent, cent + spun, up);
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

    GLApp* app = new GLApp(1280, 720, "CIS 565 OpenGL Frame");
    if (!app || !app->init(shapes))
        return EXIT_FAILURE;

    return app->Run();
}
