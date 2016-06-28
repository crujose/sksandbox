#include <GL/glew.h>
#include "Window.h"

#include <vector>
#include <string>
#include <unordered_map>

Window::Window(int width, int height, const std::string& title)
    : m_title(title)
{
    setWH(SkIntToScalar(width), SkIntToScalar(height));
}

Window::~Window()
{
    close();
}

bool Window::init()
{
    glfwWindowHint(GLFW_STENCIL_BITS, 16);
    m_window = glfwCreateWindow(widthI(), heightI(), m_title.c_str(), NULL, NULL);
    if (!m_window) {
        return false;
    }
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, Window::key_callback);
    glfwSetFramebufferSizeCallback(m_window, Window::framebuffer_size_callback);
    glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);
    glfwSetCursorPosCallback(m_window, Window::cursor_position_callback);
    glfwSetWindowRefreshCallback(m_window, Window::refresh_callback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    m_gc.init();

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        printf("glewInit error: %s\n", glewGetErrorString(err));
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    resize(widthI(), heightI());
    return true;
}

void Window::reset()
{
    m_defaultTarget.reset();
    m_gc.reset();
    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

void Window::run()
{
    if (init()) {
        while (!glfwWindowShouldClose(m_window)) {
            update(m_input);
            m_input.poll();
            beginDraw();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
        exit();
        reset();
    }
}

void Window::resize(int width, int height)
{
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    setWH(SkIntToScalar(width), SkIntToScalar(height));
    m_defaultTarget = m_gc.createDefaultTarget(width, height, 16);
}

void Window::beginDraw()
{
    SkCanvas* canvas = m_defaultTarget.getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorBLACK);
        draw(*canvas);
        canvas->flush();
    }
}

void Window::onKey(int key, int action)
{
    m_input.setKey(key, action);
}

void Window::onCursor(double x, double y)
{
    m_input.setCursor(x, y);
}

void Window::onButton(int button, int action)
{
    m_input.setButton(button, action);
}

void Window::show()
{
    run();
}

void Window::close()
{
    if (m_window) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void Window::key_callback(GLFWwindow * w, int key, int scancode, int action, int mods)
{
    Window* window = (Window*)glfwGetWindowUserPointer(w);

    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) {
            window->close();
        }
        break;
    default:
        window->onKey(key, action);
        break;
    }
}

void Window::framebuffer_size_callback(GLFWwindow * w, int width, int height)
{
    Window* window = (Window*)glfwGetWindowUserPointer(w);
    window->resize(width, height);
}

void Window::refresh_callback(GLFWwindow * w)
{
    Window* window = (Window*)glfwGetWindowUserPointer(w);
    window->beginDraw();
    glfwSwapBuffers(w);
}

void Window::cursor_position_callback(GLFWwindow * w, double x, double y)
{
    Window* window = (Window*)glfwGetWindowUserPointer(w);
    window->onCursor(x, y);
}

void Window::mouse_button_callback(GLFWwindow * w, int button, int action, int mods)
{
    Window* window = (Window*)glfwGetWindowUserPointer(w);
    window->onButton(button, action);
}