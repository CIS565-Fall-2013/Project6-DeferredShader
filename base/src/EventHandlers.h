#pragma once
#include <cstdint>

struct GLFWwindow;
namespace EventHandler
{
    void OnKeyPress(GLFWwindow* windowHandle, int32_t pressedKey, int32_t pressedKeyScancode, int32_t action, int32_t modifiers);
    void OnMouseClick(GLFWwindow* windowHandle, int32_t pressedButton, int32_t action, int32_t modifiers);
    void OnMouseMove(GLFWwindow* windowHandle, double xPos, double yPos);
}