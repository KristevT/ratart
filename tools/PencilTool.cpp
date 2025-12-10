// PencilTool.cpp
#include "PencilTool.hpp"

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

extern std::vector<CanvasStroke> g_CanvasStrokes;
extern CanvasStroke* g_CurrentStroke;

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
    DrawRectangle(x, y + 20, 100, 15, LIGHTGRAY);
    DrawRectangle(x, y + 20, (int)size, 15, BLACK);

    if (IsKeyDown(KEY_LEFT_BRACKET) && size > 1) size -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && size < 100) size += 0.25f;
}
