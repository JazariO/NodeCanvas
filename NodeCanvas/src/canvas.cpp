#define NOMINMAX
#include "../include/canvas.h"
#include "../include/app.h"
#include "../include/render.h"
#include <windowsx.h>
#include <algorithm>

void Canvas::Init() {
    zoom = 1.0f;
    pan = { 0, 0 };
    is_dragging = false;
    drag_thing = -1;
    is_connecting = false;
    connect_from_node = -1;
    connect_from_port = -1;
}

void Canvas::Render(HDC hdc, App* app) {
    NodeCanvasRender::DrawCanvasBackground(hdc, bounds);
    for (int i = 0; i < app->thing_count; i++) {
        if (!app->things[i].is_active) continue;
        switch (app->things[i].type) {
        case THING_NODE: NodeCanvasRender::DrawNode(hdc, &app->things[i], app); break;
        case THING_EDGE: NodeCanvasRender::DrawEdge(hdc, &app->things[i], app); break;
        case THING_STICKY_NOTE: NodeCanvasRender::DrawStickyNote(hdc, &app->things[i]); break;
        }
    }
}

bool Canvas::HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam, App* app) {
    switch (uMsg) {
    case WM_RBUTTONDOWN: {
        POINT pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        app->ui->ShowContextMenu(pos, app);
        return true;
    }
    case WM_LBUTTONDOWN: {
        POINT pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        pos.x = (int)((pos.x - pan.x) / zoom);
        pos.y = (int)((pos.y - pan.y) / zoom);
        for (int i = app->thing_count - 1; i >= 0; i--) {
            if (!app->things[i].is_active) continue;
            if (app->things[i].type == THING_NODE) {
                RECT rect = { app->things[i].data.node.pos.x, app->things[i].data.node.pos.y,
                             app->things[i].data.node.pos.x + 100, app->things[i].data.node.pos.y + 50 };
                if (PtInRect(&rect, pos)) {
                    app->SaveUndo();
                    app->things[i].is_selected = true;
                    app->selected_thing = i;
                    is_dragging = true;
                    drag_thing = i;
                    drag_start = pos;
                    InvalidateRect(app->hwnd, nullptr, TRUE);
                    return true;
                }
            }
        }
        app->selected_thing = -1;
        is_dragging = false;
        return true;
    }
    case WM_MOUSEMOVE: {
        if (is_dragging && drag_thing >= 0) {
            POINT pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            pos.x = (int)((pos.x - pan.x) / zoom);
            pos.y = (int)((pos.y - pan.y) / zoom);
            app->things[drag_thing].data.node.pos.x += pos.x - drag_start.x;
            app->things[drag_thing].data.node.pos.y += pos.y - drag_start.y;
            drag_start = pos;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }
        return false;
    }
    case WM_LBUTTONUP: {
        is_dragging = false;
        drag_thing = -1;
        return true;
    }
    case WM_KEYDOWN: {
        if (wParam == 'F') {
            FocusAll(app);
            return true;
        }
        if (wParam == VK_ESCAPE) {
            app->ui->is_text_editing = false;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }
        return false;
    }
    }
    return false;
}

void Canvas::FocusAll(App* app) {
    int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
    bool has_nodes = false;
    for (int i = 0; i < app->thing_count; i++) {
        if (!app->things[i].is_active || app->things[i].type != THING_NODE) continue;
        has_nodes = true;
        min_x = std::min(min_x, static_cast<int>(app->things[i].data.node.pos.x));
        min_y = std::min(min_y, static_cast<int>(app->things[i].data.node.pos.y));
        max_x = std::max(max_x, static_cast<int>(app->things[i].data.node.pos.x + 100));
        max_y = std::max(max_y, static_cast<int>(app->things[i].data.node.pos.y + 50));
    }
    if (!has_nodes) return;
    int width = max_x - min_x;
    int height = max_y - min_y;
    width = static_cast<int>(width * 1.1f);
    height = static_cast<int>(height * 1.1f);
    RECT client_rect;
    GetClientRect(app->hwnd, &client_rect);
    float zoom_x = static_cast<float>(client_rect.right) / width;
    float zoom_y = static_cast<float>(client_rect.bottom) / height;
    zoom = std::min(zoom_x, zoom_y);
    pan.x = (client_rect.right - width * zoom) / 2 - min_x * zoom;
    pan.y = (client_rect.bottom - height * zoom) / 2 - min_y * zoom;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::AddNode(App* app, POINT pos) {
    if (app->thing_count >= MAX_THINGS) return;
    app->SaveUndo();
    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_NODE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.node.pos = pos;
    thing->data.node.color = RGB(200, 200, 200);
    thing->data.node.text[0] = '\0';
    thing->data.node.output_port_count = 0;
    thing->data.node.input_edge = -1;
    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::AddEdge(App* app, int from_node, int from_port, int to_node) {
    if (app->thing_count >= MAX_THINGS) return;
    app->SaveUndo();
    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_EDGE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.edge.from_node = from_node;
    thing->data.edge.from_port = from_port;
    thing->data.edge.to_node = to_node;
    thing->data.edge.has_control_point_overrides = false;
    app->things[from_node].data.node.output_ports[app->things[from_node].data.node.output_port_count++] = app->thing_count;
    app->things[to_node].data.node.input_edge = app->thing_count;
    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::AddStickyNote(App* app, POINT pos) {
    int sticky_note_count = 0;
    for (int i = 0; i < app->thing_count; i++) {
        if (app->things[i].is_active && app->things[i].type == THING_STICKY_NOTE) {
            sticky_note_count++;
        }
    }
    if (app->thing_count >= MAX_THINGS || sticky_note_count >= MAX_STICKY_NOTES) return;
    app->SaveUndo();
    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_STICKY_NOTE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.sticky_note.pos = pos;
    thing->data.sticky_note.size = { 200, 100 };
    thing->data.sticky_note.text[0] = '\0';
    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}