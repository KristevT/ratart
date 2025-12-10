// EraserTool.cpp
#include "EraserTool.hpp"

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

extern std::vector<CanvasStroke> g_CanvasStrokes;

void EraserTool::OnMouseDown(Vector2 pos) { OnMouseHold(pos); }
void EraserTool::OnMouseUp(Vector2 /*pos*/) {}

void EraserTool::OnMouseHold(Vector2 pos) {
    std::vector<CanvasStroke> newStrokeList;

    for (auto& stroke : g_CanvasStrokes) {
        std::vector<Vector2> buffer;

        for (auto& p : stroke.points) {
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
}

void EraserTool::Draw() {}

void EraserTool::DrawPreview(Vector2 mouse) {
    DrawCircleLines(mouse.x, mouse.y, size, GRAY);
}

void EraserTool::DrawUI(int x, int y) {
    DrawText(TextFormat("Size: %dpx", (int)size), x, y, 16, BLACK);
    DrawRectangle(x, y + 20, 100, 15, LIGHTGRAY);
    DrawRectangle(x, y + 20, (int)size, 15, BLACK);

    if (IsKeyDown(KEY_LEFT_BRACKET) && size > 1) size -= 0.25f;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && size < 150) size += 0.25f;
}
