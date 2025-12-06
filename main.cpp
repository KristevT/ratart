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

struct MenuItem {
    std::string label;
    Rectangle rect;
};

struct MenuTab {
    std::string label;
    Rectangle rect;
    std::vector<MenuItem> items;
    bool open = false;
};

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;

    raylib::Window window(screenWidth, screenHeight, "ratart - Simple Drawing App");
    SetTargetFPS(60);

    const int menuBarHeight = 25;
    const int toolbarWidth = 150;

    // Current tool (Pencil by default)
    std::unique_ptr<PencilTool> pencilTool = std::make_unique<PencilTool>();
    // std::unique_ptr<EraserTool> eraserTool = std::make_unique<EraserTool>();
    // ...

    Tool* currentTool = pencilTool.get();

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

    std::vector<MenuTab> menu = {
        { "File",   { 0, 0, menuBarHeight * 2, menuBarHeight },
            { {"New", {}}, {"Open", {}}, {"Save", {}}, {"Save As", {}} }
        },
        { "Edit",   { menuBarHeight * 2, 0, menuBarHeight * 2, menuBarHeight },
            { {"Copy", {}}, {"Cut", {}}, {"Paste", {}} }
        },
        { "Canvas", { menuBarHeight * 4, 0, menuBarHeight * 3.5, menuBarHeight },
            { {"Change Canvas Size", {}}, {"Change Canvas BG", {}} }
        }
    };

    Rectangle undoBtn = { menuBarHeight * 7.5, 0, menuBarHeight * 2.5, menuBarHeight };
    Rectangle redoBtn = { menuBarHeight * 10, 0, menuBarHeight * 2.5, menuBarHeight };


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
                    case ToolButton::PENCIL: currentTool = pencilTool.get(); break;
                    case ToolButton::ERASER: /* currentTool = eraserTool.get(); */ break;
                    case ToolButton::DROPPER: /* currentTool = dropperTool.get(); */ break;
                    case ToolButton::BUCKET: /* currentTool = bucketTool.get(); */ break;
                    case ToolButton::SQUARE: /* currentTool = squareTool.get(); */ break;
                    case ToolButton::CIRCLE: /* currentTool = circleTool.get(); */ break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // DRAWING SURFACE
        DrawRectangle(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, RAYWHITE);
        DrawRectangleLines(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, LIGHTGRAY);

        currentTool->Draw();

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
                    case ToolButton::PENCIL: currentTool = pencilTool.get(); break;
                    case ToolButton::ERASER: /* currentTool = eraserTool.get(); */ break;
                    case ToolButton::DROPPER: /* currentTool = dropperTool.get(); */ break;
                    case ToolButton::BUCKET: /* currentTool = bucketTool.get(); */ break;
                    case ToolButton::SQUARE: /* currentTool = squareTool.get(); */ break;
                    case ToolButton::CIRCLE: /* currentTool = circleTool.get(); */ break;
                }
            }
        }

        // Size slider UI for current tool
        currentTool->DrawUI((toolbarWidth - 100) * 0.5, toolSectionHeight + iconSize * 2 + 60);

        // MENU BAR
        DrawRectangle(0, 0, screenWidth, menuBarHeight, LIGHTGRAY);

        Vector2 m = GetMousePosition();

        // Draw tabs & handle clicks
        for (auto& tab : menu) {
            Color bg = (tab.open || CheckCollisionPointRec(m, tab.rect)) ?
                    GRAY :
                    LIGHTGRAY;

            DrawRectangleRec(tab.rect, bg);
            DrawText(tab.label.c_str(), tab.rect.x + menuBarHeight * 0.3, tab.rect.y + menuBarHeight * 0.1, menuBarHeight * 0.8, BLACK);

            // Toggle dropdown
            if (CheckCollisionPointRec(m, tab.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (auto& t : menu) t.open = false;
                tab.open = true;
            }

            // Draw dropdown
            if (tab.open) {
                int itemHeight = 22;
                int menuWidth = 180;

                for (int i = 0; i < tab.items.size(); i++) {
                    MenuItem& it = tab.items[i];
                    it.rect = {
                        tab.rect.x,
                        tab.rect.y + tab.rect.height + i * itemHeight,
                        (float)menuWidth,
                        (float)itemHeight
                    };

                    Color bg2 = CheckCollisionPointRec(m, it.rect)
                                ? Color{220, 220, 220, 255}
                                : Color{245, 245, 245, 255};

                    DrawRectangleRec(it.rect, bg2);
                    DrawRectangleLinesEx(it.rect, 1, BLACK);
                    DrawText(it.label.c_str(), it.rect.x + 6, it.rect.y + 4, 16, BLACK);

                    // Placeholder functionality
                    if (CheckCollisionPointRec(m, it.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        printf("Menu clicked: %s -> %s\n", tab.label.c_str(), it.label.c_str());
                        tab.open = false;
                    }
                }
            }
        }

        // Close all menus if clicked outside
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool clickedMenu = false;

            for (auto& tab : menu) {
                if (CheckCollisionPointRec(m, tab.rect)) clickedMenu = true;
                for (auto& it : tab.items)
                    if (CheckCollisionPointRec(m, it.rect)) clickedMenu = true;
            }

            if (!clickedMenu)
                for (auto& tab : menu) tab.open = false;
        }

        // Undo / Redo
        DrawRectangleRec(undoBtn, CheckCollisionPointRec(m, undoBtn) ? GRAY : LIGHTGRAY);
        DrawRectangleRec(redoBtn, CheckCollisionPointRec(m, redoBtn) ? GRAY : LIGHTGRAY);

        DrawText("Undo", undoBtn.x + menuBarHeight * 0.3, undoBtn.y + menuBarHeight * 0.1, menuBarHeight * 0.8, BLACK);
        DrawText("Redo", redoBtn.x + menuBarHeight * 0.3, redoBtn.y + menuBarHeight * 0.1 , menuBarHeight * 0.8, BLACK);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(m, undoBtn)) {
                printf("Undo clicked\n");
                // TODO: Undo logic
            }
            if (CheckCollisionPointRec(m, redoBtn)) {
                printf("Redo clicked\n");
                // TODO: Redo logic
            }
        }

        EndDrawing();
    }

    return 0;
}
