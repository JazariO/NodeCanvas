#include "../include/ui.h"
#include "../include/app.h"
#include <windows.h>

void UI::Init() {
    is_text_editing = false;
    popup_thing = -1;
    text_buffer[0] = '\0';

    // Add these initializations
    editing_thing = -1;
    edit_control = nullptr;
}

void UI::StartTextEditing(int thing_index, App* app) {
    if (thing_index < 0 || thing_index >= app->thing_count) return;
    if (editing_thing != -1) EndTextEditing(app, false); // End any existing editing

    Thing* thing = &app->things[thing_index];
    if (thing->type != THING_NODE && thing->type != THING_STICKY_NOTE) return;

    editing_thing = thing_index;

    // Calculate edit control position and size
    RECT edit_rect;
    if (thing->type == THING_NODE) {
        POINT screen_pos = app->canvas->CanvasToScreen(thing->data.node.pos);
        edit_rect = {
            screen_pos.x,
            screen_pos.y,
            screen_pos.x + (int)(100 * app->canvas->zoom),
            screen_pos.y + (int)(50 * app->canvas->zoom)
        };
    }
    else { // THING_STICKY_NOTE
        POINT screen_pos = app->canvas->CanvasToScreen(thing->data.sticky_note.pos);
        edit_rect = {
            screen_pos.x,
            screen_pos.y,
            screen_pos.x + (int)(thing->data.sticky_note.size.cx * app->canvas->zoom),
            screen_pos.y + (int)(thing->data.sticky_note.size.cy * app->canvas->zoom)
        };
    }

    // Create edit control - added ES_WANTRETURN to allow for multiline input
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN;

    edit_control = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        style,
        edit_rect.left,
        edit_rect.top,
        edit_rect.right - edit_rect.left,
        edit_rect.bottom - edit_rect.top,
        app->hwnd,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (edit_control) {
        // Subclass the edit control to handle Enter key properly
        extern WNDPROC original_edit_proc;
        extern LRESULT CALLBACK EditControlProc(HWND, UINT, WPARAM, LPARAM);
        original_edit_proc = (WNDPROC)SetWindowLongPtr(edit_control, GWLP_WNDPROC, (LONG_PTR)EditControlProc);

        // Set current text
        const char* current_text = (thing->type == THING_NODE) ?
            thing->data.node.text : thing->data.sticky_note.text;

        wchar_t wtext[MAX_TEXT_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, current_text, -1, wtext, MAX_TEXT_LENGTH);
        SetWindowTextW(edit_control, wtext);

        // Focus and select all text
        SetFocus(edit_control);
        SendMessage(edit_control, EM_SETSEL, 0, -1);
    }
}

void UI::EndTextEditing(App* app, bool save_changes) {
    if (editing_thing == -1 || !edit_control) return;

    if (save_changes && editing_thing < app->thing_count) {
        // Get text from edit control
        wchar_t wtext[MAX_TEXT_LENGTH];
        GetWindowTextW(edit_control, wtext, MAX_TEXT_LENGTH);

        // Convert to UTF-8 and replace \r\n with \n
        char utf8_text[MAX_TEXT_LENGTH];
        WideCharToMultiByte(CP_UTF8, 0, wtext, -1, utf8_text, MAX_TEXT_LENGTH, nullptr, nullptr);

        std::string text(utf8_text);
        size_t pos = 0;
        while ((pos = text.find("\r\n", pos)) != std::string::npos) {
            text.replace(pos, 2, "\n");
            pos += 1;
        }

        // Save to thing
        Thing* thing = &app->things[editing_thing];
        if (thing->type == THING_NODE) {
            strcpy_s(thing->data.node.text, MAX_TEXT_LENGTH, text.c_str());
        }
        else if (thing->type == THING_STICKY_NOTE) {
            strcpy_s(thing->data.sticky_note.text, MAX_TEXT_LENGTH, text.c_str());
        }

        app->unsaved_changes = true;
    }

    // Destroy edit control
    if (edit_control) {
        DestroyWindow(edit_control);
        edit_control = nullptr;
    }

    editing_thing = -1;
    SetFocus(app->hwnd); // Return focus to main window
    InvalidateRect(app->hwnd, nullptr, TRUE);
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