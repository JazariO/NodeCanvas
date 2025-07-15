#include "../include/ui.h"
#include "../include/app.h"
#include <windows.h>

void UI::Init() {
    is_text_editing = false;
}

void UI::Render(HDC hdc, App* app) {
    // Render the close button in the top-right corner
    RECT client_rect;
    GetClientRect(app->hwnd, &client_rect);
    RECT close_button = { client_rect.right - 30, 10, client_rect.right - 10, 30 };
    HBRUSH brush = CreateSolidBrush(RGB(255, 100, 100)); // Red close button
    FillRect(hdc, &close_button, brush);
    DeleteObject(brush);
    SetBkColor(hdc, RGB(255, 100, 100));
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawText(hdc, L"X", -1, &close_button, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void UI::ShowContextMenu(POINT pos, App* app) {
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Add Node");
    AppendMenu(hMenu, MF_STRING, 2, L"Add Sticky Note");
    AppendMenu(hMenu, MF_STRING, 3, L"Save");
    AppendMenu(hMenu, MF_STRING, 4, L"Load");

    // Convert client coordinates to screen coordinates
    ClientToScreen(app->hwnd, &pos);

    // Show the context menu and handle selection
    int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, 0, app->hwnd, nullptr);
    switch (selection) {
    case 1: // Add Node
        app->canvas->AddNode(app, pos);
        break;
    case 2: // Add Sticky Note
        app->canvas->AddStickyNote(app, pos);
        break;
    case 3: // Save
        app->fileio->Save(app);
        break;
    case 4: // Load
        if (app->fileio->PromptSaveUnsaved(app->hwnd, app)) {
            app->fileio->Load(app);
        }
        break;
    }
    DestroyMenu(hMenu);
}