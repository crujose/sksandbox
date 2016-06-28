#pragma once

#include <GrContext.h>
#include <gl/GrGLInterface.h>

#include "RenderTarget.h"

class GraphicsContext
{
    SkAutoTUnref<GrContext> m_grctx;
public:
    GraphicsContext();
    ~GraphicsContext();

    void init();
    void reset();

    RenderTarget createDefaultTarget(int width, int height, int stencilBits);
    RenderTarget createRenderTarget(int width, int height);
};

