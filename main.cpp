#include <raylib-cpp.hpp>
#include <memory>
#include "tools/Tool.hpp"
#include "tools/PencilTool.hpp"

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;

    raylib::Window window(screenWidth, screenHeight, "Simple Drawing App (Prototype)");
    SetTargetFPS(60);

    const int menuBarHeight = 30;
    const int toolbarWidth = 150;

    // Current tool (Pencil by default)
    std::unique_ptr<Tool> currentTool = std::make_unique<PencilTool>();

    while (!window.ShouldClose()) {
        // --- Update logic here (placeholder) ---
        // Menu clicks, toolbar selection, drawing, etc.

        Vector2 mouse = GetMousePosition();
        bool insideCanvas = (mouse.x >= toolbarWidth && mouse.y >= menuBarHeight);

        if (insideCanvas) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseDown(mouse);
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseHold(mouse);
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                currentTool->OnMouseUp(mouse);
        }

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Menu Bar
        DrawRectangle(0, 0, screenWidth, menuBarHeight, LIGHTGRAY);
        DrawText("File  Edit  Canvas  Undo  Redo", 10, 7, 16, BLACK);

        // Toolbar
        DrawRectangle(0, menuBarHeight, toolbarWidth, screenHeight - menuBarHeight, Fade(GRAY, 0.3f));
        DrawText("Tools", 20, menuBarHeight + 10, 20, BLACK);

        // Pencil tool button (placeholder clickable UI)
        DrawRectangle(20, menuBarHeight + 50, 110, 40, DARKGRAY);
        DrawText("Pencil", 45, menuBarHeight + 60, 18, WHITE);

        // Let pencil draw its UI (size slider etc.)
        currentTool->DrawUI(20, menuBarHeight + 110);

        // Drawing surface
        DrawRectangle(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, RAYWHITE);
        DrawRectangleLines(toolbarWidth, menuBarHeight, screenWidth - toolbarWidth, screenHeight - menuBarHeight, LIGHTGRAY);

        currentTool->Draw();

        EndDrawing();
    }

    return 0;
}
