#include "GraphicsContext.h"

GraphicsContext::GraphicsContext()
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::init()
{
    SkAutoTUnref<const GrGLInterface> glinterface(GrGLCreateNativeInterface());
    m_grctx.reset(GrContext::Create(GrBackend::kOpenGL_GrBackend, (GrBackendContext)glinterface.get()));
}

void GraphicsContext::reset()
{
    m_grctx.reset();
}

RenderTarget GraphicsContext::createDefaultTarget(int width, int height, int stencilBits)
{
    GrBackendRenderTargetDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = GrPixelConfig::kRGBA_8888_GrPixelConfig;
    desc.fStencilBits = stencilBits;
    return RenderTarget(SkSurface::MakeFromBackendRenderTarget(m_grctx.get(), desc, nullptr));
}

RenderTarget GraphicsContext::createRenderTarget(int width, int height)
{
    return RenderTarget(SkSurface::MakeRenderTarget(m_grctx.get(), SkBudgeted::kYes, SkImageInfo::MakeN32Premul(width, height)));
}
