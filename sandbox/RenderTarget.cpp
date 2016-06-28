#include "RenderTarget.h"



RenderTarget::RenderTarget()
{
}

RenderTarget::RenderTarget(sk_sp<SkSurface> surface)
    : m_surface(surface)
{
}

RenderTarget::~RenderTarget()
{
}

SkCanvas * RenderTarget::getCanvas()
{
    return m_surface->getCanvas();
}

void RenderTarget::reset()
{
    m_surface.reset();
}
