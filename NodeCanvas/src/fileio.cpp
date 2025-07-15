#include "../include/fileio.h"
#include "../include/app.h"
#include <commdlg.h>
#include <stdio.h>

bool FileIO::PromptSaveUnsaved(HWND hwnd, App* app) {
    if (!app->unsaved_changes) return true;

    int result = MessageBoxW(hwnd, L"You have unsaved changes. Save before closing?", L"Save Changes", MB_YESNOCANCEL | MB_ICONWARNING);
    if (result == IDYES) {
        Save(app);
        return true;
    }
    else if (result == IDNO) {
        return true;
    }
    return false; // IDCANCEL
}

void FileIO::Save(App* app) {
    wchar_t filename[MAX_PATH] = L"";
    if (!has_file) {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
        ofn.hwndOwner = app->hwnd;
        ofn.lpstrFilter = L"NodeCanvas Files (*.nodec)\0*.nodec\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = L"nodec";
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn)) {
            has_file = true;
            wcscpy_s(current_file, MAX_PATH, filename);
        }
        else {
            return;
        }
    }

    FILE* file;
    if (_wfopen_s(&file, current_file, L"wb") == 0) {
        fwrite(&app->thing_count, sizeof(int), 1, file);
        fwrite(app->things, sizeof(Thing), app->thing_count, file);
        fclose(file);
        app->unsaved_changes = false;
    }
}

void FileIO::Load(App* app) {
    wchar_t filename[MAX_PATH] = L"";
    OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
    ofn.hwndOwner = app->hwnd;
    ofn.lpstrFilter = L"NodeCanvas Files (*.nodec)\0*.nodec\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        FILE* file;
        if (_wfopen_s(&file, filename, L"rb") == 0) {
            // Save current state before loading
            app->SaveUndo();

            int loaded_count;
            if (fread(&loaded_count, sizeof(int), 1, file) == 1) {
                if (loaded_count > 0 && loaded_count <= MAX_THINGS) {
                    // Clear existing things
                    for (int i = 0; i < app->thing_count; i++) {
                        app->things[i].is_active = false;
                    }

                    // Load new things
                    if (fread(app->things, sizeof(Thing), loaded_count, file) == loaded_count) {
                        app->thing_count = loaded_count;
                        has_file = true;
                        wcscpy_s(current_file, MAX_PATH, filename);
                        app->unsaved_changes = false;

                        // Reset canvas view to default instead of auto-focusing
                        app->canvas->zoom = 1.0f;
                        app->canvas->pan = { 0, 0 };

                        // Optional: Focus on loaded content (comment out if causing issues)
                        // app->canvas->FocusAll(app);

                        InvalidateRect(app->hwnd, nullptr, TRUE);
                    }
                }
            }
            fclose(file);
        }
        else {
            MessageBoxW(app->hwnd, L"Failed to open file", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}