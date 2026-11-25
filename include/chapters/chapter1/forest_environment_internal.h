// ============================================================
// forest_environment_internal.h - state shared between the three
// forest environment files (environment / trees / water).
// Not part of Chapter 1's public interface.
// ============================================================
#ifndef FOREST_ENVIRONMENT_INTERNAL_H
#define FOREST_ENVIRONMENT_INTERNAL_H

#include <vector>
#include "shared/paper.h"

namespace env
{
    extern const pl::Col HAZE;                         // distant-haze tint
    pl::Col hazeMix(const pl::Col& c, float lightness); // 0 = full color, 1 = hazy

    struct TreeDef { float x, y, s; int kind; float fold; bool folding; };
    extern std::vector<TreeDef> g_trees;
    extern float g_wfBoost;                            // waterfall click boost
}

#endif
