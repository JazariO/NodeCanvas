#ifndef RENDER_H
#define RENDER_H

#include <windows.h>
#include "thing.h"

struct App;

namespace NodeCanvasRender {
    void DrawCanvasBackground(HDC hdc, RECT bounds);
    void DrawNode(HDC hdc, Thing* thing, App* app);
    void DrawEdge(HDC hdc, Thing* thing, App* app);
    void DrawStickyNote(HDC hdc, Thing* thing, App* app);
}

#endif