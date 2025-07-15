#include "../include/thing.h"
#include "../include/render.h"
#include <windows.h>

namespace NodeCanvasRender {
    void DrawStickyNote(HDC hdc, Thing* thing) {
        if (!thing || thing->type != THING_STICKY_NOTE || !thing->is_active) return;
        RECT rect = {
            thing->data.sticky_note.pos.x,
            thing->data.sticky_note.pos.y,
            thing->data.sticky_note.pos.x + thing->data.sticky_note.size.cx,
            thing->data.sticky_note.pos.y + thing->data.sticky_note.pos.y + thing->data.sticky_note.size.cy
        };
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 200)); // Yellow background
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        SetBkColor(hdc, RGB(255, 255, 200));
        SetTextColor(hdc, RGB(0, 0, 0));

        // Convert char to wchar_t
        wchar_t wtext[MAX_TEXT_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, thing->data.sticky_note.text, -1, wtext, MAX_TEXT_LENGTH);
        DrawTextW(hdc, wtext, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
}