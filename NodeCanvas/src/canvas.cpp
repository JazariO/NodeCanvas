#define NOMINMAX
#include "../include/canvas.h"
#include "../include/app.h"
#include "../include/render.h"
#include <windowsx.h>
#include <algorithm>

void Canvas::Init() {
    bounds = { 0, 0, 0, 0 };
    zoom = 1.0f;
    pan = { 0, 0 };
    is_dragging = false;
    drag_thing = -1;
    is_connecting = false;
    connect_from_node = -1;
    connect_from_port = -1;
    connect_end = { 0, 0 };
    is_selecting = false;
    select_start = { 0, 0 };
    select_rect = { 0, 0, 0, 0 };
    hovered_node = -1;
    cached_cursor_pos = { 0,0 };
}

POINT Canvas::ScreenToCanvas(POINT screen_pos) {
    POINT canvas_pos;
    canvas_pos.x = (int)((screen_pos.x - pan.x) / zoom);
    canvas_pos.y = (int)((screen_pos.y - pan.y) / zoom);
    return canvas_pos;
}

POINT Canvas::CanvasToScreen(POINT canvas_pos) {
    POINT screen_pos;
    screen_pos.x = (int)(canvas_pos.x * zoom + pan.x);
    screen_pos.y = (int)(canvas_pos.y * zoom + pan.y);
    return screen_pos;
}

bool Canvas::HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam, App* app) {
    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        POINT cursor_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        POINT canvas_pos = ScreenToCanvas(cursor_pos);
        bool ctrl_pressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        int clicked_thing = -1;

        // Check if clicking on a node
        for (int i = 0; i < app->thing_count; i++) {
            if (app->things[i].type == THING_NODE && app->things[i].is_active) {
                RECT node_rect = {
                    app->things[i].data.node.pos.x,
                    app->things[i].data.node.pos.y,
                    app->things[i].data.node.pos.x + 100,
                    app->things[i].data.node.pos.y + 50
                };

                if (PtInRect(&node_rect, canvas_pos)) {
                    clicked_thing = i;
                    break;
                }
            }
        }

        // If no node clicked, check sticky notes
        if (clicked_thing == -1) {
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].type == THING_STICKY_NOTE && app->things[i].is_active) {
                    RECT note_rect = {
                        app->things[i].data.sticky_note.pos.x,
                        app->things[i].data.sticky_note.pos.y,
                        app->things[i].data.sticky_note.pos.x + app->things[i].data.sticky_note.size.cx,
                        app->things[i].data.sticky_note.pos.y + app->things[i].data.sticky_note.size.cy
                    };

                    if (PtInRect(&note_rect, canvas_pos)) {
                        clicked_thing = i;
                        break;
                    }
                }
            }
        }

        if (clicked_thing != -1) {
            app->SaveUndo();

            // Handle selection logic
            if (ctrl_pressed) {
                // Toggle selection of clicked item
                app->things[clicked_thing].is_selected = !app->things[clicked_thing].is_selected;
            }
            else {
                // If the clicked item is already selected and we have multiple selections,
                // don't change selection yet (allow dragging of group)
                bool item_already_selected = app->things[clicked_thing].is_selected;
                int selected_count = 0;
                for (int j = 0; j < app->thing_count; j++) {
                    if (app->things[j].is_selected) selected_count++;
                }

                if (!item_already_selected || selected_count <= 1) {
                    // Clear all selections and select only the clicked item
                    for (int j = 0; j < app->thing_count; j++) {
                        app->things[j].is_selected = false;
                    }
                    app->things[clicked_thing].is_selected = true;
                }
            }

            app->selected_thing = clicked_thing;
            is_dragging = true;
            drag_thing = clicked_thing;
            drag_start = canvas_pos;
            return true;
        }

        // If we didn't click on anything and Ctrl is not pressed, clear selections
        if (!ctrl_pressed) {
            for (int j = 0; j < app->thing_count; j++) {
                app->things[j].is_selected = false;
            }
        }

        // Start selection box
        is_selecting = true;
        select_start = canvas_pos;
        select_rect = { canvas_pos.x, canvas_pos.y, canvas_pos.x, canvas_pos.y };
        return true;
    }

    case WM_RBUTTONDOWN: {
        POINT cursor_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        cached_cursor_pos = cursor_pos;
        app->ui->ShowContextMenu(cursor_pos, app);
        return true;
    }

    case WM_MOUSEMOVE: {
        POINT cursor_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        POINT canvas_pos = ScreenToCanvas(cursor_pos);

        // Update hover state
        hovered_node = -1;
        for (int i = 0; i < app->thing_count; i++) {
            if (app->things[i].type == THING_NODE && app->things[i].is_active) {
                RECT node_rect = {
                    app->things[i].data.node.pos.x,
                    app->things[i].data.node.pos.y,
                    app->things[i].data.node.pos.x + 100,
                    app->things[i].data.node.pos.y + 50
                };

                if (PtInRect(&node_rect, canvas_pos)) {
                    hovered_node = i;
                    break;
                }
            }
        }

        if (is_dragging && drag_thing >= 0) {
            POINT delta = { canvas_pos.x - drag_start.x, canvas_pos.y - drag_start.y };

            // Move all selected items together
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].is_selected && app->things[i].is_active) {
                    if (app->things[i].type == THING_NODE) {
                        app->things[i].data.node.pos.x += delta.x;
                        app->things[i].data.node.pos.y += delta.y;
                    }
                    else if (app->things[i].type == THING_STICKY_NOTE) {
                        app->things[i].data.sticky_note.pos.x += delta.x;
                        app->things[i].data.sticky_note.pos.y += delta.y;
                    }
                }
            }

            drag_start = canvas_pos;
            app->unsaved_changes = true;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }

        if (is_connecting) {
            connect_end = canvas_pos;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }

        if (is_selecting) {
            select_rect.right = canvas_pos.x;
            select_rect.bottom = canvas_pos.y;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }

        return false;
    }

    case WM_LBUTTONUP: {
        if (is_dragging) {
            is_dragging = false;
            drag_thing = -1;
            return true;
        }

        if (is_connecting) {
            POINT cursor_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            POINT canvas_pos = ScreenToCanvas(cursor_pos);

            // Check if we're dropping on a node
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].type == THING_NODE && app->things[i].is_active && i != connect_from_node) {
                    RECT node_rect = {
                        app->things[i].data.node.pos.x,
                        app->things[i].data.node.pos.y,
                        app->things[i].data.node.pos.x + 100,
                        app->things[i].data.node.pos.y + 50
                    };

                    if (PtInRect(&node_rect, canvas_pos)) {
                        AddEdge(app, connect_from_node, connect_from_port, i);
                        break;
                    }
                }
            }

            is_connecting = false;
            connect_from_node = -1;
            connect_from_port = -1;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }

        if (is_selecting) {
            is_selecting = false;
            bool ctrl_pressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            // Normalize selection rectangle
            RECT normalized_rect = select_rect;
            if (normalized_rect.left > normalized_rect.right) {
                int temp = normalized_rect.left;
                normalized_rect.left = normalized_rect.right;
                normalized_rect.right = temp;
            }
            if (normalized_rect.top > normalized_rect.bottom) {
                int temp = normalized_rect.top;
                normalized_rect.top = normalized_rect.bottom;
                normalized_rect.bottom = temp;
            }

            // Only clear previous selections if Ctrl is not pressed
            if (!ctrl_pressed) {
                for (int i = 0; i < app->thing_count; i++) {
                    app->things[i].is_selected = false;
                }
            }

            // Select items within the rectangle
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].is_active) {
                    bool in_selection = false;
                    POINT pos = { 0, 0 };

                    if (app->things[i].type == THING_NODE) {
                        pos = app->things[i].data.node.pos;
                        in_selection = (pos.x >= normalized_rect.left && pos.x <= normalized_rect.right &&
                            pos.y >= normalized_rect.top && pos.y <= normalized_rect.bottom);
                    }
                    else if (app->things[i].type == THING_STICKY_NOTE) {
                        pos = app->things[i].data.sticky_note.pos;
                        in_selection = (pos.x >= normalized_rect.left && pos.x <= normalized_rect.right &&
                            pos.y >= normalized_rect.top && pos.y <= normalized_rect.bottom);
                    }

                    if (in_selection) {
                        if (ctrl_pressed) {
                            app->things[i].is_selected = !app->things[i].is_selected;
                        }
                        else {
                            app->things[i].is_selected = true;
                        }
                    }
                }
            }

            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }

        return false;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_DELETE) {
            // Delete all selected items
            app->SaveUndo();
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].is_selected) {
                    app->things[i].is_active = false;
                    app->things[i].is_selected = false;
                }
            }
            app->unsaved_changes = true;
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }
        else if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            // Select all (Ctrl+A)
            for (int i = 0; i < app->thing_count; i++) {
                if (app->things[i].is_active &&
                    (app->things[i].type == THING_NODE || app->things[i].type == THING_STICKY_NOTE)) {
                    app->things[i].is_selected = true;
                }
            }
            InvalidateRect(app->hwnd, nullptr, TRUE);
            return true;
        }
        return false;
    }
    }

    return false;
}

void Canvas::AddNode(App* app, POINT pos) {
    if (app->thing_count >= MAX_THINGS) return;

    app->SaveUndo();

    POINT canvas_pos = ScreenToCanvas(cached_cursor_pos);

    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_NODE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.node.pos = canvas_pos;
    thing->data.node.color = RGB(200, 200, 255);
    strcpy_s(thing->data.node.text, MAX_TEXT_LENGTH, "Node");
    thing->data.node.output_port_count = 1;
    thing->data.node.output_ports[0] = -1;
    thing->data.node.input_edge = -1;

    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::AddStickyNote(App* app, POINT pos) {
    if (app->thing_count >= MAX_THINGS) return;

    app->SaveUndo();

    POINT canvas_pos = ScreenToCanvas(cached_cursor_pos);

    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_STICKY_NOTE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.sticky_note.pos = canvas_pos;
    thing->data.sticky_note.size = { 150, 100 };
    strcpy_s(thing->data.sticky_note.text, MAX_TEXT_LENGTH, "Sticky Note");

    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::AddEdge(App* app, int from_node, int from_port, int to_node) {
    if (app->thing_count >= MAX_THINGS) return;
    if (from_node < 0 || from_node >= app->thing_count || to_node < 0 || to_node >= app->thing_count) return;
    if (app->things[from_node].type != THING_NODE || app->things[to_node].type != THING_NODE) return;

    app->SaveUndo();

    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_EDGE;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.edge.from_node = from_node;
    thing->data.edge.from_port = from_port;
    thing->data.edge.to_node = to_node;
    thing->data.edge.has_control_point_overrides = false;

    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::FocusAll(App* app) {
    if (app->thing_count == 0) return;

    // Find bounding box of all active things
    RECT bounding_box = { INT_MAX, INT_MAX, INT_MIN, INT_MIN };
    bool found_any = false;

    for (int i = 0; i < app->thing_count; i++) {
        if (!app->things[i].is_active) continue;

        RECT thing_rect = { 0, 0, 0, 0 };

        if (app->things[i].type == THING_NODE) {
            thing_rect = {
                app->things[i].data.node.pos.x,
                app->things[i].data.node.pos.y,
                app->things[i].data.node.pos.x + 100,
                app->things[i].data.node.pos.y + 50
            };
        }
        else if (app->things[i].type == THING_STICKY_NOTE) {
            thing_rect = {
                app->things[i].data.sticky_note.pos.x,
                app->things[i].data.sticky_note.pos.y,
                app->things[i].data.sticky_note.pos.x + app->things[i].data.sticky_note.size.cx,
                app->things[i].data.sticky_note.pos.y + app->things[i].data.sticky_note.size.cy
            };
        }
        else {
            continue;
        }

        if (!found_any) {
            bounding_box = thing_rect;
            found_any = true;
        }
        else {
            bounding_box.left = std::min(bounding_box.left, thing_rect.left);
            bounding_box.top = std::min(bounding_box.top, thing_rect.top);
            bounding_box.right = std::max(bounding_box.right, thing_rect.right);
            bounding_box.bottom = std::max(bounding_box.bottom, thing_rect.bottom);
        }
    }

    if (!found_any) return;

    // Calculate zoom to fit all things
    int content_width = bounding_box.right - bounding_box.left;
    int content_height = bounding_box.bottom - bounding_box.top;
    int canvas_width = bounds.right - bounds.left;
    int canvas_height = bounds.bottom - bounds.top;

    float zoom_x = (float)canvas_width / (content_width + 100);
    float zoom_y = (float)canvas_height / (content_height + 100);
    zoom = std::min(zoom_x, zoom_y);

    // Clamp zoom
    if (zoom < 0.1f) zoom = 0.1f;
    if (zoom > 5.0f) zoom = 5.0f;

    // Center the content
    int center_x = (bounding_box.left + bounding_box.right) / 2;
    int center_y = (bounding_box.top + bounding_box.bottom) / 2;

    pan.x = canvas_width / 2 - center_x * zoom;
    pan.y = canvas_height / 2 - center_y * zoom;

    InvalidateRect(app->hwnd, nullptr, TRUE);
}

void Canvas::Render(HDC hdc, App* app) {
    NodeCanvasRender::DrawCanvasBackground(hdc, bounds);

    // Draw all things with proper transforms
    for (int i = 0; i < app->thing_count; i++) {
        if (!app->things[i].is_active) continue;
        switch (app->things[i].type) {
        case THING_NODE:
            NodeCanvasRender::DrawNode(hdc, &app->things[i], app);
            break;
        case THING_EDGE:
            NodeCanvasRender::DrawEdge(hdc, &app->things[i], app);
            break;
        case THING_STICKY_NOTE:
            NodeCanvasRender::DrawStickyNote(hdc, &app->things[i], app);
            break;
        case THING_POPUP_MENU: {
            // Transform popup menu position to screen coordinates
            POINT screen_pos = CanvasToScreen(app->things[i].data.popup_menu.pos);
            RECT rect = { screen_pos.x, screen_pos.y, screen_pos.x + 100, screen_pos.y + 90 };
            HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            // Draw border
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HBRUSH no_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, pen);
            SelectObject(hdc, no_brush);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            DeleteObject(pen);

            SetBkColor(hdc, RGB(200, 200, 200));
            SetTextColor(hdc, RGB(0, 0, 0));
            RECT text_rect = rect;
            text_rect.top += 5;
            text_rect.bottom = text_rect.top + 25;
            DrawText(hdc, L"Change Color", -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            text_rect.top += 30;
            text_rect.bottom = text_rect.top + 25;
            DrawText(hdc, L"Edit Text", -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            text_rect.top += 30;
            text_rect.bottom = text_rect.top + 25;
            DrawText(hdc, L"Clear Edges", -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            break;
        }
        }
    }

    // Draw connection preview
    if (is_connecting && connect_from_node >= 0) {
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, pen);
        POINT start = CanvasToScreen({ app->things[connect_from_node].data.node.pos.x + 100,
                                      app->things[connect_from_node].data.node.pos.y + 25 });
        POINT end = CanvasToScreen(connect_end);
        MoveToEx(hdc, start.x, start.y, nullptr);
        LineTo(hdc, end.x, end.y);
        DeleteObject(pen);
    }

    // Draw selection box
    if (is_selecting) {
        HPEN pen = CreatePen(PS_DOT, 1, RGB(0, 0, 255));
        HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, pen);
        SelectObject(hdc, brush);
        POINT tl = CanvasToScreen({ select_rect.left, select_rect.top });
        POINT br = CanvasToScreen({ select_rect.right, select_rect.bottom });
        Rectangle(hdc, tl.x, tl.y, br.x, br.y);
        DeleteObject(pen);
    }
}