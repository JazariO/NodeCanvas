#ifndef APP_H
#define APP_H

#include <windows.h>

// Forward declarations to avoid circular dependencies
struct Canvas;
struct UI;
struct FileIO;
struct Thing;

#define MAX_THINGS 32000
#define MAX_UNDO 32

struct App {
    HWND hwnd;
    Canvas* canvas; // Use pointer to break circular dependency
    UI* ui;
    FileIO* fileio;
    Thing* things; // Will be allocated as an array
    int thing_count;
    int selected_thing;
    struct Undo {
        Thing* things; // Will be allocated as an array
        int thing_count;
    } undo_stack[MAX_UNDO];
    int undo_index;
    int undo_count;
    bool unsaved_changes;

    void Init(HWND hwnd);
    void Update();
    void SaveUndo();
};

// Include dependent headers after App definition
#include "canvas.h"
#include "ui.h"
#include "fileio.h"
#include "thing.h"

#endif