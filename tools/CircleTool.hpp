#pragma once
#include "Tool.hpp"

class CircleTool : public Tool {
public:
    Color color = BLACK;
    float thickness = 5.0f;

    bool dragging = false;
    Vector2 center{};
    Vector2 edge{};

    void OnMouseDown(Vector2 pos) override;
    void OnMouseHold(Vector2 pos) override;
    void OnMouseUp(Vector2 pos) override;

    void Draw() override {}
    void DrawUI(int x, int y) override;
    void DrawPreview(Vector2 mouse) override;

    void SetColor(const Color& c) override { color = c; }
};
