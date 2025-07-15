#include "GLFW/glfw3.h"
#include <GlfwWindow.h>

#include <cassert>

namespace VKRE {

    Window::Window(const WindowSpecs& specs)
        :mGLFWwindow(nullptr), mSpecs(specs) {

        if (!glfwInit())
            assert("Couldn't initialise glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mGLFWwindow = glfwCreateWindow(static_cast<int>(mSpecs.width), static_cast<int>(mSpecs.height), mSpecs.title.c_str(), nullptr, nullptr);
        if (!mGLFWwindow) {
            glfwTerminate();
            assert("Couldn't initialise window");
        }

        glfwMakeContextCurrent(mGLFWwindow);

        if (mSpecs.vsync)
            glfwSwapInterval(1);

        if (mSpecs.fullscreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(mGLFWwindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            mSpecs.width = mode->width;
            mSpecs.height = mode->height;
        }

        Input::SetCurrentWindow(mGLFWwindow);

        glfwSetWindowUserPointer(mGLFWwindow, this);
        glfwSetWindowSizeCallback(mGLFWwindow, [](GLFWwindow* window, int width, int height) {
                Window* ogreWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                ogreWindow->Resize(width, height);
        });
   }

    Window::~Window() {
        glfwDestroyWindow(mGLFWwindow);
        glfwTerminate();
    }

    void Window::Close() const {
        glfwSetWindowShouldClose(mGLFWwindow, true);
    }

    void Window::SetWidth(int width) {
        Resize(width, mSpecs.height);
    }

    void Window::SetHeight(int height) {
        Resize(mSpecs.width, height);
    }

    void Window::Resize(int width, int height) {
        mSpecs.width = width;
        mSpecs.height = height;
        glfwSetWindowSize(mGLFWwindow, static_cast<int>(mSpecs.width), static_cast<int>(mSpecs.height));
    }

    void Window::OnUpdate() const {
        glfwSwapBuffers(mGLFWwindow);
        Input::OnUpdate();
    }

    std::vector<const char*>Window::GetWindowExtensions() const {
        uint32_t extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return std::vector<const char*>(extensions, extensions + extensionCount);
    }

}

