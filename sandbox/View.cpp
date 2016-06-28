#include "View.h"

View::View()
{
}

View::~View()
{
    auto copy = m_children;
    for (View* v : copy) {
        removeView(v);
    }
}

void View::addView(View* view)
{
    if (!view || view->m_parent) {
        return;
    }
    auto it = std::find(m_children.begin(), m_children.end(), view);
    if (it != m_children.end()) {
        return;
    }
    m_children.emplace_back(view);
    view->m_parent = this;
}

void View::removeView(View* view)
{
    if (!view || view->m_parent != this) {
        return;
    }
    m_children.remove(view);
    view->m_parent = nullptr;
}

View * View::getParent()
{
    return m_parent;
}

SkMatrix View::currentTransformMatrix(View* ancestor)
{
    SkMatrix m;
    m.reset();
    View* v = this;
    while (v && v != ancestor) {
        m.postConcat(v->matrix());
        v = v->m_parent;
    }
    return m;
}

SkPoint View::convertToLocal(SkPoint point, View* reference)
{
    SkMatrix m;
    if (currentTransformMatrix(reference).invert(&m)) {
        m.mapPoints(&point, 1);
    }
    return point;
}

bool View::containsPoint(SkPoint point, View * reference)
{
    SkPoint p = convertToLocal(point, reference);
    return localRect().intersects(p.x(), p.y(), p.x() + 1, p.y() + 1);
}

void View::draw(SkCanvas & canvas)
{
    SkAutoCanvasRestore restore(&canvas, true);
    canvas.concat(m_props.matrix());
    canvas.clipRect(m_props.localRect(), SkRegion::kIntersect_Op, true);

    onDraw(canvas);

    for (View* v : m_children) {
        v->draw(canvas);
    }
}

bool View::update(const InputState & state)
{
    std::list<View*> copy = m_children;
    copy.sort([](View* v1, View* v2) { return v1->z() > v2->z(); });
    for (View* v : copy) {
        if (v->containsPoint(state.getCursor())) {
            if (v->update(state)) {
                return true;
            }
        }
    }
    return onUpdate(state);
}

void View::exit()
{
    onExit();
    for (View* v : m_children) {
        v->exit();
    }
}
