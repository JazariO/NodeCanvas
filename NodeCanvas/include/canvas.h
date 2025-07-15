#ifndef CANVAS_H
#define CANVAS_H

#include <windows.h>

// Forward declarations
struct App;
struct Thing;

struct Canvas {
    float zoom;
    POINT pan;
    bool is_dragging;
    int drag_thing;
    POINT drag_start;
    bool is_connecting;
    int connect_from_node;
    int connect_from_port;
    RECT bounds;

    void Init();
    void Render(HDC hdc, App* app);
    bool HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam, App* app);
    void FocusAll(App* app);
    void AddNode(App* app, POINT pos);
    void AddEdge(App* app, int from_node, int from_port, int to_node);
    void AddStickyNote(App* app, POINT pos);
};

#endif