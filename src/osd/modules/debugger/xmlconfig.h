// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_OSD_DEBUGGER_XMLCONFIG_H
#define MAME_OSD_DEBUGGER_XMLCONFIG_H

#pragma once

namespace osd::debugger {

// Qt debugger started using these numeric types - they should be switched to mnemonics at some point
enum
{

WINDOW_TYPE_CONSOLE = 1,
WINDOW_TYPE_MEMORY_VIEWER,
WINDOW_TYPE_DISASSEMBLY_VIEWER,
WINDOW_TYPE_ERROR_LOG_VIEWER,
WINDOW_TYPE_POINTS_VIEWER,
WINDOW_TYPE_DEVICES_VIEWER,
WINDOW_TYPE_DEVICE_INFO_VIEWER

};

extern char const *const NODE_WINDOW;
extern char const *const NODE_COLORS;

extern char const *const NODE_WINDOW_SPLITS;
extern char const *const NODE_WINDOW_SELECTION;
extern char const *const NODE_WINDOW_SCROLL;
extern char const *const NODE_WINDOW_EXPRESSION;
extern char const *const NODE_WINDOW_HISTORY;

extern char const *const NODE_HISTORY_ITEM;

extern char const *const ATTR_DEBUGGER_SAVE_WINDOWS;
extern char const *const ATTR_DEBUGGER_GROUP_WINDOWS;

extern char const *const ATTR_WINDOW_TYPE;
extern char const *const ATTR_WINDOW_POSITION_X;
extern char const *const ATTR_WINDOW_POSITION_Y;
extern char const *const ATTR_WINDOW_WIDTH;
extern char const *const ATTR_WINDOW_HEIGHT;

extern char const *const ATTR_WINDOW_MEMORY_REGION;
extern char const *const ATTR_WINDOW_MEMORY_REVERSE_COLUMNS;
extern char const *const ATTR_WINDOW_MEMORY_ADDRESS_MODE;
extern char const *const ATTR_WINDOW_MEMORY_ADDRESS_RADIX;
extern char const *const ATTR_WINDOW_MEMORY_DATA_FORMAT;
extern char const *const ATTR_WINDOW_MEMORY_ROW_CHUNKS;

extern char const *const ATTR_WINDOW_DISASSEMBLY_CPU;
extern char const *const ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN;

extern char const *const ATTR_WINDOW_POINTS_TYPE;

extern char const *const ATTR_WINDOW_DEVICE_TAG;

extern char const *const ATTR_COLORS_THEME;

extern char const *const ATTR_SPLITS_CONSOLE_STATE;
extern char const *const ATTR_SPLITS_CONSOLE_DISASSEMBLY;

extern char const *const ATTR_SELECTION_CURSOR_VISIBLE;
extern char const *const ATTR_SELECTION_CURSOR_X;
extern char const *const ATTR_SELECTION_CURSOR_Y;

extern char const *const ATTR_SCROLL_ORIGIN_X;
extern char const *const ATTR_SCROLL_ORIGIN_Y;

} // namespace osd::debugger

#endif // MAME_OSD_DEBUGGER_XMLCONFIG_H
