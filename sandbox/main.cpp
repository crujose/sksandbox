#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>

#include <SkPaint.h>
#include <SkCodec.h>

#include <GL\glew.h>

#include "Window.h"

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

class MyView : public View
{
    SkPoint m_pos;
    SkPoint m_prev;
    SkColor m_color;
    SkScalar m_size;

    void onDraw(SkCanvas& canvas) override
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(m_color);
        canvas.drawCircle(m_pos.x(), m_pos.y(), m_size, paint);

        canvas.drawLine(m_pos.x(), m_pos.y(), m_prev.x(), m_prev.y(), paint);

        paint.setStyle(SkPaint::kStroke_Style);
        canvas.drawRect(localRect(), paint);
    }

protected:
    bool onUpdate(const InputState& state) override
    {
        if (state.isButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
            return false;
        }
        SkPoint pos = state.getCursor();
        m_pos = convertToLocal(pos);
        m_prev = m_pos - (pos - state.getPreviousCursor());
        return true;
    }

public:
    MyView(SkColor color, SkScalar size)
        : m_color(color)
        , m_size(size)
    {
    }
};

class MovingView : public View
{
    bool m_dragging;
    SkPoint m_relPos;
    SkColor m_color;
    bool onUpdate(const InputState& state) override
    {
        if (state.isButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
            if (!m_dragging || state.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                m_relPos = convertToLocal(state.getCursor());
                m_dragging = true;
            }
            if (localRect().intersects(SkRect::MakeXYWH(m_relPos.x(), m_relPos.y(), 1, 1))) {
                SkPoint p = getParent()->convertToLocal(state.getCursor());
                p -= m_relPos;
                setXY(p.x(), p.y());
                return true;
            }
        }
        m_dragging = false;
        return false;
    }
    void onDraw(SkCanvas& canvas) override
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(m_color);
        paint.setAlpha(255);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas.drawRect(localRect(), paint);

        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(20);
        canvas.drawRect(localRect(), paint);
    }
public:
    MovingView(SkColor color = SkColorSetRGB(40, 140, 40))
        : m_dragging(false)
        , m_color(color)
    {
    }
};

#define SHADER_STR(s) #s

const char* vstxt = SHADER_STR(
    attribute vec2 vPos; \n
    uniform mat4 inv_mvp; \n
    varying vec3 tex_coord; \n
    void main() { \n
        gl_Position = vec4(vPos, 0, 1); \n
        tex_coord = (inv_mvp * gl_Position).xyz; \n
    }
);
const char* fstxt = SHADER_STR(
#ifdef GL_ES \n
    precision highp float; \n
#endif \n
    uniform samplerCube samp; \n
    varying vec3 tex_coord; \n
    void main() {
    \n
        gl_FragColor = textureCube(samp, tex_coord);
    }
);
GLuint getShader(const char* str, GLenum type)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &str, nullptr);
    glCompileShader(id);
    GLint res;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE) {
        GLint len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> log(len);
        glGetShaderInfoLog(id, len, &len, &log[0]);
        printf("Shader log: [%s]\n", &log[0]);
        glDeleteShader(id);
        id = 0;
    }
    return id;
}
sk_sp<SkImage> loadImage(const std::string& path)
{
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    return SkImage::MakeFromEncoded(encoded);
}

void rotateXY(GLfloat mat[16], GLfloat x, GLfloat y)
{
    const GLfloat cosX = cosf(x);
    const GLfloat sinX = sinf(x);
    const GLfloat cosY = cosf(y);
    const GLfloat sinY = sinf(y);
    memset(mat, 0, sizeof(mat) * 16);
    mat[0] = cosY;
    mat[2] = -sinY;

    mat[4] = -sinX * sinY;
    mat[5] = cosX;
    mat[6] = -sinX * cosY;

    mat[8] = cosX * sinY;
    mat[9] = sinX;
    mat[10] = cosX * cosY;

    mat[15] = 1;
}

void perspectiveMatrixInverse(GLfloat mat[16], GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f)
{
    GLfloat h = tanf(0.5f * fov);
    GLfloat w = h * aspect;
    GLfloat z0 = (f - n) / (-2 * f * n);
    GLfloat z1 = (f + n) / (-2 * f * n);
    memset(mat, 0, sizeof(mat) * 16);
    mat[0] = w;

    mat[5] = h;

    mat[11] = z0;

    mat[14] = 1;
    mat[15] = z1;
}

void multiply(GLfloat r[16], GLfloat a[16], GLfloat b[16])
{
    memset(r, 0, sizeof(r) * 16);
    r[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
    r[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
    r[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
    r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

    r[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
    r[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
    r[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
    r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

    r[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
    r[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
    r[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
    r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

    r[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
    r[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
    r[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
    r[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
}


void checkGlError(const char* fn, int ln)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("glGetError %s %d %x\n", fn, ln, err);
    }
}
#define CHECK_ERROR() checkGlError(__FUNCTION__, __LINE__)

class GlView : public View
{
    GLuint m_program;
    GLuint m_posBuffer;

    GLint m_inv_mvp;
    GLint m_sampler;
    GLint m_cubemap;
    GLint m_vPos;

    sk_sp<SkSurface> m_surface;
    GLuint m_fb;

    GLfloat m_fov;
    GLfloat m_angleX;
    GLfloat m_angleY;
    SkScalar m_alpha;
    bool m_drag;

    std::string m_path;

    GLuint loadCubeMap()
    {
        GLuint texture;
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::unordered_map<std::string, GLenum> faces = {
            { "/posx.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X },
            { "/negx.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X },
            { "/posy.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y },
            { "/negy.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y },
            { "/posz.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z },
            { "/negz.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z },
        };
        for (auto f : faces) {
            sk_sp<SkImage> img(loadImage(m_path + f.first));
            if (!img) {
                printf("Failed to load %s\n", f.first.c_str());
                continue;
            }

            SkBitmap bm;
            bm.allocN32Pixels(img->width(), img->height());

            SkPixmap pixmap;
            bm.peekPixels(&pixmap);
            if (!img->readPixels(pixmap, 0, 0)) {
                printf("Failed to peek pixels for %s\n", f.first.c_str());
                continue;
            }
            glTexImage2D(f.second, 0, GL_RGBA, img->width(), img->height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, pixmap.addr());
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        return texture;
    }

    GLuint getProgram()
    {
        GLuint progId;
        GLuint vId = getShader(vstxt, GL_VERTEX_SHADER);
        GLuint fId = getShader(fstxt, GL_FRAGMENT_SHADER);
        if (vId && fId) {
            progId = glCreateProgram();
            glAttachShader(progId, vId);
            glAttachShader(progId, fId);
            glLinkProgram(progId);
            glDeleteShader(vId);
            glDeleteShader(fId);
        } else {
            return 0;
        }
        glUseProgram(progId);
        m_vPos = glGetAttribLocation(progId, "vPos");
        m_inv_mvp = glGetUniformLocation(progId, "inv_mvp");
        m_sampler = glGetUniformLocation(progId, "samp");

        glCreateBuffers(1, &m_posBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);

        GLfloat verts[] = { -1, -1,  3, -1,  -1, 3 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        return progId;
    }
    void onDraw(SkCanvas& canvas) override
    {
        if (!canvas.getGrContext()) {
            return;
        }
        canvas.flush();
        if (!m_program) {
            m_program = getProgram();
            m_cubemap = loadCubeMap();
            CHECK_ERROR();

            m_surface = SkSurface::MakeRenderTarget(canvas.getGrContext(), SkBudgeted::kYes, SkImageInfo::MakeN32Premul(SkScalarTruncToInt(width()), SkScalarTruncToInt(height())));
            GrBackendObject obj;
            m_surface->getRenderTargetHandle(&obj, SkSurface::BackendHandleAccess::kFlushWrite_BackendHandleAccess);
            m_fb = obj;
        }

        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
        glViewport(0, 0, width(), height());

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_program);

        glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);
        glEnableVertexAttribArray(m_vPos);
        glVertexAttribPointer(m_vPos, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);

        GLfloat v[16];
        GLfloat p[16];
        GLfloat mat[16];
        rotateXY(v, m_angleX, m_angleY);
        perspectiveMatrixInverse(p, m_fov, width() / height(), 0.01f, 100.0f);
        multiply(mat, p, v);

        glUniformMatrix4fv(m_inv_mvp, 1, false, mat);
        glUniform1i(m_sampler, 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        canvas.getGrContext()->resetContext();
        SkPaint paint;
        paint.setAlpha(m_alpha);
        m_surface->draw(&canvas, 0, 0, &paint);
    }

    bool onUpdate(const InputState& state) override
    {
        bool consumed = false;
        if (state.isKeyDown(GLFW_KEY_W)) {
            m_fov *= std::expf(-0.3f);
            consumed = true;
        }
        else if (state.isKeyDown(GLFW_KEY_S)) {
            m_fov *= std::expf(0.3f);
            consumed = true;
        }
        if (state.isKeyDown(GLFW_KEY_A)) {
            m_alpha -= 2;
            consumed = true;
        } else if (state.isKeyDown(GLFW_KEY_D)) {
            m_alpha += 2;
            consumed = true;
        }
        m_alpha = std::max(std::min(m_alpha, 255.f), 0.f);
        m_fov = std::max(std::min(m_fov, 2.1f), 0.1f);
        if (!state.isButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
            return consumed;
        } else if (state.isKeyDown(GLFW_KEY_LEFT_CONTROL)) {
            return false;
        }
        SkPoint pos = state.getCursor();
        SkPoint prevPos = state.getPreviousCursor();
        m_angleX += (pos.y() - prevPos.y()) * 0.006f;
        m_angleY -= (pos.x() - prevPos.x()) * 0.006f;
        m_angleX = std::max(std::min(m_angleX, 3.1415f / 2), -3.1415f / 2);
        return true;
    }

    void onExit() override
    {
        m_surface.reset();
        glDeleteProgram(m_program);
        glDeleteBuffers(1, &m_posBuffer);
    }
public:
    GlView(const std::string& path)
        : m_inv_mvp(0)
        , m_sampler(0)
        , m_cubemap(0)
        , m_program(0)
        , m_fov(1.5)
        , m_angleX(0)
        , m_angleY(0)
        , m_path(path)
        , m_alpha(255)
    {
    }
};

void showWin()
{
    MovingView root;
    root.setWH(500, 400);
    root.setZ(10);

    MyView v(SK_ColorRED, 4);
    v.setWH(250, 200);
    v.setXY(10, 10);
    root.addView(&v);

    MyView v2(SK_ColorWHITE, 6);
    v2.setWH(120, 120);
    v2.setXYZ(30, 30, 20);

    MovingView mv(SK_ColorMAGENTA);
    mv.setWH(90, 150);
    mv.setXYZ(50, 10, 50);
    v.addView(&mv);
    v.addView(&v2);

    MyView v3(SK_ColorCYAN, 12);
    v3.setWH(90, 150);
    mv.addView(&v3);


    GlView glview("cubemap/yokohama");
    glview.setWH(350, 200);
    MovingView glViewContainer;
    glViewContainer.setXY(150, 200);
    glViewContainer.setWH(350, 200);
    glViewContainer.addView(&glview);
    root.addView(&glViewContainer);

    GlView gv("cubemap/yokohama3");
    gv.setXY(0, 0);
    gv.setWH(500, 400);

    Window win(640, 480, "sandbox");
    win.addView(&gv);
    win.addView(&root);
    win.show();
}

int main(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    showWin();

    glfwTerminate();
    return 0;
}