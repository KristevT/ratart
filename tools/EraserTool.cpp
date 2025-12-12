// EraserTool.cpp
#include "EraserTool.hpp"
#include <raylib-cpp.hpp>

extern void EraseBackgroundAt(const Vector2 &screenPos, float radius);
extern std::vector<struct CanvasStroke> g_CanvasStrokes;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

void EraserTool::OnMouseDown(Vector2 pos) { OnMouseHold(pos); }
void EraserTool::OnMouseUp(Vector2 /*pos*/) {}

void EraserTool::OnMouseHold(Vector2 pos) {
    std::vector<CanvasStroke> newStrokeList;

    for (auto &stroke : g_CanvasStrokes) {
        std::vector<Vector2> buffer;
        for (auto &p : stroke.points) {
            bool hit = CheckCollisionPointCircle(pos, p, size);
            if (!hit) {
                buffer.push_back(p);
            } else {
                if (buffer.size() > 1) {
                    CanvasStroke split;
                    split.color = stroke.color;
                    split.size = stroke.size;
                    split.points = buffer;
                    newStrokeList.push_back(split);
                }
                buffer.clear();
            }
        }
        if (buffer.size() > 1) {
            CanvasStroke split;
            split.color = stroke.color;
            split.size = stroke.size;
            split.points = buffer;
            newStrokeList.push_back(split);
        }
    }

    g_CanvasStrokes = newStrokeList;

    EraseBackgroundAt(pos, size);
}

void EraserTool::Draw() {}

void EraserTool::DrawPreview(Vector2 mouse) {
    DrawCircleLines(mouse.x, mouse.y, size, GRAY);
}

void EraserTool::DrawUI(int x, int y) {
    DrawText(TextFormat("Size: %dpx", (int)size), x, y, 16, BLACK);
    int sliderW = 100;
    int sliderH = 15;

    DrawRectangle(x, y + 20, sliderW, sliderH, LIGHTGRAY);

    float t = (size - 1) / (150 - 1);
    int handleX = x + (int)(t * sliderW);
    DrawRectangle(handleX - 3, y + 20, 6, sliderH, BLACK);

    Vector2 m = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
        m.y >= y + 20 && m.y <= y + 20 + sliderH &&
        m.x >= x && m.x <= x + sliderW)
    {
        float newT = (m.x - x) / (float)sliderW;
        size = 1 + newT * (150 - 1);
    }

    if (IsKeyDown(KEY_LEFT_BRACKET) && size > 1) size -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && size < 150) size += 0.25f;
}
