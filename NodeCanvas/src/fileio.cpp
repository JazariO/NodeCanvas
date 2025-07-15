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
    wchar_t filename[MAX_PATH] = L""; // Changed to wchar_t
    if (!has_file) {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) }; // Use OPENFILENAMEW for Unicode
        ofn.hwndOwner = app->hwnd;
        ofn.lpstrFilter = L"NodeCanvas Files (*.nodec)\0*.nodec\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = L"nodec";
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn)) { // Use GetSaveFileNameW
            has_file = true;
            wcscpy_s(current_file, MAX_PATH, filename); // Use wcscpy_s
        }
        else {
            return;
        }
    }

    FILE* file;
    if (_wfopen_s(&file, current_file, L"wb") == 0) { // Use _wfopen_s for Unicode
        fwrite(&app->thing_count, sizeof(int), 1, file);
        fwrite(app->things, sizeof(Thing), app->thing_count, file);
        fclose(file);
        app->unsaved_changes = false;
    }
}

void FileIO::Load(App* app) {
    wchar_t filename[MAX_PATH] = L""; // Changed to wchar_t
    OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) }; // Use OPENFILENAMEW
    ofn.hwndOwner = app->hwnd;
    ofn.lpstrFilter = L"NodeCanvas Files (*.nodec)\0*.nodec\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) { // Use GetOpenFileNameW
        FILE* file;
        if (_wfopen_s(&file, filename, L"rb") == 0) { // Use _wfopen_s
            app->SaveUndo();
            fread(&app->thing_count, sizeof(int), 1, file);
            if (app->thing_count <= MAX_THINGS) {
                fread(app->things, sizeof(Thing), app->thing_count, file);
            }
            fclose(file);
            has_file = true;
            wcscpy_s(current_file, MAX_PATH, filename); // Use wcscpy_s
            app->unsaved_changes = false;
            InvalidateRect(app->hwnd, nullptr, TRUE);
        }
    }
}