#pragma once

#include <GLFW/glfw3.h>

class WindowGLFW
{
private:
    GLFWwindow* m_handle;

    int width = 1366;
    int height = 768;

public:
    static int init();
    static void terminate();

public:
    WindowGLFW();

    void makeContextCurrent();
    bool shouldClose();
    void swapBuffers();
    void pollEvents();
};