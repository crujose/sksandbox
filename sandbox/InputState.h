#pragma once

#include "glfw.h"
#include <SkPoint.h>

class InputState
{
    char m_keys[GLFW_KEY_LAST];
    char m_mouse[GLFW_MOUSE_BUTTON_LAST];
    SkPoint m_cursor;
    SkPoint m_prevCursor;

public:
    InputState();
    ~InputState();

    void setCursor(double x, double y);
    void setKey(int key, int action);
    void setButton(int button, int action);
    void poll();

    bool isKeyPressed(int key) const;
    bool isKeyDown(int key) const;
    bool isButtonPressed(int button) const;
    bool isButtonDown(int button) const;
    SkPoint getCursor() const;
    SkPoint getPreviousCursor() const;
};

