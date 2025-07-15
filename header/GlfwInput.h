#pragma once

#include <GLFW/glfw3.h>
#include <utility>

namespace VKRE {

    class Input {
    public:
        static bool KeyPressed(uint32_t key) { return mPressedKeys[key]; }
        static bool KeyHeld(uint32_t key) { return glfwGetKey(mCurrentWindow, key) == GLFW_PRESS; }
        static bool KeyReleased(uint32_t key) { return mReleasedKeys[key]; }

        static bool MouseButtonPressed(uint32_t button) { return mPressedMouseButtons[button]; }
        static bool MouseButtonHeld(uint32_t button) { return glfwGetMouseButton(mCurrentWindow, button); }
        static bool MouseButtonReleased(uint32_t button) { return mReleasedMouseButtons[button]; }

        static void LockMouse() { glfwSetInputMode(mCurrentWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); };
        static void UnLockMouse() { glfwSetInputMode(mCurrentWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); };
        static bool IsMouseLocked() { return glfwGetInputMode(mCurrentWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }

        static std::pair<float, float> GetMousePosition() {
            double x, y;
            glfwGetCursorPos(mCurrentWindow, &x, &y);
            return std::make_pair(static_cast<float>(x), static_cast<float>(y));
        }

    protected:
        static void SetCurrentWindow(GLFWwindow* window) { mCurrentWindow = window; }
        static void OnUpdate() {
            glfwPollEvents();
            for (int key = 0; key < NUM_KEYS; key++) {
                int current_key_state = glfwGetKey(mCurrentWindow, key);

                mPressedKeys[key] =  current_key_state == GLFW_PRESS && mPreviousKeyState[key] == GLFW_RELEASE;
                mReleasedKeys[key] = current_key_state == GLFW_RELEASE && mPreviousKeyState[key] == GLFW_PRESS;

                mPreviousKeyState[key] = current_key_state;
            }

            for (int button = 0; button < NUM_MOUSE_BUTTONS; button++) {
                int current_button_state = glfwGetMouseButton(mCurrentWindow, button);

                mPressedMouseButtons[button] =  current_button_state == GLFW_PRESS && mPreviousMouseButtonState[button] == GLFW_RELEASE;
                mReleasedMouseButtons[button] = current_button_state == GLFW_RELEASE && mPreviousMouseButtonState[button] == GLFW_PRESS;

                mPreviousMouseButtonState[button] = current_button_state;
            }
        }

        inline static GLFWwindow* mCurrentWindow = nullptr;
    protected:
        inline static const int NUM_KEYS = 348;
        inline static int mPreviousKeyState[NUM_KEYS] = {0};
        inline static bool mPressedKeys[NUM_KEYS] = {false};
        inline static bool mReleasedKeys[NUM_KEYS] = {false};

        inline static const int NUM_MOUSE_BUTTONS= 8;
        inline static int mPreviousMouseButtonState[NUM_MOUSE_BUTTONS] = {0};
        inline static bool mPressedMouseButtons[NUM_MOUSE_BUTTONS] = {false};
        inline static bool mReleasedMouseButtons[NUM_MOUSE_BUTTONS] = {false};
    private:
        friend class Window;
    };

}



