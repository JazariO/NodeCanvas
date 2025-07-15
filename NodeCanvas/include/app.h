#ifndef APP_H
#define APP_H

#include <windows.h>
#include "thing.h"
#include "canvas.h"
#include "ui.h"
#include "fileio.h"

#define MAX_UNDO 32

struct App {
    HWND hwnd;
    Canvas* canvas;
    UI* ui;
    FileIO* fileio;
    Thing* things;
    int thing_count;
    int selected_thing;
    struct Undo {
        Thing* things;
        int thing_count;
    } undo_stack[MAX_UNDO];
    int undo_index;
    int undo_count;
    bool unsaved_changes;

    void Init(HWND hwnd);
    void Update();
    void SaveUndo();
    void Undo();
    void Redo();
};

#endif