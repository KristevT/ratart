#include "DropperTool.hpp"
#include <algorithm>
#include <cmath>

extern int g_ScreenWidth;
extern int g_ScreenHeight;
extern int toolbarWidth;
extern int menuBarHeight;

extern Color g_SelectedColor;
extern float g_SelectedHue;
extern float g_SelectedSat;
extern float g_ColorValue;

extern Image g_BackgroundImage;
extern std::vector<struct CanvasStroke> g_CanvasStrokes;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased;
};

static float DistPointSegment(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 ab = { b.x - a.x, b.y - a.y };
    Vector2 ap = { p.x - a.x, p.y - a.y };
    float abLen2 = ab.x*ab.x + ab.y*ab.y;
    float t = (abLen2 > 0) ? (ap.x*ab.x + ap.y*ab.y) / abLen2 : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);
    Vector2 c = { a.x + ab.x*t, a.y + ab.y*t };
    float dx = p.x - c.x;
    float dy = p.y - c.y;
    return std::sqrt(dx*dx + dy*dy);
}

static Color SampleFromStrokes(Vector2 screenPos, bool &hit) {
    hit = false;

    for (auto it = g_CanvasStrokes.rbegin(); it != g_CanvasStrokes.rend(); ++it) {
        const auto &s = *it;
        if (s.points.size() < 2 || s.erased) continue;

        for (size_t i = 1; i < s.points.size(); ++i) {
            if (DistPointSegment(screenPos, s.points[i-1], s.points[i]) <= s.size * 0.5f) {
                hit = true;
                return s.color;
            }
        }
    }

    return WHITE;
}

static Color SampleFromBackground(Vector2 screenPos, bool &hit) {
    hit = false;
    if (!g_BackgroundImage.data) return WHITE;

    int x = (int)(screenPos.x - toolbarWidth);
    int y = (int)(screenPos.y - menuBarHeight);

    if (x < 0 || y < 0 ||
        x >= g_BackgroundImage.width ||
        y >= g_BackgroundImage.height)
        return WHITE;

    unsigned char* px = (unsigned char*)g_BackgroundImage.data;
    int idx = (y * g_BackgroundImage.width + x) * 4;

    hit = true;
    return {
        px[idx],
        px[idx + 1],
        px[idx + 2],
        px[idx + 3]
    };
}

static void UpdateHSVFromColor(Color c) {
    if (c.a == 0) return;

    Vector3 hsv = ColorToHSV(c);
    g_SelectedHue  = hsv.x;
    g_SelectedSat  = hsv.y;
    g_ColorValue  = hsv.z;
    g_SelectedColor = c;
}


void DropperTool::OnMouseDown(Vector2 pos) {
    bool hitStroke = false;
    Color c = SampleFromStrokes(pos, hitStroke);

    if (!hitStroke) {
        bool hitBg = false;
        c = SampleFromBackground(pos, hitBg);
        if (!hitBg) c = WHITE;
    }

    UpdateHSVFromColor(c);
}

void DropperTool::DrawPreview(Vector2 mouse) {
    DrawCircleLines(mouse.x, mouse.y, 3, GRAY);
}
