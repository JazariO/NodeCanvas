#ifndef UI_H
#define UI_H

#include <windows.h>

// Forward declaration
struct App;

struct UI {
    bool is_text_editing;
    void Init();
    void Render(HDC hdc, App* app);
    void ShowContextMenu(POINT pos, App* app);
};

#endif