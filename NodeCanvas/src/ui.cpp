#include "../include/ui.h"
#include "../include/app.h"
#include <windows.h>

void UI::Init() {
    is_text_editing = false;
    popup_thing = -1;
    text_buffer[0] = '\0';
}

void UI::Render(HDC hdc, App* app) {
    for (int i = 0; i < app->thing_count; i++) {
        if (!app->things[i].is_active || app->things[i].type != THING_POPUP_MENU) continue;
        RECT rect = { app->things[i].data.popup_menu.pos.x, app->things[i].data.popup_menu.pos.y,
                     app->things[i].data.popup_menu.pos.x + 100, app->things[i].data.popup_menu.pos.y + 90 };
        HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        SetBkColor(hdc, RGB(200, 200, 200));
        SetTextColor(hdc, RGB(0, 0, 0));
        RECT text_rect = rect;
        text_rect.top += 10;
        DrawText(hdc, L"Change Color", -1, &text_rect, DT_CENTER | DT_SINGLELINE);
        text_rect.top += 30;
        DrawText(hdc, L"Edit Text", -1, &text_rect, DT_CENTER | DT_SINGLELINE);
        text_rect.top += 30;
        DrawText(hdc, L"Clear Edges", -1, &text_rect, DT_CENTER | DT_SINGLELINE);
    }
}

void UI::ShowContextMenu(POINT pos, App* app) {
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Add Node");
    AppendMenu(hMenu, MF_STRING, 2, L"Add Sticky Note");
    AppendMenu(hMenu, MF_STRING, 3, L"Save");
    AppendMenu(hMenu, MF_STRING, 4, L"Load");
    AppendMenu(hMenu, MF_STRING, 5, L"Undo");
    AppendMenu(hMenu, MF_STRING, 6, L"Redo");
    ClientToScreen(app->hwnd, &pos);
    int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, 0, app->hwnd, nullptr);
    switch (selection) {
    case 1: app->canvas->AddNode(app, app->canvas->cached_cursor_pos); break;  // Use cached position
    case 2: app->canvas->AddStickyNote(app, app->canvas->cached_cursor_pos); break;  // Use cached position
    case 3: app->fileio->Save(app); break;
    case 4: if (app->fileio->PromptSaveUnsaved(app->hwnd, app)) app->fileio->Load(app); break;
    case 5: app->Undo(); break;
    case 6: app->Redo(); break;
    }
    DestroyMenu(hMenu);
}

void UI::ShowPopupMenu(POINT pos, int thing_index, App* app) {
    if (app->thing_count >= MAX_THINGS || popup_thing != -1) return;
    app->SaveUndo();
    Thing* thing = &app->things[app->thing_count];
    thing->type = THING_POPUP_MENU;
    thing->is_active = true;
    thing->is_selected = false;
    thing->data.popup_menu.pos = pos;
    thing->data.popup_menu.target_thing = thing_index;
    popup_thing = app->thing_count;
    app->thing_count++;
    app->unsaved_changes = true;
    InvalidateRect(app->hwnd, nullptr, TRUE);
}