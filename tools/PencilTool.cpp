//PencilTool.cpp
#include "PencilTool.hpp"
#include <raylib.h>

void PencilTool::OnMouseDown(Vector2 pos) {
    strokes.push_back(Stroke());
    currentStroke = &strokes.back();
    currentStroke->color = color;
    currentStroke->size = size;
    currentStroke->points.push_back(pos);
}

void PencilTool::OnMouseHold(Vector2 pos) {
    if (currentStroke) {
        currentStroke->points.push_back(pos);
    }
}

void PencilTool::OnMouseUp(Vector2 pos) {
    currentStroke = nullptr;
}

void PencilTool::Draw() {
    for (const auto& stroke : strokes) {
        const auto& pts = stroke.points;
        if (pts.size() < 2) continue;

        for (size_t i = 1; i < pts.size(); i++) {
            Vector2 p0 = pts[i - 1];
            Vector2 p1 = pts[i];

            float s = stroke.size;
            Color c = stroke.color;

            DrawCircleV(p0, s / 2.0f, c);
            DrawCircleV(p1, s / 2.0f, c);
            DrawLineEx(p0, p1, s, c);
        }
    }
}

void PencilTool::DrawUI(int x, int y) {
    DrawText(TextFormat("Size: %dpx", (int)size), x, y, 16, BLACK);
    DrawRectangle(x, y + 20, 100, 15, LIGHTGRAY);
    DrawRectangle(x, y + 20, (int)size, 15, BLACK);

    if (IsKeyDown(KEY_LEFT_BRACKET) && size > 1) size -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && size < 100) size += 0.25f;
}
