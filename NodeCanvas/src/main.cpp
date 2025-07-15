#include <windows.h>
#include "../include/app.h"

App app;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
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
        HDC mem_dc = CreateCompatibleDC(hdc);
        HBITMAP mem_bitmap = CreateCompatibleBitmap(hdc, client_rect.right, client_rect.bottom);
        HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, mem_bitmap);

        HBRUSH bg_brush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(mem_dc, &client_rect, bg_brush);
        DeleteObject(bg_brush);

        app.canvas->Render(mem_dc, &app);
        app.ui->Render(mem_dc, &app);

        BitBlt(hdc, 0, 0, client_rect.right, client_rect.bottom, mem_dc, 0, 0, SRCCOPY);

        SelectObject(mem_dc, old_bitmap);
        DeleteObject(mem_bitmap);
        DeleteDC(mem_dc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
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
        // Removed app.Update() - only update when needed via InvalidateRect
    }

    return 0;
}