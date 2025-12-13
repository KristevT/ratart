#pragma once
#include "Tool.hpp"

class DropperTool : public Tool {
public:
    void OnMouseDown(Vector2 pos) override;
    void OnMouseHold(Vector2 /*pos*/) override {}
    void OnMouseUp(Vector2 /*pos*/) override {}

    void Draw() override {}
    void DrawPreview(Vector2 mouse) override;
};
