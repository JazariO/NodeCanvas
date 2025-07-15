#ifndef FILEIO_H
#define FILEIO_H

#include <windows.h>

// Forward declaration
struct App;

class FileIO {
public:
    bool has_file;
    wchar_t current_file[MAX_PATH];

    bool PromptSaveUnsaved(HWND hwnd, App* app);
    void Save(App* app);
    void Load(App* app);
};

#endif