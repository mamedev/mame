// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ioport.h

    Input/output port handling.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __INPTPORT_H__
#define __INPTPORT_H__

#include <time.h>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// input ports support up to 32 bits each
typedef UINT32 ioport_value;

// active high/low values for input ports
const ioport_value IP_ACTIVE_HIGH = 0x00000000;
const ioport_value IP_ACTIVE_LOW = 0xffffffff;

// maximum number of players supported
const int MAX_PLAYERS = 8;

// INP file parameters
const UINT32 INP_HEADER_SIZE = 64;
const UINT32 INP_HEADER_MAJVERSION = 3;
const UINT32 INP_HEADER_MINVERSION = 0;

// unicode constants
const unicode_char UCHAR_PRIVATE = 0x100000;
const unicode_char UCHAR_SHIFT_1 = UCHAR_PRIVATE + 0;
const unicode_char UCHAR_SHIFT_2 = UCHAR_PRIVATE + 1;
const unicode_char UCHAR_SHIFT_BEGIN = UCHAR_SHIFT_1;
const unicode_char UCHAR_SHIFT_END = UCHAR_SHIFT_2;
const unicode_char UCHAR_MAMEKEY_BEGIN = UCHAR_PRIVATE + 2;


// sequence types for input_port_seq() call
enum input_seq_type
{
	SEQ_TYPE_INVALID = -1,
	SEQ_TYPE_STANDARD = 0,
	SEQ_TYPE_INCREMENT,
	SEQ_TYPE_DECREMENT,
	SEQ_TYPE_TOTAL
};
DECLARE_ENUM_OPERATORS(input_seq_type)


// crosshair types
enum crosshair_axis_t
{
	CROSSHAIR_AXIS_NONE = 0,
	CROSSHAIR_AXIS_X,
	CROSSHAIR_AXIS_Y
};


// groups for input ports
enum ioport_group
{
	IPG_UI = 0,
	IPG_PLAYER1,
	IPG_PLAYER2,
	IPG_PLAYER3,
	IPG_PLAYER4,
	IPG_PLAYER5,
	IPG_PLAYER6,
	IPG_PLAYER7,
	IPG_PLAYER8,
	IPG_OTHER,
	IPG_TOTAL_GROUPS,
	IPG_INVALID
};


// various input port types
enum ioport_type
{
	// pseudo-port types
	IPT_INVALID = 0,
	IPT_UNUSED,
	IPT_END,
	IPT_UNKNOWN,
	IPT_PORT,
	IPT_DIPSWITCH,
	IPT_CONFIG,

	// start buttons
	IPT_START1,
	IPT_START2,
	IPT_START3,
	IPT_START4,
	IPT_START5,
	IPT_START6,
	IPT_START7,
	IPT_START8,

	// coin slots
	IPT_COIN1,
	IPT_COIN2,
	IPT_COIN3,
	IPT_COIN4,
	IPT_COIN5,
	IPT_COIN6,
	IPT_COIN7,
	IPT_COIN8,
	IPT_COIN9,
	IPT_COIN10,
	IPT_COIN11,
	IPT_COIN12,
	IPT_BILL1,

	// service coin
	IPT_SERVICE1,
	IPT_SERVICE2,
	IPT_SERVICE3,
	IPT_SERVICE4,

	// tilt inputs
	IPT_TILT1,
	IPT_TILT2,
	IPT_TILT3,
	IPT_TILT4,

	// misc other digital inputs
	IPT_SERVICE,
	IPT_TILT,
	IPT_INTERLOCK,
	IPT_VOLUME_UP,
	IPT_VOLUME_DOWN,
	IPT_START,                  // MESS only
	IPT_SELECT,                 // MESS only
	IPT_KEYPAD,                 // MESS only
	IPT_KEYBOARD,               // MESS only

	// digital joystick inputs
	IPT_DIGITAL_JOYSTICK_FIRST,

		// use IPT_JOYSTICK for panels where the player has one single joystick
		IPT_JOYSTICK_UP,
		IPT_JOYSTICK_DOWN,
		IPT_JOYSTICK_LEFT,
		IPT_JOYSTICK_RIGHT,

		// use IPT_JOYSTICKLEFT and IPT_JOYSTICKRIGHT for dual joystick panels
		IPT_JOYSTICKRIGHT_UP,
		IPT_JOYSTICKRIGHT_DOWN,
		IPT_JOYSTICKRIGHT_LEFT,
		IPT_JOYSTICKRIGHT_RIGHT,
		IPT_JOYSTICKLEFT_UP,
		IPT_JOYSTICKLEFT_DOWN,
		IPT_JOYSTICKLEFT_LEFT,
		IPT_JOYSTICKLEFT_RIGHT,

	IPT_DIGITAL_JOYSTICK_LAST,

	// action buttons
	IPT_BUTTON1,
	IPT_BUTTON2,
	IPT_BUTTON3,
	IPT_BUTTON4,
	IPT_BUTTON5,
	IPT_BUTTON6,
	IPT_BUTTON7,
	IPT_BUTTON8,
	IPT_BUTTON9,
	IPT_BUTTON10,
	IPT_BUTTON11,
	IPT_BUTTON12,
	IPT_BUTTON13,
	IPT_BUTTON14,
	IPT_BUTTON15,
	IPT_BUTTON16,

	// mahjong inputs
	IPT_MAHJONG_FIRST,

		IPT_MAHJONG_A,
		IPT_MAHJONG_B,
		IPT_MAHJONG_C,
		IPT_MAHJONG_D,
		IPT_MAHJONG_E,
		IPT_MAHJONG_F,
		IPT_MAHJONG_G,
		IPT_MAHJONG_H,
		IPT_MAHJONG_I,
		IPT_MAHJONG_J,
		IPT_MAHJONG_K,
		IPT_MAHJONG_L,
		IPT_MAHJONG_M,
		IPT_MAHJONG_N,
		IPT_MAHJONG_O,
		IPT_MAHJONG_P,
		IPT_MAHJONG_Q,
		IPT_MAHJONG_KAN,
		IPT_MAHJONG_PON,
		IPT_MAHJONG_CHI,
		IPT_MAHJONG_REACH,  //IPT_MAHJONG_RIICHI,   // REACH is Japanglish
		IPT_MAHJONG_RON,
		IPT_MAHJONG_BET,
		IPT_MAHJONG_LAST_CHANCE,
		IPT_MAHJONG_SCORE,
		IPT_MAHJONG_DOUBLE_UP,
		IPT_MAHJONG_FLIP_FLOP,
		IPT_MAHJONG_BIG,
		IPT_MAHJONG_SMALL,

	IPT_MAHJONG_LAST,

	// hanafuda inputs
	IPT_HANAFUDA_FIRST,

		IPT_HANAFUDA_A,
		IPT_HANAFUDA_B,
		IPT_HANAFUDA_C,
		IPT_HANAFUDA_D,
		IPT_HANAFUDA_E,
		IPT_HANAFUDA_F,
		IPT_HANAFUDA_G,
		IPT_HANAFUDA_H,
		IPT_HANAFUDA_YES,
		IPT_HANAFUDA_NO,

	IPT_HANAFUDA_LAST,

	// gambling inputs
	IPT_GAMBLING_FIRST,

		IPT_GAMBLE_KEYIN,   // attendant
		IPT_GAMBLE_KEYOUT,  // attendant
		IPT_GAMBLE_SERVICE, // attendant
		IPT_GAMBLE_BOOK,    // attendant
		IPT_GAMBLE_DOOR,    // attendant
	//  IPT_GAMBLE_DOOR2,   // many gambling games have several doors.
	//  IPT_GAMBLE_DOOR3,
	//  IPT_GAMBLE_DOOR4,
	//  IPT_GAMBLE_DOOR5,

		IPT_GAMBLE_HIGH,    // player
		IPT_GAMBLE_LOW,     // player
		IPT_GAMBLE_HALF,    // player
		IPT_GAMBLE_DEAL,    // player
		IPT_GAMBLE_D_UP,    // player
		IPT_GAMBLE_TAKE,    // player
		IPT_GAMBLE_STAND,   // player
		IPT_GAMBLE_BET,     // player
		IPT_GAMBLE_PAYOUT,  // player
	//  IPT_GAMBLE_BUTTON1, // player
	//  IPT_GAMBLE_BUTTON2, // many many gambling games have multi-games and/or multi-function-buttons
	//  IPT_GAMBLE_BUTTON3, // I suggest to eliminate specific names
	//  IPT_GAMBLE_BUTTON4,
	//  IPT_GAMBLE_BUTTON5,
	//  IPT_GAMBLE_BUTTON6,
	//  IPT_GAMBLE_BUTTON7,
	//  IPT_GAMBLE_BUTTON8,
	//  IPT_GAMBLE_BUTTON9,
	//  IPT_GAMBLE_BUTTON10,
	//  IPT_GAMBLE_BUTTON11,
	//  IPT_GAMBLE_BUTTON12,
	//  IPT_GAMBLE_BUTTON13,
	//  IPT_GAMBLE_BUTTON14,
	//  IPT_GAMBLE_BUTTON15,
	//  IPT_GAMBLE_BUTTON16,

		// poker-specific inputs
		IPT_POKER_HOLD1,
		IPT_POKER_HOLD2,
		IPT_POKER_HOLD3,
		IPT_POKER_HOLD4,
		IPT_POKER_HOLD5,
		IPT_POKER_CANCEL,
		IPT_POKER_BET,

		// slot-specific inputs
		IPT_SLOT_STOP1,
		IPT_SLOT_STOP2,
		IPT_SLOT_STOP3,
		IPT_SLOT_STOP4,
		IPT_SLOT_STOP_ALL,

	IPT_GAMBLING_LAST,

	// analog inputs
	IPT_ANALOG_FIRST,

		IPT_ANALOG_ABSOLUTE_FIRST,

			IPT_AD_STICK_X,     // absolute // autocenter
			IPT_AD_STICK_Y,     // absolute // autocenter
			IPT_AD_STICK_Z,     // absolute // autocenter
			IPT_PADDLE,         // absolute // autocenter
			IPT_PADDLE_V,       // absolute // autocenter
			IPT_PEDAL,          // absolute // autocenter
			IPT_PEDAL2,         // absolute // autocenter
			IPT_PEDAL3,         // absolute // autocenter
			IPT_LIGHTGUN_X,     // absolute
			IPT_LIGHTGUN_Y,     // absolute
			IPT_POSITIONAL,     // absolute // autocenter if not wraps
			IPT_POSITIONAL_V,   // absolute // autocenter if not wraps

		IPT_ANALOG_ABSOLUTE_LAST,

		IPT_DIAL,           // relative
		IPT_DIAL_V,         // relative
		IPT_TRACKBALL_X,    // relative
		IPT_TRACKBALL_Y,    // relative
		IPT_MOUSE_X,        // relative
		IPT_MOUSE_Y,        // relative

	IPT_ANALOG_LAST,

	// analog adjuster support
	IPT_ADJUSTER,

	// the following are special codes for user interface handling - not to be used by drivers!
	IPT_UI_FIRST,

		IPT_UI_CONFIGURE,
		IPT_UI_ON_SCREEN_DISPLAY,
		IPT_UI_DEBUG_BREAK,
		IPT_UI_PAUSE,
		IPT_UI_RESET_MACHINE,
		IPT_UI_SOFT_RESET,
		IPT_UI_SHOW_GFX,
		IPT_UI_FRAMESKIP_DEC,
		IPT_UI_FRAMESKIP_INC,
		IPT_UI_THROTTLE,
		IPT_UI_FAST_FORWARD,
		IPT_UI_SHOW_FPS,
		IPT_UI_SNAPSHOT,
		IPT_UI_RECORD_MOVIE,
		IPT_UI_TOGGLE_CHEAT,
		IPT_UI_UP,
		IPT_UI_DOWN,
		IPT_UI_LEFT,
		IPT_UI_RIGHT,
		IPT_UI_HOME,
		IPT_UI_END,
		IPT_UI_PAGE_UP,
		IPT_UI_PAGE_DOWN,
		IPT_UI_SELECT,
		IPT_UI_CANCEL,
		IPT_UI_DISPLAY_COMMENT,
		IPT_UI_CLEAR,
		IPT_UI_ZOOM_IN,
		IPT_UI_ZOOM_OUT,
		IPT_UI_PREV_GROUP,
		IPT_UI_NEXT_GROUP,
		IPT_UI_ROTATE,
		IPT_UI_SHOW_PROFILER,
		IPT_UI_TOGGLE_UI,
		IPT_UI_TOGGLE_DEBUG,
		IPT_UI_PASTE,
		IPT_UI_SAVE_STATE,
		IPT_UI_LOAD_STATE,
		IPT_UI_TAPE_START,
		IPT_UI_TAPE_STOP,

		// additional MEWUI options
		IPT_UI_HISTORY,
		IPT_UI_MAMEINFO,
		IPT_UI_COMMAND,
		IPT_UI_SYSINFO,
		IPT_UI_FAVORITES,
		IPT_UI_STORY,
		IPT_UI_UP_FILTER,
		IPT_UI_DOWN_FILTER,
		IPT_UI_LEFT_PANEL,
		IPT_UI_RIGHT_PANEL,
		IPT_UI_UP_PANEL,
		IPT_UI_DOWN_PANEL,
		IPT_UI_EXPORT,
		IPT_UI_AUDIT_FAST,
		IPT_UI_AUDIT_ALL,

		// additional OSD-specified UI port types (up to 16)
		IPT_OSD_1,
		IPT_OSD_2,
		IPT_OSD_3,
		IPT_OSD_4,
		IPT_OSD_5,
		IPT_OSD_6,
		IPT_OSD_7,
		IPT_OSD_8,
		IPT_OSD_9,
		IPT_OSD_10,
		IPT_OSD_11,
		IPT_OSD_12,
		IPT_OSD_13,
		IPT_OSD_14,
		IPT_OSD_15,
		IPT_OSD_16,

	IPT_UI_LAST,

	// other meaning not mapped to standard defaults
	IPT_OTHER,

	// special meaning handled by custom code
	IPT_SPECIAL,
	IPT_CUSTOM,
	IPT_OUTPUT,

	IPT_COUNT
};
DECLARE_ENUM_OPERATORS(ioport_type)
// aliases for some types
#define IPT_PADDLE_H        IPT_PADDLE
#define IPT_PEDAL1          IPT_PEDAL
#define IPT_POSITIONAL_H    IPT_POSITIONAL
#define IPT_DIAL_H          IPT_DIAL


// input type classes
enum ioport_type_class
{
	INPUT_CLASS_INTERNAL,
	INPUT_CLASS_KEYBOARD,
	INPUT_CLASS_CONTROLLER,
	INPUT_CLASS_CONFIG,
	INPUT_CLASS_DIPSWITCH,
	INPUT_CLASS_MISC
};


// default strings used in port definitions
enum
{
	INPUT_STRING_Off = 1,
	INPUT_STRING_On,
	INPUT_STRING_No,
	INPUT_STRING_Yes,
	INPUT_STRING_Lives,
	INPUT_STRING_Bonus_Life,
	INPUT_STRING_Difficulty,
	INPUT_STRING_Demo_Sounds,
	INPUT_STRING_Coinage,
	INPUT_STRING_Coin_A,
	INPUT_STRING_Coin_B,
//  INPUT_STRING_20C_1C,    //  0.050000
//  INPUT_STRING_15C_1C,    //  0.066667
//  INPUT_STRING_10C_1C,    //  0.100000
#define __input_string_coinage_start INPUT_STRING_9C_1C
	INPUT_STRING_9C_1C,     //  0.111111
	INPUT_STRING_8C_1C,     //  0.125000
	INPUT_STRING_7C_1C,     //  0.142857
	INPUT_STRING_6C_1C,     //  0.166667
//  INPUT_STRING_10C_2C,    //  0.200000
	INPUT_STRING_5C_1C,     //  0.200000
//  INPUT_STRING_9C_2C,     //  0.222222
//  INPUT_STRING_8C_2C,     //  0.250000
	INPUT_STRING_4C_1C,     //  0.250000
//  INPUT_STRING_7C_2C,     //  0.285714
//  INPUT_STRING_10C_3C,    //  0.300000
//  INPUT_STRING_9C_3C,     //  0.333333
//  INPUT_STRING_6C_2C,     //  0.333333
	INPUT_STRING_3C_1C,     //  0.333333
	INPUT_STRING_8C_3C,     //  0.375000
//  INPUT_STRING_10C_4C,    //  0.400000
//  INPUT_STRING_5C_2C,     //  0.400000
//  INPUT_STRING_7C_3C,     //  0.428571
//  INPUT_STRING_9C_4C,     //  0.444444
//  INPUT_STRING_10C_5C,    //  0.500000
//  INPUT_STRING_8C_4C,     //  0.500000
//  INPUT_STRING_6C_3C,     //  0.500000
	INPUT_STRING_4C_2C,     //  0.500000
	INPUT_STRING_2C_1C,     //  0.500000
//  INPUT_STRING_9C_5C,     //  0.555556
//  INPUT_STRING_7C_4C,     //  0.571429
//  INPUT_STRING_10C_6C,    //  0.600000
	INPUT_STRING_5C_3C,     //  0.600000
//  INPUT_STRING_8C_5C,     //  0.625000
//  INPUT_STRING_9C_6C,     //  0.666667
//  INPUT_STRING_6C_4C,     //  0.666667
	INPUT_STRING_3C_2C,     //  0.666667
//  INPUT_STRING_10C_7C,    //  0.700000
//  INPUT_STRING_7C_5C,     //  0.714286
//  INPUT_STRING_8C_6C,     //  0.750000
	INPUT_STRING_4C_3C,     //  0.750000
//  INPUT_STRING_9C_7C,     //  0.777778
//  INPUT_STRING_10C_8C,    //  0.800000
//  INPUT_STRING_5C_4C,     //  0.800000
//  INPUT_STRING_6C_5C,     //  0.833333
//  INPUT_STRING_7C_6C,     //  0.857143
//  INPUT_STRING_8C_7C,     //  0.875000
//  INPUT_STRING_9C_8C,     //  0.888889
//  INPUT_STRING_10C_9C,    //  0.900000
//  INPUT_STRING_10C_10C,   //  1.000000
//  INPUT_STRING_9C_9C,     //  1.000000
//  INPUT_STRING_8C_8C,     //  1.000000
//  INPUT_STRING_7C_7C,     //  1.000000
//  INPUT_STRING_6C_6C,     //  1.000000
//  INPUT_STRING_5C_5C,     //  1.000000
	INPUT_STRING_4C_4C,     //  1.000000
	INPUT_STRING_3C_3C,     //  1.000000
	INPUT_STRING_2C_2C,     //  1.000000
	INPUT_STRING_1C_1C,     //  1.000000
//  INPUT_STRING_9C_10C,    //  1.111111
//  INPUT_STRING_8C_9C,     //  1.125000
//  INPUT_STRING_7C_8C,     //  1.142857
//  INPUT_STRING_6C_7C,     //  1.166667
//  INPUT_STRING_5C_6C,     //  1.200000
//  INPUT_STRING_8C_10C,    //  1.250000
	INPUT_STRING_4C_5C,     //  1.250000
//  INPUT_STRING_7C_9C,     //  1.285714
//  INPUT_STRING_6C_8C,     //  1.333333
	INPUT_STRING_3C_4C,     //  1.333333
//  INPUT_STRING_5C_7C,     //  1.400000
//  INPUT_STRING_7C_10C,    //  1.428571
//  INPUT_STRING_6C_9C,     //  1.500000
//  INPUT_STRING_4C_6C,     //  1.500000
	INPUT_STRING_2C_3C,     //  1.500000
//  INPUT_STRING_5C_8C,     //  1.600000
//  INPUT_STRING_6C_10C,    //  1.666667
//  INPUT_STRING_3C_5C,     //  1.666667
	INPUT_STRING_4C_7C,     //  1.750000
//  INPUT_STRING_5C_9C,     //  1.800000
//  INPUT_STRING_5C_10C,    //  2.000000
//  INPUT_STRING_4C_8C,     //  2.000000
//  INPUT_STRING_3C_6C,     //  2.000000
	INPUT_STRING_2C_4C,     //  2.000000
	INPUT_STRING_1C_2C,     //  2.000000
//  INPUT_STRING_4C_9C,     //  2.250000
//  INPUT_STRING_3C_7C,     //  2.333333
//  INPUT_STRING_4C_10C,    //  2.500000
	INPUT_STRING_2C_5C,     //  2.500000
//  INPUT_STRING_3C_8C,     //  2.666667
//  INPUT_STRING_3C_9C,     //  3.000000
	INPUT_STRING_2C_6C,     //  3.000000
	INPUT_STRING_1C_3C,     //  3.000000
//  INPUT_STRING_3C_10C,    //  3.333333
	INPUT_STRING_2C_7C,     //  3.500000
	INPUT_STRING_2C_8C,     //  4.000000
	INPUT_STRING_1C_4C,     //  4.000000
//  INPUT_STRING_2C_9C,     //  4.500000
//  INPUT_STRING_2C_10C,    //  5.000000
	INPUT_STRING_1C_5C,     //  5.000000
	INPUT_STRING_1C_6C,     //  6.000000
	INPUT_STRING_1C_7C,     //  7.000000
	INPUT_STRING_1C_8C,     //  8.000000
	INPUT_STRING_1C_9C,     //  9.000000
#define __input_string_coinage_end INPUT_STRING_1C_9C
//  INPUT_STRING_1C_10C,    //  10.000000
//  INPUT_STRING_1C_11C,    //  11.000000
//  INPUT_STRING_1C_12C,    //  12.000000
//  INPUT_STRING_1C_13C,    //  13.000000
//  INPUT_STRING_1C_14C,    //  14.000000
//  INPUT_STRING_1C_15C,    //  15.000000
//  INPUT_STRING_1C_20C,    //  20.000000
//  INPUT_STRING_1C_25C,    //  25.000000
//  INPUT_STRING_1C_30C,    //  30.000000
//  INPUT_STRING_1C_40C,    //  40.000000
//  INPUT_STRING_1C_50C,    //  50.000000
//  INPUT_STRING_1C_99C,    //  99.000000
//  INPUT_STRING_1C_100C,   //  100.000000
//  INPUT_STRING_1C_120C,   //  120.000000
//  INPUT_STRING_1C_125C,   //  125.000000
//  INPUT_STRING_1C_150C,   //  150.000000
//  INPUT_STRING_1C_200C,   //  200.000000
//  INPUT_STRING_1C_250C,   //  250.000000
//  INPUT_STRING_1C_500C,   //  500.000000
//  INPUT_STRING_1C_1000C,  //  1000.000000
	INPUT_STRING_Free_Play,
	INPUT_STRING_Cabinet,
	INPUT_STRING_Upright,
	INPUT_STRING_Cocktail,
	INPUT_STRING_Flip_Screen,
	INPUT_STRING_Service_Mode,
	INPUT_STRING_Pause,
	INPUT_STRING_Test,
	INPUT_STRING_Tilt,
	INPUT_STRING_Version,
	INPUT_STRING_Region,
	INPUT_STRING_International,
	INPUT_STRING_Japan,
	INPUT_STRING_USA,
	INPUT_STRING_Europe,
	INPUT_STRING_Asia,
	INPUT_STRING_China,
	INPUT_STRING_Hong_Kong,
	INPUT_STRING_Korea,
	INPUT_STRING_Southeast_Asia,
	INPUT_STRING_Taiwan,
	INPUT_STRING_World,
	INPUT_STRING_Language,
	INPUT_STRING_English,
	INPUT_STRING_Japanese,
	INPUT_STRING_Chinese,
	INPUT_STRING_French,
	INPUT_STRING_German,
	INPUT_STRING_Italian,
	INPUT_STRING_Korean,
	INPUT_STRING_Spanish,
	INPUT_STRING_Very_Easy,
	INPUT_STRING_Easiest,
	INPUT_STRING_Easier,
	INPUT_STRING_Easy,
	INPUT_STRING_Medium_Easy,
	INPUT_STRING_Normal,
	INPUT_STRING_Medium,
	INPUT_STRING_Medium_Hard,
	INPUT_STRING_Hard,
	INPUT_STRING_Harder,
	INPUT_STRING_Hardest,
	INPUT_STRING_Very_Hard,
	INPUT_STRING_Medium_Difficult,
	INPUT_STRING_Difficult,
	INPUT_STRING_Very_Difficult,
	INPUT_STRING_Very_Low,
	INPUT_STRING_Low,
	INPUT_STRING_High,
	INPUT_STRING_Higher,
	INPUT_STRING_Highest,
	INPUT_STRING_Very_High,
	INPUT_STRING_Players,
	INPUT_STRING_Controls,
	INPUT_STRING_Dual,
	INPUT_STRING_Single,
	INPUT_STRING_Game_Time,
	INPUT_STRING_Continue_Price,
	INPUT_STRING_Controller,
	INPUT_STRING_Light_Gun,
	INPUT_STRING_Joystick,
	INPUT_STRING_Trackball,
	INPUT_STRING_Continues,
	INPUT_STRING_Allow_Continue,
	INPUT_STRING_Level_Select,
//  INPUT_STRING_Allow,
//  INPUT_STRING_Forbid,
//  INPUT_STRING_Enable,
//  INPUT_STRING_Disable,
	INPUT_STRING_Infinite,
//  INPUT_STRING_Invincibility,
//  INPUT_STRING_Invulnerability,
	INPUT_STRING_Stereo,
	INPUT_STRING_Mono,
	INPUT_STRING_Unused,
	INPUT_STRING_Unknown,
//  INPUT_STRING_Undefined,
	INPUT_STRING_Standard,
	INPUT_STRING_Reverse,
	INPUT_STRING_Alternate,
//  INPUT_STRING_Reserve,
//  INPUT_STRING_Spare,
//  INPUT_STRING_Invalid,
	INPUT_STRING_None,

	INPUT_STRING_COUNT
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// opaque types pointing to live state
struct input_port_state;
struct input_field_state;

// forward declarations
class ioport_list;
class ioport_port;
struct ioport_port_live;
class ioport_field;
struct ioport_field_live;
class ioport_manager;
class emu_timer;
struct xml_data_node;
class analog_field;

// constructor function pointer
typedef void(*ioport_constructor)(device_t &owner, ioport_list &portlist, std::string &errorbuf);

// I/O port callback function delegates
typedef device_delegate<ioport_value (ioport_field &, void *)> ioport_field_read_delegate;
typedef device_delegate<void (ioport_field &, void *, ioport_value, ioport_value)> ioport_field_write_delegate;
typedef device_delegate<float (ioport_field &, float)> ioport_field_crossmap_delegate;

// keyboard helper function delegates
typedef delegate<int (const unicode_char *, size_t)> ioport_queue_chars_delegate;
typedef delegate<bool (unicode_char)> ioport_accept_char_delegate;
typedef delegate<bool ()> ioport_charqueue_empty_delegate;


// ======================> inp_header

// header at the front of INP files
struct inp_header
{
	char                        header[8];      // +00: 8 byte header - must be "MAMEINP\0"
	UINT64                      basetime;       // +08: base time of recording
	UINT8                       majversion;     // +10: major INP version
	UINT8                       minversion;     // +11: minor INP version
	UINT8                       reserved[2];    // +12: must be zero
	char                        gamename[12];   // +14: game name string, NULL-terminated
	char                        version[32];    // +20: system version string, NULL-terminated
};


// ======================> input_device_default

// device defined default input settings
struct input_device_default
{
	const char *            tag;            // tag of port to update
	ioport_value            mask;           // mask to apply to the port
	ioport_value            defvalue;       // new default value
};


// ======================> input_type_entry

// describes a fundamental input type, including default input sequences
class input_type_entry
{
	friend class simple_list<input_type_entry>;
	friend class ioport_manager;

public:
	// construction/destruction
	input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard);
	input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard, input_seq decrement, input_seq increment);

	// getters
	input_type_entry *next() const { return m_next; }
	ioport_type type() const { return m_type; }
	ioport_group group() const { return m_group; }
	UINT8 player() const { return m_player; }
	const char *token() const { return m_token; }
	const char *name() const { return m_name; }
	input_seq &defseq(input_seq_type seqtype = SEQ_TYPE_STANDARD) { return m_defseq[seqtype]; }
	const input_seq &seq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const { return m_seq[seqtype]; }
	void restore_default_seq();

	// setters
	void configure_osd(const char *token, const char *name);

private:
	// internal state
	input_type_entry *  m_next;             // next description in the list
	ioport_type         m_type;             // IPT_* for this entry
	ioport_group        m_group;            // which group the port belongs to
	UINT8               m_player;           // player number (0 is player 1)
	const char *        m_token;            // token used to store settings
	const char *        m_name;             // user-friendly name
	input_seq           m_defseq[SEQ_TYPE_TOTAL];// default input sequence
	input_seq           m_seq[SEQ_TYPE_TOTAL];// currently configured sequences
};


// ======================> digital_joystick

// tracking information about a digital joystick input
class digital_joystick
{
	DISABLE_COPYING(digital_joystick);
	friend class simple_list<digital_joystick>;

public:
	// directions
	enum direction_t
	{
		JOYDIR_UP,
		JOYDIR_DOWN,
		JOYDIR_LEFT,
		JOYDIR_RIGHT,
		JOYDIR_COUNT
	};

	// bit constants
	static const UINT8 UP_BIT = 1 << JOYDIR_UP;
	static const UINT8 DOWN_BIT = 1 << JOYDIR_DOWN;
	static const UINT8 LEFT_BIT = 1 << JOYDIR_LEFT;
	static const UINT8 RIGHT_BIT = 1 << JOYDIR_RIGHT;

	// construction/destruction
	digital_joystick(int player, int number);

	// getters
	digital_joystick *next() const { return m_next; }
	int player() const { return m_player; }
	int number() const { return m_number; }
	UINT8 current() const { return m_current; }
	UINT8 current4way() const { return m_current4way; }

	// configuration
	direction_t add_axis(ioport_field &field);

	// updates
	void frame_update();

private:
	// internal state
	digital_joystick *          m_next;                                         // next joystick in the list
	int                         m_player;                                       // player number represented
	int                         m_number;                                       // joystick number represented
	simple_list<simple_list_wrapper<ioport_field> > m_field[JOYDIR_COUNT];  // potential input fields for each direction
	UINT8                       m_current;                                      // current value
	UINT8                       m_current4way;                                  // current 4-way value
	UINT8                       m_previous;                                     // previous value
};
DECLARE_ENUM_OPERATORS(digital_joystick::direction_t)


// ======================> natural_keyboard

// buffer to handle copy/paste/insert of keys
class natural_keyboard
{
	DISABLE_COPYING(natural_keyboard);

public:
	// construction/destruction
	natural_keyboard(running_machine &machine);

	void initialize();

	// getters and queries
	running_machine &machine() const { return m_machine; }
	bool empty() const { return (m_bufbegin == m_bufend); }
	bool full() const { return ((m_bufend + 1) % m_buffer.size()) == m_bufbegin; }
	bool can_post() const { return (!m_queue_chars.isnull() || !m_keycode_map.empty()); }
	bool is_posting() const { return (!empty() || (!m_charqueue_empty.isnull() && !m_charqueue_empty())); }

	// configuration
	void configure(ioport_queue_chars_delegate queue_chars, ioport_accept_char_delegate accept_char, ioport_charqueue_empty_delegate charqueue_empty);

	// posting
	void post(unicode_char ch);
	void post(const unicode_char *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_utf8(const char *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_coded(const char *text, size_t length = 0, const attotime &rate = attotime::zero);

	void frame_update(ioport_port &port, ioport_value &digital);
	const char *key_name(std::string &str, unicode_char ch);

	// debugging
	std::string dump();

private:
	// internal keyboard code information
	struct keycode_map_entry
	{
		unicode_char    ch;
		ioport_field *  field[UCHAR_SHIFT_END + 1 - UCHAR_SHIFT_BEGIN];
	};

	// internal helpers
	void build_codes(ioport_manager &manager);
	bool can_post_directly(unicode_char ch);
	bool can_post_alternate(unicode_char ch);
	attotime choose_delay(unicode_char ch);
	void internal_post(unicode_char ch);
	void timer(void *ptr, int param);
	const char *unicode_to_string(std::string &buffer, unicode_char ch);
	const keycode_map_entry *find_code(unicode_char ch) const;

	// internal state
	running_machine &       m_machine;              // reference to our machine
	UINT32                  m_bufbegin;             // index of starting character
	UINT32                  m_bufend;               // index of ending character
	std::vector<unicode_char> m_buffer;           // actual buffer
	bool                    m_status_keydown;       // current keydown status
	bool                    m_last_cr;              // was the last char a CR?
	emu_timer *             m_timer;                // timer for posting characters
	attotime                m_current_rate;         // current rate for posting
	ioport_queue_chars_delegate m_queue_chars;      // queue characters callback
	ioport_accept_char_delegate m_accept_char;      // accept character callback
	ioport_charqueue_empty_delegate m_charqueue_empty; // character queue empty callback
	std::vector<keycode_map_entry> m_keycode_map; // keycode map
};


// ======================> ioport_condition

// encapsulates a condition on a port field or setting
class ioport_condition
{
public:
	// condition types
	enum condition_t
	{
		ALWAYS = 0,
		EQUALS,
		NOTEQUALS,
		GREATERTHAN,
		NOTGREATERTHAN,
		LESSTHAN,
		NOTLESSTHAN
	};

	// construction/destruction
	ioport_condition() { reset(); }
	ioport_condition(condition_t condition, const char *tag, ioport_value mask, ioport_value value) { set(condition, tag, mask, value); }

	// getters
	const char *tag() const { return m_tag; }

	// operators
	bool operator==(const ioport_condition &rhs) const { return (m_mask == rhs.m_mask && m_value == rhs.m_value && m_condition == rhs.m_condition && strcmp(m_tag, rhs.m_tag) == 0); }
	bool eval() const;
	bool none() const { return (m_condition == ALWAYS); }

	// configuration
	void reset() { set(ALWAYS, NULL, 0, 0); }
	void set(condition_t condition, const char *tag, ioport_value mask, ioport_value value)
	{
		m_condition = condition;
		m_tag = tag;
		m_mask = mask;
		m_value = value;
	}

	void initialize(device_t &device);

private:
	// internal state
	condition_t     m_condition;    // condition to use
	const char *    m_tag;          // tag of port whose condition is to be tested
	ioport_port *   m_port;         // reference to the port to be tested
	ioport_value    m_mask;         // mask to apply to the port
	ioport_value    m_value;        // value to compare against
};


// ======================> ioport_setting

// a single setting for a configuration or DIP switch
class ioport_setting
{
	DISABLE_COPYING(ioport_setting);
	friend class simple_list<ioport_setting>;

public:
	// construction/destruction
	ioport_setting(ioport_field &field, ioport_value value, const char *name);

	// getters
	ioport_setting *next() const { return m_next; }
	ioport_field &field() const { return m_field; }
	device_t &device() const;
	running_machine &machine() const;
	ioport_value value() const { return m_value; }
	ioport_condition &condition() { return m_condition; }
	const char *name() const { return m_name; }

	// helpers
	bool enabled() { return m_condition.eval(); }

private:
	// internal state
	ioport_setting *    m_next;             // pointer to next setting in sequence
	ioport_field &      m_field;            // pointer back to the field that owns us
	ioport_value        m_value;            // value of the bits in this setting
	const char *        m_name;             // user-friendly name to display
	ioport_condition    m_condition;        // condition under which this setting is valid
};


// ======================> ioport_diplocation

// a mapping from a bit to a physical DIP switch description
class ioport_diplocation
{
	DISABLE_COPYING(ioport_diplocation);
	friend class simple_list<ioport_diplocation>;

public:
	// construction/destruction
	ioport_diplocation(const char *name, UINT8 swnum, bool invert);

	// getters
	ioport_diplocation *next() const { return m_next; }
	const char *name() const { return m_name.c_str(); }
	UINT8 number() const { return m_number; }
	bool inverted() const { return m_invert; }

private:
	ioport_diplocation *    m_next;         // pointer to the next bit
	std::string             m_name;         // name of the physical DIP switch
	UINT8                   m_number;       // physical switch number
	bool                    m_invert;       // is this an active-high DIP?
};


// ======================> ioport_field

// a single bitfield within an input port
class ioport_field
{
	DISABLE_COPYING(ioport_field);
	friend class simple_list<ioport_field>;
	friend class ioport_configurer;
	friend class dynamic_field;

	// flags for ioport_fields
	static const int FIELD_FLAG_UNUSED =   0x0001;    // set if this field is unused but relevant to other games on the same hw
	static const int FIELD_FLAG_COCKTAIL = 0x0002;    // set if this field is relevant only for cocktail cabinets
	static const int FIELD_FLAG_TOGGLE =   0x0004;    // set if this field should behave as a toggle
	static const int FIELD_FLAG_ROTATED =  0x0008;    // set if this field represents a rotated control
	static const int ANALOG_FLAG_REVERSE = 0x0010;    // analog only: reverse the sense of the axis
	static const int ANALOG_FLAG_RESET =   0x0020;    // analog only: always preload in->default for relative axes, returning only deltas
	static const int ANALOG_FLAG_WRAPS =   0x0040;    // analog only: positional count wraps around
	static const int ANALOG_FLAG_INVERT =  0x0080;    // analog only: bitwise invert bits

public:
	// construction/destruction
	ioport_field(ioport_port &port, ioport_type type, ioport_value defvalue, ioport_value maskbits, const char *name = NULL);
	~ioport_field();

	// getters
	ioport_field *next() const { return m_next; }
	ioport_port &port() const { return m_port; }
	device_t &device() const;
	ioport_manager &manager() const;
	running_machine &machine() const;
	int modcount() const { return m_modcount; }
	ioport_setting *first_setting() const { return m_settinglist.first(); }
	ioport_diplocation *first_diplocation() const { return m_diploclist.first(); }

	ioport_value mask() const { return m_mask; }
	ioport_value defvalue() const { return m_defvalue; }
	ioport_condition &condition() { return m_condition; }
	ioport_type type() const { return m_type; }
	UINT8 player() const { return m_player; }
	void set_value(ioport_value value);

	bool unused() const { return ((m_flags & FIELD_FLAG_UNUSED) != 0); }
	bool cocktail() const { return ((m_flags & FIELD_FLAG_COCKTAIL) != 0); }
	bool toggle() const { return ((m_flags & FIELD_FLAG_TOGGLE) != 0); }
	bool rotated() const { return ((m_flags & FIELD_FLAG_ROTATED) != 0); }
	bool analog_reverse() const { return ((m_flags & ANALOG_FLAG_REVERSE) != 0); }
	bool analog_reset() const { return ((m_flags & ANALOG_FLAG_RESET) != 0); }
	bool analog_wraps() const { return ((m_flags & ANALOG_FLAG_WRAPS) != 0); }
	bool analog_invert() const { return ((m_flags & ANALOG_FLAG_INVERT) != 0); }

	UINT8 impulse() const { return m_impulse; }
	const char *name() const;
	const char *specific_name() const { return m_name; }
	const input_seq &seq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const;
	const input_seq &defseq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const;
	const input_seq &defseq_unresolved(input_seq_type seqtype = SEQ_TYPE_STANDARD) const { return m_seq[seqtype]; }
	bool has_dynamic_read() const { return !m_read.isnull(); }
	bool has_dynamic_write() const { return !m_write.isnull(); }

	ioport_value minval() const { return m_min; }
	ioport_value maxval() const { return m_max; }
	INT32 sensitivity() const { return m_sensitivity; }
	INT32 delta() const { return m_delta; }
	INT32 centerdelta() const { return m_centerdelta; }
	crosshair_axis_t crosshair_axis() const { return m_crosshair_axis; }
	double crosshair_scale() const { return m_crosshair_scale; }
	double crosshair_offset() const { return m_crosshair_offset; }
	UINT16 full_turn_count() const { return m_full_turn_count; }
	const ioport_value *remap_table() const { return m_remap_table; }

	UINT8 way() const { return m_way; }
	unicode_char keyboard_code(int which) const;
	ioport_field_live &live() const { assert(m_live != NULL); return *m_live; }

	// setters
	void set_crosshair_scale(double scale) { m_crosshair_scale = scale; }
	void set_crosshair_offset(double offset) { m_crosshair_offset = offset; }
	void set_player(UINT8 player) { m_player = player; }

	// derived getters
	ioport_type_class type_class() const;
	bool is_analog() const { return (m_type > IPT_ANALOG_FIRST && m_type < IPT_ANALOG_LAST); }
	bool is_digital_joystick() const { return (m_type > IPT_DIGITAL_JOYSTICK_FIRST && m_type < IPT_DIGITAL_JOYSTICK_LAST); }

	// additional operations
	bool enabled() const { return m_condition.eval(); }
	const char *setting_name() const;
	bool has_previous_setting() const;
	void select_previous_setting();
	bool has_next_setting() const;
	void select_next_setting();
	void crosshair_position(float &x, float &y, bool &gotx, bool &goty);
	void init_live_state(analog_field *analog);
	void frame_update(ioport_value &result, bool mouse_down);
	void reduce_mask(ioport_value bits_to_remove) { m_mask &= ~bits_to_remove; }

	// user-controllable settings for a field
	struct user_settings
	{
		ioport_value    value;                  // for DIP switches
		input_seq       seq[SEQ_TYPE_TOTAL];    // sequences of all types
		INT32           sensitivity;            // for analog controls
		INT32           delta;                  // for analog controls
		INT32           centerdelta;            // for analog controls
		bool            reverse;                // for analog controls
		bool            toggle;                 // for non-analog controls
	};
	void get_user_settings(user_settings &settings);
	void set_user_settings(const user_settings &settings);

private:
	void expand_diplocation(const char *location, std::string &errorbuf);

	// internal state
	ioport_field *              m_next;             // pointer to next field in sequence
	ioport_port &               m_port;             // reference to the port that owns us
	auto_pointer<ioport_field_live> m_live;         // live state of field (NULL if not live)
	int                         m_modcount;         // modification count
	simple_list<ioport_setting> m_settinglist;      // list of input_setting_configs
	simple_list<ioport_diplocation> m_diploclist;   // list of locations for various bits

	// generally-applicable data
	ioport_value                m_mask;             // mask of bits belonging to the field
	ioport_value                m_defvalue;         // default value of these bits
	ioport_condition            m_condition;        // condition under which this field is relevant
	ioport_type                 m_type;             // IPT_* type for this port
	UINT8                       m_player;           // player number (0-based)
	UINT32                      m_flags;            // combination of FIELD_FLAG_* and ANALOG_FLAG_* above
	UINT8                       m_impulse;          // number of frames before reverting to defvalue
	const char *                m_name;             // user-friendly name to display
	input_seq                   m_seq[SEQ_TYPE_TOTAL];// sequences of all types
	ioport_field_read_delegate  m_read;             // read callback routine
	void *                      m_read_param;       // parameter for read callback routine
	ioport_field_write_delegate m_write;            // write callback routine
	void *                      m_write_param;      // parameter for write callback routine

	// data relevant to digital control types
	bool                        m_digital_value;    // externally set value

	// data relevant to analog control types
	ioport_value                m_min;              // minimum value for absolute axes
	ioport_value                m_max;              // maximum value for absolute axes
	INT32                       m_sensitivity;      // sensitivity (100=normal)
	INT32                       m_delta;            // delta to apply each frame a digital inc/dec key is pressed
	INT32                       m_centerdelta;      // delta to apply each frame no digital inputs are pressed
	crosshair_axis_t            m_crosshair_axis;   // crosshair axis
	double                      m_crosshair_scale;  // crosshair scale
	double                      m_crosshair_offset; // crosshair offset
	double                      m_crosshair_altaxis;// crosshair alternate axis value
	ioport_field_crossmap_delegate m_crosshair_mapper; // crosshair mapping function
	UINT16                      m_full_turn_count;  // number of optical counts for 1 full turn of the original control
	const ioport_value *        m_remap_table;      // pointer to an array that remaps the port value

	// data relevant to other specific types
	UINT8                       m_way;              // digital joystick 2/4/8-way descriptions
	unicode_char                m_chars[4];         // unicode key data
};


// ======================> ioport_field_live

// internal live state of an input field
struct ioport_field_live
{
	// construction/destruction
	ioport_field_live(ioport_field &field, analog_field *analog);

	// public state
	analog_field *          analog;             // pointer to live analog data if this is an analog field
	digital_joystick *      joystick;           // pointer to digital joystick information
	input_seq               seq[SEQ_TYPE_TOTAL];// currently configured input sequences
	ioport_value            value;              // current value of this port
	UINT8                   impulse;            // counter for impulse controls
	bool                    last;               // were we pressed last time?
	bool                    toggle;             // current toggle setting
	digital_joystick::direction_t joydir;       // digital joystick direction index
	std::string             name;               // overridden name
};


// ======================> ioport_list

// class that holds a list of I/O ports
class ioport_list : public tagged_list<ioport_port>
{
	DISABLE_COPYING(ioport_list);

public:
	ioport_list() { }

	using tagged_list<ioport_port>::append;
	void append(device_t &device, std::string &errorbuf);
};


// ======================> ioport_port

// a single input port configuration
class ioport_port
{
	DISABLE_COPYING(ioport_port);
	friend class simple_list<ioport_port>;
	friend class ioport_configurer;

public:
	// construction/destruction
	ioport_port(device_t &owner, const char *tag);
	~ioport_port();

	// getters
	ioport_port *next() const { return m_next; }
	ioport_manager &manager() const;
	device_t &device() const { return m_device; }
	running_machine &machine() const;
	ioport_field *first_field() const { return m_fieldlist.first(); }
	const char *tag() const { return m_tag.c_str(); }
	int modcount() const { return m_modcount; }
	ioport_value active() const { return m_active; }
	ioport_value active_safe(ioport_value defval) const { return (this == NULL) ? defval : active(); }
	ioport_port_live &live() const { assert(m_live != NULL); return *m_live; }

	// read/write to the port
	ioport_value read();
	ioport_value read_safe(ioport_value defval) { return (this == NULL) ? defval : read(); }
	void write(ioport_value value, ioport_value mask = ~0);
	void write_safe(ioport_value value, ioport_value mask = ~0) { if (this != NULL) write(value, mask); }

	// other operations
	ioport_field *field(ioport_value mask);
	void collapse_fields(std::string &errorbuf);
	void frame_update(ioport_field *mouse_field);
	void init_live_state();

private:
	void insert_field(ioport_field &newfield, ioport_value &disallowedbits, std::string &errorbuf);

	// internal state
	ioport_port *               m_next;         // pointer to next port
	device_t &                  m_device;       // associated device
	simple_list<ioport_field>   m_fieldlist;    // list of ioport_fields
	std::string                 m_tag;          // copy of this port's tag
	int                         m_modcount;     // modification count
	ioport_value                m_active;       // mask of active bits in the port
	auto_pointer<ioport_port_live> m_live;      // live state of port (NULL if not live)
};


// ======================> analog_field

// live analog field information
class analog_field
{
	friend class simple_list<analog_field>;
	friend class ioport_manager;
	friend void ioport_field::set_user_settings(const ioport_field::user_settings &settings);

public:
	// construction/destruction
	analog_field(ioport_field &field);

	// getters
	analog_field *next() const { return m_next; }
	ioport_manager &manager() const { return m_field.manager(); }
	ioport_field &field() const { return m_field; }
	INT32 sensitivity() const { return m_sensitivity; }
	bool reverse() const { return m_reverse; }
	INT32 delta() const { return m_delta; }
	INT32 centerdelta() const { return m_centerdelta; }

	// readers
	void read(ioport_value &value);
	float crosshair_read();
	void frame_update(running_machine &machine);

private:
	// helpers
	INT32 apply_min_max(INT32 value) const;
	INT32 apply_settings(INT32 value) const;
	INT32 apply_sensitivity(INT32 value) const;
	INT32 apply_inverse_sensitivity(INT32 value) const;

	// internal state
	analog_field *      m_next;                 // link to the next analog state for this port
	ioport_field &      m_field;                // pointer to the input field referenced

	// adjusted values (right-justified and tweaked)
	UINT8               m_shift;                // shift to align final value in the port
	INT32               m_adjdefvalue;          // adjusted default value from the config
	INT32               m_adjmin;               // adjusted minimum value from the config
	INT32               m_adjmax;               // adjusted maximum value from the config

	// live values of configurable parameters
	INT32               m_sensitivity;          // current live sensitivity (100=normal)
	bool                m_reverse;              // current live reverse flag
	INT32               m_delta;                // current live delta to apply each frame a digital inc/dec key is pressed
	INT32               m_centerdelta;          // current live delta to apply each frame no digital inputs are pressed

	// live analog value tracking
	INT32               m_accum;                // accumulated value (including relative adjustments)
	INT32               m_previous;             // previous adjusted value
	INT32               m_previousanalog;       // previous analog value

	// parameters for modifying live values
	INT32               m_minimum;              // minimum adjusted value
	INT32               m_maximum;              // maximum adjusted value
	INT32               m_center;               // center adjusted value for autocentering
	INT32               m_reverse_val;          // value where we subtract from to reverse directions

	// scaling factors
	INT64               m_scalepos;             // scale factor to apply to positive adjusted values
	INT64               m_scaleneg;             // scale factor to apply to negative adjusted values
	INT64               m_keyscalepos;          // scale factor to apply to the key delta field when pos
	INT64               m_keyscaleneg;          // scale factor to apply to the key delta field when neg
	INT64               m_positionalscale;      // scale factor to divide a joystick into positions

	// misc flags
	bool                m_absolute;             // is this an absolute or relative input?
	bool                m_wraps;                // does the control wrap around?
	bool                m_autocenter;           // autocenter this input?
	bool                m_single_scale;         // scale joystick differently if default is between min/max
	bool                m_interpolate;          // should we do linear interpolation for mid-frame reads?
	bool                m_lastdigital;          // was the last modification caused by a digital form?
};


// ======================> dynamic_field

// live device field information
class dynamic_field
{
	friend class simple_list<dynamic_field>;

public:
	// construction/destruction
	dynamic_field(ioport_field &field);

	// getters
	dynamic_field *next() const { return m_next; }
	ioport_field &field() const { return m_field; }

	// read/write
	void read(ioport_value &result);
	void write(ioport_value newval);

private:
	// internal state
	dynamic_field *         m_next;             // linked list of info for this port
	ioport_field &          m_field;            // reference to the input field
	UINT8                   m_shift;            // shift to apply to the final result
	ioport_value            m_oldval;           // last value
};


// ======================> ioport_port_live

// internal live state of an input port
struct ioport_port_live
{
	// construction/destruction
	ioport_port_live(ioport_port &port);

	// public state
	simple_list<analog_field> analoglist;       // list of analog port info
	simple_list<dynamic_field> readlist;        // list of dynamic read fields
	simple_list<dynamic_field> writelist;       // list of dynamic write fields
	ioport_value            defvalue;           // combined default value across the port
	ioport_value            digital;            // current value from all digital inputs
	ioport_value            outputvalue;        // current value for outputs
};


// ======================> ioport_manager

// private input port state
class ioport_manager
{
	DISABLE_COPYING(ioport_manager);
	friend class device_t;
	friend class ioport_configurer;

public:
	// construction/destruction
	ioport_manager(running_machine &machine);
	time_t initialize();

	// getters
	running_machine &machine() const { return m_machine; }
	ioport_port *first_port() const { return m_portlist.first(); }
	bool safe_to_read() const { return m_safe_to_read; }
	natural_keyboard &natkeyboard() { return m_natkeyboard; }

	// has... getters
	bool has_configs() const { return m_has_configs; }
	bool has_analog() const { return m_has_analog; }
	bool has_dips() const { return m_has_dips; }
	bool has_bioses() const { return m_has_bioses; }

	// type helpers
	input_type_entry *first_type() const { return m_typelist.first(); }
	bool type_pressed(ioport_type type, int player = 0);
	const char *type_name(ioport_type type, UINT8 player);
	ioport_group type_group(ioport_type type, int player);
	const input_seq &type_seq(ioport_type type, int player = 0, input_seq_type seqtype = SEQ_TYPE_STANDARD);
	void set_type_seq(ioport_type type, int player, input_seq_type seqtype, const input_seq &newseq);
	static bool type_is_analog(ioport_type type) { return (type > IPT_ANALOG_FIRST && type < IPT_ANALOG_LAST); }
	bool type_class_present(ioport_type_class inputclass);

	// other helpers
	digital_joystick &digjoystick(int player, int joysticknum);
	int count_players() const;
	bool crosshair_position(int player, float &x, float &y);
	bool has_keyboard() const;
	void setup_natural_keyboard(ioport_queue_chars_delegate queue_chars, ioport_accept_char_delegate accept_char, ioport_charqueue_empty_delegate charqueue_empty);
	INT32 frame_interpolate(INT32 oldval, INT32 newval);
	ioport_type token_to_input_type(const char *string, int &player) const;
	const char *input_type_to_token(std::string &str, ioport_type type, int player);

private:
	// internal helpers
	void init_port_types();
	void init_autoselect_devices(int type1, int type2, int type3, const char *option, const char *ananame);

	void frame_update_callback();
	void frame_update();

	ioport_port *port(const char *tag) const { return m_portlist.find(tag); }
	void exit();
	input_seq_type token_to_seq_type(const char *string);
	void update_defaults();

	void load_config(int config_type, xml_data_node *parentnode);
	void load_remap_table(xml_data_node *parentnode);
	bool load_default_config(xml_data_node *portnode, int type, int player, const input_seq *newseq);
	bool load_game_config(xml_data_node *portnode, int type, int player, const input_seq *newseq);

	void save_config(int config_type, xml_data_node *parentnode);
	void save_sequence(xml_data_node *parentnode, input_seq_type type, ioport_type porttype, const input_seq &seq);
	bool save_this_input_field_type(ioport_type type);
	void save_default_inputs(xml_data_node *parentnode);
	void save_game_inputs(xml_data_node *parentnode);

	template<typename _Type> _Type playback_read(_Type &result);
	time_t playback_init();
	void playback_end(const char *message = NULL);
	void playback_frame(const attotime &curtime);
	void playback_port(ioport_port &port);

	template<typename _Type> void record_write(_Type value);
	void record_init();
	void record_end(const char *message = NULL);
	void record_frame(const attotime &curtime);
	void record_port(ioport_port &port);

	// internal state
	running_machine &       m_machine;              // reference to owning machine
	bool                    m_safe_to_read;         // clear at start; set after state is loaded
	ioport_list             m_portlist;             // list of input port configurations

	// types
	simple_list<input_type_entry> m_typelist;       // list of live type states
	input_type_entry *      m_type_to_entry[IPT_COUNT][MAX_PLAYERS]; // map from type/player to type state

	// specific special global input states
	simple_list<digital_joystick> m_joystick_list;  // list of digital joysticks
	natural_keyboard        m_natkeyboard;          // natural keyboard support

	// frame time tracking
	attotime                m_last_frame_time;      // time of the last frame callback
	attoseconds_t           m_last_delta_nsec;      // nanoseconds that passed since the previous callback

	// playback/record information
	emu_file                m_record_file;          // recording file (NULL if not recording)
	emu_file                m_playback_file;        // playback file (NULL if not recording)
	UINT64                  m_playback_accumulated_speed; // accumulated speed during playback
	UINT32                  m_playback_accumulated_frames; // accumulated frames during playback

	// has...
	bool                    m_has_configs;
	bool                    m_has_analog;
	bool                    m_has_dips;
	bool                    m_has_bioses;
};


// ======================> ioport_configurer

// class to wrap helper functions
class ioport_configurer
{
public:
	// construction/destruction
	ioport_configurer(device_t &owner, ioport_list &portlist, std::string &errorbuf);

	// static helpers
	static const char *string_from_token(const char *string);

	// port helpers
	void port_alloc(const char *tag);
	void port_modify(const char *tag);

	// field helpers
	void field_alloc(ioport_type type, ioport_value defval, ioport_value mask, const char *name = NULL);
	void field_add_char(unicode_char ch);
	void field_add_code(input_seq_type which, input_code code);
	void field_set_way(int way) const { m_curfield->m_way = way; }
	void field_set_rotated() const { m_curfield->m_flags |= ioport_field::FIELD_FLAG_ROTATED; }
	void field_set_name(const char *name) const { m_curfield->m_name = string_from_token(name); }
	void field_set_player(int player) const { m_curfield->m_player = player - 1; }
	void field_set_cocktail() const { m_curfield->m_flags |= ioport_field::FIELD_FLAG_COCKTAIL; field_set_player(2); }
	void field_set_toggle() const { m_curfield->m_flags |= ioport_field::FIELD_FLAG_TOGGLE; }
	void field_set_impulse(UINT8 impulse) const { m_curfield->m_impulse = impulse; }
	void field_set_analog_reverse() const { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_REVERSE; }
	void field_set_analog_reset() const { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_RESET; }
	void field_set_unused() const { m_curfield->m_flags |= ioport_field::FIELD_FLAG_UNUSED; }
	void field_set_min_max(ioport_value minval, ioport_value maxval) const { m_curfield->m_min = minval; m_curfield->m_max = maxval; }
	void field_set_sensitivity(INT32 sensitivity) const { m_curfield->m_sensitivity = sensitivity; }
	void field_set_delta(INT32 delta) const { m_curfield->m_centerdelta = m_curfield->m_delta = delta; }
	void field_set_centerdelta(INT32 delta) const { m_curfield->m_centerdelta = delta; }
	void field_set_crosshair(crosshair_axis_t axis, double altaxis, double scale, double offset) const { m_curfield->m_crosshair_axis = axis; m_curfield->m_crosshair_altaxis = altaxis; m_curfield->m_crosshair_scale = scale; m_curfield->m_crosshair_offset = offset; }
	void field_set_crossmapper(ioport_field_crossmap_delegate callback) const { m_curfield->m_crosshair_mapper = callback; }
	void field_set_full_turn_count(UINT16 count) const { m_curfield->m_full_turn_count = count; }
	void field_set_analog_wraps() const { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_WRAPS; }
	void field_set_remap_table(const ioport_value *table) { m_curfield->m_remap_table = table; }
	void field_set_analog_invert() const { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_INVERT; }
	void field_set_dynamic_read(ioport_field_read_delegate delegate, void *param = NULL) const { m_curfield->m_read = delegate; m_curfield->m_read_param = param; }
	void field_set_dynamic_write(ioport_field_write_delegate delegate, void *param = NULL) const { m_curfield->m_write = delegate; m_curfield->m_write_param = param; }
	void field_set_diplocation(const char *location) const { m_curfield->expand_diplocation(location, m_errorbuf); }

	// setting helpers
	void setting_alloc(ioport_value value, const char *name);

	// misc helpers
	void set_condition(ioport_condition::condition_t condition, const char *tag, ioport_value mask, ioport_value value);
	void onoff_alloc(const char *name, ioport_value defval, ioport_value mask, const char *diplocation);

private:
	// internal state
	device_t &          m_owner;
	ioport_list &       m_portlist;
	std::string &       m_errorbuf;

	ioport_port *       m_curport;
	ioport_field *      m_curfield;
	ioport_setting *    m_cursetting;
};



//**************************************************************************
//  MACROS
//**************************************************************************

#define UCHAR_MAMEKEY(code) (UCHAR_MAMEKEY_BEGIN + ITEM_ID_##code)

// macro for a read callback function (PORT_CUSTOM)
#define CUSTOM_INPUT_MEMBER(name)   ioport_value name(ioport_field &field, void *param)
#define DECLARE_CUSTOM_INPUT_MEMBER(name)   ioport_value name(ioport_field &field, void *param)

// macro for port write callback functions (PORT_CHANGED)
#define INPUT_CHANGED_MEMBER(name)  void name(ioport_field &field, void *param, ioport_value oldval, ioport_value newval)
#define DECLARE_INPUT_CHANGED_MEMBER(name)  void name(ioport_field &field, void *param, ioport_value oldval, ioport_value newval)

// macro for port changed callback functions (PORT_CROSSHAIR_MAPPER)
#define CROSSHAIR_MAPPER(name)  float name(device_t &device, ioport_field &field, float linear_value)
#define CROSSHAIR_MAPPER_MEMBER(name)   float name(ioport_field &field, float linear_value)
#define DECLARE_CROSSHAIR_MAPPER_MEMBER(name)   float name(ioport_field &field, float linear_value)

// macro for wrapping a default string
#define DEF_STR(str_num) ((const char *)INPUT_STRING_##str_num)



//**************************************************************************
//  MACROS FOR BUILDING INPUT PORTS
//**************************************************************************

// so that "0" can be used for unneeded input ports
#define construct_ioport_0 NULL

// name of table
#define INPUT_PORTS_NAME(_name) construct_ioport_##_name

// start of table
#define INPUT_PORTS_START(_name) \
ATTR_COLD void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, std::string &errorbuf) \
{ \
	ioport_configurer configurer(owner, portlist, errorbuf);
// end of table
#define INPUT_PORTS_END \
}

// aliasing
#define INPUT_PORTS_EXTERN(_name) \
	extern void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, std::string &errorbuf)

// including
#define PORT_INCLUDE(_name) \
	INPUT_PORTS_NAME(_name)(owner, portlist, errorbuf);
// start of a new input port (with included tag)
#define PORT_START(_tag) \
	configurer.port_alloc(_tag);
// modify an existing port
#define PORT_MODIFY(_tag) \
	configurer.port_modify(_tag);
// input bit definition
#define PORT_BIT(_mask, _default, _type) \
	configurer.field_alloc((_type), (_default), (_mask));
#define PORT_SPECIAL_ONOFF(_mask, _default, _strindex) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, NULL)

#define PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, _diploc) \
	configurer.onoff_alloc(DEF_STR(_strindex), _default, _mask, _diploc);
// append a code
#define PORT_CODE(_code) \
	configurer.field_add_code(SEQ_TYPE_STANDARD, _code);

#define PORT_CODE_DEC(_code) \
	configurer.field_add_code(SEQ_TYPE_DECREMENT, _code);

#define PORT_CODE_INC(_code) \
	configurer.field_add_code(SEQ_TYPE_INCREMENT, _code);

// joystick flags
#define PORT_2WAY \
	configurer.field_set_way(2);

#define PORT_4WAY \
	configurer.field_set_way(4);

#define PORT_8WAY \
	configurer.field_set_way(8);

#define PORT_16WAY \
	configurer.field_set_way(16);

#define PORT_ROTATED \
	configurer.field_set_rotated();

// general flags
#define PORT_NAME(_name) \
	configurer.field_set_name(_name);

#define PORT_PLAYER(_player) \
	configurer.field_set_player(_player);

#define PORT_COCKTAIL \
	configurer.field_set_cocktail();

#define PORT_TOGGLE \
	configurer.field_set_toggle();

#define PORT_IMPULSE(_duration) \
	configurer.field_set_impulse(_duration);

#define PORT_REVERSE \
	configurer.field_set_analog_reverse();

#define PORT_RESET \
	configurer.field_set_analog_reset();

#define PORT_UNUSED \
	configurer.field_set_unused();

// analog settings
// if this macro is not used, the minimum defaults to 0 and maximum defaults to the mask value
#define PORT_MINMAX(_min, _max) \
	configurer.field_set_min_max(_min, _max);

#define PORT_SENSITIVITY(_sensitivity) \
	configurer.field_set_sensitivity(_sensitivity);

#define PORT_KEYDELTA(_delta) \
	configurer.field_set_delta(_delta);
// note that PORT_CENTERDELTA must appear after PORT_KEYDELTA
#define PORT_CENTERDELTA(_delta) \
	configurer.field_set_centerdelta(_delta);

#define PORT_CROSSHAIR(axis, scale, offset, altaxis) \
	configurer.field_set_crosshair(CROSSHAIR_AXIS_##axis, altaxis, scale, offset);

#define PORT_CROSSHAIR_MAPPER(_callback) \
	configurer.field_set_crossmapper(ioport_field_crossmap_delegate(_callback, #_callback, DEVICE_SELF, (device_t *)NULL));

#define PORT_CROSSHAIR_MAPPER_MEMBER(_device, _class, _member) \
	configurer.field_set_crossmapper(ioport_field_crossmap_delegate(&_class::_member, #_class "::" #_member, _device, (_class *)NULL));

// how many optical counts for 1 full turn of the control
#define PORT_FULL_TURN_COUNT(_count) \
	configurer.field_set_full_turn_count(_count);

// positional controls can be binary or 1 of X
// 1 of X not completed yet
// if it is specified as PORT_REMAP_TABLE then it is binary, but remapped
// otherwise it is binary
#define PORT_POSITIONS(_positions) \
	configurer.field_set_min_max(0, _positions);

// positional control wraps at min/max
#define PORT_WRAPS \
	configurer.field_set_analog_wraps();

// positional control uses this remap table
#define PORT_REMAP_TABLE(_table) \
	configurer.field_set_remap_table(_table);

// positional control bits are active low
#define PORT_INVERT \
	configurer.field_set_analog_invert();

// read callbacks
#define PORT_CUSTOM(_callback, _param) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(_callback, #_callback, DEVICE_SELF, (device_t *)NULL), (void *)(_param));

#define PORT_CUSTOM_MEMBER(_device, _class, _member, _param) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(&_class::_member, #_class "::" #_member, _device, (_class *)NULL), (void *)(_param));

// write callbacks
#define PORT_CHANGED_MEMBER(_device, _class, _member, _param) \
	configurer.field_set_dynamic_write(ioport_field_write_delegate(&_class::_member, #_class "::" #_member, _device, (_class *)NULL), (void *)(_param));

// input device handler
#define PORT_READ_LINE_DEVICE(_device, _read_line_device) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(&ioport_read_line_wrapper<_read_line_device>, #_read_line_device, _device, (device_t *)NULL));

#define PORT_READ_LINE_DEVICE_MEMBER(_device, _class, _member) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(&ioport_read_line_wrapper<_class, &_class::_member>, #_class "::" #_member, _device, (_class *)NULL));

// output device handler
#define PORT_WRITE_LINE_DEVICE(_device, _write_line_device) \
	configurer.field_set_dynamic_write(ioport_field_write_delegate(&ioport_write_line_wrapper<_write_line_device>, #_write_line_device, _device, (device_t *)NULL));

#define PORT_WRITE_LINE_DEVICE_MEMBER(_device, _class, _member) \
	configurer.field_set_dynamic_write(ioport_field_write_delegate(&ioport_write_line_wrapper<_class, &_class::_member>, #_class "::" #_member, _device, (_class *)NULL));

// dip switch definition
#define PORT_DIPNAME(_mask, _default, _name) \
	configurer.field_alloc(IPT_DIPSWITCH, (_default), (_mask), (_name));
#define PORT_DIPSETTING(_default, _name) \
	configurer.setting_alloc((_default), (_name));
// physical location, of the form: name:[!]sw,[name:][!]sw,...
// note that these are specified LSB-first
#define PORT_DIPLOCATION(_location) \
	configurer.field_set_diplocation(_location);
// conditionals for dip switch settings
#define PORT_CONDITION(_tag, _mask, _condition, _value) \
	configurer.set_condition(ioport_condition::_condition, _tag, _mask, _value);
// analog adjuster definition
#define PORT_ADJUSTER(_default, _name) \
	configurer.field_alloc(IPT_ADJUSTER, (_default), 0xff, (_name)); \
	configurer.field_set_min_max(0, 100);
// config definition
#define PORT_CONFNAME(_mask, _default, _name) \
	configurer.field_alloc(IPT_CONFIG, (_default), (_mask), (_name));
#define PORT_CONFSETTING(_default, _name) \
	configurer.setting_alloc((_default), (_name));

// keyboard chars
#define PORT_CHAR(_ch) \
	configurer.field_add_char(_ch);


// name of table
#define DEVICE_INPUT_DEFAULTS_NAME(_name) device_iptdef_##_name

#define device_iptdef_0 NULL
#define device_iptdef_0L NULL
#define device_iptdef_0LL NULL
#define device_iptdef___null NULL

// start of table
#define DEVICE_INPUT_DEFAULTS_START(_name) \
	const input_device_default DEVICE_INPUT_DEFAULTS_NAME(_name)[] = {
// end of table
#define DEVICE_INPUT_DEFAULTS(_tag,_mask,_defval) \
	{ _tag ,_mask, _defval },
// end of table
#define DEVICE_INPUT_DEFAULTS_END \
	{NULL,0,0} };



//**************************************************************************
//  HELPER MACROS
//**************************************************************************

#define PORT_DIPUNUSED_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Unused, _diploc)

#define PORT_DIPUNUSED(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Unused)

#define PORT_DIPUNKNOWN_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Unknown, _diploc)

#define PORT_DIPUNKNOWN(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Unknown)

#define PORT_SERVICE_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Service_Mode, _diploc)

#define PORT_SERVICE(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Service_Mode)

#define PORT_SERVICE_NO_TOGGLE(_mask, _default) \
	PORT_BIT( _mask, _mask & _default, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode ))

#define PORT_VBLANK(_screen) \
	PORT_READ_LINE_DEVICE_MEMBER(_screen, screen_device, vblank)

#define PORT_HBLANK(_screen) \
	PORT_READ_LINE_DEVICE_MEMBER(_screen, screen_device, hblank)



//**************************************************************************
//  INLINE TEMPLATES
//**************************************************************************

template<int (*_FunctionPointer)(device_t *)>
ioport_value ioport_read_line_wrapper(device_t &device, ioport_field &field, void *param)
{
	return ((*_FunctionPointer)(&device) & 1) ? ~ioport_value(0) : 0;
}

template<class _FunctionClass, int (_FunctionClass::*_FunctionPointer)()>
ioport_value ioport_read_line_wrapper(_FunctionClass &device, ioport_field &field, void *param)
{
	return ((device.*_FunctionPointer)() & 1) ? ~ioport_value(0) : 0;
}

template<void (*_FunctionPointer)(device_t *, int)>
void ioport_write_line_wrapper(device_t &device, ioport_field &field, void *param, ioport_value oldval, ioport_value newval)
{
	return (*_FunctionPointer)(&device, newval);
}

template<class _FunctionClass, void (_FunctionClass::*_FunctionPointer)(int)>
void ioport_write_line_wrapper(_FunctionClass &device, ioport_field &field, void *param, ioport_value oldval, ioport_value newval)
{
	return (device.*_FunctionPointer)(newval);
}



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline ioport_manager &ioport_field::manager() const { return m_port.manager(); }
inline device_t &ioport_field::device() const { return m_port.device(); }
inline running_machine &ioport_field::machine() const { return m_port.machine(); }

inline device_t &ioport_setting::device() const { return m_field.device(); }
inline running_machine &ioport_setting::machine() const { return m_field.machine(); }


#endif  // __INPTPORT_H__ */
