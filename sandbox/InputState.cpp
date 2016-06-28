#include "InputState.h"

InputState::InputState()
{
    memset(m_keys, 0, sizeof(m_keys));
    memset(m_mouse, 0, sizeof(m_mouse));
}

InputState::~InputState()
{
}

void InputState::setCursor(double x, double y)
{
    m_cursor.set(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void InputState::setKey(int key, int action)
{
    if (key < GLFW_KEY_LAST) {
        m_keys[key] = action;
    }
}

void InputState::setButton(int button, int action)
{
    if (button < GLFW_MOUSE_BUTTON_LAST) {
        m_mouse[button] = action;
    }
}

void InputState::poll()
{
    m_prevCursor = m_cursor;
    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) {
        if (m_mouse[i] == GLFW_PRESS) {
            m_mouse[i] = GLFW_REPEAT;
        }
    }
}

bool InputState::isKeyPressed(int key) const
{
    if (key < GLFW_KEY_LAST) {
        return m_keys[key] == GLFW_PRESS;
    }
    return false;
}

bool InputState::isKeyDown(int key) const
{
    if (key < GLFW_KEY_LAST) {
        return isKeyPressed(key) || m_keys[key] == GLFW_REPEAT;
    }
    return false;
}

bool InputState::isButtonPressed(int button) const
{
    if (button < GLFW_MOUSE_BUTTON_LAST) {
        return m_mouse[button] == GLFW_PRESS;
    }
    return false;
}

bool InputState::isButtonDown(int button) const
{
    if (button < GLFW_MOUSE_BUTTON_LAST) {
        return isButtonPressed(button) || m_mouse[button] == GLFW_REPEAT;
    }
    return false;
}

SkPoint InputState::getCursor() const
{
    return m_cursor;
}

SkPoint InputState::getPreviousCursor() const
{
    return m_prevCursor;
}
