// ============================================================
// characters.cpp - Zone 5: the living forest
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

#include "chapters/chapter1/forest_characters.h"
#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_layout.h"
#include "shared/audio.h"

using namespace pl;

namespace chara
{
// ------------------------------------------------------------
// shared colors
// ------------------------------------------------------------
static const Col FOX(0.95f, 0.52f, 0.18f);
static const Col FOX_D(0.80f, 0.38f, 0.12f);
static const Col CREAM(1.0f, 0.96f, 0.86f);
static const Col DARK(0.22f, 0.12f, 0.08f);


// ------------------------------------------------------------
// FLICK THE FOX
// ------------------------------------------------------------
static void foxLeg(float hx, float swingDeg, bool isFar)
{
    glPushMatrix();
    glTranslatef(hx, 19, 0);
    glRotatef(swingDeg, 0, 0, 1);
    Col leg = isFar ? FOX_D : FOX;
    rect(-3.2f, -19, 3.2f, 0, leg);
    rect(-3.2f, -19, 3.2f, -10, isFar ? Col(0.28f, 0.16f, 0.10f) : DARK);   // dark sock
    ellipse(1.2f, -18.5f, 5.2f, 2.6f, isFar ? Col(0.28f, 0.16f, 0.10f) : DARK);
    glPopMatrix();
}

void drawFlick(float x, float y, float s, int dir, float walk, float jump, float talk)
{
    float t = now();
    // ground shadow
    float sh = clamp01(1.0f - jump / 160.0f);
    ellipse(x, y - 1, 36 * s * (0.5f + 0.5f * sh), 6 * s, Col(0.10f, 0.20f, 0.08f, 0.22f * sh + 0.05f));

    float moving = walk != 0.0f ? 1.0f : 0.0f;
    float bob = moving * std::fabs(std::sin(walk * 2.0f)) * 2.5f;

    glPushMatrix();
    glTranslatef(x, y + jump, 0);
    glScalef(dir * s, s, 1);

    // tail (behind the body): chain of fluffy circles + white tip
    float wag = std::sin(t * (moving ? 7.0f : 2.6f)) * (moving ? 7.0f : 10.0f) + (jump > 2 ? 18.0f : 0.0f);
    glPushMatrix();
    glTranslatef(-26, 26 + bob, 0);
    glRotatef(wag, 0, 0, 1);
    paperShadow(2, -2, 0.2f);
    for (int i = 0; i < 7; i++)
    {
        float u = i / 6.0f;
        float tx = -u * 38.0f, ty = u * 30.0f + std::sin(u * PI) * 6.0f;
        float r = 7.0f + 10.5f * std::sin(clamp01(u * 1.05f) * PI * 0.9f) + 2.0f;
        circle(tx, ty, r, u > 0.74f ? CREAM : (u > 0.45f ? FOX : FOX_D));
    }
    noShadow();
    glPopMatrix();

    // far legs
    float ph = walk;
    foxLeg(-17, std::sin(ph + PI) * 28 * moving - jump * 0.2f, true);
    foxLeg(15, std::sin(ph) * 28 * moving + jump * 0.25f, true);

    // body
    paperShadow(3, -3, 0.22f);
    ellipse(0, 28 + bob, 31, 15.5f, FOX);
    noShadow();
    ellipse(2, 19 + bob, 24, 7, CREAM);
    ellipse(-8, 36 + bob, 17, 6, Col(1, 1, 1, 0.13f));
    ellipse(22, 25 + bob, 10, 11, CREAM);                       // white chest

    // near legs
    foxLeg(-9, std::sin(ph) * 28 * moving - jump * 0.2f, false);
    foxLeg(23, std::sin(ph + PI) * 28 * moving + jump * 0.25f, false);

    // head
    float headTilt = std::sin(t * 1.3f) * 2.5f + talk * std::sin(t * 13.0f) * 3.5f - (jump > 2 ? 8.0f : 0.0f);
    glPushMatrix();
    glTranslatef(36, 44 + bob, 0);
    glRotatef(headTilt, 0, 0, 1);
    // ears
    float twitch = std::sin(t * 5.0f) > 0.97f ? 8.0f : 0.0f;
    tri(-11, 8, -3, 29 + twitch * 0.2f, 6, 11, FOX_D);
    tri(-8, 10, -3, 22, 2, 11, Col(0.30f, 0.17f, 0.10f));
    tri(2, 9, 12, 29, 15, 7, FOX);
    tri(5, 10, 11, 22, 13, 9, Col(0.36f, 0.20f, 0.12f));
    paperShadow(2, -2, 0.2f);
    ellipse(0, 0, 16, 13.5f, FOX);
    noShadow();
    // white cheeks
    float ch[] = { -9, -4, 8, -11, 22, -9, 12, -2, -2, 1 };
    poly(ch, 5, CREAM);
    // snout
    tri(8, 8, 8, -9, 33, -3, FOX);
    tri(8, -9, 33, -3, 14, -5, CREAM);
    circle(33, -3, 3.0f, DARK);
    // eye with blink
    float blink = std::fmod(t, 4.3f) > 4.15f ? 0.12f : 1.0f;
    ellipse(11, 4, 2.7f, 3.0f * blink, DARK);
    circle(11.8f, 5.2f, 0.9f * blink, Col(1, 1, 1));
    // mouth when talking
    if (talk > 0.05f)
    {
        float open = std::fabs(std::sin(t * 13.0f)) * talk;
        ellipse(21, -6.5f, 5.0f, 1.0f + 3.0f * open, Col(0.35f, 0.10f, 0.08f));
    }
    else line(16, -6.2f, 25, -6.0f, 1.0f, Col(0.35f, 0.15f, 0.10f, 0.7f));
    glPopMatrix();

    glPopMatrix();
}

// ------------------------------------------------------------
// PIP THE RABBIT
// ------------------------------------------------------------
void drawPip(float x, float y, float s, int dir, float hop, float jump, float talk)
{
    float t = now();
    float sh = clamp01(1.0f - jump / 90.0f);
    ellipse(x, y - 1, 24 * s * (0.55f + 0.45f * sh), 5 * s, Col(0.10f, 0.20f, 0.08f, 0.22f * sh + 0.05f));

    // squash on landing / stretch in the air
    float stretch = jump > 3 ? 1.12f : 1.0f - 0.10f * hop;
    float squash = jump > 3 ? 0.90f : 1.0f + 0.10f * hop;

    Col fur(0.97f, 0.95f, 0.92f), furD(0.84f, 0.80f, 0.78f), pink(1.0f, 0.70f, 0.78f);

    glPushMatrix();
    glTranslatef(x, y + jump, 0);
    glScalef(dir * s * squash, s * stretch, 1);

    // tail
    circle(-19, 17, 7.5f, Col(1, 1, 1));
    // hind foot + haunch
    paperShadow(2, -2, 0.2f);
    ellipse(-5, 18, 17, 15, fur);
    noShadow();
    ellipse(-8, 21, 11, 9, furD);
    ellipse(3, 3.5f, 14, 5, jump > 3 ? furD : fur);
    // front paws
    ellipse(14, 4, 5, 3.4f, fur);
    // body front
    paperShadow(2, -2, 0.2f);
    ellipse(8, 22, 14, 17, fur);
    noShadow();
    ellipse(11, 16, 8, 9, Col(1.0f, 0.98f, 0.95f));

    // head
    float bob = std::sin(t * 2.0f) * 0.8f;
    glPushMatrix();
    glTranslatef(15, 40 + bob, 0);
    glRotatef(talk * std::sin(t * 14.0f) * 3.0f, 0, 0, 1);
    // ears: long, with a lazy flick
    float flick = std::sin(t * 1.7f) > 0.94f ? 14.0f : 0.0f;
    glPushMatrix(); glTranslatef(-3, 8, 0); glRotatef(18 - 4 + flick, 0, 0, 1);
    ellipse(0, 18, 5.2f, 19, furD);
    ellipse(0, 18, 3.0f, 15, pink);
    glPopMatrix();
    glPushMatrix(); glTranslatef(5, 9, 0); glRotatef(-12 + flick * 0.5f, 0, 0, 1);
    ellipse(0, 18, 5.2f, 19, fur);
    ellipse(0, 18, 3.0f, 15, pink);
    glPopMatrix();
    paperShadow(2, -2, 0.2f);
    circle(0, 0, 12.5f, fur);
    noShadow();
    // cheeks + muzzle
    circle(4, -5, 6.5f, Col(1, 0.99f, 0.97f));
    circle(-4, -2, 3.2f, Col(1.0f, 0.78f, 0.82f, 0.7f));
    // nose twitch
    float nz = 1.0f + 0.25f * std::sin(t * 9.0f);
    tri(10, -1, 15.5f, 0.5f, 11, 2.5f * nz, pink);
    float blink = std::fmod(t, 5.1f) > 4.95f ? 0.1f : 1.0f;
    ellipse(6.5f, 4.5f, 2.2f, 2.8f * blink, DARK);
    circle(7.2f, 5.6f, 0.8f * blink, Col(1, 1, 1));
    // whiskers
    line(11, -1, 22, 2, 0.9f, Col(0.5f, 0.45f, 0.45f, 0.7f));
    line(11, -2.5f, 22, -3.5f, 0.9f, Col(0.5f, 0.45f, 0.45f, 0.7f));
    if (talk > 0.05f)
        ellipse(12, -6, 2.2f, 0.8f + 2.0f * std::fabs(std::sin(t * 14.0f)) * talk, Col(0.55f, 0.2f, 0.25f));
    glPopMatrix();

    glPopMatrix();
}

// ------------------------------------------------------------
// THE DEER
// ------------------------------------------------------------
static void deerLeg(float hx, float swingDeg)
{
    glPushMatrix();
    glTranslatef(hx, 58, 0);
    glRotatef(swingDeg, 0, 0, 1);
    rect(-2.8f, -58, 2.8f, 0, Col(0.74f, 0.52f, 0.31f));
    rect(-2.8f, -58, 2.8f, -52, DARK);
    ellipse(0.5f, -57.5f, 4.2f, 2.4f, DARK);
    glPopMatrix();
}

void drawDeer(float x, float y, float s, int dir, float walk, float headDown,
              float lookDeg, float talk)
{
    float t = now();
    ellipse(x, y - 1, 42 * s, 7 * s, Col(0.10f, 0.20f, 0.08f, 0.24f));
    float moving = walk != 0.0f ? 1.0f : 0.0f;
    float bob = moving * std::fabs(std::sin(walk * 2)) * 1.6f;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(dir * s, s, 1);

    Col coat(0.82f, 0.60f, 0.36f), coatD(0.70f, 0.48f, 0.28f);

    // far legs
    deerLeg(-22, std::sin(walk + PI) * 22 * moving);
    deerLeg(22, std::sin(walk) * 22 * moving);

    // tail
    tri(-33, 66 + bob, -45, 78 + bob, -30, 80 + bob, CREAM);

    // body
    paperShadow(3, -3, 0.2f);
    ellipse(0, 70 + bob, 38, 20, coat);
    noShadow();
    ellipse(2, 61 + bob, 30, 8.5f, CREAM);
    ellipse(-10, 78 + bob, 22, 6, Col(1, 1, 1, 0.13f));
    for (int i = 0; i < 6; i++)
        circle(-24 + i * 9.5f, 76 + (i % 2) * 5 + bob, 2.3f, Col(1, 0.97f, 0.88f, 0.9f));

    // near legs
    deerLeg(-14, std::sin(walk) * 22 * moving);
    deerLeg(29, std::sin(walk + PI) * 22 * moving);

    // neck + head (head rotates from the shoulder)
    float down = clamp01(headDown);
    float theta = -30.0f - 95.0f * down + lookDeg * 0.6f + std::sin(t * 0.9f) * 1.2f;
    float headRot = -10.0f - 50.0f * down + lookDeg * 0.5f - theta;
    glPushMatrix();
    glTranslatef(26, 78 + bob, 0);
    glRotatef(theta, 0, 0, 1);
    float neck[] = { 0, 0, 2, 18, 4, 36 };
    ribbonTaper(neck, 3, 24, 13, coat);
    glTranslatef(4, 36, 0);
    glRotatef(headRot, 0, 0, 1);
    // antlers
    float tw = std::sin(t * 4.0f) > 0.96f ? 6.0f : 0.0f;
    Col ant(0.52f, 0.36f, 0.20f);
    float a1[] = { -3, 8, -8, 22, -4, 36 };
    float a1b[] = { -7, 21, -15, 28 };
    float a2[] = { 5, 8, 9, 23, 6, 38 };
    float a2b[] = { 8, 23, 16, 30 };
    ribbonTaper(a1, 3, 3.6f, 1.6f, ant); ribbonTaper(a1b, 2, 2.6f, 1.2f, ant);
    ribbonTaper(a2, 3, 3.6f, 1.6f, ant); ribbonTaper(a2b, 2, 2.6f, 1.2f, ant);
    // ears
    glPushMatrix(); glTranslatef(-6, 6, 0); glRotatef(40 + tw, 0, 0, 1);
    leafShape(0, 0, 18, 9, 0, coatD); leafShape(2, 0, 13, 5, 0, Col(1.0f, 0.78f, 0.78f)); glPopMatrix();
    glPushMatrix(); glTranslatef(5, 7, 0); glRotatef(-5 - tw, 0, 0, 1);
    leafShape(0, 0, 18, 9, 0, coat); leafShape(2, 0, 13, 5, 0, Col(1.0f, 0.78f, 0.78f)); glPopMatrix();
    paperShadow(2, -2, 0.2f);
    ellipse(0, 0, 13, 10.5f, coat);
    noShadow();
    ellipse(15, -4, 11, 7.5f, Col(0.90f, 0.72f, 0.50f));       // muzzle
    circle(24, -3, 2.8f, DARK);                                  // nose
    float blink = std::fmod(t, 3.7f) > 3.58f ? 0.1f : 1.0f;
    ellipse(5, 3, 2.6f, 3.0f * blink, DARK);
    circle(5.8f, 4.2f, 0.9f * blink, Col(1, 1, 1));
    if (talk > 0.05f)
        ellipse(18, -8.5f, 4.5f, 0.8f + 2.2f * std::fabs(std::sin(t * 12.0f)) * talk, Col(0.35f, 0.10f, 0.08f));
    glPopMatrix();

    glPopMatrix();
}

// ------------------------------------------------------------
// BUTTERFLY / BIRD
// ------------------------------------------------------------
void drawButterflyShape(float x, float y, float s, float flap, const Col& wing)
{
    float f = 0.25f + 0.75f * std::fabs(flap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);
    Col wd = shade(wing, 0.8f);
    for (int side = -1; side <= 1; side += 2)
    {
        glPushMatrix();
        glScalef(side * f, 1, 1);
        paperShadow(1.5f, -1.5f, 0.2f);
        leafShape(1, 1, 15, 14, 40, wing);
        leafShape(1, -1, 11, 10, -45, wd);
        noShadow();
        circle(8, 6, 2.4f, Col(1, 1, 1, 0.85f));
        circle(5, -6, 1.7f, Col(1, 1, 0.8f, 0.85f));
        glPopMatrix();
    }
    ellipse(0, 0, 1.6f, 7, Col(0.25f, 0.15f, 0.12f));
    line(0, 6, -3, 12, 0.8f, Col(0.25f, 0.15f, 0.12f));
    line(0, 6, 3, 12, 0.8f, Col(0.25f, 0.15f, 0.12f));
    glPopMatrix();
}

void drawBirdShape(float x, float y, float s, int dir, float flap)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(dir * s, s, 1);
    Col body(0.98f, 0.97f, 0.95f), wing(0.55f, 0.68f, 0.92f), wingN(0.40f, 0.55f, 0.88f);
    leafShape(-1, 1, 15, 7, 20 + 85 * flap, shade(wing, 0.85f));      // far wing
    tri(-6, 1, -15, 4, -14, -2, wingN);                              // tail
    paperShadow(1.5f, -1.5f, 0.18f);
    ellipse(0, 0, 7, 4.2f, body);
    circle(6, 1.8f, 3.1f, body);
    noShadow();
    tri(8.5f, 2.4f, 12.5f, 1.0f, 8.5f, 0.2f, Col(1.0f, 0.65f, 0.2f));
    circle(6.9f, 2.5f, 0.8f, DARK);
    leafShape(-2, 1, 16, 8, 20 + 85 * flap, wing);                    // near wing
    glPopMatrix();
}

// ------------------------------------------------------------
// forest state
// ------------------------------------------------------------
struct Fox { float x, targetX, y, vy, jump, walk; int dir; bool hasTarget; };
struct Rabbit { float x, y, jump, vy; int dir; int hopsLeft; float hopT, hop; float homeX; };
struct Deer { float x, y; int dir; float lookT, grazeT, nextGraze, walk; };
struct Butterfly { float px, py, cx, cy, ph, speed; Col c; int state; float t, vx, vy; };
struct Bird { float ox, oy, ph; };

static Fox g_fox;
static Rabbit g_rabbit;
static Deer g_deer;
static Butterfly g_bf[3];
static std::vector<Bird> g_birds;
static float g_flockX = 0, g_flockY = 440, g_flockBoost = 0, g_flockSpeed = 24;
static float g_arrive = 0.0f;                       // current arrival amount
static float g_birdAmt = 0.0f, g_bfAmt = 0.0f;

float flickHomeX() { return 190.0f; }

void initCharacters()
{
    g_fox.x = 190; g_fox.targetX = 190; g_fox.y = 150; g_fox.vy = 0; g_fox.jump = 0;
    g_fox.walk = 0; g_fox.dir = 1; g_fox.hasTarget = false;

    g_rabbit.x = 675; g_rabbit.y = 136; g_rabbit.jump = 0; g_rabbit.vy = 0; g_rabbit.dir = -1;
    g_rabbit.hopsLeft = 0; g_rabbit.hopT = 0; g_rabbit.hop = 0; g_rabbit.homeX = 675;

    g_deer.x = 850; g_deer.y = 134; g_deer.dir = -1; g_deer.lookT = 0; g_deer.grazeT = 0;
    g_deer.nextGraze = 6.0f; g_deer.walk = 0;

    static const Col bc[3] = { Col(0.98f, 0.45f, 0.62f), Col(0.62f, 0.45f, 0.95f), Col(1.0f, 0.72f, 0.25f) };
    for (int i = 0; i < 3; i++)
    {
        Butterfly& b = g_bf[i];
        b.cx = 300 + i * 160 + rnd(-30, 30);
        b.cy = 330 + (i % 2) * 40 + rnd(-10, 10);
        b.px = b.cx; b.py = b.cy;
        b.ph = rnd(0, 6.28f); b.speed = rnd(0.7f, 1.1f);
        b.c = bc[i]; b.state = 0; b.t = 0; b.vx = b.vy = 0;
    }

    g_birds.clear();
    static const float off[5][2] = { { 0, 0 }, { -26, 14 }, { -26, -14 }, { -52, 28 }, { -52, -28 } };
    for (int i = 0; i < 5; i++)
    {
        Bird b; b.ox = off[i][0]; b.oy = off[i][1]; b.ph = i * 0.9f;
        g_birds.push_back(b);
    }
    g_flockX = layout::L + 150; g_flockY = 452; g_flockBoost = 0; g_flockSpeed = 24;
    g_arrive = 0; g_birdAmt = 0; g_bfAmt = 0;
}

void updateCharacters(float dt)
{
    float t = now();

    // ---------------- fox ----------------
    Fox& f = g_fox;
    if (f.jump > 0.0f || f.vy > 0.0f)
    {
        f.vy -= 900.0f * dt;
        f.jump += f.vy * dt;
        if (f.hasTarget) f.x += (f.targetX > f.x ? 1 : -1) * 110.0f * dt;
        if (f.jump <= 0.0f) { f.jump = 0.0f; f.vy = 0.0f; }
    }
    else if (f.hasTarget)
    {
        float dx = f.targetX - f.x;
        if (std::fabs(dx) < 3.0f) { f.hasTarget = false; f.walk = 0.0f; }
        else
        {
            f.dir = dx > 0 ? 1 : -1;
            f.x += f.dir * 70.0f * dt;
            f.walk += dt * 9.0f;
        }
    }

    // ---------------- rabbit ----------------
    Rabbit& r = g_rabbit;
    if (r.hopsLeft > 0 || r.jump > 0.0f || r.vy > 0.0f)
    {
        if (r.jump <= 0.0f && r.vy <= 0.0f && r.hopsLeft > 0)
        {
            r.vy = 300.0f; r.hopsLeft--;
            audio::play(audio::SFX_HOP);
        }
        r.vy -= 900.0f * dt;
        r.jump += r.vy * dt;
        r.x += r.dir * 62.0f * dt;
        // stay on the grass right of the pond (the pond rim ends near x = 630)
        if (r.x < 650) { r.x = 650; r.dir = 1; }
        if (r.x > 745) { r.x = 745; r.dir = -1; }
        if (r.jump <= 0.0f) { r.jump = 0.0f; r.vy = 0.0f; r.hop = 1.0f; }
    }
    r.hop *= (1.0f - clamp01(dt * 8.0f));

    // ---------------- deer ----------------
    Deer& d = g_deer;
    if (d.lookT > 0.0f) d.lookT -= dt;
    else
    {
        d.nextGraze -= dt;
        if (d.nextGraze <= 0.0f) { d.grazeT = 3.2f; d.nextGraze = rnd(7.0f, 12.0f); }
    }
    if (d.grazeT > 0.0f) d.grazeT -= dt;

    // ---------------- butterflies ----------------
    for (int i = 0; i < 3; i++)
    {
        Butterfly& b = g_bf[i];
        b.t += dt;
        float tx = b.cx + std::sin(t * 0.6f * b.speed + b.ph) * 85.0f;
        float ty = b.cy + std::cos(t * 0.9f * b.speed + b.ph * 1.7f) * 32.0f;
        if (b.state == 0)
        {
            float k = clamp01(dt * 2.0f);
            b.px += (tx - b.px) * k;
            b.py += (ty - b.py) * k;
        }
        else if (b.state == 1)                // fleeing
        {
            b.vx += 260.0f * dt; b.vy += 200.0f * dt;
            b.px += b.vx * dt; b.py += b.vy * dt;
            if (b.px > layout::R + 30 || b.py > layout::T + 30)
            {
                b.state = 2; b.t = 0;
            }
        }
        else                                  // away, then returns from the left
        {
            if (b.t > 3.5f)
            {
                b.state = 0;
                b.px = layout::L - 20; b.py = rnd(300, 420);
            }
        }
    }

    // ---------------- birds ----------------
    if (g_flockBoost > 0.0f) g_flockBoost -= dt;
    float targetSpeed = g_flockBoost > 0.0f ? 300.0f : 24.0f;
    g_flockSpeed += (targetSpeed - g_flockSpeed) * clamp01(dt * 2.5f);
    g_flockX += g_flockSpeed * dt;
    if (g_flockBoost > 0.0f) g_flockY += 40.0f * dt;
    else g_flockY += (452.0f + std::sin(t * 0.2f) * 30.0f - g_flockY) * clamp01(dt * 0.5f);
    if (g_flockX > layout::R + 90.0f)
    {
        g_flockX = layout::L - 90.0f;
        g_flockY = rnd(430, 500);
        g_flockBoost = 0.0f;
    }
}

// ------------------------------------------------------------
// forest drawing
// ------------------------------------------------------------
void drawBirds(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    float rate = 5.0f + g_flockSpeed * 0.03f;
    for (size_t i = 0; i < g_birds.size(); i++)
    {
        const Bird& b = g_birds[i];
        float flap = std::sin(t * rate + b.ph);
        float bob = std::sin(t * 1.3f + b.ph) * 3.0f;
        drawBirdShape(g_flockX + b.ox, g_flockY + b.oy + bob, 1.1f * easeOutBack(a), 1, flap);
    }
}

void drawButterflies(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    for (int i = 0; i < 3; i++)
    {
        const Butterfly& b = g_bf[i];
        if (b.state == 2) continue;
        float rate = b.state == 1 ? 24.0f : 11.0f;
        drawButterflyShape(b.px, b.py, 1.15f * easeOutBack(a), std::sin(t * rate + b.ph), b.c);
    }
}

void drawGroundCharacters(float a)
{
    if (a <= 0.0f) return;
    float e = easeOutCubic(a);
    float t = now();

    // Fox walks in from the left, the deer from the right, the rabbit hops in.
    float foxOff = (1.0f - e) * -300.0f;
    float deerOff = (1.0f - e) * 260.0f;
    float rabOff = (1.0f - e) * 240.0f;
    bool arriving = a < 0.999f;
    g_arrive = a;

    // back to front: the deer (y 134) / rabbit (136) / fox (150) -> nearest last
    float dwalk = arriving ? t * 7.0f : g_deer.walk;
    float dHead = g_deer.grazeT > 0.0f ? smooth(clamp01(((g_deer.grazeT < 3.2f - g_deer.grazeT) ? g_deer.grazeT : 3.2f - g_deer.grazeT) * 2.0f)) : 0.0f;
    float look = 0.0f;
    if (g_deer.lookT > 0.0f)
    {
        float u = 1.0f - g_deer.lookT / 2.6f;
        look = std::sin(u * PI * 3.0f) * 34.0f;
        dHead = 0.0f;
    }
    glPushMatrix();
    drawDeer(g_deer.x + deerOff, g_deer.y, 1.12f, g_deer.dir, dwalk * (arriving ? 1.0f : 0.0f), dHead, look, 0.0f);
    glPopMatrix();

    float rjump = g_rabbit.jump;
    if (arriving) rjump = std::fabs(std::sin(a * PI * 5.0f)) * 30.0f * (1.0f - a * 0.5f);
    drawPip(g_rabbit.x + rabOff, g_rabbit.y, 1.18f, g_rabbit.dir, g_rabbit.hop, rjump, 0.0f);

    float fwalk = arriving ? t * 9.0f : g_fox.walk;
    drawFlick(g_fox.x + foxOff, g_fox.y, 1.25f, arriving ? 1 : g_fox.dir, fwalk * (arriving ? 1.0f : (g_fox.hasTarget && g_fox.jump <= 0 ? 1.0f : 0.0f)), arriving ? 0.0f : g_fox.jump, 0.0f);
}

// ------------------------------------------------------------
// interaction
// ------------------------------------------------------------
static bool hitFox(float x, float y) { return inEllipse(x, y, g_fox.x + g_fox.dir * 8, g_fox.y + 34 + g_fox.jump, 52, 36); }
static bool hitRabbit(float x, float y) { return inEllipse(x, y, g_rabbit.x + g_rabbit.dir * 6, g_rabbit.y + 42 + g_rabbit.jump, 28, 44); }
static bool hitDeer(float x, float y) { return inEllipse(x, y, g_deer.x - g_deer.dir * 4, g_deer.y + 74, 56, 62); }

bool hover(float x, float y)
{
    if (g_arrive < 0.99f) return false;
    if (hitFox(x, y) || hitRabbit(x, y) || hitDeer(x, y)) return true;
    for (int i = 0; i < 3; i++)
        if (g_bf[i].state != 2 && inEllipse(x, y, g_bf[i].px, g_bf[i].py, 20, 16)) return true;
    for (size_t i = 0; i < g_birds.size(); i++)
        if (inEllipse(x, y, g_flockX + g_birds[i].ox, g_flockY + g_birds[i].oy, 20, 14)) return true;
    return false;
}

bool click(float x, float y)
{
    if (g_arrive < 0.99f) return false;

    // butterflies and birds sit in the air in front of the scenery
    for (int i = 0; i < 3; i++)
    {
        Butterfly& b = g_bf[i];
        if (b.state == 0 && inEllipse(x, y, b.px, b.py, 22, 18))
        {
            b.state = 1; b.vx = 60; b.vy = 40;
            fx::forestParticles().sparkles(b.px, b.py, 10, b.c, 70, 6);
            audio::play(audio::SFX_FLUTTER);
            return true;
        }
    }
    for (size_t i = 0; i < g_birds.size(); i++)
    {
        if (inEllipse(x, y, g_flockX + g_birds[i].ox, g_flockY + g_birds[i].oy, 22, 16))
        {
            g_flockBoost = 3.0f;
            audio::play(audio::SFX_FLAP);
            return true;
        }
    }

    if (hitFox(x, y))
    {
        float nx;
        int guard = 0;
        do { nx = rnd(110, 340); guard++; } while (std::fabs(nx - g_fox.x) < 80 && guard < 20);
        g_fox.targetX = nx; g_fox.hasTarget = true;
        g_fox.dir = nx > g_fox.x ? 1 : -1;
        g_fox.vy = 330.0f; g_fox.jump = 0.01f;
        fx::forestParticles().sparkles(g_fox.x, g_fox.y + 10, 8, Col(1.0f, 0.75f, 0.4f), 60, 6);
        audio::play(audio::SFX_YIP);
        return true;
    }
    if (hitRabbit(x, y))
    {
        if (g_rabbit.hopsLeft == 0 && g_rabbit.jump <= 0.0f)
        {
            g_rabbit.hopsLeft = 3;
            g_rabbit.dir = (g_rabbit.x > 700) ? -1 : 1;
        }
        return true;
    }
    if (hitDeer(x, y))
    {
        g_deer.lookT = 2.6f;
        g_deer.grazeT = 0.0f;
        fx::forestParticles().sparkles(g_deer.x, g_deer.y + 110, 6, Col(1.0f, 0.95f, 0.7f), 50, 5);
        audio::play(audio::SFX_SPARKLE);
        return true;
    }
    return false;
}

} // namespace chara
