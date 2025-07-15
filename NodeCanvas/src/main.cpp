#include <windows.h>
#include "../include/app.h"

App app;

// Menu IDs
#define ID_FILE_NEW     1001
#define ID_FILE_OPEN    1002
#define ID_FILE_SAVE    1003
#define ID_FILE_SAVEAS  1004
#define ID_FILE_EXIT    1005
#define ID_EDIT_UNDO    2001
#define ID_EDIT_REDO    2002
#define ID_EDIT_SELECTALL 2003
#define ID_VIEW_FOCUSALL 3001

void CreateMenuBar(HWND hwnd) {
    HMENU hMenuBar = CreateMenu();

    // File menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New\tCtrl+N");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVEAS, L"Save &As...\tCtrl+Shift+S");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

    // Edit menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_REDO, L"&Redo\tCtrl+Y");
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SELECTALL, L"Select &All\tCtrl+A");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

    // View menu
    HMENU hViewMenu = CreatePopupMenu();
    AppendMenu(hViewMenu, MF_STRING, ID_VIEW_FOCUSALL, L"&Focus All\tF");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");

    SetMenu(hwnd, hMenuBar);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateMenuBar(hwnd);
        return 0;

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_FILE_NEW:
            if (app.fileio->PromptSaveUnsaved(hwnd, &app)) {
                app.SaveUndo();
                // Clear all things except canvas background
                for (int i = 1; i < app.thing_count; i++) {
                    app.things[i].is_active = false;
                }
                app.thing_count = 1; // Keep only canvas background
                app.fileio->has_file = false;
                app.unsaved_changes = false;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case ID_FILE_OPEN:
            if (app.fileio->PromptSaveUnsaved(hwnd, &app)) {
                app.fileio->Load(&app);
            }
            break;
        case ID_FILE_SAVE:
            app.fileio->Save(&app);
            break;
        case ID_FILE_SAVEAS:
            app.fileio->has_file = false;
            app.fileio->Save(&app);
            break;
        case ID_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_EDIT_UNDO:
            app.Undo();
            break;
        case ID_EDIT_REDO:
            app.Redo();
            break;
        case ID_EDIT_SELECTALL:
            for (int i = 0; i < app.thing_count; i++) {
                if (app.things[i].is_active &&
                    (app.things[i].type == THING_NODE || app.things[i].type == THING_STICKY_NOTE)) {
                    app.things[i].is_selected = true;
                }
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case ID_VIEW_FOCUSALL:
            app.canvas->FocusAll(&app);
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        for (int i = 0; i < MAX_UNDO; i++) {
            delete[] app.undo_stack[i].things;
        }
        delete[] app.things;
        delete app.fileio;
        delete app.ui;
        delete app.canvas;
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        if (app.fileio->PromptSaveUnsaved(hwnd, &app)) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_SIZE:
        // Update canvas bounds when window is resized
        GetClientRect(hwnd, &app.canvas->bounds);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);

        // Use double buffering to reduce flicker
        HDC mem_dc = CreateCompatibleDC(hdc);
        HBITMAP mem_bitmap = CreateCompatibleBitmap(hdc, client_rect.right, client_rect.bottom);
        HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, mem_bitmap);

        // Clear background
        HBRUSH bg_brush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(mem_dc, &client_rect, bg_brush);
        DeleteObject(bg_brush);

        // Render canvas and UI
        app.canvas->Render(mem_dc, &app);
        app.ui->Render(mem_dc, &app);

        // Copy to screen
        BitBlt(hdc, 0, 0, client_rect.right, client_rect.bottom, mem_dc, 0, 0, SRCCOPY);

        // Clean up
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(mem_bitmap);
        DeleteDC(mem_dc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        // Handle keyboard shortcuts
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            switch (wParam) {
            case 'N':
                PostMessage(hwnd, WM_COMMAND, ID_FILE_NEW, 0);
                return 0;
            case 'O':
                PostMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
                return 0;
            case 'S':
                if (GetKeyState(VK_SHIFT) & 0x8000) {
                    PostMessage(hwnd, WM_COMMAND, ID_FILE_SAVEAS, 0);
                }
                else {
                    PostMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
                }
                return 0;
            case 'Z':
                PostMessage(hwnd, WM_COMMAND, ID_EDIT_UNDO, 0);
                return 0;
            case 'Y':
                PostMessage(hwnd, WM_COMMAND, ID_EDIT_REDO, 0);
                return 0;
            case 'A':
                PostMessage(hwnd, WM_COMMAND, ID_EDIT_SELECTALL, 0);
                return 0;
            }
        }
        else if (wParam == 'F') {
            PostMessage(hwnd, WM_COMMAND, ID_VIEW_FOCUSALL, 0);
            return 0;
        }

        // Handle other keyboard input through canvas
        if (app.canvas->HandleInput(uMsg, wParam, lParam, &app)) {
            return 0;
        }
        break;
    }

    // Handle all other input through canvas
    return app.canvas->HandleInput(uMsg, wParam, lParam, &app) ?
        0 : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"NodeCanvas Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"NodeCanvas",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    app.Init(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}