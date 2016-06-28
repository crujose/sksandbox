#pragma once

#include <list>
#include <string>

#include "glfw.h"
#include "InputState.h"
#include "GraphicsContext.h"
#include "View.h"

class Window : public View
{
    InputState m_input;
    GLFWwindow* m_window;
    GraphicsContext m_gc;
    RenderTarget m_defaultTarget;

    std::string m_title;

    bool init();
    void reset();
    void run();
    void resize(int width, int height);
    void beginDraw();
    void onKey(int key, int action);
    void onCursor(double x, double y);
    void onButton(int button, int action);

    static void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void framebuffer_size_callback(GLFWwindow* w, int width, int height);
    static void refresh_callback(GLFWwindow* w);
    static void cursor_position_callback(GLFWwindow* w, double x, double y);
    static void mouse_button_callback(GLFWwindow* w, int button, int action, int mods);

public:
    Window(int width, int height, const std::string& title);
    ~Window();

    void show();
    void close();
};

