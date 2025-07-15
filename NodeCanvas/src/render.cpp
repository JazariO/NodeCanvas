#include "../include/render.h"
#include "../include/app.h"
#include <windows.h>

namespace NodeCanvasRender {
    void DrawCanvasBackground(HDC hdc, RECT bounds) {
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &bounds, brush);
        DeleteObject(brush);
    }

    void DrawNode(HDC hdc, Thing* thing, App* app) {
        if (!thing || thing->type != THING_NODE || !thing->is_active) return;

        // Transform coordinates to screen space
        POINT screen_pos = app->canvas->CanvasToScreen(thing->data.node.pos);

        RECT rect = {
            screen_pos.x,
            screen_pos.y,
            screen_pos.x + (int)(100 * app->canvas->zoom),
            screen_pos.y + (int)(50 * app->canvas->zoom)
        };

        HBRUSH brush = CreateSolidBrush(thing->data.node.color);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        if (thing->is_selected) {
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
            HBRUSH no_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, pen);
            SelectObject(hdc, no_brush);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            DeleteObject(pen);
        }

        if (app->canvas->hovered_node == (thing - app->things)) {
            HBRUSH port_brush = CreateSolidBrush(RGB(0, 0, 0));
            RECT port_rect = {
                rect.right - (int)(10 * app->canvas->zoom),
                rect.top + (int)(20 * app->canvas->zoom),
                rect.right,
                rect.top + (int)(30 * app->canvas->zoom)
            };
            FillRect(hdc, &port_rect, port_brush);
            DeleteObject(port_brush);
        }

        SetBkColor(hdc, thing->data.node.color);
        SetTextColor(hdc, RGB(0, 0, 0));
        wchar_t wtext[MAX_TEXT_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, thing->data.node.text, -1, wtext, MAX_TEXT_LENGTH);
        DrawTextW(hdc, wtext, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void DrawEdge(HDC hdc, Thing* thing, App* app) {
        if (!thing || thing->type != THING_EDGE || !thing->is_active) return;

        // Transform coordinates to screen space
        POINT from_pos = app->canvas->CanvasToScreen({
            app->things[thing->data.edge.from_node].data.node.pos.x + 100,
            app->things[thing->data.edge.from_node].data.node.pos.y + 25
            });

        POINT to_pos = app->canvas->CanvasToScreen({
            app->things[thing->data.edge.to_node].data.node.pos.x,
            app->things[thing->data.edge.to_node].data.node.pos.y + 25
            });

        HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, pen);
        MoveToEx(hdc, from_pos.x, from_pos.y, nullptr);
        LineTo(hdc, to_pos.x, to_pos.y);
        DeleteObject(pen);
    }

    void DrawStickyNote(HDC hdc, Thing* thing, App* app) {
        if (!thing || thing->type != THING_STICKY_NOTE || !thing->is_active) return;

        // Transform coordinates to screen space
        POINT screen_pos = app->canvas->CanvasToScreen(thing->data.sticky_note.pos);

        RECT rect = {
            screen_pos.x,
            screen_pos.y,
            screen_pos.x + (int)(thing->data.sticky_note.size.cx * app->canvas->zoom),
            screen_pos.y + (int)(thing->data.sticky_note.size.cy * app->canvas->zoom)
        };

        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 200));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        if (thing->is_selected) {
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
            HBRUSH no_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, pen);
            SelectObject(hdc, no_brush);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            DeleteObject(pen);
        }

        SetBkColor(hdc, RGB(255, 255, 200));
        SetTextColor(hdc, RGB(0, 0, 0));
        wchar_t wtext[MAX_TEXT_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, thing->data.sticky_note.text, -1, wtext, MAX_TEXT_LENGTH);
        DrawTextW(hdc, wtext, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
}