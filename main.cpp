//main.cpp
#include <raylib-cpp.hpp>
#include <memory>
#include <vector>
#include "tools/Tool.hpp"
#include "tools/PencilTool.hpp"

Color g_SelectedColor = BLACK;
float g_ColorValue = 1.0f;

// Draw an HSV color wheel
void DrawColorWheel(int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            float dist = sqrtf(x*x + y*y);
            if (dist <= radius) {
                float angle = atan2f(y, x);
                float hue = (angle + PI) / (2*PI) * 360.0f;
                float sat = dist / radius;
                Color c = ColorFromHSV(hue, sat, g_ColorValue);
                DrawPixel(cx + x, cy + y, c);
            }
        }
    }
}

// Detect and pick color from wheel
bool PickColorFromWheel(int cx, int cy, int radius, Color& outColor) {
    Vector2 m = GetMousePosition();
    float dx = m.x - cx;
    float dy = m.y - cy;
    float dist = sqrtf(dx*dx + dy*dy);

    if (dist > radius) return false;

    float angle = atan2f(dy, dx); 
    float hue = (angle + PI) / (2*PI) * 360.0f;
    float sat = dist / radius;

    outColor = ColorFromHSV(hue, sat, g_ColorValue);
    return true;
}

// Draw a horizontal slider for selecting a value between 0 and 1
float DrawValueSlider(int x, int y, int w, int h, float value) {
    for (int i = 0; i < w; i++) {
        float v = (float)i / w;
        Color c = ColorFromHSV(0, 0, v);
        DrawLine(x + i, y, x + i, y + h, c);
    }

    int handleX = x + value * w;
    DrawRectangle(handleX - 3, y, 6, h, BLACK);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (m.x >= x && m.x <= x + w && m.y >= y && m.y <= y + h) {
            value = (m.x - x) / w;
            if (value < 0) value = 0;
            if (value > 1) value = 1;
        }
    }

    return value;
}

struct ToolButton {
    std::string name;
    std::string iconPath;
    Rectangle rect;
    KeyboardKey shortcut;
    enum ToolID { PENCIL, ERASER, DROPPER, BUCKET, SQUARE, CIRCLE } id;
    Texture2D icon;
};

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;

    raylib::Window window(screenWidth, screenHeight, "ratart - Simple Drawing App");
    SetTargetFPS(60);

    const int menuBarHeight = 30;
    const int toolbarWidth = 150;

    // Current tool (Pencil by default)
    std::unique_ptr<Tool> currentTool = std::make_unique<PencilTool>();

    std::vector<ToolButton> toolButtons = {
        { "Pencil",  "icons/pencil.png",  {}, KEY_P, ToolButton::PENCIL },
        { "Eraser",  "icons/eraser.png",  {}, KEY_E, ToolButton::ERASER },
        { "Dropper", "icons/dropper.png", {}, KEY_I, ToolButton::DROPPER },
        { "Bucket",  "icons/bucket.png",  {}, KEY_K, ToolButton::BUCKET },
        { "Square",  "icons/square.png",  {}, KEY_S, ToolButton::SQUARE },
        { "Circle",  "icons/circle.png",  {}, KEY_C, ToolButton::CIRCLE }
    };

    // Load icons
    for (auto& b : toolButtons)
        b.icon = LoadTexture(b.iconPath.c_str());

    while (!window.ShouldClose()) {
        // --- Update logic here (placeholder) ---
        // Menu clicks, toolbar selection, drawing, etc.

        Vector2 mouse = GetMousePosition();
        // bool insideCanvas = (mouse.x >= toolbarWidth && mouse.y >= menuBarHeight); unused
        bool insideCanvas = true;
        currentTool->SetColor(g_SelectedColor);

        if (insideCanvas) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseDown(mouse);
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseHold(mouse);
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseUp(mouse);
        }

        for (auto& b : toolButtons) {
            if (IsKeyPressed(b.shortcut)) {
                switch (b.id) {
                    case ToolButton::PENCIL: currentTool = std::make_unique<PencilTool>(); break;
                    case ToolButton::ERASER: /* create EraserTool */ break;
                    case ToolButton::DROPPER: /* create DropperTool */ break;
                    case ToolButton::BUCKET: /* create BucketTool */ break;
                    case ToolButton::SQUARE: /* create RectangleTool */ break;
                    case ToolButton::CIRCLE: /* create CircleTool */ break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // DRAWING SURFACE
        DrawRectangle(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, RAYWHITE);
        DrawRectangleLines(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, LIGHTGRAY);

        currentTool->Draw();

        // MENU BAR
        DrawRectangle(0, 0, screenWidth, menuBarHeight, LIGHTGRAY);
        DrawText("File  Edit  Canvas  Undo  Redo", 10, 7, 16, BLACK);

        // TOOL BAR
        DrawRectangle(0, menuBarHeight, toolbarWidth, screenHeight - menuBarHeight, ColorFromHSV(0, 0, 25));
        DrawRectangle(toolbarWidth, menuBarHeight, 1, screenHeight - menuBarHeight, ColorFromHSV(0, 0, 50));

        // Color wheel
        int wheelRadius = toolbarWidth / 3;
        int wheelCx = toolbarWidth / 2;
        int wheelCy = menuBarHeight + wheelRadius + 30;

        DrawText("Color", 20, menuBarHeight + 10, 20, BLACK);
        DrawColorWheel(wheelCx, wheelCy + 10, wheelRadius);

        // Highlight current color
        DrawRectangle(wheelCx + wheelRadius, wheelCy + wheelRadius, 15, 15, g_SelectedColor);
        DrawRectangleLines(wheelCx + wheelRadius, wheelCy + wheelRadius, 15, 15, BLACK);

        // Handle picking
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Color picked;
            if (PickColorFromWheel(wheelCx, wheelCy, wheelRadius, picked))
                g_SelectedColor = picked;
        }

        // Lightness slider under wheel
        DrawText("Lightness", toolbarWidth * 0.15, wheelCy + wheelRadius + 15, 16, BLACK);
        g_ColorValue = DrawValueSlider(toolbarWidth * 0.15, wheelCy + wheelRadius + 35, toolbarWidth * 0.7, 15, g_ColorValue);

        // Divider
        int toolSectionHeight = wheelCy + wheelRadius + 60;
        DrawRectangle(toolbarWidth * 0.1, toolSectionHeight, toolbarWidth * 0.8, 1, LIGHTGRAY);

        // Tools
        DrawText("Tools", 20, toolSectionHeight + 15, 20, BLACK);

        int cols = 3;
        int spacing = 6;
        int iconSize = toolbarWidth * 0.25;

        int startX = toolbarWidth * 0.1;
        int startY = toolSectionHeight + 45;

        for (int i = 0; i < toolButtons.size(); i++) {
            int row = i / cols;
            int col = i % cols;

            ToolButton& b = toolButtons[i];
            b.rect = {
                (float)(startX + col * (iconSize + spacing)),
                (float)(startY + row * (iconSize + spacing)),
                (float)iconSize,
                (float)iconSize
            };

            // Background
            DrawRectangleRec(b.rect, ColorFromHSV(0, 0, 10));

            // Icon
            float scale = iconSize / (float)b.icon.width;
            DrawTextureEx(b.icon, { b.rect.x, b.rect.y }, 0, scale, WHITE);

            // Hover highlight
            if (CheckCollisionPointRec(mouse, b.rect))
                DrawRectangleLinesEx(b.rect, 2, YELLOW);

            // Click handler
            if (CheckCollisionPointRec(mouse, b.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                switch (b.id) {
                    case ToolButton::PENCIL: currentTool = std::make_unique<PencilTool>(); break;
                    case ToolButton::ERASER: /* create EraserTool */ break;
                    case ToolButton::DROPPER: /* create DropperTool */ break;
                    case ToolButton::BUCKET: /* create BucketTool */ break;
                    case ToolButton::SQUARE: /* create RectangleTool */ break;
                    case ToolButton::CIRCLE: /* create CircleTool */ break;
                }
            }
        }

        // Size slider UI for current tool
        currentTool->DrawUI((toolbarWidth - 100) * 0.5, toolSectionHeight + iconSize * 2 + 60);

        EndDrawing();
    }

    return 0;
}
