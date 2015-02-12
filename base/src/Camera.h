#pragma once

#include <glm/glm.hpp>

class Camera
{
private:
    glm::mat4 m_view;
    glm::mat4 m_transform;

    float rx;
    float ry;

    glm::vec3 pos;
    glm::vec3 start_dir;

public:
    glm::vec3 up;
    glm::vec3 start_left;

    Camera(glm::vec3 start_pos, glm::vec3 start_dir, glm::vec3 up);
    void adjust(float dx, float dy, float dz, float tx, float ty, float tz);
    void CalculateView();
    glm::mat4 get_view();
};
