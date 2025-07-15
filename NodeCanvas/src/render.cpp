#include "../include/render.h"
#include "../include/app.h"
#include <windows.h>

namespace NodeCanvasRender {
    void DrawCanvasBackground(HDC hdc, RECT bounds) {
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255)); // White background
        FillRect(hdc, &bounds, brush);
        DeleteObject(brush);
    }

    void DrawNode(HDC hdc, Thing* thing, App* app) {
        if (!thing || thing->type != THING_NODE || !thing->is_active) return;
        RECT rect = {
            thing->data.node.pos.x,
            thing->data.node.pos.y,
            thing->data.node.pos.x + 100,
            thing->data.node.pos.y + 50
        };
        HBRUSH brush = CreateSolidBrush(thing->data.node.color);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        SetBkColor(hdc, thing->data.node.color);
        SetTextColor(hdc, RGB(0, 0, 0));
        wchar_t wtext[MAX_TEXT_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, thing->data.node.text, -1, wtext, MAX_TEXT_LENGTH);
        DrawTextW(hdc, wtext, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void DrawEdge(HDC hdc, Thing* thing, App* app) {
        if (!thing || thing->type != THING_EDGE || !thing->is_active) return;
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, pen);
        MoveToEx(hdc, app->things[thing->data.edge.from_node].data.node.pos.x + 100,
            app->things[thing->data.edge.from_node].data.node.pos.y + 25, nullptr);
        LineTo(hdc, app->things[thing->data.edge.to_node].data.node.pos.x,
            app->things[thing->data.edge.to_node].data.node.pos.y + 25);
        DeleteObject(pen);
    }

    // Note: DrawStickyNote is already defined correctly in thing.cpp
}