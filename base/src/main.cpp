#include <Windows.h>
#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <string>
#include <sstream>

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
    m_perspective = glm::perspective(fov, width / height, nearPlane, farPlane);

    m_view = glm::inverse(m_transform);
    m_inversePerspective = glm::inverse(m_perspective);
}

mat4 Camera::GetView() const
{
    return m_view;
}

mat4 Camera::GetPerspective() const
{
    return m_perspective;
}

mat4 Camera::GetInverseView() const
{
    return m_transform;
}

mat4 Camera::GetInversePerspective() const
{
    return m_inversePerspective;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::map<std::string, std::string> argumentList;

    {   // We'll leak two strings and an istringstream otherwise.
        std::string argument, value;
        std::istringstream liness(lpCmdLine);
        while (1)
        {
            if (liness.peek() == EOF)
                break;

            std::getline(liness, argument, '=');
            std::getline(liness, value, ' ');
            if ((value.find_first_of('\"') != std::string::npos) && (value.find_first_of('\"') == value.find_last_of('\"')))
            {
                // There's an unclosed quote in this string and spaces are okay within quotes. Fetch till we find the end quote and keep appending.
                std::string _value;
                do
                {
                    value.append(" ");
                    std::getline(liness, _value, ' ');
                    value.append(_value);
                } while (value.back() != '\"');
            }

            // Clean strings enclosed within quotes to remove them.
            if ((value.front() == '\"') && (value.back() == '\"'))
            {
                std::string _value(value);
                value = _value.substr(1, _value.length() - 2);
            }

            argumentList[argument] = value;
        }
    }

    {   // We'll leak an std::map::iterator otherwise
        auto mapItr = argumentList.find(GLApp::c_meshArgumentString);
        if (mapItr == argumentList.end())
        {
            Utility::LogOutput("Usage: ");
            Utility::LogOutput(GLApp::c_meshArgumentString.c_str());
            Utility::LogOutputAndEndLine("=\"obj file\"");
            return EXIT_SUCCESS;
        }
    }

    GLApp* app = GLApp::Create(1280, 720, "P6");
    if (!app || !app->Initialize(argumentList))
    {
        delete app;
        return EXIT_FAILURE;
    }

    int32_t returnCode = app->Run();
    delete app;
    return returnCode;
}
