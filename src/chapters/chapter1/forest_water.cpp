// ============================================================
// forest_water.cpp - Chapter 1 water zone: waterfall, river, bridge,
//                    pond, and the village path stones
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

#include "chapters/chapter1/forest_environment.h"
#include "chapters/chapter1/forest_environment_internal.h"
#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_layout.h"

using namespace pl;

namespace env
{
// ------------------------------------------------------------
// ZONE 2 - waterfall cliff + waterfall
// ------------------------------------------------------------
void drawWaterfallCliff(float a)
{
    if (a <= 0.0f) return;
    if (!beginPopUp(layout::WF_X, layout::HORIZON - 6, a, 0)) return;
    float ox = -layout::WF_X, oy = -(layout::HORIZON - 6);   // draw in absolute coords
    glTranslatef(ox, oy, 0);
    paperShadow(4, -3, 0.25f);
    float back[] = { 428, 288, 440, 340, 462, 380, 538, 384, 562, 340, 574, 288 };
    poly(back, 6, Col(0.58f, 0.54f, 0.50f));
    noShadow();
    float front[] = { 438, 288, 450, 336, 470, 372, 530, 374, 552, 336, 562, 288 };
    poly(front, 6, Col(0.68f, 0.63f, 0.57f));
    // rock facets
    tri(438, 288, 470, 372, 470, 288, Col(0.50f, 0.46f, 0.43f, 0.5f));
    tri(562, 288, 530, 374, 530, 288, Col(0.50f, 0.46f, 0.43f, 0.45f));
    // mossy top
    paperShadow(2, -2, 0.2f);
    dome(500, 368, 52, 20, Col(0.42f, 0.72f, 0.40f));
    noShadow();
    glPopMatrix();
}

void sparkleWaterfall() { g_wfBoost = 1.4f; fx::waterSparkle(); }
float waterfallBoost() { return g_wfBoost; }

void drawWaterfall(float a, float boost)
{
    if (a <= 0.0f) return;
    float t = now();
    float topY = layout::WF_TOP - 4;
    float botY = topY - (topY - layout::WF_BASE) * smooth(a);
    float x = layout::WF_X;
    float b = clamp01(boost);

    // water sheet
    rectGradH(x - 19, botY, x + 19, topY, Col(0.58f, 0.82f, 0.96f, 0.95f), Col(0.72f, 0.92f, 1.0f, 0.95f));
    // lip
    ellipse(x, topY, 22, 5, Col(0.80f, 0.95f, 1.0f));
    // moving streaks
    for (int k = 0; k < 9; k++)
    {
        float sx = x - 15 + (k % 5) * 7.5f + (k / 5) * 3.5f;
        float off = std::fmod(t * (70.0f + k * 9.0f) + k * 17.0f, 80.0f);
        float y1 = topY - off, y2 = y1 - 18;
        if (y2 < botY) y2 = botY;
        if (y1 < botY) continue;
        line(sx, y1, sx, y2, 2.2f, Col(1, 1, 1, 0.75f));
    }
    // foam at the base
    if (a > 0.7f)
    {
        float pulse = 1.0f + 0.08f * std::sin(t * 6.0f);
        ellipse(x, layout::WF_BASE, 30 * pulse, 8, Col(0.92f, 0.98f, 1.0f, 0.9f));
        ellipse(x - 14, layout::WF_BASE + 2, 8, 4, Col(1, 1, 1, 0.9f));
        ellipse(x + 14, layout::WF_BASE + 1, 9, 4, Col(1, 1, 1, 0.9f));
    }
    if (b > 0.0f)
    {
        glow(x, (topY + layout::WF_BASE) * 0.5f, 62.0f, Col(0.7f, 0.95f, 1.0f, 0.55f * b));
        // twinkles that run down the fall
        for (int k = 0; k < 6; k++)
        {
            float yy = topY - std::fmod(t * 110.0f + k * 31.0f, topY - layout::WF_BASE);
            sparkle4(x + std::sin(t * 4 + k) * 12, yy, 6 + 3 * b, t * 3 + k, Col(1, 1, 1, 0.9f * b));
        }
    }
}

// ------------------------------------------------------------
// ZONE 2 - river
// ------------------------------------------------------------
static std::vector<float> g_river;                // smoothed centerline
static std::vector<float> g_riverW;
static void buildRiver()
{
    if (!g_river.empty()) return;
    const float cp[][2] = {
        { 500, 300 }, { 498, 296 }, { 490, 280 }, { 507, 262 }, { 497, 243 },
        { 505, 226 }, { 496, 208 }, { 501, 190 }, { 500, 172 }, { 500, 168 } };
    int n = (int)(sizeof(cp) / sizeof(cp[0]));
    for (int i = 0; i < n - 1; i++)
    {
        const float* p0 = cp[i > 0 ? i - 1 : 0];
        const float* p1 = cp[i];
        const float* p2 = cp[i + 1];
        const float* p3 = cp[i + 2 < n ? i + 2 : n - 1];
        for (int s = 0; s < 8; s++)
        {
            float u = s / 8.0f, u2 = u * u, u3 = u2 * u;
            for (int d = 0; d < 2; d++)
            {
                float v = 0.5f * ((2 * p1[d]) + (-p0[d] + p2[d]) * u +
                                  (2 * p0[d] - 5 * p1[d] + 4 * p2[d] - p3[d]) * u2 +
                                  (-p0[d] + 3 * p1[d] - 3 * p2[d] + p3[d]) * u3);
                g_river.push_back(v);
            }
        }
    }
    g_river.push_back(cp[n - 1][0]);
    g_river.push_back(cp[n - 1][1]);
    int pts = (int)g_river.size() / 2;
    for (int i = 0; i < pts; i++)
    {
        float t = (float)i / (pts - 1);
        g_riverW.push_back(lerp(26.0f, 66.0f, std::pow(t, 1.15f)));
    }
}

static void riverPoint(float t, float& x, float& y, float& w)
{
    buildRiver();
    int pts = (int)g_riverW.size();
    float f = clamp01(t) * (pts - 1);
    int i = (int)f;
    if (i >= pts - 1) i = pts - 2;
    float u = f - i;
    x = lerp(g_river[2 * i], g_river[2 * i + 2], u);
    y = lerp(g_river[2 * i + 1], g_river[2 * i + 3], u);
    w = lerp(g_riverW[i], g_riverW[i + 1], u);
}

static void riverStrip(int upTo, float wAdd, const Col& cL, const Col& cR, float ox, float oy)
{
    glBegin(GL_QUAD_STRIP);
    int pts = (int)g_riverW.size();
    for (int i = 0; i <= upTo && i < pts; i++)
    {
        int a = i > 0 ? i - 1 : 0, b = i < pts - 1 ? i + 1 : pts - 1;
        float dx = g_river[2 * b] - g_river[2 * a], dy = g_river[2 * b + 1] - g_river[2 * a + 1];
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) { dx = 0; dy = -1; len = 1; }
        float nx = -dy / len, ny = dx / len;
        float w = (g_riverW[i] + wAdd) * 0.5f;
        glColor4f(cL.r, cL.g, cL.b, cL.a);
        glVertex2f(g_river[2 * i] + nx * w + ox, g_river[2 * i + 1] + ny * w + oy);
        glColor4f(cR.r, cR.g, cR.b, cR.a);
        glVertex2f(g_river[2 * i] - nx * w + ox, g_river[2 * i + 1] - ny * w + oy);
    }
    glEnd();
}

void drawRiver(float a)
{
    if (a <= 0.0f) return;
    buildRiver();
    float t = now();
    int pts = (int)g_riverW.size();
    int upTo = (int)(smooth(clamp01(a * 1.15f)) * (pts - 1));
    if (upTo < 2) return;

    // soft shadow + bank + water
    riverStrip(upTo, 12.0f, Col(0.1f, 0.06f, 0.02f, 0.18f), Col(0.1f, 0.06f, 0.02f, 0.18f), 3, -3);
    riverStrip(upTo, 10.0f, Col(0.55f, 0.42f, 0.28f), Col(0.55f, 0.42f, 0.28f), 0, 0);
    riverStrip(upTo, 0.0f, Col(0.42f, 0.74f, 0.92f), Col(0.34f, 0.64f, 0.88f), 0, 0);

    // flowing highlight streaks
    for (int k = 0; k < 9; k++)
    {
        float s = std::fmod(t * 0.10f + k / 9.0f, 1.0f);
        float lat = ((k * 37) % 11 - 5) / 5.0f;
        std::vector<float> seg;
        for (int j = 0; j < 5; j++)
        {
            float u = clamp01(s + j * 0.014f);
            float x, y, w;
            riverPoint(u, x, y, w);
            // offset sideways
            float x2, y2, w2;
            riverPoint(clamp01(u + 0.01f), x2, y2, w2);
            float dx = x2 - x, dy = y2 - y, len = std::sqrt(dx * dx + dy * dy) + 0.0001f;
            float nx = -dy / len, ny = dx / len;
            seg.push_back(x + nx * lat * w * 0.32f);
            seg.push_back(y + ny * lat * w * 0.32f);
        }
        float uMid = s + 0.03f;
        if ((int)(uMid * (pts - 1)) > upTo) continue;
        ribbon(seg.data(), 5, 2.2f, Col(1, 1, 1, 0.55f));
    }

    // floating leaves riding the current
    for (int k = 0; k < 5; k++)
    {
        float s = std::fmod(t * 0.045f + k * 0.21f, 1.0f);
        if ((int)(s * (pts - 1)) > upTo) continue;
        float x, y, w;
        riverPoint(s, x, y, w);
        float lat = std::sin(k * 2.3f) * w * 0.25f;
        float bob = std::sin(t * 2.5f + k) * 1.5f;
        static const Col lc[3] = { Col(0.95f, 0.72f, 0.25f), Col(0.55f, 0.78f, 0.30f), Col(0.93f, 0.50f, 0.25f) };
        paperShadow(1.5f, -1.5f, 0.25f);
        leafShape(x + lat - 6, y + bob, 12, 7, 60 + std::sin(t + k) * 25, lc[k % 3]);
        noShadow();
    }
}

// ------------------------------------------------------------
// ZONE 2 - bridge
// ------------------------------------------------------------
void drawBridge(float a)
{
    if (a <= 0.0f) return;
    float cx = layout::BRIDGE_X, by = layout::BRIDGE_Y;
    const int N = 13;
    float span = 62.0f, hgt = 20.0f;

    // planks appear one by one along the arch
    for (int i = 0; i < N; i++)
    {
        float k = stagger(window(a, 0.0f, 0.75f), i, N, 0.7f);
        if (k <= 0.0f) continue;
        float u = (float)i / (N - 1);
        float px = cx - span + 2 * span * u;
        float py = by + std::sin(u * PI) * hgt;
        float slope = std::cos(u * PI) * hgt * PI / (2 * span);
        float ang = std::atan(slope) * 180.0f / PI;
        glPushMatrix();
        glTranslatef(px, py, 0);
        glRotatef(ang, 0, 0, 1);
        glScalef(1.0f, easeOutBack(k), 1.0f);
        paperShadow(2, -3, 0.28f);
        Col wood = (i % 2 == 0) ? Col(0.70f, 0.48f, 0.26f) : Col(0.64f, 0.43f, 0.22f);
        rect(-6.0f, -7.0f, 6.0f, 7.0f, wood);
        noShadow();
        line(-6, 0, 6, 0, 1.0f, Col(0.45f, 0.28f, 0.14f, 0.6f));
        glPopMatrix();
    }
    // rails rise after the deck is laid
    float rk = stage(a, 0.55f, 1.0f);
    if (rk > 0.0f)
    {
        for (int i = 0; i < N; i += 3)
        {
            float u = (float)i / (N - 1);
            float px = cx - span + 2 * span * u;
            float py = by + std::sin(u * PI) * hgt;
            rect(px - 2.0f, py + 6, px + 2.0f, py + 6 + 20 * rk, Col(0.52f, 0.34f, 0.17f));
        }
        std::vector<float> rail;
        int steps = 14;
        for (int i = 0; i <= steps; i++)
        {
            float u = (float)i / steps;
            rail.push_back(cx - span + 2 * span * u);
            rail.push_back(by + std::sin(u * PI) * hgt + 6 + 20 * rk);
        }
        ribbon(rail.data(), steps + 1, 3.5f, Col(0.60f, 0.40f, 0.20f));
        // rope loops
        std::vector<float> rope;
        for (int i = 0; i <= steps; i++)
        {
            float u = (float)i / steps;
            rope.push_back(cx - span + 2 * span * u);
            rope.push_back(by + std::sin(u * PI) * hgt + 6 + 9 * rk);
        }
        ribbon(rope.data(), steps + 1, 1.8f, Col(0.85f, 0.72f, 0.45f));
    }
}

// ------------------------------------------------------------
// ZONE 2 - pond
// ------------------------------------------------------------
void drawPond(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    float cx = layout::POND_X, cy = layout::POND_Y;
    float k = easeOutCubic(a);
    float rx = layout::POND_RX * k, ry = layout::POND_RY * k;

    ellipse(cx + 4, cy - 5, rx + 12, ry + 9, Col(0.1f, 0.06f, 0.02f, 0.22f));
    ellipse(cx, cy, rx + 11, ry + 8, Col(0.60f, 0.55f, 0.46f));          // stone rim
    // rim stones
    for (int i = 0; i < 18; i++)
    {
        float an = 2 * PI * i / 18.0f;
        ellipse(cx + std::cos(an) * (rx + 9), cy + std::sin(an) * (ry + 6.5f), 7 * k, 4.5f * k,
                Col(0.72f - 0.04f * (i % 3), 0.68f, 0.60f));
    }
    ellipse(cx, cy, rx, ry, Col(0.30f, 0.60f, 0.84f));
    ellipse(cx, cy - 2, rx * 0.94f, ry * 0.82f, Col(0.40f, 0.72f, 0.92f));

    // reflections: drifting cloud + shimmer + a faint reflection of the tree glow
    float drift = std::sin(t * 0.3f) * 18.0f;
    ellipse(cx - 30 + drift, cy + 8, 34 * k, 6 * k, Col(1, 1, 1, 0.35f));
    ellipse(cx + 38 + drift * 0.6f, cy - 6, 22 * k, 4 * k, Col(1, 1, 1, 0.28f));
    for (int i = 0; i < 5; i++)
    {
        float s = 0.5f + 0.5f * std::sin(t * 2.0f + i * 1.7f);
        float x = cx - 60 + i * 30;
        line(x - 8 * k, cy - 14 + (i % 2) * 18, x + 8 * k, cy - 14 + (i % 2) * 18, 1.6f, Col(1, 1, 1, 0.25f + 0.4f * s));
    }
    // river mouth glow
    ellipse(cx, cy + ry * 0.78f, 18 * k, 5 * k, Col(0.75f, 0.92f, 1.0f, 0.7f));

    // lily pads with a pink bloom
    struct Lily { float dx, dy, r; bool bloom; };
    static const Lily ly[] = { { -62, -8, 11, true }, { -44, 12, 8, false }, { 64, 6, 10, false }, { 80, -10, 7, true } };
    for (int i = 0; i < 4; i++)
    {
        float bob = std::sin(t * 1.5f + i) * 1.2f;
        float lx = cx + ly[i].dx * k, lyy = cy + ly[i].dy * k + bob;
        ellipse(lx, lyy, ly[i].r * k, ly[i].r * 0.5f * k, Col(0.28f, 0.62f, 0.34f));
        tri(lx, lyy, lx + ly[i].r * k, lyy + 3, lx + ly[i].r * k, lyy - 1, Col(0.30f, 0.60f, 0.85f));
        if (ly[i].bloom)
        {
            for (int p = 0; p < 5; p++)
                leafShape(lx, lyy + 2, 9 * k, 5 * k, 20 + p * 34, Col(1.0f, 0.70f, 0.82f));
            circle(lx + 2, lyy + 3, 2.5f * k, Col(1.0f, 0.88f, 0.35f));
        }
    }
    // reeds at the left and right edges
    for (int side = -1; side <= 1; side += 2)
    {
        for (int r = 0; r < 4; r++)
        {
            float bx = cx + side * (rx + 2 + r * 5.0f);
            float by = cy + 4 - r * 3;
            float sway = std::sin(t * 1.4f + r + side) * 3.0f;
            float h = (34 + r * 6) * k;
            float rp[] = { bx, by, bx + sway * 0.5f, by + h * 0.5f, bx + sway, by + h };
            ribbonTaper(rp, 3, 3.0f, 1.2f, Col(0.30f, 0.58f, 0.30f));
            ellipse(bx + sway, by + h - 4, 3.0f, 8.0f, Col(0.50f, 0.32f, 0.16f));
        }
    }
}

bool hitWaterfall(float x, float y)
{
    return inRect(x, y, layout::WF_X - 26, layout::WF_BASE - 6, layout::WF_X + 26, layout::WF_TOP + 6);
}
bool hitPond(float x, float y)
{
    return inEllipse(x, y, layout::POND_X, layout::POND_Y, layout::POND_RX + 8, layout::POND_RY + 6);
}

// ------------------------------------------------------------
// stone paths (village zone)
// ------------------------------------------------------------
void drawPathStones(float a)
{
    if (a <= 0.0f) return;
    static const float st[][3] = {
        { 262, 236, 12 }, { 290, 231, 11 }, { 320, 234, 12 }, { 349, 239, 11 }, { 378, 238, 12 },
        { 408, 243, 11 }, { 434, 248, 10 },
        { 568, 247, 10 }, { 596, 242, 11 }, { 626, 236, 12 }, { 658, 230, 11 }, { 690, 225, 12 },
        { 716, 228, 10 }, { 750, 226, 11 } };
    int n = (int)(sizeof(st) / sizeof(st[0]));
    for (int i = 0; i < n; i++)
    {
        float k = stagger(a, i, n, 0.7f);
        if (k <= 0.0f) continue;
        float s = easeOutBack(k);
        paperShadow(2, -2, 0.25f);
        ellipse(st[i][0], st[i][1], st[i][2] * s, st[i][2] * 0.52f * s,
                Col(0.80f - 0.03f * (i % 3), 0.76f - 0.02f * (i % 3), 0.66f));
        noShadow();
        ellipse(st[i][0] - 2, st[i][1] + 1.5f, st[i][2] * 0.6f * s, st[i][2] * 0.25f * s, Col(1, 1, 1, 0.22f));
    }
}

} // namespace env
