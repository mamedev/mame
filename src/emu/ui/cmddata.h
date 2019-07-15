// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/cmddata.h

*********************************************************************/
#ifndef MAME_EMU_UI_CMDDATA_H
#define MAME_EMU_UI_CMDDATA_H

#pragma once

#define BUTTON_COLOR_RED        rgb_t(255,64,64)
#define BUTTON_COLOR_YELLOW     rgb_t(255,238,0)
#define BUTTON_COLOR_GREEN      rgb_t(0,255,64)
#define BUTTON_COLOR_BLUE       rgb_t(0,170,255)
#define BUTTON_COLOR_PURPLE     rgb_t(170,0,255)
#define BUTTON_COLOR_PINK       rgb_t(255,0,170)
#define BUTTON_COLOR_AQUA       rgb_t(0,255,204)
#define BUTTON_COLOR_SILVER     rgb_t(255,0,255)
#define BUTTON_COLOR_NAVY       rgb_t(255,160,0)
#define BUTTON_COLOR_LIME       rgb_t(190,190,190)

enum
{
	B_COLOR_RED,
	B_COLOR_YELLOW,
	B_COLOR_GREEN,
	B_COLOR_BLUE,
	B_COLOR_PURPLE,
	B_COLOR_PINK,
	B_COLOR_AQUA,
	B_COLOR_SILVER,
	B_COLOR_NAVY,
	B_COLOR_LIME,
	MAX_COLORTABLE
};

// command.dat symbols assigned to Unicode PUA U+E000
#define COMMAND_UNICODE (0xe000)
#define MAX_GLYPH_FONT  (150)

// Define Game Command Font Converting Conditions
#define COMMAND_DEFAULT_TEXT    '_'

// Define Expanded Game Command ShortCut
#define COMMAND_EXPAND_TEXT  '^'

// Define Simple Game Command ShortCut
#define COMMAND_CONVERT_TEXT    '@'

// Defined Game Command Font Color Array
static rgb_t const color_table[] =
{
	0,                   // dummy
	BUTTON_COLOR_RED,    // BTN_A
	BUTTON_COLOR_YELLOW, // BTN_B
	BUTTON_COLOR_GREEN,  // BTN_C
	BUTTON_COLOR_BLUE,   // BTN_D
	BUTTON_COLOR_PINK,   // BTN_E
	BUTTON_COLOR_PURPLE, // BTN_F
	BUTTON_COLOR_AQUA,   // BTN_G
	BUTTON_COLOR_SILVER, // BTN_H
	BUTTON_COLOR_NAVY,   // BTN_I
	BUTTON_COLOR_LIME,   // BTN_J
	BUTTON_COLOR_RED,    // BTN_K
	BUTTON_COLOR_YELLOW, // BTN_L
	BUTTON_COLOR_GREEN,  // BTN_M
	BUTTON_COLOR_BLUE,   // BTN_N
	BUTTON_COLOR_PINK,   // BTN_O
	BUTTON_COLOR_PURPLE, // BTN_P
	BUTTON_COLOR_AQUA,   // BTN_Q
	BUTTON_COLOR_SILVER, // BTN_R
	BUTTON_COLOR_NAVY,   // BTN_S
	BUTTON_COLOR_LIME,   // BTN_T
	BUTTON_COLOR_RED,    // BTN_U
	BUTTON_COLOR_YELLOW, // BTN_V
	BUTTON_COLOR_GREEN,  // BTN_W
	BUTTON_COLOR_BLUE,   // BTN_X
	BUTTON_COLOR_PINK,   // BTN_Y
	BUTTON_COLOR_PURPLE, // BTN_Z
	BUTTON_COLOR_RED,    // BTN_1
	BUTTON_COLOR_YELLOW, // BTN_2
	BUTTON_COLOR_GREEN,  // BTN_3
	BUTTON_COLOR_BLUE,   // BTN_4
	BUTTON_COLOR_PINK,   // BTN_5
	BUTTON_COLOR_PURPLE, // BTN_6
	BUTTON_COLOR_AQUA,   // BTN_7
	BUTTON_COLOR_SILVER, // BTN_8
	BUTTON_COLOR_NAVY,   // BTN_9
	BUTTON_COLOR_LIME,   // BTN_10
	BUTTON_COLOR_BLUE,   // BTN_DEC
	BUTTON_COLOR_RED,    // BTN_INC
	0,                   // BTN_+
	0,                   // DIR_...
	0,                   // DIR_1
	0,                   // DIR_2
	0,                   // DIR_3
	0,                   // DIR_4
	BUTTON_COLOR_RED,    // Joystick Ball
	0,                   // DIR_6
	0,                   // DIR_7
	0,                   // DIR_8
	0,                   // DIR_9
	0,                   // DIR_N
	BUTTON_COLOR_RED,    // BTN_START
	BUTTON_COLOR_YELLOW, // BTN_SELECT
	BUTTON_COLOR_PINK,   // BTN_PUNCH
	BUTTON_COLOR_PURPLE, // BTN_KICK
	BUTTON_COLOR_BLUE,   // BTN_GUARD
	0,
	BUTTON_COLOR_YELLOW, // Light Punch
	BUTTON_COLOR_NAVY,   // Middle Punch
	BUTTON_COLOR_RED,    // Strong Punch
	BUTTON_COLOR_LIME,   // Light Kick
	BUTTON_COLOR_AQUA,   // Middle Kick
	BUTTON_COLOR_BLUE,   // Strong Kick
	BUTTON_COLOR_PURPLE, // 3 Kick
	BUTTON_COLOR_PINK,   // 3 Punch
	BUTTON_COLOR_PURPLE, // 2 Kick
	BUTTON_COLOR_PINK,   // 2 Punch
	BUTTON_COLOR_RED,    // CUSTOM_1
	BUTTON_COLOR_YELLOW, // CUSTOM_2
	BUTTON_COLOR_GREEN,  // CUSTOM_3
	BUTTON_COLOR_BLUE,   // CUSTOM_4
	BUTTON_COLOR_PINK,   // CUSTOM_5
	BUTTON_COLOR_PURPLE, // CUSTOM_6
	BUTTON_COLOR_AQUA,   // CUSTOM_7
	BUTTON_COLOR_SILVER, // CUSTOM_8
	BUTTON_COLOR_RED,    // (Cursor Up)
	BUTTON_COLOR_YELLOW, // (Cursor Down)
	BUTTON_COLOR_GREEN,  // (Cursor Left)
	BUTTON_COLOR_BLUE,   // (Cursor Right)
	0,                   // Non Player Lever
	BUTTON_COLOR_LIME,   // Gray Color Lever
	BUTTON_COLOR_RED,    // 1 Player Lever
	BUTTON_COLOR_YELLOW, // 2 Player Lever
	BUTTON_COLOR_GREEN,  // 3 Player Lever
	BUTTON_COLOR_BLUE,   // 4 Player Lever
	BUTTON_COLOR_PINK,   // 5 Player Lever
	BUTTON_COLOR_PURPLE, // 6 Player Lever
	BUTTON_COLOR_AQUA,   // 7 Player Lever
	BUTTON_COLOR_SILVER  // 8 Player Lever
};

#define COLOR_BUTTONS   ARRAY_LENGTH(color_table)

struct fix_command_t
{
	char        glyph_char;
	unsigned    glyph_code;
};


struct fix_strings_t
{
	char const  *glyph_str;
	int const   glyph_code;
	unsigned    glyph_str_len;
};

static fix_command_t const default_text[] =
{
	// Alphabetic Buttons (NeoGeo): A~D,H,Z
	{ 'A', 1 },     // BTN_A
	{ 'B', 2 },     // BTN_B
	{ 'C', 3 },     // BTN_C
	{ 'D', 4 },     // BTN_D
	{ 'H', 8 },     // BTN_H
	{ 'Z', 26 },    // BTN_Z
	// Numerical Buttons (Capcom): 1~10
	{ 'a', 27 },    // BTN_1
	{ 'b', 28 },    // BTN_2
	{ 'c', 29 },    // BTN_3
	{ 'd', 30 },    // BTN_4
	{ 'e', 31 },    // BTN_5
	{ 'f', 32 },    // BTN_6
	{ 'g', 33 },    // BTN_7
	{ 'h', 34 },    // BTN_8
	{ 'i', 35 },    // BTN_9
	{ 'j', 36 },    // BTN_10
	// Directions of Arrow, Joystick Ball
	{ '+', 39 },    // BTN_+
	{ '.', 40 },    // DIR_...
	{ '1', 41 },    // DIR_1
	{ '2', 42 },    // DIR_2
	{ '3', 43 },    // DIR_3
	{ '4', 44 },    // DIR_4
	{ '5', 45 },    // Joystick Ball
	{ '6', 46 },    // DIR_6
	{ '7', 47 },    // DIR_7
	{ '8', 48 },    // DIR_8
	{ '9', 49 },    // DIR_9
	{ 'N', 50 },    // DIR_N
	// Special Buttons
	{ 'S', 51 },    // BTN_START
	{ 'P', 53 },    // BTN_PUNCH
	{ 'K', 54 },    // BTN_KICK
	{ 'G', 55 },    // BTN_GUARD
	// Composition of Arrow Directions
	{ '!',  90 },   // Arrow
	{ 'k', 100 },   // Half Circle Back
	{ 'l', 101 },   // Half Circle Front Up
	{ 'm', 102 },   // Half Circle Front
	{ 'n', 103 },   // Half Circle Back Up
	{ 'o', 104 },   // 1/4 Cir For 2 Down
	{ 'p', 105 },   // 1/4 Cir Down 2 Back
	{ 'q', 106 },   // 1/4 Cir Back 2 Up
	{ 'r', 107 },   // 1/4 Cir Up 2 For
	{ 's', 108 },   // 1/4 Cir Back 2 Down
	{ 't', 109 },   // 1/4 Cir Down 2 For
	{ 'u', 110 },   // 1/4 Cir For 2 Up
	{ 'v', 111 },   // 1/4 Cir Up 2 Back
	{ 'w', 112 },   // Full Clock Forward
	{ 'x', 113 },   // Full Clock Back
	{ 'y', 114 },   // Full Count Forward
	{ 'z', 115 },   // Full Count Back
	{ 'L', 116 },   // 2x Forward
	{ 'M', 117 },   // 2x Back
	{ 'Q', 118 },   // Dragon Screw Forward
	{ 'R', 119 },   // Dragon Screw Back
	// Big letter Text
	{ '^', 121 },   // AIR
	{ '?', 122 },   // DIR
	{ 'X', 124 },   // TAP
	// Condition of Positions
	{ '|', 125 },   // Jump
	{ 'O', 126 },   // Hold
	{ '-', 127 },   // Air
	{ '=', 128 },   // Squatting
	{ '~', 131 },   // Charge
	// Special Character Text
	{ '`', 135 },   // Small Dot
	{ '@', 136 },   // Double Ball
	{ ')', 137 },   // Single Ball
	{ '(', 138 },   // Solid Ball
	{ '*', 139 },   // Star
	{ '&', 140 },   // Solid star
	{ '%', 141 },   // Triangle
	{ '$', 142 },   // Solid Triangle
	{ '#', 143 },   // Double Square
	{ ']', 144 },   // Single Square
	{ '[', 145 },   // Solid Square
	{ '{', 146 },   // Down Triangle
	{ '}', 147 },   // Solid Down Triangle
	{ '<', 148 },   // Diamond
	{ '>', 149 },   // Solid Diamond
	{ 0, 0 }    // end of array
};

static fix_command_t const expand_text[] =
{
	// Alphabetic Buttons (NeoGeo): S (Slash Button)
	{ 's', 19 },    // BTN_S
	// Special Buttons
	{ 'S', 52 },    // BTN_SELECT
	// Multiple Punches & Kicks
	{ 'E', 57 },    // Light  Punch
	{ 'F', 58 },    // Middle Punch
	{ 'G', 59 },    // Strong Punch
	{ 'H', 60 },    // Light  Kick
	{ 'I', 61 },    // Middle Kick
	{ 'J', 62 },    // Strong Kick
	{ 'T', 63 },    // 3 Kick
	{ 'U', 64 },    // 3 Punch
	{ 'V', 65 },    // 2 Kick
	{ 'W', 66 },    // 2 Pick
	// Composition of Arrow Directions
	{ '!', 91 },    // Continue Arrow
	// Charge of Arrow Directions
	{ '1', 92 },    // Charge DIR_1
	{ '2', 93 },    // Charge DIR_2
	{ '3', 94 },    // Charge DIR_3
	{ '4', 95 },    // Charge DIR_4
	{ '6', 96 },    // Charge DIR_6
	{ '7', 97 },    // Charge DIR_7
	{ '8', 98 },    // Charge DIR_8
	{ '9', 99 },    // Charge DIR_9
	// Big letter Text
	{ 'M', 123 },   // MAX
	// Condition of Positions
	{ '-', 129 },   // Close
	{ '=', 130 },   // Away
	{ '*', 132 },   // Serious Tap
	{ '?', 133 },   // Any Button
	{ 0, 0 }    // end of array
};

static fix_strings_t convert_text[] =
{
	// Alphabetic Buttons: A~Z
	{ "A-button",  1, 0 }, // BTN_A
	{ "B-button",  2, 0 }, // BTN_B
	{ "C-button",  3, 0 }, // BTN_C
	{ "D-button",  4, 0 }, // BTN_D
	{ "E-button",  5, 0 }, // BTN_E
	{ "F-button",  6, 0 }, // BTN_F
	{ "G-button",  7, 0 }, // BTN_G
	{ "H-button",  8, 0 }, // BTN_H
	{ "I-button",  9, 0 }, // BTN_I
	{ "J-button", 10, 0 }, // BTN_J
	{ "K-button", 11, 0 }, // BTN_K
	{ "L-button", 12, 0 }, // BTN_L
	{ "M-button", 13, 0 }, // BTN_M
	{ "N-button", 14, 0 }, // BTN_N
	{ "O-button", 15, 0 }, // BTN_O
	{ "P-button", 16, 0 }, // BTN_P
	{ "Q-button", 17, 0 }, // BTN_Q
	{ "R-button", 18, 0 }, // BTN_R
	{ "S-button", 19, 0 }, // BTN_S
	{ "T-button", 20, 0 }, // BTN_T
	{ "U-button", 21, 0 }, // BTN_U
	{ "V-button", 22, 0 }, // BTN_V
	{ "W-button", 23, 0 }, // BTN_W
	{ "X-button", 24, 0 }, // BTN_X
	{ "Y-button", 25, 0 }, // BTN_Y
	{ "Z-button", 26, 0 }, // BTN_Z
	// Special Moves and Buttons
	{ "decrease", 37, 0 }, // BTN_DEC
	{ "increase", 38, 0 }, // BTN_INC
	{ "BALL",     45, 0 }, // Joystick Ball
	{ "start",    51, 0 }, // BTN_START
	{ "select",   52, 0 }, // BTN_SELECT
	{ "punch",    53, 0 }, // BTN_PUNCH
	{ "kick",     54, 0 }, // BTN_KICK
	{ "guard",    55, 0 }, // BTN_GUARD
	{ "L-punch",  57, 0 }, // Light Punch
	{ "M-punch",  58, 0 }, // Middle Punch
	{ "S-punch",  59, 0 }, // Strong Punch
	{ "L-kick",   60, 0 }, // Light Kick
	{ "M-kick",   61, 0 }, // Middle Kick
	{ "S-kick",   62, 0 }, // Strong Kick
	{ "3-kick",   63, 0 }, // 3 Kick
	{ "3-punch",  64, 0 }, // 3 Punch
	{ "2-kick",   65, 0 }, // 2 Kick
	{ "2-punch",  66, 0 }, // 2 Pick
	// Custom Buttons and Cursor Buttons
	{ "custom1",  67, 0 }, // CUSTOM_1
	{ "custom2",  68, 0 }, // CUSTOM_2
	{ "custom3",  69, 0 }, // CUSTOM_3
	{ "custom4",  70, 0 }, // CUSTOM_4
	{ "custom5",  71, 0 }, // CUSTOM_5
	{ "custom6",  72, 0 }, // CUSTOM_6
	{ "custom7",  73, 0 }, // CUSTOM_7
	{ "custom8",  74, 0 }, // CUSTOM_8
	{ "up",       75, 0 }, // (Cursor Up)
	{ "down",     76, 0 }, // (Cursor Down)
	{ "left",     77, 0 }, // (Cursor Left)
	{ "right",    78, 0 }, // (Cursor Right)
	// Player Lever
	{ "lever",    79, 0 }, // Non Player Lever
	{ "nplayer",  80, 0 }, // Gray Color Lever
	{ "1player",  81, 0 }, // 1 Player Lever
	{ "2player",  82, 0 }, // 2 Player Lever
	{ "3player",  83, 0 }, // 3 Player Lever
	{ "4player",  84, 0 }, // 4 Player Lever
	{ "5player",  85, 0 }, // 5 Player Lever
	{ "6player",  86, 0 }, // 6 Player Lever
	{ "7player",  87, 0 }, // 7 Player Lever
	{ "8player",  88, 0 }, // 8 Player Lever
	// Composition of Arrow Directions
	{ "-->",      90, 0 }, // Arrow
	{ "==>",      91, 0 }, // Continue Arrow
	{ "hcb",     100, 0 }, // Half Circle Back
	{ "huf",     101, 0 }, // Half Circle Front Up
	{ "hcf",     102, 0 }, // Half Circle Front
	{ "hub",     103, 0 }, // Half Circle Back Up
	{ "qfd",     104, 0 }, // 1/4 Cir For 2 Down
	{ "qdb",     105, 0 }, // 1/4 Cir Down 2 Back
	{ "qbu",     106, 0 }, // 1/4 Cir Back 2 Up
	{ "quf",     107, 0 }, // 1/4 Cir Up 2 For
	{ "qbd",     108, 0 }, // 1/4 Cir Back 2 Down
	{ "qdf",     109, 0 }, // 1/4 Cir Down 2 For
	{ "qfu",     110, 0 }, // 1/4 Cir For 2 Up
	{ "qub",     111, 0 }, // 1/4 Cir Up 2 Back
	{ "fdf",     112, 0 }, // Full Clock Forward
	{ "fub",     113, 0 }, // Full Clock Back
	{ "fuf",     114, 0 }, // Full Count Forward
	{ "fdb",     115, 0 }, // Full Count Back
	{ "xff",     116, 0 }, // 2x Forward
	{ "xbb",     117, 0 }, // 2x Back
	{ "dsf",     118, 0 }, // Dragon Screw Forward
	{ "dsb",     119, 0 }, // Dragon Screw Back
	// Big letter Text
	{ "AIR",     121, 0 }, // AIR
	{ "DIR",     122, 0 }, // DIR
	{ "MAX",     123, 0 }, // MAX
	{ "TAP",     124, 0 }, // TAP
	// Condition of Positions
	{ "jump",    125, 0 }, // Jump
	{ "hold",    126, 0 }, // Hold
	{ "air",     127, 0 }, // Air
	{ "sit",     128, 0 }, // Squatting
	{ "close",   129, 0 }, // Close
	{ "away",    130, 0 }, // Away
	{ "charge",  131, 0 }, // Charge
	{ "tap",     132, 0 }, // Serious Tap
	{ "button",  133, 0 }, // Any Button
	{ nullptr,     0, 0 }  // end of array
};

#endif // MAME_EMU_UI_CMDDATA_H
