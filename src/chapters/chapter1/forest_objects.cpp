// ============================================================
// objects.cpp - Zone 3 (village) + Zone 4 (magical heart)
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

#include "chapters/chapter1/forest_objects.h"
#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_layout.h"
#include "chapters/chapter1/forest_environment.h"
#include "shared/audio.h"

using namespace pl;

namespace obj
{
// ------------------------------------------------------------
// state
// ------------------------------------------------------------
struct House { float x, y, s; Col cap; float door, doorTarget, closeTimer; };
static std::vector<House> g_houses;

struct Flower { float x, y, s; Col petal; bool magical; float bloom, target; int appearIdx; };
static std::vector<Flower> g_flowers;

struct Mush { float x, y, s; Col cap; float squash; };
static std::vector<Mush> g_mush;

static float g_chestOpen = 0.0f, g_chestTarget = 0.0f;
static float g_treeBoost = 0.0f;         // click glow
static float g_magic = 0.0f;             // stage-6 amount
static float g_chestVis = 0.0f;
static float g_treeDim = 0.0f;           // used by the cinematic
void setTreeDim(float d) { g_treeDim = d; }

void initObjects()
{
    g_houses.clear();
    House h1 = { layout::HOUSE1_X, layout::HOUSE1_Y, 0.90f, Col(0.93f, 0.46f, 0.42f), 0, 0, 0 };
    House h2 = { layout::HOUSE2_X, layout::HOUSE2_Y, 1.10f, Col(0.72f, 0.58f, 0.88f), 0, 0, 0 };
    House h3 = { layout::HOUSE3_X, layout::HOUSE3_Y, 0.80f, Col(0.98f, 0.78f, 0.34f), 0, 0, 0 };
    // far to near so the nearest house is drawn last
    g_houses.push_back(h3);
    g_houses.push_back(h1);
    g_houses.push_back(h2);

    g_flowers.clear();
    struct F { float x, y, s; Col c; };
    static const F deco[] = {
        {  98, 206, 0.9f, Col(1.00f, 0.62f, 0.72f) }, { 205, 222, 0.8f, Col(1.00f, 0.85f, 0.35f) },
        { 300, 205, 0.9f, Col(0.98f, 0.55f, 0.40f) }, { 396, 215, 0.8f, Col(0.85f, 0.65f, 0.95f) },
        { 120, 150, 1.1f, Col(1.00f, 0.55f, 0.65f) }, { 262, 118, 1.2f, Col(1.00f, 0.82f, 0.30f) },
        { 335, 160, 1.1f, Col(0.80f, 0.60f, 0.95f) }, { 585, 205, 0.9f, Col(1.00f, 0.60f, 0.45f) },
        { 635, 118, 1.2f, Col(1.00f, 0.72f, 0.80f) }, { 690, 168, 1.1f, Col(1.00f, 0.85f, 0.35f) },
        { 890, 175, 1.1f, Col(0.95f, 0.55f, 0.70f) }, { 118, 110, 1.0f, Col(0.98f, 0.70f, 0.35f) } };
    for (size_t i = 0; i < sizeof(deco) / sizeof(deco[0]); i++)
    {
        Flower f = { deco[i].x, deco[i].y, deco[i].s, deco[i].c, false, 0, 0.62f, (int)i };
        g_flowers.push_back(f);
    }
    static const F mag[] = {
        { 692, 232, 1.1f, Col(0.45f, 0.85f, 1.00f) }, { 765, 218, 1.2f, Col(1.00f, 0.55f, 0.90f) },
        { 838, 214, 1.2f, Col(0.70f, 0.60f, 1.00f) }, { 880, 232, 1.1f, Col(0.50f, 1.00f, 0.80f) },
        { 802, 205, 1.0f, Col(1.00f, 0.90f, 0.40f) } };
    for (size_t i = 0; i < sizeof(mag) / sizeof(mag[0]); i++)
    {
        Flower f = { mag[i].x, mag[i].y, mag[i].s, mag[i].c, true, 0, 0.8f, (int)i };
        g_flowers.push_back(f);
    }

    g_mush.clear();
    Mush m[] = {
        { 884, 126, 1.00f, Col(0.92f, 0.30f, 0.28f), 0 }, { 910, 118, 0.70f, Col(0.95f, 0.50f, 0.30f), 0 },
        { 858, 116, 0.65f, Col(0.92f, 0.30f, 0.28f), 0 }, { 310, 120, 0.85f, Col(0.95f, 0.45f, 0.35f), 0 },
        { 336, 114, 0.60f, Col(0.88f, 0.28f, 0.30f), 0 }, { 72, 196, 0.60f, Col(0.95f, 0.55f, 0.35f), 0 } };
    for (size_t i = 0; i < sizeof(m) / sizeof(m[0]); i++) g_mush.push_back(m[i]);

    g_chestOpen = g_chestTarget = 0.0f;
    g_treeBoost = 0.0f;
    g_magic = 0.0f;
    g_chestVis = 0.0f;
}

void updateObjects(float dt)
{
    for (size_t i = 0; i < g_houses.size(); i++)
    {
        House& h = g_houses[i];
        if (h.doorTarget > 0.5f)
        {
            h.closeTimer -= dt;
            if (h.closeTimer <= 0.0f) h.doorTarget = 0.0f;
        }
        h.door += (h.doorTarget - h.door) * clamp01(dt * 6.0f);
    }
    for (size_t i = 0; i < g_flowers.size(); i++)
    {
        Flower& f = g_flowers[i];
        f.bloom += (f.target - f.bloom) * clamp01(dt * 3.5f);
        // a click-bloom relaxes back to the resting open state
        if (f.target > 0.95f)
        {
            f.target -= dt * 0.12f;
        }
    }
    for (size_t i = 0; i < g_mush.size(); i++)
        g_mush[i].squash *= (1.0f - clamp01(dt * 5.0f));
    g_chestOpen += (g_chestTarget - g_chestOpen) * clamp01(dt * 4.0f);
    if (g_treeBoost > 0.0f) g_treeBoost = g_treeBoost - dt < 0.0f ? 0.0f : g_treeBoost - dt;
}

float treeGlow() { return g_magic + 0.9f * smooth(clamp01(g_treeBoost / 1.2f)) * (g_treeBoost > 0 ? 1.0f : 0.0f); }
bool chestOpen() { return g_chestTarget > 0.5f; }
void setChestVisible(float a) { g_chestVis = a; }

// ------------------------------------------------------------
// mushroom house
// ------------------------------------------------------------
void drawMushroomHouseAt(float x, float y, float s, const Col& cap,
                         float doorOpen, float windowGlow)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    paperShadow(4, -3, 0.25f);
    // stem
    float stem[] = { -31, 0, 31, 0, 25, 74, -25, 74 };
    poly(stem, 4, Col(0.99f, 0.94f, 0.82f));
    noShadow();
    tri(8, 0, 31, 0, 25, 74, Col(0.90f, 0.83f, 0.68f, 0.6f));
    // little flower box
    rect(-18, 40, 18, 48, Col(0.58f, 0.38f, 0.20f));
    for (int i = -2; i <= 2; i++) circle(i * 7.0f, 52, 3.4f, (i % 2) ? Col(1, 0.6f, 0.7f) : Col(1, 0.85f, 0.4f));

    // door
    glow(0, 14, 34 * (0.4f + doorOpen), Col(1.0f, 0.82f, 0.40f, 0.55f * doorOpen));
    rect(-11, 0, 11, 28, Col(1.0f, 0.86f, 0.45f, 1.0f * clamp01(doorOpen * 3.0f)));
    dome(0, 28, 11, 10, Col(1.0f, 0.86f, 0.45f, clamp01(doorOpen * 3.0f)));
    glPushMatrix();
    glTranslatef(-11, 0, 0);
    glScalef(1.0f - 0.78f * doorOpen, 1.0f, 1.0f);
    glTranslatef(11, 0, 0);
    Col wood(0.50f, 0.31f, 0.16f);
    rect(-11, 0, 11, 28, wood);
    dome(0, 28, 11, 10, wood);
    line(0, 0, 0, 36, 1.2f, Col(0.32f, 0.18f, 0.08f, 0.7f));
    circle(6, 15, 1.8f, Col(1.0f, 0.85f, 0.4f));
    glPopMatrix();

    // round window with a warm glow
    glow(0, 60, 26, Col(1.0f, 0.85f, 0.40f, 0.65f * windowGlow));
    circle(0, 60, 8.5f, Col(0.45f, 0.30f, 0.18f));
    circle(0, 60, 6.8f, mix(Col(0.80f, 0.90f, 0.95f), Col(1.0f, 0.90f, 0.45f), windowGlow));
    line(-6.8f, 60, 6.8f, 60, 1.2f, Col(0.45f, 0.30f, 0.18f));
    line(0, 53.2f, 0, 66.8f, 1.2f, Col(0.45f, 0.30f, 0.18f));

    // cap
    paperShadow(4, -4, 0.28f);
    dome(0, 66, 75, 54, shade(cap, 0.86f));
    noShadow();
    ellipse(0, 66, 75, 9, shade(cap, 0.62f));                 // gills underside
    dome(-6, 68, 66, 46, cap);
    dome(-20, 90, 30, 18, Col(1, 1, 1, 0.16f));                 // paper highlight
    circle(-38, 96, 10, Col(1.0f, 0.97f, 0.88f));
    circle(10, 114, 8, Col(1.0f, 0.97f, 0.88f));
    circle(42, 92, 9, Col(1.0f, 0.97f, 0.88f));
    circle(-10, 88, 6, Col(1.0f, 0.97f, 0.88f));
    circle(62, 72, 5, Col(1.0f, 0.97f, 0.88f));
    glPopMatrix();
}

int houseCount() { return (int)g_houses.size(); }
float houseBaseY(int i) { return g_houses[i].y; }

void drawHouse(int i, float a, float windowGlow)
{
    if (i < 0 || i >= (int)g_houses.size()) return;
    const House& h = g_houses[i];
    float k = stagger(a, i, 3, 0.5f);
    if (beginPopUp(h.x, h.y, k, i % 2 ? 30.0f : -30.0f))
    {
        creaseShadow(0, 0, 48 * h.s, k);
        float wob = 1.0f + 0.01f * std::sin(now() * 2.0f + i);
        glScalef(1, wob, 1);
        glTranslatef(-h.x, -h.y, 0);
        drawMushroomHouseAt(h.x, h.y, h.s, h.cap, h.door, windowGlow * (0.85f + 0.15f * std::sin(now() * 3.0f + i * 2.0f)));
        endPopUp();
    }
}

void drawSign(float a)
{
    if (a <= 0.0f) return;
    float x = 396, y = 236;
    if (!beginPopUp(x, y, a, 20)) return;
    paperShadow(2, -2, 0.25f);
    rect(-3, 0, 3, 46, Col(0.50f, 0.32f, 0.16f));
    float board[] = { -42, 36, 30, 36, 44, 47, 30, 58, -42, 58 };
    poly(board, 5, Col(0.82f, 0.60f, 0.34f));
    noShadow();
    line(-40, 52, 30, 52, 1.0f, Col(0.62f, 0.42f, 0.22f, 0.6f));
    text(-36, 43, "To the Tree", GLUT_BITMAP_HELVETICA_10, Col(0.30f, 0.16f, 0.06f));
    glPopMatrix();
}

// ------------------------------------------------------------
// Ancient Tree
// ------------------------------------------------------------
void drawAncientTreeAt(float x, float y, float s, float glowAmt, float swayPhase)
{
    float t = now() + swayPhase;
    float g = clamp01(glowAmt);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    // aura behind the tree
    if (glowAmt > 0.01f)
    {
        float pulse = 0.85f + 0.15f * std::sin(t * 2.2f);
        glow(0, 230, 250, Col(1.0f, 0.88f, 0.40f, 0.28f * glowAmt * pulse));
        glow(0, 250, 150, Col(1.0f, 0.95f, 0.60f, 0.25f * glowAmt * pulse));
    }

    // roots
    const Col nightTint(0.14f, 0.16f, 0.30f);
    Col bark = mix(Col(0.46f, 0.29f, 0.16f), nightTint, g_treeDim * 0.6f);
    Col barkDark = mix(Col(0.34f, 0.20f, 0.11f), nightTint, g_treeDim * 0.6f);
    paperShadow(5, -4, 0.28f);
    {
        float r1[] = { -22, 22, -60, 10, -105, -6 };
        float r2[] = { 22, 22, 62, 10, 108, -4 };
        float r3[] = { -14, 18, -34, 0, -52, -14 };
        float r4[] = { 14, 18, 36, 0, 56, -12 };
        ribbonTaper(r1, 3, 26, 6, bark);
        ribbonTaper(r2, 3, 26, 6, bark);
        ribbonTaper(r3, 3, 18, 5, barkDark);
        ribbonTaper(r4, 3, 18, 5, barkDark);
    }
    // trunk
    float trunk[] = { -46, 0, -42, 40, -36, 100, -33, 150, -28, 196, 28, 196, 33, 150, 36, 100, 42, 40, 46, 0 };
    poly(trunk, 10, bark);
    noShadow();
    // paper-fold facets
    float facet[] = { 8, 0, 46, 0, 42, 40, 36, 100, 33, 150, 28, 196, 8, 196 };
    poly(facet, 7, Col(0.55f, 0.36f, 0.20f, 0.55f));
    for (int i = 0; i < 4; i++)
    {
        float bx = -26 + i * 15.0f;
        float rb[] = { bx, 12, bx + (i % 2 ? 3 : -3), 70, bx - 1, 150 };
        ribbon(rb, 3, 2.0f, Col(0.28f, 0.16f, 0.08f, 0.45f));
    }

    // glowing hollow
    ellipse(0, 70, 17, 27, Col(0.20f, 0.10f, 0.06f));
    if (glowAmt > 0.01f)
    {
        ellipse(0, 70, 13, 22, Col(1.0f, 0.88f, 0.45f, 0.9f * g));
        glow(0, 70, 55, Col(1.0f, 0.85f, 0.35f, 0.5f * g));
    }

    // magical runes carved into the trunk
    {
        float ra = 0.25f + 0.75f * g;
        float pulse = 0.7f + 0.3f * std::sin(t * 3.0f);
        Col rc(1.0f, 0.88f, 0.40f, ra * pulse);
        // spiral
        std::vector<float> sp;
        for (int i = 0; i < 16; i++)
        {
            float an = i * 0.55f, r = 1.0f + i * 0.55f;
        sp.push_back(-17 + std::cos(an) * r); sp.push_back(138 + std::sin(an) * r);
        }
        ribbon(sp.data(), 16, 1.8f, rc);
        // triangle with an eye
        line(10, 124, 22, 124, 1.8f, rc); line(22, 124, 16, 136, 1.8f, rc); line(16, 136, 10, 124, 1.8f, rc);
        circle(16, 128, 1.6f, rc);
        // little star rune
        line(-8, 166, 8, 166, 1.8f, rc); line(0, 158, 0, 174, 1.8f, rc);
        line(-5, 160, 5, 172, 1.4f, rc); line(5, 160, -5, 172, 1.4f, rc);
        if (glowAmt > 0.1f) glow(0, 145, 60, Col(1.0f, 0.88f, 0.4f, 0.18f * g));
    }

    // branches
    paperShadow(3, -3, 0.2f);
    {
        float bl[] = { -22, 182, -62, 214, -112, 244 };
        float br[] = { 22, 182, 66, 216, 118, 240 };
        float bm[] = { 0, 190, -6, 230, 8, 270 };
        float bl2[] = { -52, 207, -86, 196, -122, 206 };
        float br2[] = { 58, 210, 96, 204, 128, 214 };
        ribbonTaper(bl, 3, 22, 7, bark);
        ribbonTaper(br, 3, 22, 7, bark);
        ribbonTaper(bm, 3, 20, 7, bark);
        ribbonTaper(bl2, 3, 10, 4, bark);
        ribbonTaper(br2, 3, 10, 4, bark);
    }
    noShadow();
    // glowing veins on the branches (light travels outward)
    if (glowAmt > 0.02f)
    {
        float travel = std::fmod(t * 0.35f, 1.0f);
        const float* br[3] = { 0, 0, 0 };
        (void)br;
        float L[] = { -22, 182, -62, 214, -112, 244 };
        float R[] = { 22, 182, 66, 216, 118, 240 };
        float M[] = { 0, 190, -6, 230, 8, 270 };
        ribbon(L, 3, 2.4f, Col(1.0f, 0.90f, 0.45f, 0.55f * g));
        ribbon(R, 3, 2.4f, Col(1.0f, 0.90f, 0.45f, 0.55f * g));
        ribbon(M, 3, 2.4f, Col(1.0f, 0.90f, 0.45f, 0.55f * g));
        for (int k = 0; k < 3; k++)
        {
            const float* P = (k == 0) ? L : (k == 1 ? R : M);
            float u = travel;
            float px = (1 - u) * (1 - u) * P[0] + 2 * u * (1 - u) * P[2] + u * u * P[4];
            float py = (1 - u) * (1 - u) * P[1] + 2 * u * (1 - u) * P[3] + u * u * P[5];
            glow(px, py, 16, Col(1.0f, 0.95f, 0.6f, 0.9f * g));
        }
    }

    // canopy (three paper layers, slowly swaying)
    struct Blob { float x, y, r; };
    static const Blob back[] = { { -100, 226, 60 }, { -45, 262, 70 }, { 32, 270, 72 }, { 102, 230, 62 }, { 0, 224, 72 } };
    static const Blob mid[] = { { -80, 246, 50 }, { -8, 284, 58 }, { 64, 262, 52 }, { -118, 212, 34 }, { 116, 214, 34 } };
    static const Blob top[] = { { -40, 300, 38 }, { 42, 298, 36 }, { -92, 268, 30 }, { 96, 256, 30 }, { 0, 250, 44 } };
    float lift = g * 0.10f;
    Col cb = mix(mix(Col(0.16f, 0.44f, 0.28f), Col(0.20f, 0.55f, 0.34f), lift * 5), nightTint, g_treeDim * 0.65f);
    Col cm = mix(mix(Col(0.26f, 0.60f, 0.36f), Col(0.36f, 0.72f, 0.40f), lift * 5), nightTint, g_treeDim * 0.65f);
    Col ct = mix(mix(Col(0.40f, 0.74f, 0.42f), Col(0.58f, 0.86f, 0.46f), lift * 5), nightTint, g_treeDim * 0.65f);
    paperShadow(5, -5, 0.24f);
    for (int i = 0; i < 5; i++)
        circle(back[i].x + std::sin(t * 0.7f + i) * 2.0f, back[i].y, back[i].r, cb);
    for (int i = 0; i < 5; i++)
        circle(mid[i].x + std::sin(t * 0.8f + i * 1.3f) * 2.5f, mid[i].y, mid[i].r, cm);
    for (int i = 0; i < 5; i++)
        circle(top[i].x + std::sin(t * 0.9f + i * 0.7f) * 3.0f, top[i].y, top[i].r, ct);
    noShadow();
    // paper highlights
    for (int i = 0; i < 5; i++)
        circle(top[i].x - top[i].r * 0.3f, top[i].y + top[i].r * 0.3f, top[i].r * 0.45f, Col(1, 1, 1, 0.10f));

    // glowing leaves / lights hanging in the canopy
    if (glowAmt > 0.02f)
    {
        for (int i = 0; i < 16; i++)
        {
            float lx = -112 + std::fmod(i * 53.0f, 224.0f);
            float ly = 215 + std::fmod(i * 37.0f, 120.0f);
            float p = 0.5f + 0.5f * std::sin(t * 2.0f + i * 1.9f);
            glow(lx, ly + std::sin(t + i) * 3, 10 + 5 * p, Col(1.0f, 0.92f, 0.45f, (0.18f + 0.35f * p) * g));
            circle(lx, ly + std::sin(t + i) * 3, 2.0f, Col(1, 1, 0.85f, 0.9f * g));
        }
        // expanding pulse rings from the trunk
        for (int k = 0; k < 2; k++)
        {
            float u = std::fmod(t * 0.28f + k * 0.5f, 1.0f);
            ring(0, 200, 40 + 190 * u, 43 + 190 * u, Col(1.0f, 0.90f, 0.5f, 0.35f * g * (1.0f - u)));
        }
    }
    glPopMatrix();
}

void drawAncientTree(float a, float magic)
{
    g_magic = magic;
    if (a <= 0.0f) return;
    float tilt = 14.0f;
    if (beginPopUp(layout::TREE_X, layout::TREE_Y, a, tilt))
    {
        creaseShadow(0, 0, 100 * layout::TREE_S, a);
        glTranslatef(-layout::TREE_X, -layout::TREE_Y, 0);
        drawAncientTreeAt(layout::TREE_X, layout::TREE_Y, layout::TREE_S, treeGlow(), 0.0f);
        endPopUp();
    }
}

// ------------------------------------------------------------
// treasure chest
// ------------------------------------------------------------
void drawChestAt(float x, float y, float s, float open, float glowAmt)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);
    float t = now();
    Col wood(0.60f, 0.38f, 0.20f), woodD(0.48f, 0.29f, 0.14f), gold(1.0f, 0.80f, 0.28f);

    if (open > 0.02f)
    {
        glow(0, 30, 70 * open, Col(1.0f, 0.88f, 0.40f, 0.55f * open));
        // light rays
        for (int i = 0; i < 6; i++)
        {
            float an = PI * 0.5f + (i - 2.5f) * 0.22f + std::sin(t * 1.5f + i) * 0.04f;
            float len = (60.0f + 12 * std::sin(t * 2 + i)) * open;
            tri(-3, 30, 3, 30, std::cos(an) * len, 30 + std::sin(an) * len, Col(1.0f, 0.92f, 0.5f, 0.28f * open));
        }
    }

    // lid (behind the box when open)
    float lidLift = 24.0f * open;
    glPushMatrix();
    glTranslatef(0, 26 + lidLift, 0);
    glScalef(1.0f, 1.0f - 0.45f * open, 1.0f);
    paperShadow(3, -3, 0.25f);
    dome(0, 0, 29, 17, woodD);
    noShadow();
    dome(0, 0, 25, 13, wood);
    rect(-5, 0, 5, 14, gold);
    glPopMatrix();

    // body
    paperShadow(4, -3, 0.28f);
    rect(-29, 0, 29, 27, wood);
    noShadow();
    for (int i = 1; i < 3; i++) line(-29, i * 9.0f, 29, i * 9.0f, 1.0f, Col(0.38f, 0.22f, 0.10f, 0.6f));
    rect(-29, 0, -22, 27, gold);
    rect(22, 0, 29, 27, gold);
    rect(-4, 0, 4, 27, gold);

    // interior and treasure
    if (open > 0.02f)
    {
        ellipse(0, 27, 26, 5, Col(0.20f, 0.10f, 0.05f));
        ellipse(0, 28, 23 * open, 4 * open, Col(1.0f, 0.88f, 0.4f, 0.95f));
        for (int i = 0; i < 7; i++)
            circle(-18 + i * 6.0f, 29 + (i % 2) * 2.5f, 3.0f * open, Col(1.0f, 0.80f + 0.1f * (i % 2), 0.25f));
        glow(0, 34, 40, Col(1.0f, 0.90f, 0.5f, 0.6f * open * (0.8f + 0.2f * std::sin(t * 5.0f))));
    }
    else
    {
        circle(0, 17, 3.5f, Col(0.95f, 0.70f, 0.20f));
        rect(-1.2f, 11, 1.2f, 17, Col(0.35f, 0.20f, 0.08f));
    }
    if (glowAmt > 0.01f) glow(0, 20, 50, Col(1.0f, 0.88f, 0.4f, 0.35f * glowAmt));
    glPopMatrix();
}

void drawChest(float a)
{
    g_chestVis = a;
    if (a <= 0.0f) return;
    float t = now();
    float shimmer = (1.0f - g_chestOpen) * 0.5f * (0.5f + 0.5f * std::sin(t * 3.0f));
    if (beginPopUp(layout::CHEST_X, layout::CHEST_Y, a, 0))
    {
        glTranslatef(-layout::CHEST_X, -layout::CHEST_Y, 0);
        drawChestAt(layout::CHEST_X, layout::CHEST_Y, 1.0f, g_chestOpen, shimmer);
        endPopUp();
    }
}

// ------------------------------------------------------------
// flowers
// ------------------------------------------------------------
void drawFlowerAt(float x, float y, float s, float bloom, const Col& petal, bool magical)
{
    float t = now();
    float sway = std::sin(t * 1.7f + x * 0.07f) * 2.5f * s;
    float h = 26.0f * s;
    float hx = x + sway, hy = y + h;
    glPushMatrix();
    // stem + leaves
    float st[] = { x, y, x + sway * 0.4f, y + h * 0.5f, hx, hy };
    ribbon(st, 3, 2.4f * s, Col(0.25f, 0.55f, 0.28f));
    leafShape(x + sway * 0.2f, y + h * 0.28f, 11 * s, 5 * s, 25, Col(0.32f, 0.64f, 0.32f));
    leafShape(x + sway * 0.2f, y + h * 0.42f, 11 * s, 5 * s, 155, Col(0.30f, 0.60f, 0.30f));

    if (magical)
    {
        float p = 0.7f + 0.3f * std::sin(t * 2.5f + x);
        glow(hx, hy, 34 * s * (0.4f + bloom), Col(petal.r, petal.g, petal.b, 0.55f * p * bloom));
    }
    // petals: tight bud -> open bloom
    float b = clamp01(bloom);
    float over = bloom > 1.0f ? bloom - 1.0f : 0.0f;
    float plen = (5.0f + 9.0f * b + 3.0f * over) * s;
    float pwid = (4.0f + 6.0f * b) * s;
    int n = 6;
    for (int i = 0; i < n; i++)
    {
        float ang = 360.0f * i / n + t * (magical ? 12.0f : 0.0f);
        float spread = 1.0f - 0.7f * (1.0f - b);          // bud keeps petals closed
        float a2 = ang * spread + 90.0f * (1.0f - spread);
        paperShadow(1.2f, -1.2f, 0.2f);
        ellipse(hx + std::cos(a2 * PI / 180.0f) * plen * 0.55f, hy + std::sin(a2 * PI / 180.0f) * plen * 0.55f,
                plen * 0.5f, pwid * 0.5f, (i % 2) ? petal : shade(petal, 0.93f));
        noShadow();
    }
    circle(hx, hy, (2.6f + 2.2f * b) * s, Col(1.0f, 0.86f, 0.30f));
    glPopMatrix();
}

void drawFlowers(float a)
{
    if (a <= 0.0f) return;
    for (size_t i = 0; i < g_flowers.size(); i++)
    {
        const Flower& f = g_flowers[i];
        if (f.magical) continue;
        float k = stagger(a, f.appearIdx, 12, 0.7f);
        if (k <= 0.0f) continue;
        drawFlowerAt(f.x, f.y, f.s * easeOutBack(k), f.bloom * k, f.petal, false);
    }
}

void drawMagicalFlowers(float a)
{
    if (a <= 0.0f) return;
    for (size_t i = 0; i < g_flowers.size(); i++)
    {
        const Flower& f = g_flowers[i];
        if (!f.magical) continue;
        float k = stagger(a, f.appearIdx, 5, 0.6f);
        if (k <= 0.0f) continue;
        drawFlowerAt(f.x, f.y, f.s * easeOutBack(k), f.bloom * k, f.petal, true);
    }
}

void drawMushrooms(float a)
{
    if (a <= 0.0f) return;
    for (size_t i = 0; i < g_mush.size(); i++)
    {
        const Mush& m = g_mush[i];
        float k = stagger(a, (int)i, (int)g_mush.size(), 0.6f);
        if (k <= 0.0f) continue;
        float sq = m.squash;
        glPushMatrix();
        glTranslatef(m.x, m.y, 0);
        glScalef(m.s * (1.0f + 0.25f * sq) * easeOutBack(k), m.s * (1.0f - 0.3f * sq) * easeOutBack(k), 1);
        paperShadow(2.5f, -2.5f, 0.25f);
        float stem[] = { -6, 0, 6, 0, 5, 18, -5, 18 };
        poly(stem, 4, Col(0.98f, 0.93f, 0.82f));
        dome(0, 14, 17, 14, m.cap);
        noShadow();
        circle(-7, 22, 2.8f, Col(1, 0.97f, 0.9f));
        circle(5, 25, 2.2f, Col(1, 0.97f, 0.9f));
        circle(9, 19, 2.0f, Col(1, 0.97f, 0.9f));
        glPopMatrix();
    }
}

// ------------------------------------------------------------
// interaction
// ------------------------------------------------------------
Hit click(float x, float y)
{
    // ancient tree first (largest)
    if (g_magic >= 0.0f)
    {
        float tx = layout::TREE_X, ty = layout::TREE_Y;
        if (inEllipse(x, y, tx, ty + 255 * layout::TREE_S, 140 * layout::TREE_S, 95 * layout::TREE_S) || inRect(x, y, tx - 45 * layout::TREE_S, ty, tx + 45 * layout::TREE_S, ty + 200 * layout::TREE_S))
        {
            g_treeBoost = 2.4f;
            fx::forestParticles().sparkles(tx + rnd(-60, 60), ty + 230, 26, Col(1.0f, 0.9f, 0.45f), 110, 8);
            for (int i = 0; i < 6; i++) fx::forestParticles().leaf(tx + rnd(-100, 100), ty + 200 + rnd(0, 60), rnd(120, 210));
            audio::play(audio::SFX_TREE);
            return H_ANCIENT_TREE;
        }
    }
    // chest
    if (g_chestVis > 0.8f && inRect(x, y, layout::CHEST_X - 34, layout::CHEST_Y - 4, layout::CHEST_X + 34, layout::CHEST_Y + 50))
    {
        g_chestTarget = g_chestTarget > 0.5f ? 0.0f : 1.0f;
        if (g_chestTarget > 0.5f)
        {
            fx::forestParticles().coins(layout::CHEST_X, layout::CHEST_Y + 34, 14);
            fx::forestParticles().sparkles(layout::CHEST_X, layout::CHEST_Y + 40, 22, Col(1.0f, 0.88f, 0.4f), 110, 8);
            audio::play(audio::SFX_CHEST);
        }
        else audio::play(audio::SFX_CLICK);
        return H_CHEST;
    }
    // flowers
    for (int i = (int)g_flowers.size() - 1; i >= 0; i--)
    {
        Flower& f = g_flowers[i];
        if (f.magical && g_magic < 0.4f) continue;
        float hx = f.x, hy = f.y + 26.0f * f.s;
        float dx = x - hx, dy = y - hy;
        if (dx * dx + dy * dy < (20 * f.s) * (20 * f.s) + 120)
        {
            f.target = 1.45f;
            fx::forestParticles().sparkles(hx, hy, 12, f.petal, 70, 6);
            fx::forestParticles().motes(hx, hy, 4, Col(f.petal.r, f.petal.g, f.petal.b, 1), 14);
            audio::play(audio::SFX_BLOOM);
            return H_FLOWER;
        }
    }
    // houses (nearest first)
    for (int i = (int)g_houses.size() - 1; i >= 0; i--)
    {
        House& h = g_houses[i];
        if (inRect(x, y, h.x - 60 * h.s, h.y, h.x + 60 * h.s, h.y + 76 * h.s) ||
            inEllipse(x, y, h.x, h.y + 70 * h.s, 76 * h.s, 56 * h.s))
        {
            h.doorTarget = 1.0f;
            h.closeTimer = 4.0f;
            fx::forestParticles().sparkles(h.x, h.y + 18 * h.s, 10, Col(1.0f, 0.85f, 0.4f), 60, 6);
            audio::play(audio::SFX_DOOR);
            return H_HOUSE;
        }
    }
    // mushrooms
    for (size_t i = 0; i < g_mush.size(); i++)
    {
        Mush& m = g_mush[i];
        if (inEllipse(x, y, m.x, m.y + 12 * m.s, 20 * m.s, 18 * m.s))
        {
            m.squash = 1.0f;
            fx::forestParticles().motes(m.x, m.y + 22 * m.s, 7, Col(1.0f, 0.95f, 0.7f, 1.0f), 12);
            audio::play(audio::SFX_BOING);
            return H_MUSHROOM;
        }
    }
    return H_NONE;
}

bool hover(float x, float y)
{
    float tx = layout::TREE_X, ty = layout::TREE_Y;
    if (inEllipse(x, y, tx, ty + 255 * layout::TREE_S, 140 * layout::TREE_S, 95 * layout::TREE_S) || inRect(x, y, tx - 45 * layout::TREE_S, ty, tx + 45 * layout::TREE_S, ty + 200 * layout::TREE_S)) return true;
    if (g_chestVis > 0.8f && inRect(x, y, layout::CHEST_X - 34, layout::CHEST_Y - 4, layout::CHEST_X + 34, layout::CHEST_Y + 50)) return true;
    for (size_t i = 0; i < g_flowers.size(); i++)
    {
        const Flower& f = g_flowers[i];
        if (f.magical && g_magic < 0.4f) continue;
        float dx = x - f.x, dy = y - (f.y + 26.0f * f.s);
        if (dx * dx + dy * dy < (20 * f.s) * (20 * f.s) + 120) return true;
    }
    for (size_t i = 0; i < g_houses.size(); i++)
    {
        const House& h = g_houses[i];
        if (inRect(x, y, h.x - 60 * h.s, h.y, h.x + 60 * h.s, h.y + 76 * h.s) ||
            inEllipse(x, y, h.x, h.y + 70 * h.s, 76 * h.s, 56 * h.s)) return true;
    }
    for (size_t i = 0; i < g_mush.size(); i++)
        if (inEllipse(x, y, g_mush[i].x, g_mush[i].y + 12 * g_mush[i].s, 20 * g_mush[i].s, 18 * g_mush[i].s)) return true;
    return false;
}

// ------------------------------------------------------------
// discovery icons
// ------------------------------------------------------------
const char* discoveryName(int kind)
{
    static const char* n[] = { "Glowing Crystal", "Ancient Leaf", "Magical Feather", "Tiny Golden Key", "Star Fragment" };
    return (kind >= 0 && kind < 5) ? n[kind] : "";
}

void drawDiscoveryIcon(int kind, float x, float y, float s, float glowAmt)
{
    float t = now();
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);
    float pulse = 0.75f + 0.25f * std::sin(t * 4.0f + kind);
    switch (kind)
    {
    case 0: // crystal
        glow(0, 2, 26, Col(0.45f, 0.95f, 1.0f, 0.55f * glowAmt * pulse));
        {
            float c1[] = { -7, -12, 7, -12, 11, 3, 0, 19, -11, 3 };
            poly(c1, 5, Col(0.42f, 0.84f, 0.96f));
            tri(0, 19, 11, 3, 0, -12, Col(0.28f, 0.66f, 0.88f, 0.9f));
            tri(-11, 3, -7, -12, 0, 4, Col(0.78f, 0.97f, 1.0f, 0.9f));
            float c2[] = { 10, -10, 17, -10, 19, -2, 14, 6, 9, -2 };
            poly(c2, 5, Col(0.55f, 0.90f, 0.98f));
        }
        break;
    case 1: // ancient leaf
        glow(0, 0, 24, Col(1.0f, 0.82f, 0.35f, 0.5f * glowAmt * pulse));
        leafShape(-12, -10, 30, 17, 45, Col(0.96f, 0.76f, 0.22f));
        line(-12, -10, 9, 11, 1.3f, Col(0.70f, 0.46f, 0.10f));
        line(-4, -2, 0, -9, 1.0f, Col(0.70f, 0.46f, 0.10f));
        line(1, 3, 6, -3, 1.0f, Col(0.70f, 0.46f, 0.10f));
        line(-12, -10, -17, -16, 1.6f, Col(0.55f, 0.36f, 0.10f));
        break;
    case 2: // feather
        glow(0, 0, 24, Col(0.8f, 0.85f, 1.0f, 0.5f * glowAmt * pulse));
        leafShape(-14, -14, 36, 13, 45, Col(0.84f, 0.88f, 1.0f));
        leafShape(-14, -14, 30, 7, 45, Col(0.62f, 0.72f, 0.98f));
        line(-16, -16, 12, 12, 1.4f, Col(0.45f, 0.52f, 0.80f));
        line(-16, -16, -21, -21, 1.8f, Col(0.95f, 0.95f, 1.0f));
        break;
    case 3: // golden key
        glow(0, 0, 24, Col(1.0f, 0.88f, 0.35f, 0.55f * glowAmt * pulse));
        ring(-10, 6, 3.3f, 7.5f, Col(1.0f, 0.80f, 0.26f));
        rect(-3, 4.4f, 17, 7.6f, Col(1.0f, 0.80f, 0.26f));
        rect(11, -3, 14, 4.4f, Col(1.0f, 0.80f, 0.26f));
        rect(15, -1, 18, 4.4f, Col(1.0f, 0.80f, 0.26f));
        break;
    default: // star fragment
        glow(0, 0, 30, Col(1.0f, 0.95f, 0.5f, 0.65f * glowAmt * pulse));
        star(0, 0, 13, 5.5f, 5, t * 0.3f * 0 + 0.2f, Col(1.0f, 0.88f, 0.30f));
        star(0, 0, 8, 3.2f, 5, 0.2f, Col(1.0f, 0.97f, 0.65f));
        line(-3, -10, 2, -3, 1.2f, Col(0.80f, 0.55f, 0.10f));
        break;
    }
    glPopMatrix();
}

} // namespace obj
