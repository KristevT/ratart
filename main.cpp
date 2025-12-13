// main.cpp
#include <raylib-cpp.hpp>
#include "tinyfiledialogs.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <deque>
#include "tools/Tool.hpp"
#include "tools/PencilTool.hpp"
#include "tools/EraserTool.hpp"
#include "tools/DropperTool.hpp"
#include "tools/SquareTool.hpp"
#include "tools/CircleTool.hpp"

int g_ScreenWidth = 1200;
int g_ScreenHeight = 800;
int menuBarHeight = 25;
int toolbarWidth = 150;

Color g_SelectedColor = BLACK;
float g_ColorValue = 1.0f;
float g_SelectedHue = 0.0f;
float g_SelectedSat = 0.0f;

struct CanvasStroke {
    std::vector<Vector2> points;
    float size;
    Color color;
    bool erased = false;
};

std::vector<CanvasStroke> g_CanvasStrokes;
CanvasStroke* g_CurrentStroke = nullptr;

Image g_BackgroundImage = { 0 };
Texture2D g_BackgroundTexture = { 0 };
RenderTexture2D g_RenderTex = { 0 };

std::string g_CurrentFile = "";
bool g_HasUnsavedChanges = false;

// --- Undo/Redo state snapshot ---
struct AppState {
    std::vector<CanvasStroke> strokes;

    std::vector<unsigned char> bgPixels;
    int bgW = 0;
    int bgH = 0;
    bool hasBg = false;
};

static std::deque<AppState> g_UndoStack;
static std::deque<AppState> g_RedoStack;
static const size_t kUndoLimit = 50;

Image RenderCanvasImage(int canvasW, int canvasH);
void File_New();
void File_Open();
void File_Save();
void File_SaveAs();

float DrawValueSlider(int x, int y, int w, int h, float value);

// Draw an HSV color wheel (simple per-pixel)
void DrawColorWheel(int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            float dist = sqrtf((float)(x*x + y*y));
            if (dist <= radius) {
                float angle = atan2f((float)y, (float)x);
                float hue = (angle + PI) / (2*PI) * 360.0f;
                float sat = dist / radius;
                Color c = ColorFromHSV(hue, sat, g_ColorValue);
                DrawPixel(cx + x, cy + y, c);
            }
        }
    }

    // Draw small circle to show current hue/sat selection
    float angleRad = (g_SelectedHue / 360.0f) * 2*PI - PI;
    int highlightX = cx + (int)(g_SelectedSat * radius * cosf(angleRad));
    int highlightY = cy + (int)(g_SelectedSat * radius * sinf(angleRad));
    DrawCircleLines(highlightX, highlightY, 6, BLACK);
    DrawCircleLines(highlightX, highlightY, 5, WHITE);
}

// Pick smoothing: when mouse is down & inside wheel we update hue/sat continuously
bool PointInCircle(float px, float py, int cx, int cy, int r) {
    float dx = px - cx;
    float dy = py - cy;
    return (dx*dx + dy*dy) <= (r * r);
}

// Value slider draws gradient according to current hue/sat
float DrawValueSlider(int x, int y, int w, int h, float value) {
    for (int i = 0; i < w; ++i) {
        float v = (float)i / (float)w;
        Color c = ColorFromHSV(g_SelectedHue, g_SelectedSat, v);
        DrawLine(x + i, y, x + i, y + h, c);
    }

    int handleX = x + (int)(value * w);
    DrawRectangle(handleX - 3, y - 2, 6, h + 4, BLACK);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (m.x >= x && m.x <= x + w && m.y >= y && m.y <= y + h) {
            value = (m.x - x) / (float)w;
            value = std::clamp(value, 0.0f, 1.0f);
            g_ColorValue = value;
        }
    }

    return value;
}

// --- File actions ---

void FlattenToWhite(Image &img) {
    Color* px = (Color*)img.data;
    int count = img.width * img.height;

    for (int i = 0; i < count; i++) {
        float a = px[i].a / 255.0f;

        px[i].r = (unsigned char)(px[i].r * a + 255 * (1.0f - a));
        px[i].g = (unsigned char)(px[i].g * a + 255 * (1.0f - a));
        px[i].b = (unsigned char)(px[i].b * a + 255 * (1.0f - a));

        px[i].a = 255;
    }
}

void DoExportImage(const std::string &dst, int canvasW, int canvasH) {
    Image img = RenderCanvasImage(canvasW, canvasH);

    FlattenToWhite(img); // optional: remove alpha for saving
    ImageFlipVertical(&img);

    ExportImage(img, dst.c_str());
    UnloadImage(img);
}

void File_New() {
    if (g_HasUnsavedChanges) {
        int result = tinyfd_messageBox("Unsaved Changes",
            "Do you want to save the current file?",
            "yesno", "question", 1);

        if (result == 1) {
            if (g_CurrentFile.empty()) {
                const char* patterns[] = {"*.png"};
                const char** pp = patterns;
                const char* dst = tinyfd_saveFileDialog("Save As", "image.png", 1, pp, "PNG files");
                if (!dst) return;
                g_CurrentFile = dst;
            }

            int canvasW = g_RenderTex.texture.width;
            int canvasH = g_RenderTex.texture.height;
            DoExportImage(g_CurrentFile, canvasW, canvasH);
        }
    }

    g_CanvasStrokes.clear();

    if (g_BackgroundTexture.id != 0) {
        UnloadTexture(g_BackgroundTexture);
        g_BackgroundTexture = {};
    }
    if (g_BackgroundImage.data != nullptr) {
        UnloadImage(g_BackgroundImage);
        g_BackgroundImage = {};
    }

    g_CurrentFile.clear();
    g_UndoStack.clear();
    g_RedoStack.clear();
    g_HasUnsavedChanges = false;
}

void RecreateRenderTex(int canvasW, int canvasH) {
    if (g_RenderTex.texture.id != 0) UnloadRenderTexture(g_RenderTex);
    g_RenderTex = LoadRenderTexture(canvasW, canvasH);
}

void File_Open() {
    if (g_HasUnsavedChanges) {
        int result = tinyfd_messageBox("Unsaved Changes",
            "Do you want to save the current file before opening another?",
            "yesno", "question", 1);

        if (result == 1) {
            File_Save();
        }
    }

    const char* patterns[] = {"*.png"};
    const char** pp = patterns;

    const char* file = tinyfd_openFileDialog("Open PNG", "", 1, pp, "PNG images", 0);
    if (!file) return;

    Image img = LoadImage(file);
    if (img.data == nullptr) {
        tinyfd_messageBox("Error", "Failed to open image.", "ok", "error", 1);
        return;
    }

    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    if (g_BackgroundTexture.id != 0) {
        UnloadTexture(g_BackgroundTexture);
        g_BackgroundTexture = {};
    }
    if (g_BackgroundImage.data != nullptr) {
        UnloadImage(g_BackgroundImage);
        g_BackgroundImage = {};
    }

    g_BackgroundImage = img;
    g_BackgroundTexture = LoadTextureFromImage(g_BackgroundImage);

    int newWindowW = toolbarWidth + g_BackgroundImage.width;
    int newWindowH = menuBarHeight + g_BackgroundImage.height;
    g_ScreenWidth = newWindowW;
    g_ScreenHeight = newWindowH;
    SetWindowSize(g_ScreenWidth, g_ScreenHeight);

    RecreateRenderTex(g_BackgroundImage.width, g_BackgroundImage.height);

    g_CanvasStrokes.clear();
    g_UndoStack.clear();
    g_RedoStack.clear(); 

    g_CurrentFile = file;
    g_HasUnsavedChanges = false;
}

void File_SaveAs() {
    const char* patterns[] = {"*.png"};
    const char** pp = patterns;

    const char* dst = tinyfd_saveFileDialog("Save As", "image.png", 1, pp, "PNG files");
    if (!dst) return;

    g_CurrentFile = dst;

    int canvasW = g_RenderTex.texture.width;
    int canvasH = g_RenderTex.texture.height;
    DoExportImage(g_CurrentFile, canvasW, canvasH);

    g_HasUnsavedChanges = false;
}

void File_Save() {
    if (g_CurrentFile.empty()) {
        File_SaveAs();
        return;
    }

    int canvasW = g_RenderTex.texture.width;
    int canvasH = g_RenderTex.texture.height;
    DoExportImage(g_CurrentFile, canvasW, canvasH);

    g_HasUnsavedChanges = false;
}

// Helper to duplicate current background image pixels into vector
static void CaptureBackgroundPixels(AppState &s) {
    s.bgPixels.clear();
    if (g_BackgroundImage.data != nullptr && g_BackgroundImage.width > 0 && g_BackgroundImage.height > 0) {
        s.hasBg = true;
        s.bgW = g_BackgroundImage.width;
        s.bgH = g_BackgroundImage.height;
        int bytes = s.bgW * s.bgH * 4;
        unsigned char *src = (unsigned char*)g_BackgroundImage.data;
        s.bgPixels.assign(src, src + bytes);
    } else {
        s.hasBg = false;
        s.bgW = s.bgH = 0;
    }
}

static void ApplyBackgroundFromState(const AppState &s) {
    if (g_BackgroundTexture.id != 0) {
        UnloadTexture(g_BackgroundTexture);
        g_BackgroundTexture = {};
    }
    if (g_BackgroundImage.data != nullptr) {
        UnloadImage(g_BackgroundImage);
        g_BackgroundImage = {};
    }

    if (!s.hasBg) {
        return;
    }

    int bytes = s.bgW * s.bgH * 4;
    Image img = {};
    img.width = s.bgW;
    img.height = s.bgH;
    img.mipmaps = 1;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.data = malloc(bytes);
    if (!img.data) {
        return;
    }
    memcpy(img.data, s.bgPixels.data(), bytes);

    g_BackgroundImage = img;
    g_BackgroundTexture = LoadTextureFromImage(g_BackgroundImage);
    RecreateRenderTex(g_BackgroundImage.width, g_BackgroundImage.height);
}

static void PushState() {
    AppState s;
    s.strokes = g_CanvasStrokes;
    CaptureBackgroundPixels(s);

    g_UndoStack.push_back(std::move(s));
    // cap size
    while (g_UndoStack.size() > kUndoLimit) g_UndoStack.pop_front();
    g_RedoStack.clear();
}

static void ApplyState(const AppState &s) {
    g_CanvasStrokes = s.strokes;
    ApplyBackgroundFromState(s);
    g_HasUnsavedChanges = true;
}

static void DoUndo() {
    if (g_UndoStack.empty()) return;
    AppState current;
    current.strokes = g_CanvasStrokes;
    CaptureBackgroundPixels(current);
    g_RedoStack.push_back(std::move(current));
    AppState last = std::move(g_UndoStack.back());
    g_UndoStack.pop_back();
    ApplyState(last);
}

static void DoRedo() {
    if (g_RedoStack.empty()) return;
    AppState current;
    current.strokes = g_CanvasStrokes;
    CaptureBackgroundPixels(current);
    g_UndoStack.push_back(std::move(current));

    AppState next = std::move(g_RedoStack.back());
    g_RedoStack.pop_back();
    ApplyState(next);
}


Image RenderCanvasImage(int canvasW, int canvasH) {
    RenderTexture2D temp = LoadRenderTexture(canvasW, canvasH);

    BeginTextureMode(temp);
    ClearBackground({ 0,0,0,0 });

    if (g_BackgroundTexture.id != 0) {
        DrawTexture(g_BackgroundTexture, 0, 0, WHITE);
    } else {
        DrawRectangle(0, 0, canvasW, canvasH, WHITE);
    }

    for (const auto &stroke : g_CanvasStrokes) {
        if (stroke.erased) continue;
        if (stroke.points.size() < 2) continue;

        for (size_t i = 1; i < stroke.points.size(); ++i) {
            Vector2 p1 = { stroke.points[i - 1].x - toolbarWidth, stroke.points[i - 1].y - menuBarHeight };
            Vector2 p2 = { stroke.points[i].x - toolbarWidth, stroke.points[i].y - menuBarHeight };
            DrawLineEx(p1, p2, stroke.size, stroke.color);
            DrawCircleV(p2, stroke.size * 0.5f, stroke.color);
        }
    }

    EndTextureMode();

    Image img = LoadImageFromTexture(temp.texture);
    UnloadRenderTexture(temp);
    return img;
}

void EraseBackgroundAt(const Vector2 &screenPos, float radius) {
    if (g_BackgroundImage.data == nullptr) return;

    int ix = (int)(screenPos.x - toolbarWidth);
    int iy = (int)(screenPos.y - menuBarHeight);

    if (ix < - (int)radius || iy < - (int)radius) return;
    if (ix >= (int)g_BackgroundImage.width + (int)radius || iy >= (int)g_BackgroundImage.height + (int)radius) return;

    int x0 = std::max(0, ix - (int)radius);
    int x1 = std::min((int)g_BackgroundImage.width - 1, ix + (int)radius);
    int y0 = std::max(0, iy - (int)radius);
    int y1 = std::min((int)g_BackgroundImage.height - 1, iy + (int)radius);

    int imgW = g_BackgroundImage.width;
    unsigned char *pixels = (unsigned char *)g_BackgroundImage.data;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            int dx = x - ix;
            int dy = y - iy;
            if (dx*dx + dy*dy <= (int)(radius*radius)) {
                int idx = (y * imgW + x) * 4;
                pixels[idx + 3] = 0;
            }
        }
    }

    if (g_BackgroundTexture.id != 0) UnloadTexture(g_BackgroundTexture);
    g_BackgroundTexture = LoadTextureFromImage(g_BackgroundImage);
    g_HasUnsavedChanges = true;
}


// -------------------- Main --------------------
int main() {
    InitWindow(g_ScreenWidth, g_ScreenHeight, "ratart - Simple Drawing App");
    SetTargetFPS(60);

    RecreateRenderTex(g_ScreenWidth - toolbarWidth, g_ScreenHeight - menuBarHeight);
    PushState();

    std::unique_ptr<PencilTool> pencilTool = std::make_unique<PencilTool>();
    std::unique_ptr<EraserTool> eraserTool = std::make_unique<EraserTool>();
    std::unique_ptr<DropperTool> dropperTool = std::make_unique<DropperTool>();
    std::unique_ptr<SquareTool> squareTool = std::make_unique<SquareTool>();
    std::unique_ptr<CircleTool> circleTool = std::make_unique<CircleTool>();
    Tool* currentTool = pencilTool.get();

    std::vector<std::string> iconPaths = {
        "icons/pencil.png",
        "icons/eraser.png",
        "icons/dropper.png",
        "icons/bucket.png",
        "icons/square.png",
        "icons/circle.png"
    };

    struct ToolButton { std::string name, iconPath; Rectangle rect; KeyboardKey shortcut; int id; Texture2D icon; };
    std::vector<ToolButton> toolButtons = {
        { "Pencil",  iconPaths[0], {}, KEY_B, 0, {} },
        { "Eraser",  iconPaths[1], {}, KEY_E, 1, {} },
        { "Dropper", iconPaths[2], {}, KEY_I, 2, {} },
        { "Bucket",  iconPaths[3], {}, KEY_K, 3, {} },
        { "Square",  iconPaths[4], {}, KEY_S, 4, {} },
        { "Circle",  iconPaths[5], {}, KEY_C, 5, {} }
    };

    for (auto &b : toolButtons) {
        if (FileExists(b.iconPath.c_str()))
            b.icon = LoadTexture(b.iconPath.c_str());
    }

    std::vector<std::pair<std::string, std::vector<std::string>>> menu = {
        {"File", {"New", "Open", "Save", "Save As"}},
        {"Edit", {"Change Canvas Size", "Change Canvas BG"}}
    };

    struct MenuTab { std::string label; Rectangle rect; std::vector<std::string> items; bool open; };
    std::vector<MenuTab> menuTabs;
    float tabX = 0.0f;
    for (auto &m : menu) {
        MenuTab t;
        t.label = m.first;
        t.rect = { tabX, 0.0f, (float)menuBarHeight*2.0f, (float)menuBarHeight };
        t.items = m.second;
        t.open = false;
        menuTabs.push_back(t);
        tabX += menuBarHeight * 2.0f;
    }

    Rectangle undoBtn = { menuBarHeight * 4.0f, 0, menuBarHeight*2.5f, menuBarHeight };
    Rectangle redoBtn = { menuBarHeight * 6.5f, 0, menuBarHeight*2.5f, menuBarHeight };

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        int wheelRadius = toolbarWidth / 3;
        int wheelCx = toolbarWidth / 2;
        int wheelCy = menuBarHeight + wheelRadius + 30;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (PointInCircle(mouse.x, mouse.y, wheelCx, wheelCy + 10, wheelRadius)) {
                float dx = mouse.x - wheelCx;
                float dy = mouse.y - (wheelCy + 10);
                float dist = sqrtf(dx*dx + dy*dy);
                g_SelectedHue = (atan2f(dy, dx) + PI) / (2*PI) * 360.0f;
                g_SelectedSat = std::clamp(dist / (float)wheelRadius, 0.0f, 1.0f);
                g_SelectedColor = ColorFromHSV(g_SelectedHue, g_SelectedSat, g_ColorValue);
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            if (PointInCircle(mouse.x, mouse.y, wheelCx, wheelCy + 10, wheelRadius)) {
                g_SelectedColor = WHITE;
                g_SelectedHue = 0.0f;
                g_SelectedSat = 0.0f;
                g_ColorValue = 1.0f;
            }
        }

        currentTool->SetColor(g_SelectedColor);

        bool insideCanvas = (mouse.x >= toolbarWidth &&
                             mouse.y >= menuBarHeight &&
                             mouse.x < g_ScreenWidth &&
                             mouse.y < g_ScreenHeight);

        if (insideCanvas) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                PushState();
                currentTool->OnMouseDown(mouse);
                g_HasUnsavedChanges = true;
            }
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                currentTool->OnMouseHold(mouse);
                g_HasUnsavedChanges = true;
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                currentTool->OnMouseUp(mouse);
                g_HasUnsavedChanges = true;
            }
        }

        // shortkey tool switching
        if (IsKeyPressed(KEY_B)) currentTool = pencilTool.get();
        if (IsKeyPressed(KEY_E)) currentTool = eraserTool.get();
        if (IsKeyPressed(KEY_I)) currentTool = dropperTool.get();
        if (IsKeyPressed(KEY_S)) currentTool = squareTool.get();
        if (IsKeyPressed(KEY_C)) currentTool = circleTool.get();
        
        // keyboard shortcuts for undo/redo
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            if (IsKeyPressed(KEY_Z)) DoUndo();
            if (IsKeyPressed(KEY_Y)) DoRedo();
        }

        BeginDrawing();
        ClearBackground(WHITE);

        // draw background canvas area
        int canvasW = g_RenderTex.texture.width;
        int canvasH = g_RenderTex.texture.height;

        if (g_BackgroundTexture.id != 0) {
            DrawTexture(g_BackgroundTexture, toolbarWidth, menuBarHeight, WHITE);
        } else {
            DrawRectangle(toolbarWidth, menuBarHeight, canvasW, canvasH, WHITE);
        }

        // draw strokes on top
        for (const auto &stroke : g_CanvasStrokes) {
            if (stroke.erased) continue;
            if (stroke.points.size() < 2) continue;
            for (size_t i = 1; i < stroke.points.size(); ++i) {
                DrawLineEx(stroke.points[i-1], stroke.points[i], stroke.size, stroke.color);
                DrawCircleV(stroke.points[i], stroke.size*0.5f, stroke.color);
            }
        }

        currentTool->DrawPreview(mouse);

        // Tool bar / UI elements (color wheel etc.)
        DrawRectangle(0, menuBarHeight, toolbarWidth, g_ScreenHeight - menuBarHeight, ColorFromHSV(0,0,25));
        DrawRectangle(toolbarWidth, menuBarHeight, 1, g_ScreenHeight - menuBarHeight, ColorFromHSV(0,0,50));

        // Color wheel & value slider
        DrawText("Color", 20, menuBarHeight + 10, 20, BLACK);
        DrawColorWheel(wheelCx, wheelCy + 10, wheelRadius);

        // color preview square
        DrawRectangle(wheelCx + wheelRadius, wheelCy + wheelRadius, 15, 15, g_SelectedColor);
        DrawRectangleLines(wheelCx + wheelRadius, wheelCy + wheelRadius, 15, 15, BLACK);

        // value slider
        DrawText("Lightness", toolbarWidth * 0.15f, wheelCy + wheelRadius + 15, 16, BLACK);
        g_ColorValue = DrawValueSlider((int)(toolbarWidth * 0.15f), wheelCy + wheelRadius + 35, (int)(toolbarWidth * 0.7f), 15, g_ColorValue);
        g_SelectedColor = ColorFromHSV(g_SelectedHue, g_SelectedSat, g_ColorValue);

        // draw toolbar icons
        int cols = 3;
        int spacing = 6;
        int iconSize = toolbarWidth * 0.25f;
        int startX = toolbarWidth * 0.1f;
        int startY = (wheelCy + wheelRadius + 60);

        for (size_t i = 0; i < toolButtons.size(); ++i) {
            int row = i / cols;
            int col = i % cols;
            auto &b = toolButtons[i];
            b.rect = { (float)(startX + col * (iconSize + spacing)), (float)(startY + row*(iconSize+spacing)), (float)iconSize, (float)iconSize };
            DrawRectangleRec(b.rect, ColorFromHSV(0,0,10));
            if (b.icon.id != 0) {
                float scale = (float)iconSize / (float)b.icon.width;
                DrawTextureEx(b.icon, { b.rect.x, b.rect.y }, 0.0f, scale, WHITE);
            }
            if (CheckCollisionPointRec(mouse, b.rect)) DrawRectangleLinesEx(b.rect, 2, YELLOW);
            if (CheckCollisionPointRec(mouse, b.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (i == 0) currentTool = pencilTool.get();
                else if (i == 1) currentTool = eraserTool.get();
                else if (i == 2) currentTool = dropperTool.get();
                else if (i == 4) currentTool = squareTool.get();
                else if (i == 5) currentTool = circleTool.get();
            }
        }

        // Draw UI for active tool (slider for size)
        currentTool->DrawUI((toolbarWidth - 100) * 0.5f, startY + iconSize*2 + 20);

        // ------ MENU BAR -------
        DrawRectangle(0,0, g_ScreenWidth, menuBarHeight, LIGHTGRAY);

        // draw tabs
        for (auto &tab : menuTabs) {
            Color bg = (tab.open || CheckCollisionPointRec(mouse, tab.rect)) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(tab.rect, bg);
            DrawText(tab.label.c_str(), (int)(tab.rect.x + menuBarHeight*0.3f), (int)(tab.rect.y + menuBarHeight*0.1f), (int)(menuBarHeight*0.8f), BLACK);

            if (CheckCollisionPointRec(mouse, tab.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (auto &t : menuTabs) t.open = false;
                tab.open = true;
            }

            if (tab.open) {
                int itemHeight = 22;
                int mw = 180;
                for (size_t i = 0; i < tab.items.size(); ++i) {
                    Rectangle itrect = { tab.rect.x, tab.rect.y + tab.rect.height + i*itemHeight, (float)mw, (float)itemHeight };
                    Color bg2 = CheckCollisionPointRec(mouse, itrect) ? Color{220,220,220,255} : Color{245,245,245,255};
                    DrawRectangleRec(itrect, bg2);
                    DrawRectangleLinesEx(itrect, 1, BLACK);
                    DrawText(tab.items[i].c_str(), (int)itrect.x+6, (int)itrect.y+4, 16, BLACK);
                    if (CheckCollisionPointRec(mouse, itrect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (tab.label == "File") {
                            if (tab.items[i] == "New") File_New();
                            else if (tab.items[i] == "Open") File_Open();
                            else if (tab.items[i] == "Save") File_Save();
                            else if (tab.items[i] == "Save As") File_SaveAs();
                        }
                        tab.open = false;
                    }
                }
            }
        }

        // Close menus if clicking outside
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool clickedMenu = false;
            for (auto &tab : menuTabs) {
                if (CheckCollisionPointRec(mouse, tab.rect)) clickedMenu = true;
                if (tab.open) {
                    int itemHeight = 22; int mw = 180;
                    for (size_t i=0;i<tab.items.size();++i) {
                        Rectangle itrect = { tab.rect.x, tab.rect.y + tab.rect.height + i*itemHeight, (float)mw, (float)itemHeight };
                        if (CheckCollisionPointRec(mouse, itrect)) clickedMenu = true;
                    }
                }
            }
            if (!clickedMenu) for (auto &tab : menuTabs) tab.open = false;
        }

        // Undo / Redo buttons (placeholders)
        DrawRectangleRec(undoBtn, CheckCollisionPointRec(mouse, undoBtn) ? GRAY : LIGHTGRAY);
        DrawRectangleRec(redoBtn, CheckCollisionPointRec(mouse, redoBtn) ? GRAY : LIGHTGRAY);
        DrawText("Undo", (int)(undoBtn.x + menuBarHeight*0.3f), (int)(undoBtn.y + menuBarHeight*0.1f), (int)(menuBarHeight*0.8f), BLACK);
        DrawText("Redo", (int)(redoBtn.x + menuBarHeight*0.3f), (int)(redoBtn.y + menuBarHeight*0.1f), (int)(menuBarHeight*0.8f), BLACK);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, undoBtn)) { DoUndo(); }
            if (CheckCollisionPointRec(mouse, redoBtn)) { DoRedo(); }
        }

        EndDrawing();
    }

    // cleanup
    for (auto &b : toolButtons) if (b.icon.id != 0) UnloadTexture(b.icon);
    if (g_BackgroundTexture.id != 0) UnloadTexture(g_BackgroundTexture);
    if (g_BackgroundImage.data != nullptr) UnloadImage(g_BackgroundImage);
    if (g_RenderTex.texture.id != 0) UnloadRenderTexture(g_RenderTex);
    CloseWindow();
    return 0;
}
