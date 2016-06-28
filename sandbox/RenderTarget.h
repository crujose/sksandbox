#pragma once

#include <SkSurface.h>

class RenderTarget
{
    sk_sp<SkSurface> m_surface;

public:
    RenderTarget();
    RenderTarget(sk_sp<SkSurface> surface);
    ~RenderTarget();

    SkCanvas* getCanvas();
    void reset();
};

