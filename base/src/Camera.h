#pragma once

#include <glm/glm.hpp>

class Camera
{
private:
    glm::mat4 m_view;
    glm::mat4 m_transform;
    glm::mat4 m_perspective;
    glm::mat4 m_inversePerspective;

    float rx;
    float ry;

    glm::vec3 pos;
    glm::vec3 start_dir;

public:
    glm::vec3 up;
    glm::vec3 start_left;

    Camera(glm::vec3 start_pos, glm::vec3 start_dir, glm::vec3 up);
    void adjust(float dx, float dy, float dz, float tx, float ty, float tz);
    void CalculateViewProjection(float fov, float width, float height, float nearPlane, float farPlane);

    glm::mat4 GetView() const;
    glm::mat4 GetPerspective() const;
    glm::mat4 GetInverseView() const;
    glm::mat4 GetInversePerspective() const;
};
