#ifndef THING_H
#define THING_H

#include <windows.h>

#define MAX_TEXT_LENGTH 512
#define MAX_OUTPUT_PORTS 64
#define MAX_EDGES_PER_NODE 32
#define MAX_STICKY_NOTES 256
#define MAX_POPUPS 4

enum ThingType {
    THING_NODE,
    THING_EDGE,
    THING_STICKY_NOTE,
    THING_POPUP_MENU,
    THING_CONTEXT_MENU,
    THING_UI_BUTTON,
    THING_UI_LABEL,
    THING_FONT_STYLE,
    THING_CANVAS_BACKGROUND,
    THING_TEXT_BLOCK,
    THING_CONTROL_POINT,
    THING_SELECTION_BOX,
    THING_ZOOM_CONTROLLER,
    THING_SAVE_PROMPT,
    THING_EXPORT_PREVIEW
};

struct Thing {
    ThingType type;
    bool is_active;
    bool is_selected;
    union {
        struct { // Node
            POINT pos;
            COLORREF color;
            char text[MAX_TEXT_LENGTH];
            int output_ports[MAX_OUTPUT_PORTS];
            int output_port_count;
            int input_edge;
        } node;
        struct { // Edge
            int from_node;
            int from_port;
            int to_node;
            POINT control_points[4];
            bool has_control_point_overrides;
        } edge;
        struct { // Sticky Note
            POINT pos;
            SIZE size;
            char text[MAX_TEXT_LENGTH];
        } sticky_note;
        struct { // Popup Menu
            POINT pos;
            int target_thing;
        } popup_menu;
        // Add other Thing types as needed
    } data;
};

#endif