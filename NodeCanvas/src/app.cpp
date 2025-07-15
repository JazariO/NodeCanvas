#include "../include/app.h"
#include <algorithm>

void App::Init(HWND hwnd) {
    this->hwnd = hwnd;

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

    things[thing_count].type = THING_CANVAS_BACKGROUND;
    things[thing_count].is_active = true;
    thing_count++;

    for (int i = 0; i < MAX_UNDO; i++) {
        undo_stack[i].things = new Thing[MAX_THINGS];
    }
}

void App::Update() {
    InvalidateRect(hwnd, nullptr, TRUE);
}

void App::SaveUndo() {
    if (undo_count >= MAX_UNDO) return;

    // Clear any future undo states when we save a new one
    undo_count = undo_index + 1;

    undo_stack[undo_index].thing_count = thing_count;
    memcpy(undo_stack[undo_index].things, things, sizeof(Thing) * thing_count);
    undo_index = (undo_index + 1) % MAX_UNDO;

    // Only increment count if we haven't reached max
    if (undo_count < MAX_UNDO) {
        undo_count++;
    }

    unsaved_changes = true;
}

void App::Undo() {
    if (undo_count <= 1) return; // Need at least one state to undo to

    // Move back one step
    undo_index = (undo_index - 1 + MAX_UNDO) % MAX_UNDO;

    // Load the previous state
    thing_count = undo_stack[undo_index].thing_count;
    memcpy(things, undo_stack[undo_index].things, sizeof(Thing) * thing_count);

    unsaved_changes = true;
    InvalidateRect(hwnd, nullptr, TRUE);
}

void App::Redo() {
    // Check if we can redo (there's a state ahead of us)
    int next_index = (undo_index + 1) % MAX_UNDO;
    if (next_index >= undo_count) return;

    // Move forward one step
    undo_index = next_index;

    // Load the next state
    thing_count = undo_stack[undo_index].thing_count;
    memcpy(things, undo_stack[undo_index].things, sizeof(Thing) * thing_count);

    unsaved_changes = true;
    InvalidateRect(hwnd, nullptr, TRUE);
}