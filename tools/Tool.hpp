//Tool.hpp
#pragma once
#include <raylib-cpp.hpp>

class Tool {
public:
    virtual ~Tool() = default;

    virtual void OnMouseDown(Vector2 pos) = 0;
    virtual void OnMouseHold(Vector2 pos) = 0;
    virtual void OnMouseUp(Vector2 pos) = 0;

    virtual void Draw() = 0;
    virtual void DrawUI(int x, int y) {}

    virtual void SetColor(const ::Color& color) { (void)color; }
};