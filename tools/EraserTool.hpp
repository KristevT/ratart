// EraserTool.hpp
#pragma once
#include "Tool.hpp"
#include <vector>

struct EraseStroke {
    std::vector<Vector2> points;
    float size;
};

class EraserTool : public Tool {
public:
    float size = 20.0f;

    std::vector<EraseStroke> strokes;
    EraseStroke* currentStroke = nullptr;

    void OnMouseDown(Vector2 pos) override;
    void OnMouseHold(Vector2 pos) override;
    void OnMouseUp(Vector2 /*pos*/) override;

    void Draw() override;
    void DrawUI(int x, int y) override;
    void DrawPreview(Vector2 mouse) override;
};
