#pragma once

#include <list>

#include <SkCanvas.h>

#include "InputState.h"

struct ViewProperties
{
    SkScalar width;
    SkScalar height;
    SkScalar x;
    SkScalar y;
    SkScalar z;

    ViewProperties()
        : x(0), y(0), z(0)
        , width(0), height(0)
    {
    }

    SkRect localRect() { return SkRect::MakeWH(width, height); }
    SkMatrix matrix()
    {
        SkMatrix m;
        m.setTranslate(x, y);
        return m;
    }
};

class View
{
    std::list<View*> m_children;
    View* m_parent;

    ViewProperties m_props;

    virtual void onDraw(SkCanvas& canvas) {}
    virtual bool onUpdate(const InputState& state) { return false; }
    virtual void onExit() {}

protected:
    void draw(SkCanvas& canvas);
    bool update(const InputState& state);
    void exit();

public:
    View();
    virtual ~View();

    void addView(View* view);
    void removeView(View* view);
    View* getParent();

    SkScalar x() { return m_props.x; }
    SkScalar y() { return m_props.y; }
    SkScalar z() { return m_props.z; }
    SkScalar width() { return m_props.width; }
    SkScalar height() { return m_props.height; }
    int widthI() { return SkScalarTruncToInt(width()); }
    int heightI() { return SkScalarTruncToInt(height()); }
    SkRect localRect() { return m_props.localRect(); }
    SkMatrix matrix() { return m_props.matrix(); }
    SkMatrix currentTransformMatrix(View* ancestor = nullptr);
    SkPoint convertToLocal(SkPoint point, View* reference = nullptr);
    bool containsPoint(SkPoint point, View* reference = nullptr);

    void setX(SkScalar value) { m_props.x = value; }
    void setY(SkScalar value) { m_props.y = value; }
    void setZ(SkScalar value) { m_props.z = value; }
    void setXY(SkScalar x, SkScalar y) { m_props.x = x; m_props.y = y; }
    void setXYZ(SkScalar x, SkScalar y, SkScalar z) { m_props.x = x; m_props.y = y; m_props.z = z; }
    void setWidth(SkScalar value) { m_props.width = value; }
    void setHeight(SkScalar value) { m_props.height = value; }
    void setWH(SkScalar width, SkScalar height) { m_props.width = width; m_props.height = height; }
};
