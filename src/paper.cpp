// ============================================================
// paper.cpp  -  shared paper-cut drawing toolkit
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <cstring>

#include "../include/paper.h"

namespace pl
{
// ------------------------------------------------------------
// color helpers
// ------------------------------------------------------------
Col mix(const Col& a, const Col& b, float t)
{
    return Col(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
               a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t);
}
Col shade(const Col& c, float f) { return Col(c.r * f, c.g * f, c.b * f, c.a); }
Col withA(const Col& c, float a) { return Col(c.r, c.g, c.b, a); }

// ------------------------------------------------------------
// math / easing
// ------------------------------------------------------------
float clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
float lerp(float a, float b, float t) { return a + (b - a) * t; }
float smooth(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }
float easeOutCubic(float t) { t = clamp01(t); float u = 1.0f - t; return 1.0f - u * u * u; }
float easeInOutCubic(float t)
{
    t = clamp01(t);
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}
float easeOutBack(float t)
{
    t = clamp01(t);
    const float c1 = 1.70158f, c3 = c1 + 1.0f;
    float u = t - 1.0f;
    return 1.0f + c3 * u * u * u + c1 * u * u;
}
float stage(float p, float start, float end)
{
    if (end <= start) return p >= end ? 1.0f : 0.0f;
    return smooth((p - start) / (end - start));
}
float window(float p, float start, float end)
{
    if (end <= start) return p >= end ? 1.0f : 0.0f;
    return clamp01((p - start) / (end - start));
}
float stagger(float p, int i, int n, float spread)
{
    if (n <= 1) return clamp01(p);
    float s = spread * (float)i / (float)(n - 1);
    return clamp01((p - s) / (1.0f - spread));
}

// ------------------------------------------------------------
// random (xorshift, deterministic)
// ------------------------------------------------------------
static unsigned g_rng = 2463534242u;
void seedRandom(unsigned s) { g_rng = s ? s : 1u; }
float rnd()
{
    g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5;
    return (g_rng & 0xFFFFFF) / 16777216.0f;
}
float rnd(float a, float b) { return a + (b - a) * rnd(); }
int rndi(int n) { int v = (int)(rnd() * n); return v >= n ? n - 1 : v; }

// ------------------------------------------------------------
// time
// ------------------------------------------------------------
static float g_dt = 0.0f, g_now = 0.0f, g_fixed = -1.0f;
static int   g_last = -1;
static bool  g_paused = false;

void tick()
{
    int t = glutGet(GLUT_ELAPSED_TIME);
    float d;
    if (g_fixed > 0.0f) d = g_fixed;
    else
    {
        d = (g_last < 0) ? 0.016f : (t - g_last) / 1000.0f;
        if (d > 0.05f) d = 0.05f;
        if (d < 0.0f) d = 0.0f;
    }
    g_last = t;
    g_dt = g_paused ? 0.0f : d;
    g_now += g_dt;
}
float dt() { return g_dt; }
float now() { return g_now; }
void setPaused(bool p) { g_paused = p; }
bool paused() { return g_paused; }
void setFixedDt(float d) { g_fixed = d; }

// ------------------------------------------------------------
// window / mouse
// ------------------------------------------------------------
static int g_w = 1000, g_h = 650;
static float g_mx = -1000.0f, g_my = -1000.0f;
void setWindowSize(int w, int h) { g_w = w > 1 ? w : 1; g_h = h > 1 ? h : 1; }
int winW() { return g_w; }
int winH() { return g_h; }
void setMouse(float x, float y) { g_mx = x; g_my = y; }
float mouseX() { return g_mx; }
float mouseY() { return g_my; }

void scissor(float x1, float y1, float x2, float y2)
{
    float sx = g_w / 1000.0f, sy = g_h / 650.0f;
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)std::floor(x1 * sx), (int)std::floor(y1 * sy),
              (int)std::ceil((x2 - x1) * sx), (int)std::ceil((y2 - y1) * sy));
}
void scissorOff() { glDisable(GL_SCISSOR_TEST); }

// ------------------------------------------------------------
// paper style state
// ------------------------------------------------------------
static float g_shDx = 0, g_shDy = 0, g_shA = 0;
static bool  g_edge = false;

void paperShadow(float dx, float dy, float alpha) { g_shDx = dx; g_shDy = dy; g_shA = alpha; }
void noShadow() { g_shA = 0.0f; }
void paperEdge(bool on) { g_edge = on; }

// ------------------------------------------------------------
// core polygon emitter
// ------------------------------------------------------------
static void emitFan(const float* p, int n, const Col& c)
{
    float cx = 0, cy = 0;
    for (int i = 0; i < n; i++) { cx += p[2 * i]; cy += p[2 * i + 1]; }
    cx /= n; cy /= n;
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i < n; i++) glVertex2f(p[2 * i], p[2 * i + 1]);
    glVertex2f(p[0], p[1]);
    glEnd();
}

static void fillPoly(const float* p, int n, const Col& c)
{
    if (n < 3 || c.a <= 0.001f) return;
    if (g_shA > 0.0f && c.a > 0.5f)
    {
        glPushMatrix();
        glTranslatef(g_shDx, g_shDy, 0);
        emitFan(p, n, Col(0.08f, 0.05f, 0.02f, g_shA * c.a));
        glPopMatrix();
    }
    emitFan(p, n, c);
    if (g_edge && c.a > 0.5f)
    {
        Col e = shade(c, 0.70f);
        glColor4f(e.r, e.g, e.b, 0.85f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < n; i++) glVertex2f(p[2 * i], p[2 * i + 1]);
        glEnd();
    }
}

// ------------------------------------------------------------
// shapes
// ------------------------------------------------------------
void poly(const float* xy, int n, const Col& c) { fillPoly(xy, n, c); }

void rect(float x1, float y1, float x2, float y2, const Col& c)
{
    float p[8] = { x1, y1, x2, y1, x2, y2, x1, y2 };
    fillPoly(p, 4, c);
}

void rectGrad(float x1, float y1, float x2, float y2, const Col& b, const Col& t)
{
    glBegin(GL_QUADS);
    glColor4f(b.r, b.g, b.b, b.a); glVertex2f(x1, y1); glVertex2f(x2, y1);
    glColor4f(t.r, t.g, t.b, t.a); glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

void rectGradH(float x1, float y1, float x2, float y2, const Col& l, const Col& r)
{
    glBegin(GL_QUADS);
    glColor4f(l.r, l.g, l.b, l.a); glVertex2f(x1, y1); glVertex2f(x1, y2);
    glColor4f(r.r, r.g, r.b, r.a); glVertex2f(x2, y2); glVertex2f(x2, y1);
    glEnd();
}

void roundRect(float x1, float y1, float x2, float y2, float rad, const Col& c)
{
    float maxr = 0.5f * ((x2 - x1) < (y2 - y1) ? (x2 - x1) : (y2 - y1));
    if (rad > maxr) rad = maxr;
    if (rad < 0.5f) { rect(x1, y1, x2, y2, c); return; }
    const int S = 6;
    float p[(4 * (S + 1)) * 2];
    int k = 0;
    float cx[4] = { x2 - rad, x1 + rad, x1 + rad, x2 - rad };
    float cy[4] = { y2 - rad, y2 - rad, y1 + rad, y1 + rad };
    for (int q = 0; q < 4; q++)
        for (int i = 0; i <= S; i++)
        {
            float a = (q * 90.0f + 90.0f * i / S) * PI / 180.0f;
            p[k++] = cx[q] + std::cos(a) * rad;
            p[k++] = cy[q] + std::sin(a) * rad;
        }
    fillPoly(p, 4 * (S + 1), c);
}

void tri(float x1, float y1, float x2, float y2, float x3, float y3, const Col& c)
{
    float p[6] = { x1, y1, x2, y2, x3, y3 };
    fillPoly(p, 3, c);
}

void ellipse(float x, float y, float rx, float ry, const Col& c)
{
    float big = rx > ry ? rx : ry;
    int seg = (int)clampf(14.0f + big * 0.35f, 14.0f, 56.0f);
    float p[2 * 64];
    for (int i = 0; i < seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        p[2 * i] = x + std::cos(a) * rx;
        p[2 * i + 1] = y + std::sin(a) * ry;
    }
    fillPoly(p, seg, c);
}
void circle(float x, float y, float r, const Col& c) { ellipse(x, y, r, r, c); }

void dome(float x, float y, float rx, float ry, const Col& c)
{
    const int seg = 28;
    float p[2 * (seg + 2)];
    int k = 0;
    for (int i = 0; i <= seg; i++)
    {
        float a = PI * i / seg;
        p[k++] = x + std::cos(a) * rx;
        p[k++] = y + std::sin(a) * ry;
    }
    fillPoly(p, seg + 1, c);
}

void ring(float x, float y, float r1, float r2, const Col& c)
{
    if (c.a <= 0.001f) return;
    int seg = (int)clampf(16.0f + r2 * 0.4f, 16.0f, 64.0f);
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(x + std::cos(a) * r1, y + std::sin(a) * r1);
        glVertex2f(x + std::cos(a) * r2, y + std::sin(a) * r2);
    }
    glEnd();
}

void line(float x1, float y1, float x2, float y2, float w, const Col& c)
{
    float dx = x2 - x1, dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;
    float nx = -dy / len * w * 0.5f, ny = dx / len * w * 0.5f;
    float p[8] = { x1 + nx, y1 + ny, x2 + nx, y2 + ny,
                   x2 - nx, y2 - ny, x1 - nx, y1 - ny };
    // lines never get shadows/edges: draw directly
    emitFan(p, 4, c);
}

void ribbonTaper(const float* xy, int n, float w0, float w1, const Col& c)
{
    if (n < 2 || c.a <= 0.001f) return;
    // build outline left side forward, right side backward
    std::vector<float> L, R;
    for (int i = 0; i < n; i++)
    {
        int a = i > 0 ? i - 1 : 0, b = i < n - 1 ? i + 1 : n - 1;
        float dx = xy[2 * b] - xy[2 * a], dy = xy[2 * b + 1] - xy[2 * a + 1];
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.0001f) { dx = 1; dy = 0; len = 1; }
        float t = (float)i / (float)(n - 1);
        float w = (w0 + (w1 - w0) * t) * 0.5f;
        float nx = -dy / len * w, ny = dx / len * w;
        L.push_back(xy[2 * i] + nx); L.push_back(xy[2 * i + 1] + ny);
        R.push_back(xy[2 * i] - nx); R.push_back(xy[2 * i + 1] - ny);
    }
    // draw as quad strip (works for concave curves)
    auto strip = [&](float ox, float oy, const Col& col)
    {
        glColor4f(col.r, col.g, col.b, col.a);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i < n; i++)
        {
            glVertex2f(L[2 * i] + ox, L[2 * i + 1] + oy);
            glVertex2f(R[2 * i] + ox, R[2 * i + 1] + oy);
        }
        glEnd();
    };
    if (g_shA > 0.0f && c.a > 0.5f)
        strip(g_shDx, g_shDy, Col(0.08f, 0.05f, 0.02f, g_shA * c.a));
    strip(0, 0, c);
}
void ribbon(const float* xy, int n, float w, const Col& c) { ribbonTaper(xy, n, w, w, c); }

void star(float x, float y, float r1, float r2, int points, float rot, const Col& c)
{
    float p[2 * 24];
    int n = points * 2;
    if (n > 24) n = 24;
    for (int i = 0; i < n; i++)
    {
        float a = rot + PI * i / points - PI / 2;
        float r = (i % 2 == 0) ? r1 : r2;
        p[2 * i] = x + std::cos(a) * r;
        p[2 * i + 1] = y + std::sin(a) * r;
    }
    fillPoly(p, n, c);
}

void leafShape(float x, float y, float len, float wid, float angleDeg, const Col& c)
{
    const int S = 8;
    float p[2 * (2 * S + 2)];
    int k = 0;
    float a = angleDeg * PI / 180.0f;
    float ca = std::cos(a), sa = std::sin(a);
    for (int i = 0; i <= S; i++)
    {
        float t = (float)i / S;
        float lx = t * len;
        float ly = std::sin(t * PI) * wid * 0.5f;
        p[k++] = x + lx * ca - ly * sa;
        p[k++] = y + lx * sa + ly * ca;
    }
    for (int i = S - 1; i >= 1; i--)
    {
        float t = (float)i / S;
        float lx = t * len;
        float ly = -std::sin(t * PI) * wid * 0.5f;
        p[k++] = x + lx * ca - ly * sa;
        p[k++] = y + lx * sa + ly * ca;
    }
    fillPoly(p, k / 2, c);
}

void glow(float x, float y, float r, const Col& c, bool additive)
{
    if (c.a <= 0.002f || r <= 0.5f) return;
    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    const int seg = 28;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(c.r, c.g, c.b, c.a);
    glVertex2f(x, y);
    glColor4f(c.r, c.g, c.b, 0.0f);
    for (int i = 0; i <= seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(x + std::cos(a) * r, y + std::sin(a) * r);
    }
    glEnd();
    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void sparkle4(float x, float y, float r, float rot, const Col& c)
{
    if (c.a <= 0.002f) return;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float p[16];
    for (int i = 0; i < 8; i++)
    {
        float a = rot + PI * i / 4.0f;
        float rr = (i % 2 == 0) ? r : r * 0.22f;
        p[2 * i] = x + std::cos(a) * rr;
        p[2 * i + 1] = y + std::sin(a) * rr;
    }
    emitFan(p, 8, c);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void overlay(const Col& c)
{
    if (c.a <= 0.001f) return;
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(1000, 0); glVertex2f(1000, 650); glVertex2f(0, 650);
    glEnd();
}

// ------------------------------------------------------------
// text
// ------------------------------------------------------------
void text(float x, float y, const std::string& s, void* font, const Col& c)
{
    glColor4f(c.r, c.g, c.b, c.a);
    glRasterPos2f(x, y);
    for (size_t i = 0; i < s.size(); i++) glutBitmapCharacter(font, (unsigned char)s[i]);
}
int textWidth(const std::string& s, void* font)
{
    return glutBitmapLength(font, (const unsigned char*)s.c_str());
}
void textCentered(float cx, float y, const std::string& s, void* font, const Col& c)
{
    text(cx - textWidth(s, font) * 0.5f, y, s, font, c);
}
std::vector<std::string> wrapText(const std::string& s, void* font, int maxWidth)
{
    std::vector<std::string> out;
    std::string cur, word;
    auto flushWord = [&]()
    {
        if (word.empty()) return;
        std::string test = cur.empty() ? word : cur + " " + word;
        if (!cur.empty() && textWidth(test, font) > maxWidth)
        {
            out.push_back(cur);
            cur = word;
        }
        else cur = test;
        word.clear();
    };
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ') flushWord();
        else word += s[i];
    }
    flushWord();
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// ------------------------------------------------------------
// UI helpers
// ------------------------------------------------------------
void panel(float x1, float y1, float x2, float y2, const Col& fill, const Col& border)
{
    paperShadow(3, -3, 0.28f);
    roundRect(x1, y1, x2, y2, 9, fill);
    noShadow();
    // border
    glColor4f(border.r, border.g, border.b, border.a);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    float rad = 9;
    const int S = 5;
    float cx[4] = { x2 - rad, x1 + rad, x1 + rad, x2 - rad };
    float cy[4] = { y2 - rad, y2 - rad, y1 + rad, y1 + rad };
    for (int q = 0; q < 4; q++)
        for (int i = 0; i <= S; i++)
        {
            float a = (q * 90.0f + 90.0f * i / S) * PI / 180.0f;
            glVertex2f(cx[q] + std::cos(a) * rad, cy[q] + std::sin(a) * rad);
        }
    glEnd();
}
bool inRect(float px, float py, float x1, float y1, float x2, float y2)
{
    return px >= x1 && px <= x2 && py >= y1 && py <= y2;
}
bool inEllipse(float px, float py, float cx, float cy, float rx, float ry)
{
    float dx = (px - cx) / rx, dy = (py - cy) / ry;
    return dx * dx + dy * dy <= 1.0f;
}

// ------------------------------------------------------------
// pop-up helpers
// ------------------------------------------------------------
bool beginPopUp(float x, float baseY, float amount, float tiltDeg)
{
    if (amount <= 0.001f) return false;
    float a = easeOutBack(clamp01(amount));
    glPushMatrix();
    glTranslatef(x, baseY, 0);
    if (amount < 1.0f && tiltDeg != 0.0f)
        glRotatef(tiltDeg * (1.0f - clamp01(amount)), 0, 0, 1);
    glScalef(0.75f + 0.25f * clamp01(amount * 1.5f), a, 1.0f);
    return true;
}
void endPopUp() { glPopMatrix(); }

void creaseShadow(float x, float baseY, float halfWidth, float amount)
{
    float a = clamp01(amount);
    if (a <= 0.01f) return;
    ellipse(x, baseY - 1, halfWidth * (0.6f + 0.4f * a), 4.0f + 3.0f * a,
            Col(0.05f, 0.12f, 0.05f, 0.22f * a));
}

} // namespace pl
