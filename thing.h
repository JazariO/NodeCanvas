#pragma once
#include <cstdint>

#define MAX_THINGS 32000
#define MAX_TEXT_LENGTH 512
#define MAX_OUTPUT_PORTS 64
#define MAX_EDGES_PER_NODE 32
#define MAX_STICKY_NOTES 256
#define MAX_POPUPS 4

enum class ThingType {
    Node,
    Edge,
    StickyNote,
    PopupMenu,
    ContextMenu,
    UI_Button,
    UI_Label,
    FontStyle,
    CanvasBackground,
    TextBlock,
    ControlPoint,
    SelectionBox,
    ZoomController,
    SavePrompt,
    ExportPreview
};

struct Thing {
    ThingType type;              // Type of the Thing
    bool is_selected;            // Selection flag for operations
    bool is_hidden;              // Hidden Things are not selectable until reactivated
    int32_t id;                  // Unique ID (index in the array)
    float x, y;                  // Position (for nodes, UI elements, etc.)
    float width, height;         // Size (for nodes, sticky notes, etc.)
    char text[MAX_TEXT_LENGTH];  // Text for nodes, sticky notes, labels
    uint32_t color;              // RGBA color (for nodes, edges, etc.)
    // TODO: Add ports, edges, and other fields as needed
};

struct ThingArray {
    Thing things[MAX_THINGS];
    int32_t count;               // Number of active Things
};