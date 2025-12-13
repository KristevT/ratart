#include "CircleTool.hpp"
#include <cmath>

extern std::vector<struct CanvasStroke> g_CanvasStrokes;
extern struct CanvasStroke* g_CurrentStroke;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

static constexpr int CIRCLE_SEGMENTS = 64;

void CircleTool::OnMouseDown(Vector2 pos) {
    dragging = true;
    center = edge = pos;
}

void CircleTool::OnMouseHold(Vector2 pos) {
    if (dragging)
        edge = pos;
}

void CircleTool::OnMouseUp(Vector2 pos) {
    if (!dragging) return;
    dragging = false;
    edge = pos;

    float dx = edge.x - center.x;
    float dy = edge.y - center.y;
    float radius = sqrtf(dx*dx + dy*dy);

    CanvasStroke stroke;
    stroke.color = color;
    stroke.size = thickness;
    stroke.erased = false;

    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        float a = (2 * PI * i) / CIRCLE_SEGMENTS;
        stroke.points.push_back({
            center.x + cosf(a) * radius,
            center.y + sinf(a) * radius
        });
    }

    g_CanvasStrokes.push_back(stroke);
}

void CircleTool::DrawPreview(Vector2 /*mouse*/) {
    if (!dragging) return;

    float dx = edge.x - center.x;
    float dy = edge.y - center.y;
    float radius = sqrtf(dx*dx + dy*dy);

    DrawCircleLines(center.x, center.y, radius, Fade(color, 0.7f));
}

void CircleTool::DrawUI(int x, int y) {
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
