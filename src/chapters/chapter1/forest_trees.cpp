// ============================================================
// forest_trees.cpp - Chapter 1 generic trees (unfold, click, hover)
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
// generic trees (stage 2)
// ------------------------------------------------------------
int treeCount() { return (int)g_trees.size(); }
float treeBaseY(int i) { return g_trees[i].y; }

static float foldAmount(const TreeDef& t)
{
    // folds flat then springs back up
    if (!t.folding && t.fold >= 1.0f) return 1.0f;
    float f = t.fold;
    if (f < 0.30f) return 1.0f - smooth(f / 0.30f);
    return smooth((f - 0.30f) / 0.70f);
}

void drawTreeIndex(int i, float a)
{
    if (i < 0 || i >= (int)g_trees.size()) return;
    const TreeDef& t = g_trees[i];
    float k = a * foldAmount(t);
    if (k <= 0.001f && a > 0.0f) k = 0.001f;
    if (a <= 0.0f) return;
    float tilt = (i % 2 == 0 ? -1.0f : 1.0f) * 70.0f;
    float light = clamp01((t.y - 262.0f) / 80.0f) * 0.55f;
    float sway = std::sin(now() * 1.1f + t.x * 0.03f) * 1.1f;
    if (beginPopUp(t.x, t.y, k, tilt))
    {
        glRotatef(sway, 0, 0, 1);
        creaseShadow(0, 0, 40 * t.s, k);
        if (t.kind == 0) paperPine(0, 0, t.s, light);
        else paperOak(0, 0, t.s, light);
        endPopUp();
    }
}

bool clickTree(float x, float y)
{
    for (int i = (int)g_trees.size() - 1; i >= 0; i--)
    {
        TreeDef& t = g_trees[i];
        float cy = t.y + (t.kind == 0 ? 78.0f : 90.0f) * t.s;
        float rx = (t.kind == 0 ? 40.0f : 55.0f) * t.s;
        float ry = (t.kind == 0 ? 82.0f : 62.0f) * t.s;
        if (inEllipse(x, y, t.x, cy, rx, ry) && !t.folding)
        {
            t.folding = true;
            t.fold = 0.0f;
            fx::forestParticles().sparkles(t.x, cy, 8, Col(0.8f, 1.0f, 0.6f), 70, 6);
            for (int k = 0; k < 5; k++) fx::forestParticles().leaf(t.x + rnd(-30, 30), cy + rnd(-10, 40), t.y - 20 - rnd(0, 30));
            return true;
        }
    }
    return false;
}

bool hoverTree(float x, float y)
{
    for (size_t i = 0; i < g_trees.size(); i++)
    {
        const TreeDef& t = g_trees[i];
        float cy = t.y + (t.kind == 0 ? 78.0f : 90.0f) * t.s;
        float rx = (t.kind == 0 ? 40.0f : 55.0f) * t.s;
        float ry = (t.kind == 0 ? 82.0f : 62.0f) * t.s;
        if (inEllipse(x, y, t.x, cy, rx, ry)) return true;
    }
    return false;
}

void oakCanopyPoint(float& x, float& y)
{
    const TreeDef& t = g_trees[3];
    x = t.x + 42.0f * t.s;
    y = t.y + 118.0f * t.s;
}

} // namespace env
