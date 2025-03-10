// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "xmlconfig.h"

namespace osd::debugger {

char const *const NODE_WINDOW = "window";
char const *const NODE_COLORS = "colors";

char const *const NODE_WINDOW_SPLITS = "splits";
char const *const NODE_WINDOW_SELECTION = "selection";
char const *const NODE_WINDOW_SCROLL = "scroll";
char const *const NODE_WINDOW_EXPRESSION = "expression";
char const *const NODE_WINDOW_HISTORY = "history";

char const *const NODE_HISTORY_ITEM = "item";

char const *const ATTR_DEBUGGER_SAVE_WINDOWS = "savewindows";
char const *const ATTR_DEBUGGER_GROUP_WINDOWS = "groupwindows";

char const *const ATTR_WINDOW_TYPE = "type";
char const *const ATTR_WINDOW_POSITION_X = "position_x";
char const *const ATTR_WINDOW_POSITION_Y = "position_y";
char const *const ATTR_WINDOW_WIDTH = "size_x";
char const *const ATTR_WINDOW_HEIGHT = "size_y";

char const *const ATTR_WINDOW_MEMORY_REGION = "memoryregion";
char const *const ATTR_WINDOW_MEMORY_REVERSE_COLUMNS = "reverse";
char const *const ATTR_WINDOW_MEMORY_ADDRESS_MODE = "addressmode";
char const *const ATTR_WINDOW_MEMORY_ADDRESS_RADIX = "addressradix";
char const *const ATTR_WINDOW_MEMORY_DATA_FORMAT = "dataformat";
char const *const ATTR_WINDOW_MEMORY_ROW_CHUNKS = "rowchunks";

char const *const ATTR_WINDOW_DISASSEMBLY_CPU = "cpu";
char const *const ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN = "rightbar";

char const *const ATTR_WINDOW_POINTS_TYPE = "bwtype";

char const *const ATTR_WINDOW_DEVICE_TAG = "device-tag";

char const *const ATTR_COLORS_THEME = "theme";

char const *const ATTR_SPLITS_CONSOLE_STATE = "state";
char const *const ATTR_SPLITS_CONSOLE_DISASSEMBLY = "disassembly";

char const *const ATTR_SELECTION_CURSOR_VISIBLE = "visible";
char const *const ATTR_SELECTION_CURSOR_X = "start_x";
char const *const ATTR_SELECTION_CURSOR_Y = "start_y";

char const *const ATTR_SCROLL_ORIGIN_X = "position_x";
char const *const ATTR_SCROLL_ORIGIN_Y = "position_y";

} // namespace osd::debugger
