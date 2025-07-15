#ifndef UI_H
#define UI_H

#include <windows.h>
#include "thing.h"

class App;

class UI {
public:
    bool is_text_editing;
    int popup_thing;
    char text_buffer[MAX_TEXT_LENGTH];
    int editing_thing;
    HWND edit_control;

    void Init();
    void Render(HDC hdc, App* app);
    void ShowContextMenu(POINT pos, App* app);
    void ShowPopupMenu(POINT pos, int thing_index, App* app);
    void StartTextEditing(int thing_index, App* app);
    void EndTextEditing(App* app, bool save_changes = true);
    bool IsTextEditing() const { return editing_thing != -1; }
};

#endif