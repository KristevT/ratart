#include "SquareTool.hpp"
#include <algorithm>
#include <cmath>

extern std::vector<struct CanvasStroke> g_CanvasStrokes;
extern struct CanvasStroke* g_CurrentStroke;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

static bool IsPerfectKeyDown() {
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

static Rectangle MakeSquareRect(Vector2 a, Vector2 b) {
    float w = b.x - a.x;
    float h = b.y - a.y;
    float s = std::min(fabsf(w), fabsf(h));

    return {
        a.x,
        a.y,
        s * (w < 0 ? -1.f : 1.f),
        s * (h < 0 ? -1.f : 1.f)
    };
}

void SquareTool::OnMouseDown(Vector2 pos) {
    dragging = true;
    start = end = pos;
}

void SquareTool::OnMouseHold(Vector2 pos) {
    if (dragging)
        end = pos;
}

void SquareTool::OnMouseUp(Vector2 pos) {
    if (!dragging) return;
    dragging = false;
    end = pos;

    Vector2 a = start;
    Vector2 b = end;

    if (IsPerfectKeyDown()) {
        Rectangle r = MakeSquareRect(start, end);
        b = { r.x + r.width, r.y + r.height };
    }

    float half = thickness * 0.5f;

    float x1 = std::min(a.x, b.x) + half;
    float y1 = std::min(a.y, b.y) + half;
    float x2 = std::max(a.x, b.x) - half;
    float y2 = std::max(a.y, b.y) - half;

    if (x2 <= x1 || y2 <= y1) return;

    CanvasStroke stroke;
    stroke.color = color;
    stroke.size = thickness;
    stroke.erased = false;

    stroke.points = {
        {x1, y1},
        {x2, y1},
        {x2, y2},
        {x1, y2},
        {x1, y1}
    };

    g_CanvasStrokes.push_back(stroke);
}

void SquareTool::DrawPreview(Vector2 /*mouse*/) {
    if (!dragging) return;

    Vector2 a = start;
    Vector2 b = end;

    if (IsPerfectKeyDown()) {
        Rectangle r = MakeSquareRect(start, end);
        b = { r.x + r.width, r.y + r.height };
    }

    float half = thickness * 0.5f;

    float x = std::min(a.x, b.x) + half;
    float y = std::min(a.y, b.y) + half;
    float w = fabsf(b.x - a.x) - thickness;
    float h = fabsf(b.y - a.y) - thickness;

    if (w <= 0 || h <= 0) return;

    DrawRectangleLinesEx({x, y, w, h}, 1, Fade(color, 0.7f));
}

void SquareTool::DrawUI(int x, int y) {
    DrawText(TextFormat("Thickness: %dpx", (int)thickness), x, y, 16, BLACK);

    int w = 100, h = 15;
    DrawRectangle(x, y + 20, w, h, LIGHTGRAY);

    float t = (thickness - 1) / (50 - 1);
    int hx = x + (int)(t * w);
    DrawRectangle(hx - 3, y + 20, 6, h, BLACK);

    Vector2 m = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
        m.x >= x && m.x <= x + w &&
        m.y >= y + 20 && m.y <= y + 20 + h)
    {
        thickness = 1 + ((m.x - x) / w) * (50 - 1);
    }

    if (IsKeyDown(KEY_LEFT_BRACKET) && thickness > 1) thickness -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && thickness < 100) thickness += 0.25f;
}
