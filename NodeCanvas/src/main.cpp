#include <windows.h>
#include "../include/app.h"

App app; // Global application instance

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        // Clean up dynamically allocated memory
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
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        app.canvas->Render(hdc, &app);
        app.ui->Render(hdc, &app);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return app.canvas->HandleInput(uMsg, wParam, lParam, &app) ?
        0 : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"NodeCanvas Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"NodeCanvas",                  // Window title
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,   // Position
        1280, 720,                      // Size
        nullptr,                        // Parent window
        nullptr,                        // Menu
        hInstance,                      // Instance handle
        nullptr                         // Additional application data
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Initialize application
    app.Init(hwnd);

    // Main message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        app.Update();
    }

    return 0;
}