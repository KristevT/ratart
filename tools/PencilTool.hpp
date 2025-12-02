//PencilTool.hp
#pragma once
#include "Tool.hpp"
#include <vector>

struct Stroke {
    std::vector<Vector2> points;
    Color color;
    float size;
};

class PencilTool : public Tool {
public:
    Color color = BLACK;
    float size = 5.0f;

    std::vector<Stroke> strokes;
    Stroke* currentStroke = nullptr;

    void OnMouseDown(Vector2 pos) override;
    void OnMouseHold(Vector2 pos) override;
    void OnMouseUp(Vector2 pos) override;

    void Draw() override;
    void DrawUI(int x, int y) override;

    void SetColor(const Color& c) override { color = c; }
};
