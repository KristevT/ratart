// PencilTool.cpp
#include "PencilTool.hpp"

extern std::vector<struct CanvasStroke> g_CanvasStrokes;
extern struct CanvasStroke* g_CurrentStroke;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

void PencilTool::OnMouseDown(Vector2 pos) {
    g_CanvasStrokes.push_back(CanvasStroke());
    g_CurrentStroke = &g_CanvasStrokes.back();
    g_CurrentStroke->color = color;
    g_CurrentStroke->size = size;
    g_CurrentStroke->points.push_back(pos);
}

void PencilTool::OnMouseHold(Vector2 pos) {
    if (g_CurrentStroke)
        g_CurrentStroke->points.push_back(pos);
}

void PencilTool::OnMouseUp(Vector2 /*pos*/) {
    g_CurrentStroke = nullptr;
}

void PencilTool::Draw() {}

void PencilTool::DrawPreview(Vector2 mouse) {
    DrawCircleLines(mouse.x, mouse.y, size / 2.0f, GRAY);
}

void PencilTool::DrawUI(int x, int y) {
    DrawText(TextFormat("Size: %dpx", (int)size), x, y, 16, BLACK);
    int sliderW = 100;
    int sliderH = 15;

    DrawRectangle(x, y + 20, sliderW, sliderH, LIGHTGRAY);

    float t = (size - 1) / (100 - 1);
    int handleX = x + (int)(t * sliderW);
    DrawRectangle(handleX - 3, y + 20, 6, sliderH, BLACK);

    Vector2 m = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
        m.y >= y + 20 && m.y <= y + 20 + sliderH &&
        m.x >= x && m.x <= x + sliderW)
    {
        float newT = (m.x - x) / (float)sliderW;
        size = 1 + newT * (100 - 1);
    }

    if (IsKeyDown(KEY_LEFT_BRACKET) && size > 1) size -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && size < 100) size += 0.25f;
}
