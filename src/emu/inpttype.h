// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inpttype.ipp

    Core-defined input types.

***************************************************************************/
#ifndef MAME_EMU_INPTTYPE_H
#define MAME_EMU_INPTTYPE_H

#pragma once

#include "interface/inputfwd.h"
#include "osdcomm.h"


enum ioport_type : osd::u32
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
	IPT_START9,
	IPT_START10,

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
	IPT_POWER_ON,
	IPT_POWER_OFF,
	IPT_SERVICE,
	IPT_TILT,
	IPT_INTERLOCK,
	IPT_MEMORY_RESET,
	IPT_VOLUME_UP,
	IPT_VOLUME_DOWN,
	IPT_START,              // use the numbered start button(s) for coin-ops
	IPT_SELECT,
	IPT_KEYPAD,
	IPT_KEYBOARD,

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
		IPT_MAHJONG_REACH,
		IPT_MAHJONG_RON,
		IPT_MAHJONG_FLIP_FLOP,
		IPT_MAHJONG_BET,
		IPT_MAHJONG_SCORE,
		IPT_MAHJONG_DOUBLE_UP,
		IPT_MAHJONG_BIG,
		IPT_MAHJONG_SMALL,
		IPT_MAHJONG_LAST_CHANCE,

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

		IPT_GAMBLE_PAYOUT,  // player
		IPT_GAMBLE_BET,     // player
		IPT_GAMBLE_DEAL,    // player
		IPT_GAMBLE_STAND,   // player
		IPT_GAMBLE_TAKE,    // player
		IPT_GAMBLE_D_UP,    // player
		IPT_GAMBLE_HALF,    // player
		IPT_GAMBLE_HIGH,    // player
		IPT_GAMBLE_LOW,     // player

		// poker-specific inputs
		IPT_POKER_HOLD1,
		IPT_POKER_HOLD2,
		IPT_POKER_HOLD3,
		IPT_POKER_HOLD4,
		IPT_POKER_HOLD5,
		IPT_POKER_CANCEL,

		// slot-specific inputs
		IPT_SLOT_STOP1,
		IPT_SLOT_STOP2,
		IPT_SLOT_STOP3,
		IPT_SLOT_STOP4,
		IPT_SLOT_STOP5,
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

		IPT_UI_MENU,
		IPT_UI_SELECT,
		IPT_UI_BACK,
		IPT_UI_CANCEL,
		IPT_UI_CLEAR,
		IPT_UI_HELP,
		IPT_UI_UP,
		IPT_UI_DOWN,
		IPT_UI_LEFT,
		IPT_UI_RIGHT,
		IPT_UI_HOME,
		IPT_UI_END,
		IPT_UI_PAGE_UP,
		IPT_UI_PAGE_DOWN,
		IPT_UI_PREV_GROUP,
		IPT_UI_NEXT_GROUP,
		IPT_UI_ON_SCREEN_DISPLAY,
		IPT_UI_TOGGLE_UI,
		IPT_UI_DEBUG_BREAK,
		IPT_UI_PAUSE,
		IPT_UI_PAUSE_SINGLE,
		IPT_UI_REWIND_SINGLE,
		IPT_UI_SAVE_STATE,
		IPT_UI_SAVE_STATE_QUICK,
		IPT_UI_LOAD_STATE,
		IPT_UI_LOAD_STATE_QUICK,
		IPT_UI_RESET_MACHINE,
		IPT_UI_SOFT_RESET,
		IPT_UI_SHOW_GFX,
		IPT_UI_FRAMESKIP_DEC,
		IPT_UI_FRAMESKIP_INC,
		IPT_UI_THROTTLE,
		IPT_UI_FAST_FORWARD,
		IPT_UI_SHOW_FPS,
		IPT_UI_SNAPSHOT,
		IPT_UI_RECORD_MNG,
		IPT_UI_RECORD_AVI,
		IPT_UI_TOGGLE_CHEAT,
		IPT_UI_DISPLAY_COMMENT,
		IPT_UI_ZOOM_IN,
		IPT_UI_ZOOM_OUT,
		IPT_UI_ZOOM_DEFAULT,
		IPT_UI_ROTATE,
		IPT_UI_SHOW_PROFILER,
		IPT_UI_RELEASE_POINTER,
		IPT_UI_PASTE,
		IPT_UI_TAPE_START,
		IPT_UI_TAPE_STOP,
		IPT_UI_FOCUS_NEXT,
		IPT_UI_FOCUS_PREV,
		IPT_UI_DATS,
		IPT_UI_FAVORITES,
		IPT_UI_EXPORT,
		IPT_UI_AUDIT,
		IPT_UI_MIXER_ADD_FULL,
		IPT_UI_MIXER_ADD_CHANNEL,

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

	IPT_OTHER, // not mapped to standard defaults

	IPT_SPECIAL, // uninterpreted characters
	IPT_CUSTOM, // handled by custom code
	IPT_OUTPUT,

	IPT_COUNT,

	// aliases for some types
	IPT_PADDLE_H        = IPT_PADDLE,
	IPT_PEDAL1          = IPT_PEDAL,
	IPT_POSITIONAL_H    = IPT_POSITIONAL,
	IPT_DIAL_H          = IPT_DIAL
};

DECLARE_ENUM_INCDEC_OPERATORS(ioport_type)


// sequence types for input_port_seq() call
enum input_seq_type : int
{
	SEQ_TYPE_INVALID = -1,
	SEQ_TYPE_STANDARD = 0,
	SEQ_TYPE_INCREMENT,
	SEQ_TYPE_DECREMENT,
	SEQ_TYPE_TOTAL
};

DECLARE_ENUM_INCDEC_OPERATORS(input_seq_type)

#endif // MAME_EMU_INPTTYPE_H
