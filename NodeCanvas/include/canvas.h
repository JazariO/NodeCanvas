#ifndef CANVAS_H
#define CANVAS_H

#include <windows.h>
#include "thing.h"

class App; // Forward declaration

class Canvas {
public:
    RECT bounds;
    float zoom;
    POINT pan;
    bool is_dragging;
    int drag_thing;
    POINT drag_start;
    bool is_connecting;
    int connect_from_node;
    int connect_from_port;
    POINT connect_end;
    bool is_selecting;
    POINT select_start;
    RECT select_rect;
    int hovered_node;
    POINT cached_cursor_pos;
    DWORD last_click_time;
    POINT last_click_pos;
    int last_clicked_thing;
    static const DWORD DOUBLE_CLICK_TIME = 500; // milliseconds
    static const int DOUBLE_CLICK_DISTANCE = 5; // pixels

    void Init();
    void Render(HDC hdc, App* app);
    bool HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam, App* app);
    void FocusAll(App* app);
    void AddNode(App* app, POINT pos);
    void AddEdge(App* app, int from_node, int from_port, int to_node);
    void AddStickyNote(App* app, POINT pos);
    bool IsDoubleClick(POINT current_pos, DWORD current_time, int thing_index);

    // coordinate transformation methods
    POINT ScreenToCanvas(POINT screen_pos);
    POINT CanvasToScreen(POINT canvas_pos);
};

#endif