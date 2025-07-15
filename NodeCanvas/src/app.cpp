#include "../include/app.h"
#include <algorithm> // For std::min

void App::Init(HWND hwnd) {
    this->hwnd = hwnd;

    // Allocate memory for canvas, ui, fileio, and things
    canvas = new Canvas();
    ui = new UI();
    fileio = new FileIO();
    things = new Thing[MAX_THINGS];

    canvas->Init();
    ui->Init();
    fileio->has_file = false;
    thing_count = 0;
    selected_thing = -1;
    undo_index = 0;
    undo_count = 0;
    unsaved_changes = false;

    // Initialize canvas background
    things[thing_count].type = THING_CANVAS_BACKGROUND;
    things[thing_count].is_active = true;
    thing_count++;

    // Allocate memory for undo stack things
    for (int i = 0; i < MAX_UNDO; i++) {
        undo_stack[i].things = new Thing[MAX_THINGS];
    }
}

void App::Update() {
    // Update logic for animation, if needed
    InvalidateRect(hwnd, nullptr, TRUE);
}

void App::SaveUndo() {
    if (undo_count >= MAX_UNDO) return;
    undo_stack[undo_index].thing_count = thing_count;
    memcpy(undo_stack[undo_index].things, things, sizeof(Thing) * thing_count);
    undo_index = (undo_index + 1) % MAX_UNDO;
    undo_count = (std::min)(undo_count + 1, MAX_UNDO);
    unsaved_changes = true;
}