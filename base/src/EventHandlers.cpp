#include "EventHandlers.h"
#include "GLApp.h"
#include "GLFW/glfw3.h"

namespace EventHandler
{
    void OnMouseClick(GLFWwindow* windowHandle, int32_t pressedButton, int32_t action, int32_t modifiers)
    {
        if ((pressedButton == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS))
        {
            GLApp* thisApp = GLApp::Get();
            if (thisApp == nullptr)
                return;

            thisApp->ToggleMouseCaptured();
            glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    void OnMouseMove(GLFWwindow* windowHandle, double xPos, double yPos)
    {
        GLApp* thisApp = GLApp::Get();
        if (thisApp == nullptr)
            return;

        if (thisApp->IsMouseCaptured())
        {
            double dx, dy;
            dx = (xPos - thisApp->GetLastX()) / thisApp->GetWidth();
            dy = (yPos - thisApp->GetLastY()) / thisApp->GetHeight();

            thisApp->RotateCamera(dx*14.0, dy*14.0);

            thisApp->SetLastX(xPos);
            thisApp->SetLastY(yPos);
        }
    }

    void OnKeyPress(GLFWwindow* someWindow, int32_t pressedKey, int32_t pressedKeyScancode, int32_t action, int32_t modifiers)
    {
        float tx = 0;
        float ty = 0;
        float tz = 0;

        if (action == GLFW_RELEASE)
            return;

        GLApp* thisApp = GLApp::Get();
        if (thisApp == nullptr)
            return;

        switch (pressedKey)
        {
        case GLFW_KEY_ESCAPE:
            if (thisApp->IsMouseCaptured())
            {
                glfwSetInputMode(someWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                thisApp->ToggleMouseCaptured();
            }
            else
                glfwSetWindowShouldClose(someWindow, GL_TRUE);
            break;
        case GLFW_KEY_W:
            tz = -0.1f;
            break;
        case GLFW_KEY_S:
            tz = 0.1f;
            break;
        case GLFW_KEY_D:
            tx = 0.1f;
            break;
        case GLFW_KEY_A:
            tx = -0.1f;
            break;
        case GLFW_KEY_Q:
            ty = 0.1f;
            break;
        case GLFW_KEY_Z:
            ty = -0.1f;
            break;
        case GLFW_KEY_1:
        case GLFW_KEY_KP_1:
            thisApp->SetDisplayType(DISPLAY_DEPTH);
            break;
        case GLFW_KEY_2:
        case GLFW_KEY_KP_2:
            thisApp->SetDisplayType(DISPLAY_NORMAL);
            break;
        case GLFW_KEY_3:
        case GLFW_KEY_KP_3:
            thisApp->SetDisplayType(DISPLAY_COLOR);
            break;
        case GLFW_KEY_4:
        case GLFW_KEY_KP_4:
            thisApp->SetDisplayType(DISPLAY_POSITION);
            break;
        case GLFW_KEY_5:
        case GLFW_KEY_KP_5:
//            thisApp->SetDisplayType(DISPLAY_LIGHTS);
            break;
        case GLFW_KEY_0:
        case GLFW_KEY_KP_0:
            thisApp->SetDisplayType(DISPLAY_TOTAL);
            break;
        case GLFW_KEY_X:
            thisApp->ToggleScissor();
            break;
        case GLFW_KEY_R:
            thisApp->ReloadShaders();
            break;
        case GLFW_KEY_B:
            thisApp->ToggleBloom();
            break;
        case GLFW_KEY_T:
            thisApp->ToggleToon();
            break;
        case GLFW_KEY_F:
            thisApp->ToggleDOF();
            break;
        case GLFW_KEY_G:
            thisApp->ToggleDOFDebug();
            break;
        }

        if (abs(tx) > 0 || abs(tz) > 0 || abs(ty) > 0)
        {
            thisApp->AdjustCamera(tx, ty, tz);
        }
    }
}