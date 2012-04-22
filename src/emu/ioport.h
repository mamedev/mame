/***************************************************************************

    ioport.h

    Input/output port handling.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __INPTPORT_H__
#define __INPTPORT_H__

#include <time.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_PLAYERS			8

#define IP_ACTIVE_HIGH		0x00000000
#define IP_ACTIVE_LOW		0xffffffff


/* flags for input_field_configs */
#define FIELD_FLAG_UNUSED	0x01			/* set if this field is unused but relevant to other games on the same hw */
#define FIELD_FLAG_COCKTAIL	0x02			/* set if this field is relevant only for cocktail cabinets */
#define FIELD_FLAG_TOGGLE	0x04			/* set if this field should behave as a toggle */
#define FIELD_FLAG_ROTATED	0x08			/* set if this field represents a rotated control */
#define ANALOG_FLAG_REVERSE	0x10			/* analog only: reverse the sense of the axis */
#define ANALOG_FLAG_RESET	0x20			/* analog only: always preload in->default for relative axes, returning only deltas */
#define ANALOG_FLAG_WRAPS	0x40			/* analog only: positional count wraps around */
#define ANALOG_FLAG_INVERT	0x80			/* analog only: bitwise invert bits */


/* INP file information */
#define INP_HEADER_SIZE			64
#define INP_HEADER_MAJVERSION	3
#define INP_HEADER_MINVERSION	0


/* sequence types for input_port_seq() call */
enum _input_seq_type
{
	SEQ_TYPE_STANDARD = 0,
	SEQ_TYPE_INCREMENT,
	SEQ_TYPE_DECREMENT,
	SEQ_TYPE_TOTAL
};
typedef enum _input_seq_type input_seq_type;
DECLARE_ENUM_OPERATORS(input_seq_type)


/* conditions for DIP switches */
enum
{
	PORTCOND_ALWAYS = 0,
	PORTCOND_EQUALS,
	PORTCOND_NOTEQUALS,
	PORTCOND_GREATERTHAN,
	PORTCOND_NOTGREATERTHAN,
	PORTCOND_LESSTHAN,
	PORTCOND_NOTLESSTHAN
};


/* crosshair types */
enum
{
	CROSSHAIR_AXIS_NONE = 0,
	CROSSHAIR_AXIS_X,
	CROSSHAIR_AXIS_Y
};


/* groups for input ports */
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


/* various input port types */
enum
{
	/* pseudo-port types */
	IPT_INVALID = 0,
	IPT_UNUSED,
	IPT_END,
	IPT_UNKNOWN,
	IPT_PORT,
	IPT_DIPSWITCH,
	IPT_VBLANK,
	IPT_CONFIG,

	/* start buttons */
	IPT_START1,
	IPT_START2,
	IPT_START3,
	IPT_START4,
	IPT_START5,
	IPT_START6,
	IPT_START7,
	IPT_START8,

	/* coin slots */
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

	/* service coin */
	IPT_SERVICE1,
	IPT_SERVICE2,
	IPT_SERVICE3,
	IPT_SERVICE4,

	/* tilt inputs */
	IPT_TILT1,
	IPT_TILT2,
	IPT_TILT3,
	IPT_TILT4,

	/* misc other digital inputs */
	IPT_SERVICE,
	IPT_TILT,
	IPT_INTERLOCK,
	IPT_VOLUME_UP,
	IPT_VOLUME_DOWN,
	IPT_START,					/* MESS only */
	IPT_SELECT,					/* MESS only */
	IPT_KEYPAD,					/* MESS only */
	IPT_KEYBOARD,				/* MESS only */

#define __ipt_digital_joystick_start IPT_JOYSTICK_UP
	/* use IPT_JOYSTICK for panels where the player has one single joystick */
	IPT_JOYSTICK_UP,
	IPT_JOYSTICK_DOWN,
	IPT_JOYSTICK_LEFT,
	IPT_JOYSTICK_RIGHT,

	/* use IPT_JOYSTICKLEFT and IPT_JOYSTICKRIGHT for dual joystick panels */
	IPT_JOYSTICKRIGHT_UP,
	IPT_JOYSTICKRIGHT_DOWN,
	IPT_JOYSTICKRIGHT_LEFT,
	IPT_JOYSTICKRIGHT_RIGHT,
	IPT_JOYSTICKLEFT_UP,
	IPT_JOYSTICKLEFT_DOWN,
	IPT_JOYSTICKLEFT_LEFT,
	IPT_JOYSTICKLEFT_RIGHT,
#define __ipt_digital_joystick_end IPT_JOYSTICKLEFT_RIGHT

	/* action buttons */
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

	/* mahjong inputs */
#define __ipt_mahjong_start IPT_MAHJONG_A
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
	IPT_MAHJONG_REACH,	//IPT_MAHJONG_RIICHI,   // REACH is Japanglish
	IPT_MAHJONG_RON,
	IPT_MAHJONG_BET,
	IPT_MAHJONG_LAST_CHANCE,
	IPT_MAHJONG_SCORE,
	IPT_MAHJONG_DOUBLE_UP,
	IPT_MAHJONG_FLIP_FLOP,
	IPT_MAHJONG_BIG,
	IPT_MAHJONG_SMALL,
#define __ipt_mahjong_end IPT_MAHJONG_SMALL

	/* hanafuda inputs */
#define __ipt_hanafuda_start IPT_HANAFUDA_A
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
#define __ipt_hanafuda_end IPT_HANAFUDA_NO

#define __ipt_gambling_start IPT_GAMBLE_KEYIN

	/* gambling inputs */
	IPT_GAMBLE_KEYIN,	// attendant
	IPT_GAMBLE_KEYOUT,	// attendant
	IPT_GAMBLE_SERVICE,	// attendant
	IPT_GAMBLE_BOOK,	// attendant
	IPT_GAMBLE_DOOR,	// attendant
//  IPT_GAMBLE_DOOR2,   // many gambling games have several doors.
//  IPT_GAMBLE_DOOR3,
//  IPT_GAMBLE_DOOR4,
//  IPT_GAMBLE_DOOR5,

	IPT_GAMBLE_HIGH,	// player
	IPT_GAMBLE_LOW,		// player
	IPT_GAMBLE_HALF,	// player
	IPT_GAMBLE_DEAL,	// player
	IPT_GAMBLE_D_UP,	// player
	IPT_GAMBLE_TAKE,	// player
	IPT_GAMBLE_STAND,	// player
	IPT_GAMBLE_BET,		// player
	IPT_GAMBLE_PAYOUT,	// player
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

	/* poker-specific inputs */
	IPT_POKER_HOLD1,
	IPT_POKER_HOLD2,
	IPT_POKER_HOLD3,
	IPT_POKER_HOLD4,
	IPT_POKER_HOLD5,
	IPT_POKER_CANCEL,
	IPT_POKER_BET,

	/* slot-specific inputs */
	IPT_SLOT_STOP1,
	IPT_SLOT_STOP2,
	IPT_SLOT_STOP3,
	IPT_SLOT_STOP4,
	IPT_SLOT_STOP_ALL,

#define __ipt_gambling_end IPT_SLOT_STOP_ALL

	/* analog inputs */
#define __ipt_analog_start IPT_AD_STICK_X
#define __ipt_analog_absolute_start IPT_AD_STICK_X
	IPT_AD_STICK_X,		// absolute // autocenter
	IPT_AD_STICK_Y,		// absolute // autocenter
	IPT_AD_STICK_Z,		// absolute // autocenter
	IPT_PADDLE,			// absolute // autocenter
	IPT_PADDLE_V,		// absolute // autocenter
	IPT_PEDAL,			// absolute // autocenter
	IPT_PEDAL2,			// absolute // autocenter
	IPT_PEDAL3,			// absolute // autocenter
	IPT_LIGHTGUN_X,		// absolute
	IPT_LIGHTGUN_Y,		// absolute
	IPT_POSITIONAL,		// absolute // autocenter if not wraps
	IPT_POSITIONAL_V,	// absolute // autocenter if not wraps
#define __ipt_analog_absolute_end IPT_POSITIONAL_V

	IPT_DIAL,			// relative
	IPT_DIAL_V,			// relative
	IPT_TRACKBALL_X,	// relative
	IPT_TRACKBALL_Y,	// relative
	IPT_MOUSE_X,		// relative
	IPT_MOUSE_Y,		// relative
#define __ipt_analog_end IPT_MOUSE_Y

	/* analog adjuster support */
	IPT_ADJUSTER,

	/* the following are special codes for user interface handling - not to be used by drivers! */
#define __ipt_ui_start IPT_UI_CONFIGURE
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

	/* additional OSD-specified UI port types (up to 16) */
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
#define __ipt_ui_end IPT_OSD_16

	/* other meaning not mapped to standard defaults */
	IPT_OTHER,

	/* special meaning handled by custom code */
	IPT_SPECIAL,
	IPT_OUTPUT,

	__ipt_max
};


/* default strings used in port definitions */
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
	INPUT_STRING_9C_1C,		//  0.111111
	INPUT_STRING_8C_1C,		//  0.125000
	INPUT_STRING_7C_1C,		//  0.142857
	INPUT_STRING_6C_1C,		//  0.166667
//  INPUT_STRING_10C_2C,    //  0.200000
	INPUT_STRING_5C_1C,		//  0.200000
//  INPUT_STRING_9C_2C,     //  0.222222
//  INPUT_STRING_8C_2C,     //  0.250000
	INPUT_STRING_4C_1C,		//  0.250000
//  INPUT_STRING_7C_2C,     //  0.285714
//  INPUT_STRING_10C_3C,    //  0.300000
//  INPUT_STRING_9C_3C,     //  0.333333
//  INPUT_STRING_6C_2C,     //  0.333333
	INPUT_STRING_3C_1C,		//  0.333333
	INPUT_STRING_8C_3C,		//  0.375000
//  INPUT_STRING_10C_4C,    //  0.400000
//  INPUT_STRING_5C_2C,     //  0.400000
//  INPUT_STRING_7C_3C,     //  0.428571
//  INPUT_STRING_9C_4C,     //  0.444444
//  INPUT_STRING_10C_5C,    //  0.500000
//  INPUT_STRING_8C_4C,     //  0.500000
//  INPUT_STRING_6C_3C,     //  0.500000
	INPUT_STRING_4C_2C,		//  0.500000
	INPUT_STRING_2C_1C,		//  0.500000
//  INPUT_STRING_9C_5C,     //  0.555556
//  INPUT_STRING_7C_4C,     //  0.571429
//  INPUT_STRING_10C_6C,    //  0.600000
	INPUT_STRING_5C_3C,		//  0.600000
//  INPUT_STRING_8C_5C,     //  0.625000
//  INPUT_STRING_9C_6C,     //  0.666667
//  INPUT_STRING_6C_4C,     //  0.666667
	INPUT_STRING_3C_2C,		//  0.666667
//  INPUT_STRING_10C_7C,    //  0.700000
//  INPUT_STRING_7C_5C,     //  0.714286
//  INPUT_STRING_8C_6C,     //  0.750000
	INPUT_STRING_4C_3C,		//  0.750000
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
	INPUT_STRING_4C_4C,		//  1.000000
	INPUT_STRING_3C_3C,		//  1.000000
	INPUT_STRING_2C_2C,		//  1.000000
	INPUT_STRING_1C_1C,		//  1.000000
//  INPUT_STRING_9C_10C,    //  1.111111
//  INPUT_STRING_8C_9C,     //  1.125000
//  INPUT_STRING_7C_8C,     //  1.142857
//  INPUT_STRING_6C_7C,     //  1.166667
//  INPUT_STRING_5C_6C,     //  1.200000
//  INPUT_STRING_8C_10C,    //  1.250000
	INPUT_STRING_4C_5C,		//  1.250000
//  INPUT_STRING_7C_9C,     //  1.285714
//  INPUT_STRING_6C_8C,     //  1.333333
	INPUT_STRING_3C_4C,		//  1.333333
//  INPUT_STRING_5C_7C,     //  1.400000
//  INPUT_STRING_7C_10C,    //  1.428571
//  INPUT_STRING_6C_9C,     //  1.500000
//  INPUT_STRING_4C_6C,     //  1.500000
	INPUT_STRING_2C_3C,		//  1.500000
//  INPUT_STRING_5C_8C,     //  1.600000
//  INPUT_STRING_6C_10C,    //  1.666667
//  INPUT_STRING_3C_5C,     //  1.666667
	INPUT_STRING_4C_7C,		//  1.750000
//  INPUT_STRING_5C_9C,     //  1.800000
//  INPUT_STRING_5C_10C,    //  2.000000
//  INPUT_STRING_4C_8C,     //  2.000000
//  INPUT_STRING_3C_6C,     //  2.000000
	INPUT_STRING_2C_4C,		//  2.000000
	INPUT_STRING_1C_2C,		//  2.000000
//  INPUT_STRING_4C_9C,     //  2.250000
//  INPUT_STRING_3C_7C,     //  2.333333
//  INPUT_STRING_4C_10C,    //  2.500000
	INPUT_STRING_2C_5C,		//  2.500000
//  INPUT_STRING_3C_8C,     //  2.666667
//  INPUT_STRING_3C_9C,     //  3.000000
	INPUT_STRING_2C_6C,		//  3.000000
	INPUT_STRING_1C_3C,		//  3.000000
//  INPUT_STRING_3C_10C,    //  3.333333
	INPUT_STRING_2C_7C,		//  3.500000
	INPUT_STRING_2C_8C,		//  4.000000
	INPUT_STRING_1C_4C,		//  4.000000
//  INPUT_STRING_2C_9C,     //  4.500000
//  INPUT_STRING_2C_10C,    //  5.000000
	INPUT_STRING_1C_5C,		//  5.000000
	INPUT_STRING_1C_6C,		//  6.000000
	INPUT_STRING_1C_7C,		//  7.000000
	INPUT_STRING_1C_8C,		//  8.000000
	INPUT_STRING_1C_9C,		//  9.000000
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


/* input classes */
enum
{
	INPUT_CLASS_INTERNAL,
	INPUT_CLASS_KEYBOARD,
	INPUT_CLASS_CONTROLLER,
	INPUT_CLASS_CONFIG,
	INPUT_CLASS_DIPSWITCH,
	INPUT_CLASS_MISC
};

#define UCHAR_PRIVATE		(0x100000)
#define UCHAR_SHIFT_1		(UCHAR_PRIVATE + 0)
#define UCHAR_SHIFT_2		(UCHAR_PRIVATE + 1)
#define UCHAR_MAMEKEY_BEGIN	(UCHAR_PRIVATE + 2)
#define UCHAR_MAMEKEY(code)	(UCHAR_MAMEKEY_BEGIN + ITEM_ID_##code)

#define UCHAR_SHIFT_BEGIN	(UCHAR_SHIFT_1)
#define UCHAR_SHIFT_END		(UCHAR_SHIFT_2)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* input ports support up to 32 bits each */
typedef UINT32 input_port_value;


/* opaque types pointing to live state */
typedef struct _input_port_state input_port_state;
typedef struct _input_field_state input_field_state;


/* forward declarations */
class input_port_config;
class input_field_config;
class emu_timer;

/* template specializations */
typedef tagged_list<input_port_config> ioport_list;


/* read input port callback function */
typedef delegate<UINT32 (const input_field_config &, void *)> input_field_read_delegate;

/* input port write callback function */
typedef delegate<void (const input_field_config &, void *, input_port_value, input_port_value)> input_field_write_delegate;

/* crosshair mapping function */
typedef delegate<float (const input_field_config &, float)> input_field_crossmap_delegate;


/* encapsulates a condition on a port field or setting */
typedef struct _input_condition input_condition;
struct _input_condition
{
	const char *				tag;			/* tag of port whose condition is to be tested */
	input_port_value			mask;			/* mask to apply to the port */
	input_port_value			value;			/* value to compare against */
	UINT8						condition;		/* condition to use */
};


/* a single setting for a configuration or DIP switch */
class input_setting_config
{
	DISABLE_COPYING(input_setting_config);
	friend class simple_list<input_setting_config>;

public:
	input_setting_config(input_field_config &field, input_port_value value, const char *name);
	input_setting_config *next() const { return m_next; }

	input_port_value			value;			/* value of the bits in this setting */
	input_condition				condition;		/* condition under which this setting is valid */
	const char *				name;			/* user-friendly name to display */

private:
	input_field_config &		m_field;			/* pointer back to the field that owns us */
	input_setting_config *		m_next;			/* pointer to next setting in sequence */
};


/* a mapping from a bit to a physical DIP switch description */
class input_field_diplocation
{
	DISABLE_COPYING(input_field_diplocation);
	friend class simple_list<input_field_diplocation>;

public:
	input_field_diplocation(const char *string, UINT8 swnum, bool invert);
	input_field_diplocation *next() const { return m_next; }

	astring						swname;			/* name of the physical DIP switch */
	UINT8						swnum;			/* physical switch number */
	bool						invert;			/* is this an active-high DIP? */

private:
	input_field_diplocation *	m_next;			/* pointer to the next bit */
};


/* a single bitfield within an input port */
class input_field_config
{
	DISABLE_COPYING(input_field_config);
	friend class simple_list<input_field_config>;

public:
	input_field_config(input_port_config &port, int type, input_port_value defvalue, input_port_value maskbits, const char *name = NULL);

	input_field_config *next() const { return m_next; }
	input_port_config &port() const { return m_port; }
	running_machine &machine() const;
	simple_list<input_setting_config> &settinglist() { return m_settinglist; }
	const simple_list<input_setting_config> &settinglist() const { return m_settinglist; }
	simple_list<input_field_diplocation> &diploclist() { return m_diploclist; }
	int modcount() const { return m_modcount; }

	/* generally-applicable data */
	input_port_value			mask;			/* mask of bits belonging to the field */
	input_port_value			defvalue;		/* default value of these bits */
	input_condition				condition;		/* condition under which this field is relevant */
	UINT32						type;			/* IPT_* type for this port */
	UINT8						player;			/* player number (0-based) */
	UINT32						flags;			/* combination of FIELD_FLAG_* and ANALOG_FLAG_* above */
	UINT8						impulse;		/* number of frames before reverting to defvalue */
	const char *				name;			/* user-friendly name to display */
	input_seq					seq[SEQ_TYPE_TOTAL];/* sequences of all types */
	input_field_read_delegate	read;			/* read callback routine */
	void *						read_param;		/* parameter for read callback routine */
	const char *				read_device;	/* parameter for read callback routine */
	input_field_write_delegate	write;			/* write callback routine */
	void *						write_param;	/* parameter for write callback routine */
	const char *				write_device;	/* parameter for write callback routine */

	/* data relevant to analog control types */
	INT32						min;			/* minimum value for absolute axes */
	INT32						max;			/* maximum value for absolute axes */
	INT32						sensitivity;	/* sensitivity (100=normal) */
	INT32						delta;			/* delta to apply each frame a digital inc/dec key is pressed */
	INT32						centerdelta;	/* delta to apply each frame no digital inputs are pressed */
	UINT8						crossaxis;		/* crosshair axis */
	double						crossscale;		/* crosshair scale */
	double						crossoffset;	/* crosshair offset */
	double						crossaltaxis;	/* crosshair alternate axis value */
	input_field_crossmap_delegate crossmapper;	/* crosshair mapping function */
	const char *				crossmapper_device;	/* parameter for write callback routine */
	UINT16						full_turn_count;/* number of optical counts for 1 full turn of the original control */
	const input_port_value *	remap_table;	/* pointer to an array that remaps the port value */

	/* data relevant to other specific types */
	UINT8						way;			/* digital joystick 2/4/8-way descriptions */
	unicode_char				chars[3];		/* (MESS-specific) unicode key data */

	/* this field is only valid if the device is live */
	input_field_state *			state;			/* live state of field (NULL if not live) */

private:
	input_field_config *		m_next;				/* pointer to next field in sequence */
	input_port_config &			m_port;				/* pointer back to the port that owns us */
	int							m_modcount;
	simple_list<input_setting_config> m_settinglist;	/* list of input_setting_configs */
	simple_list<input_field_diplocation> m_diploclist;	/* list of locations for various bits */
};


/* user-controllable settings for a field */
typedef struct _input_field_user_settings input_field_user_settings;
struct _input_field_user_settings
{
	input_port_value			value;			/* for DIP switches */
	input_seq					seq[SEQ_TYPE_TOTAL];/* sequences of all types */
	INT32						sensitivity;	/* for analog controls */
	INT32						delta;			/* for analog controls */
	INT32						centerdelta;	/* for analog controls */
	UINT8						reverse;		/* for analog controls */
};

/* device defined default input settings */
typedef struct _input_device_default input_device_default;
struct _input_device_default
{
	const char *				tag;			/* tag of port to update */
	input_port_value			mask;			/* mask to apply to the port */
	input_port_value			defvalue;		/* new default value */
};

/* a single input port configuration */
class input_port_config
{
	DISABLE_COPYING(input_port_config);
	friend class simple_list<input_port_config>;

public:
	// construction/destruction
	input_port_config(device_t &owner, const char *tag);

	// getters
	input_port_config *next() const { return m_next; }
	device_t &owner() const { return m_owner; }
	running_machine &machine() const;
	input_field_config *first_field() const { return m_fieldlist.first(); }
	simple_list<input_field_config> &fieldlist() { return m_fieldlist; }
	const char *tag() const { return m_tag; }
	int modcount() const { return m_modcount; }

	void bump_modcount() { m_modcount++; }

	void collapse_fields(astring &errorbuf);

	/* these fields are only valid if the port is live */
	input_port_state *			state;			/* live state of port (NULL if not live) */
	input_port_value			active;			/* mask of active bits in the port */

private:
	input_port_config *			m_next;			/* pointer to next port */
	device_t &					m_owner;			/* associated device, when appropriate */
	simple_list<input_field_config> m_fieldlist;		/* list of input_field_configs */
	astring						m_tag;			/* pointer to this port's tag */
	int							m_modcount;
};


/* describes a fundamental input type, including default input sequences */
class input_type_entry
{
	friend class simple_list<input_type_entry>;

public:
	input_type_entry(UINT32 type, ioport_group group, int player, const char *token, const char *name, input_seq standard);
	input_type_entry(UINT32 type, ioport_group group, int player, const char *token, const char *name, input_seq standard, input_seq decrement, input_seq increment);

	input_type_entry *next() const { return m_next; }

	UINT32						type;			/* IPT_* for this entry */
	ioport_group				group;			/* which group the port belongs to */
	UINT8						player;			/* player number (0 is player 1) */
	const char *				token;			/* token used to store settings */
	const char *				name;			/* user-friendly name */
	input_seq					defseq[SEQ_TYPE_TOTAL];/* default input sequence */
	input_seq					seq[SEQ_TYPE_TOTAL];/* currently configured sequences */

private:
	input_type_entry *			m_next;			/* next description in the list */
};


/* header at the front of INP files */
typedef struct _inp_header inp_header;
struct _inp_header
{
	char						header[8];		/* +00: 8 byte header - must be "MAMEINP\0" */
	UINT64						basetime;		/* +08: base time of recording */
	UINT8						majversion;		/* +10: major INP version */
	UINT8						minversion;		/* +11: minor INP version */
	UINT8						reserved[2];	/* +12: must be zero */
	char						gamename[12];	/* +14: game name string, NULL-terminated */
	char						version[32];	/* +20: system version string, NULL-terminated */
};


struct digital_joystick_state
{
	const input_field_config *	field[4];			/* input field for up, down, left, right respectively */
	UINT8						inuse;				/* is this joystick used? */
	UINT8						current;			/* current value */
	UINT8						current4way;		/* current 4-way value */
	UINT8						previous;			/* previous value */
};

#define DIGITAL_JOYSTICKS_PER_PLAYER	3

#define NUM_SIMUL_KEYS	(UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1)
struct inputx_code
{
	unicode_char ch;
	const input_field_config * field[NUM_SIMUL_KEYS];
};

struct key_buffer
{
	int begin_pos;
	int end_pos;
	unsigned int status_keydown : 1;
	int size;
	unicode_char *buffer;
};

/* private input port state */
class ioport_manager
{
	friend class device_t;

public:
	// construction/destruction
	ioport_manager(running_machine &machine);
	time_t initialize();

	// getters
	running_machine &machine() const { return m_machine; }
	input_port_config *first_port() const { return m_portlist.first(); }

	/* global state */
	UINT8						safe_to_read;		/* clear at start; set after state is loaded */

	/* types */
	simple_list<input_type_entry> typelist;		/* list of live type states */
	input_type_entry *			type_to_entry[__ipt_max][MAX_PLAYERS]; /* map from type/player to type state */

	/* specific special global input states */
	digital_joystick_state		joystick_info[MAX_PLAYERS][DIGITAL_JOYSTICKS_PER_PLAYER]; /* joystick states */

	/* frame time tracking */
	attotime					last_frame_time;	/* time of the last frame callback */
	attoseconds_t				last_delta_nsec;	/* nanoseconds that passed since the previous callback */

	/* playback/record information */
	emu_file *					record_file;		/* recording file (NULL if not recording) */
	emu_file *					playback_file;		/* playback file (NULL if not recording) */
	UINT64						playback_accumulated_speed;/* accumulated speed during playback */
	UINT32						playback_accumulated_frames;/* accumulated frames during playback */

	/* inputx */
	inputx_code *codes;
	key_buffer keybuffer;
	emu_timer *inputx_timer;
	int (*queue_chars)(running_machine &machine, const unicode_char *text, size_t text_len);
	int (*accept_char)(running_machine &machine, unicode_char ch);
	int (*charqueue_empty)(running_machine &machine);
	attotime current_rate;

private:
	input_port_config *port(const char *tag) const { return m_portlist.find(tag); }

	// internals
	ioport_list				m_portlist;			// points to a list of input port configurations

	// internal state
	running_machine &		m_machine;
};


/***************************************************************************
    MACROS
***************************************************************************/

/* macro for a read callback function (PORT_CUSTOM) */
#define CUSTOM_INPUT(name)	input_port_value name(device_t &device, const input_field_config &field, void *param)
#define CUSTOM_INPUT_MEMBER(name)	input_port_value name(const input_field_config &field, void *param)
#define DECLARE_CUSTOM_INPUT_MEMBER(name)	input_port_value name(const input_field_config &field, void *param)

/* macro for port write callback functions (PORT_CHANGED) */
#define INPUT_CHANGED(name)	void name(device_t &device, const input_field_config &field, void *param, input_port_value oldval, input_port_value newval)
#define INPUT_CHANGED_MEMBER(name)	void name(const input_field_config &field, void *param, input_port_value oldval, input_port_value newval)
#define DECLARE_INPUT_CHANGED_MEMBER(name)	void name(const input_field_config &field, void *param, input_port_value oldval, input_port_value newval)

/* macro for port changed callback functions (PORT_CROSSHAIR_MAPPER) */
#define CROSSHAIR_MAPPER(name)	float name(device_t &device, const input_field_config &field, float linear_value)
#define CROSSHAIR_MAPPER_MEMBER(name)	float name(const input_field_config &field, float linear_value)
#define DECLARE_CROSSHAIR_MAPPER_MEMBER(name)	float name(const input_field_config &field, float linear_value)

/* macro for wrapping a default string */
#define DEF_STR(str_num) ((const char *)INPUT_STRING_##str_num)


template<int (*_FunctionPointer)(device_t *)>
input_port_value ioport_read_line_wrapper(device_t &device, const input_field_config &field, void *param)
{
	return (*_FunctionPointer)(&device);
}

template<class _FunctionClass, int (_FunctionClass::*_FunctionPointer)()>
input_port_value ioport_read_line_wrapper(_FunctionClass &device, const input_field_config &field, void *param)
{
	return (device.*_FunctionPointer)();
}

template<void (*_FunctionPointer)(device_t *, int)>
void ioport_write_line_wrapper(device_t &device, const input_field_config &field, void *param, input_port_value oldval, input_port_value newval)
{
	return (*_FunctionPointer)(&device, newval);
}

template<class _FunctionClass, void (_FunctionClass::*_FunctionPointer)(int)>
void ioport_write_line_wrapper(_FunctionClass &device, const input_field_config &field, void *param, input_port_value oldval, input_port_value newval)
{
	return (device.*_FunctionPointer)(newval);
}



/***************************************************************************
    MACROS FOR BUILDING INPUT PORTS
***************************************************************************/

typedef void (*ioport_constructor)(device_t &owner, ioport_list &portlist, astring &errorbuf);

/* so that "0" can be used for unneeded input ports */
#define construct_ioport_0 NULL

/* name of table */
#define INPUT_PORTS_NAME(_name) construct_ioport_##_name

/* start of table */
#define INPUT_PORTS_START(_name) \
ATTR_COLD void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, astring &errorbuf) \
{ \
	astring fulltag; \
	input_setting_config *cursetting = NULL; \
	input_field_config *curfield = NULL; \
	input_port_config *curport = NULL; \
	input_port_value maskbits = 0; \
	(void)cursetting; (void)curfield; (void)curport; (void)maskbits; \

/* end of table */
#define INPUT_PORTS_END \
}

/* aliasing */
#define INPUT_PORTS_EXTERN(_name) \
	extern void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, astring &errorbuf)

/* including */
#define PORT_INCLUDE(_name) \
	INPUT_PORTS_NAME(_name)(owner, portlist, errorbuf); \

/* start of a new input port (with included tag) */
#define PORT_START(_tag) \
	curport = ioconfig_alloc_port(portlist, owner, _tag); \
	curfield = NULL; \
	cursetting = NULL; \
	maskbits = 0; \

/* modify an existing port */
#define PORT_MODIFY(_tag) \
	curport = ioconfig_modify_port(portlist, owner, _tag); \
	curfield = NULL; \
	cursetting = NULL; \
	maskbits = 0; \

/* input bit definition */
#define PORT_BIT(_mask, _default, _type) \
	curfield = ioconfig_alloc_field(*curport, (_type), (_default), (_mask)); \
	cursetting = NULL;

#define PORT_SPECIAL_ONOFF(_mask, _default, _strindex) PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, NULL)

#define PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, _diploc) \
	curfield = ioconfig_alloc_onoff(*curport, DEF_STR(_strindex), _default, _mask, _diploc, errorbuf); \
	cursetting = NULL;

/* append a code */
#define PORT_CODE(_code) \
	ioconfig_add_code(*curfield, SEQ_TYPE_STANDARD, _code);

#define PORT_CODE_DEC(_code) \
	ioconfig_add_code(*curfield, SEQ_TYPE_DECREMENT, _code);

#define PORT_CODE_INC(_code) \
	ioconfig_add_code(*curfield, SEQ_TYPE_INCREMENT, _code);

/* joystick flags */
#define PORT_2WAY \
	curfield->way = 2;

#define PORT_4WAY \
	curfield->way = 4;

#define PORT_8WAY \
	curfield->way = 8;

#define PORT_16WAY \
	curfield->way = 16;

#define PORT_ROTATED \
	curfield->flags |= FIELD_FLAG_ROTATED

/* general flags */
#define PORT_NAME(_name) \
	curfield->name = input_port_string_from_token(_name);

#define PORT_PLAYER(_player) \
	curfield->player = (_player) - 1;

#define PORT_COCKTAIL \
	curfield->flags |= FIELD_FLAG_COCKTAIL; \
	curfield->player = 1;

#define PORT_TOGGLE \
	curfield->flags |= FIELD_FLAG_TOGGLE;

#define PORT_IMPULSE(_duration) \
	curfield->impulse = _duration;

#define PORT_REVERSE \
	curfield->flags |= ANALOG_FLAG_REVERSE;

#define PORT_RESET \
	curfield->flags |= ANALOG_FLAG_RESET;

#define PORT_UNUSED \
	curfield->flags |= FIELD_FLAG_UNUSED;

/* analog settings */
/* if this macro is not used, the minimum defaluts to 0 and maximum defaults to the mask value */
#define PORT_MINMAX(_min, _max) \
	curfield->min = _min; \
	curfield->max = _max;

#define PORT_SENSITIVITY(_sensitivity) \
	curfield->sensitivity = _sensitivity;

#define PORT_KEYDELTA(_delta) \
	curfield->delta = curfield->centerdelta = _delta;

/* note that PORT_CENTERDELTA must appear after PORT_KEYDELTA */
#define PORT_CENTERDELTA(_delta) \
	curfield->centerdelta = _delta;

#define PORT_CROSSHAIR(axis, scale, offset, altaxis) \
	curfield->crossaxis = CROSSHAIR_AXIS_##axis; \
	curfield->crossaltaxis = altaxis; \
	curfield->crossscale = scale; \
	curfield->crossoffset = offset;

#define PORT_CROSSHAIR_MAPPER(_callback) \
	curfield->crossmapper = input_field_crossmap_delegate(_callback, #_callback, (device_t *)NULL); \
	curfield->crossmapper_device = DEVICE_SELF;

#define PORT_CROSSHAIR_MAPPER_MEMBER(_device, _class, _member) \
	curfield->crossmapper = input_field_crossmap_delegate(&_class::_member, #_class "::" #_member, (_class *)NULL); \
	curfield->crossmapper_device = _device;

/* how many optical counts for 1 full turn of the control */
#define PORT_FULL_TURN_COUNT(_count) \
	curfield->full_turn_count = _count;

/* positional controls can be binary or 1 of X */
/* 1 of X not completed yet */
/* if it is specified as PORT_REMAP_TABLE then it is binary, but remapped */
/* otherwise it is binary */
#define PORT_POSITIONS(_positions) \
	curfield->max = _positions;

/* positional control wraps at min/max */
#define PORT_WRAPS \
	curfield->flags |= ANALOG_FLAG_WRAPS;

/* positional control uses this remap table */
#define PORT_REMAP_TABLE(_table) \
	curfield->remap_table = _table;

/* positional control bits are active low */
#define PORT_INVERT \
	curfield->flags |= ANALOG_FLAG_INVERT;

/* read callbacks */
#define PORT_CUSTOM(_callback, _param) \
	curfield->read = input_field_read_delegate(_callback, #_callback, (device_t *)NULL); \
	curfield->read_param = (void *)(_param); \
	curfield->read_device = DEVICE_SELF;

#define PORT_CUSTOM_MEMBER(_device, _class, _member, _param) \
	curfield->read = input_field_read_delegate(&_class::_member, #_class "::" #_member, (_class *)NULL); \
	curfield->read_param = (void *)(_param); \
	curfield->read_device = (_device);

/* write callbacks */
#define PORT_CHANGED(_callback, _param) \
	curfield->write = input_field_write_delegate(_callback, #_callback, (device_t *)NULL); \
	curfield->write_param = (void *)(_param); \
	curfield->write_device = DEVICE_SELF;

#define PORT_CHANGED_MEMBER(_device, _class, _member, _param) \
	curfield->write = input_field_write_delegate(&_class::_member, #_class "::" #_member, (_class *)NULL); \
	curfield->write_param = (void *)(_param); \
	curfield->write_device = (_device);

/* input device handler */
#define PORT_READ_LINE_DEVICE(_device, _read_line_device) \
	curfield->read = input_field_read_delegate(&ioport_read_line_wrapper<_read_line_device>, #_read_line_device, (device_t *)NULL); \
	curfield->read_param = NULL; \
	curfield->read_device = _device;

#define PORT_READ_LINE_DEVICE_MEMBER(_device, _class, _member) \
	curfield->read = input_field_read_delegate(&ioport_read_line_wrapper<_class, &_class::_member>, #_class "::" #_member, (_class *)NULL); \
	curfield->read_param = NULL; \
	curfield->read_device = _device;

/* output device handler */
#define PORT_WRITE_LINE_DEVICE(_device, _write_line_device) \
	curfield->write = input_field_write_delegate(&ioport_write_line_wrapper<_write_line_device>, #_write_line_device, (device_t *)NULL); \
	curfield->write_param = NULL; \
	curfield->write_device = _device;

#define PORT_WRITE_LINE_DEVICE_MEMBER(_device, _class, _member) \
	curfield->write = input_field_write_delegate(&ioport_write_line_wrapper<_class, &_class::_member>, #_class "::" #_member, (_class *)NULL); \
	curfield->write_param = NULL; \
	curfield->write_device = _device;

/* dip switch definition */
#define PORT_DIPNAME(_mask, _default, _name) \
	curfield = ioconfig_alloc_field(*curport, IPT_DIPSWITCH, (_default), (_mask), (_name)); \
	cursetting = NULL;

#define PORT_DIPSETTING(_default, _name) \
	cursetting = ioconfig_alloc_setting(*curfield, (_default) & curfield->mask, (_name));

/* physical location, of the form: name:[!]sw,[name:][!]sw,... */
/* note that these are specified LSB-first */
#define PORT_DIPLOCATION(_location) \
	diplocation_list_alloc(*curfield, _location, errorbuf);

/* conditionals for dip switch settings */
#define PORT_CONDITION(_tag, _mask, _condition, _value) \
{ \
	input_condition &condition = (cursetting != NULL) ? cursetting->condition : curfield->condition; \
	condition.tag = (_tag); \
	condition.mask = (_mask); \
	condition.condition = (_condition); \
	condition.value = (_value); \
}

/* analog adjuster definition */
#define PORT_ADJUSTER(_default, _name) \
	curfield = ioconfig_alloc_field(*curport, IPT_ADJUSTER, (_default), 0xff, (_name)); \
	cursetting = NULL; \

/* config definition */
#define PORT_CONFNAME(_mask, _default, _name) \
	curfield = ioconfig_alloc_field(*curport, IPT_CONFIG, (_default), (_mask), (_name)); \
	cursetting = NULL; \

#define PORT_CONFSETTING(_default, _name) \
	cursetting = ioconfig_alloc_setting(*curfield, (_default) & curfield->mask, (_name));

/* keyboard chars */
#define PORT_CHAR(_ch) \
	ioconfig_field_add_char(*curfield, _ch, errorbuf);


/* name of table */
#define DEVICE_INPUT_DEFAULTS_NAME(_name) device_iptdef_##_name

#define device_iptdef_0 NULL
#define device_iptdef___null NULL

/* start of table */
#define DEVICE_INPUT_DEFAULTS_START(_name) \
	const input_device_default DEVICE_INPUT_DEFAULTS_NAME(_name)[] = {

/* end of table */
#define DEVICE_INPUT_DEFAULTS(_tag,_mask,_defval) \
	{ _tag ,_mask, _defval }, \

/* end of table */
#define DEVICE_INPUT_DEFAULTS_END \
	{NULL,0,0} };

/***************************************************************************
    HELPER MACROS
***************************************************************************/

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



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- port configurations ----- */

/* initialize an input port list structure and allocate ports according to the given tokens */
void input_port_list_init(device_t &device, ioport_list &portlist, astring &errorbuf);

/* return the field that matches the given tag and mask */
const input_field_config *input_field_by_tag_and_mask(running_machine &machine, const char *tag, input_port_value mask);



/* ----- accessors for input types ----- */

/* return TRUE if the given type represents an analog control */
int input_type_is_analog(int type);

/* return the name for the given type/player */
const char *input_type_name(running_machine &machine, int type, int player);

/* return the group for the given type/player */
int input_type_group(running_machine &machine, int type, int player);

/* return the global input mapping sequence for the given type/player */
const input_seq &input_type_seq(running_machine &machine, int type, int player, input_seq_type seqtype);

/* change the global input sequence for the given type/player */
void input_type_set_seq(running_machine &machine, int type, int player, input_seq_type seqtype, const input_seq *newseq);

/* return TRUE if the sequence for the given input type/player is pressed */
int input_type_pressed(running_machine &machine, int type, int player);

/* return the list of default mappings */
const simple_list<input_type_entry> &input_type_list(running_machine &machine);



/* ----- accessors for input fields ----- */

/* return the expanded string name of the field */
const char *input_field_name(const input_field_config *field);

/* return the input sequence for the given input field */
const input_seq &input_field_seq(const input_field_config *field, input_seq_type seqtype);

/* return the current settings for the given input field */
void input_field_get_user_settings(const input_field_config *field, input_field_user_settings *settings);

/* modify the current settings for the given input field */
void input_field_set_user_settings(const input_field_config *field, const input_field_user_settings *settings);

/* return the expanded setting name for a field */
const char *input_field_setting_name(const input_field_config *field);

/* return TRUE if the given field has a "previous" setting */
int input_field_has_previous_setting(const input_field_config *field);

/* select the previous item for a DIP switch or configuration field */
void input_field_select_previous_setting(const input_field_config *field);

/* return TRUE if the given field has a "next" setting */
int input_field_has_next_setting(const input_field_config *field);

/* select the next item for a DIP switch or configuration field */
void input_field_select_next_setting(const input_field_config *field);


/* ----- port checking ----- */

/* return whether an input port exists */
bool input_port_exists(running_machine &machine, const char *tag);

/* return a bitmask of which bits of an input port are active (i.e. not unused or unknown) */
input_port_value input_port_active(running_machine &machine, const char *tag);

/* return a bitmask of which bits of an input port are active (i.e. not unused or unknown), or a default value if the port does not exist */
input_port_value input_port_active_safe(running_machine &machine, const char *tag, input_port_value defvalue);


/* ----- port reading ----- */

/* return the value of an input port */
input_port_value input_port_read_direct(const input_port_config *port);

/* return the value of an input port specified by tag */
input_port_value input_port_read(running_machine &machine, const char *tag);

/* return the value of a device input port specified by tag */
input_port_value input_port_read(device_t &device, const char *tag);

/* return the value of an input port specified by tag, or a default value if the port does not exist */
input_port_value input_port_read_safe(running_machine &machine, const char *tag, input_port_value defvalue);

/* return the extracted crosshair values for the given player */
int input_port_get_crosshair_position(running_machine &machine, int player, float *x, float *y);

/* force an update to the input port values based on current conditions */
void input_port_update_defaults(running_machine &machine);



/* ----- port writing ----- */

/* write a value to a port */
void input_port_write_direct(const input_port_config *port, input_port_value value, input_port_value mask);

/* write a value to a port specified by tag */
void input_port_write(running_machine &machine, const char *tag, input_port_value value, input_port_value mask);

/* write a value to a port, ignore if the port does not exist */
void input_port_write_safe(running_machine &machine, const char *tag, input_port_value value, input_port_value mask);



/* ----- misc helper functions ----- */

/* return the TRUE if the given condition attached is true */
int input_condition_true(running_machine &machine, const input_condition *condition,device_t &owner);

/* convert an input_port_token to a default string */
const char *input_port_string_from_token(const char *token);

/* return TRUE if machine use full keyboard emulation */
int input_machine_has_keyboard(running_machine &machine);

/* these are called by the core; they should not be called from FEs */
void inputx_init(running_machine &machine);

/* called by drivers to setup natural keyboard support */
void inputx_setup_natural_keyboard(running_machine &machine,
	int (*queue_chars)(running_machine &machine, const unicode_char *text, size_t text_len),
	int (*accept_char)(running_machine &machine, unicode_char ch),
	int (*charqueue_empty)(running_machine &machine));

/* validity checks */
int validate_natural_keyboard_statics(void);

/* these can be called from FEs */
int inputx_can_post(running_machine &machine);

/* various posting functions; can be called from FEs */
void inputx_postc(running_machine &machine, unicode_char ch);
void inputx_post_utf8(running_machine &machine, const char *text);
void inputx_post_utf8_rate(running_machine &machine, const char *text, attotime rate);
int inputx_is_posting(running_machine &machine);

/* miscellaneous functions */
int input_classify_port(const input_field_config *field);
int input_has_input_class(running_machine &machine, int inputclass);
int input_player_number(const input_field_config *field);
int input_count_players(running_machine &machine);


inline running_machine &input_field_config::machine() const
{
	return m_port.machine();
}


// temporary construction helpers
void field_config_insert(input_field_config &newfield, input_port_value &disallowedbits, astring &errorbuf);
void diplocation_list_alloc(input_field_config &field, const char *location, astring &errorbuf);


input_port_config *ioconfig_alloc_port(ioport_list &portlist, device_t &device, const char *tag);
input_port_config *ioconfig_modify_port(ioport_list &portlist, device_t &device, const char *tag);
input_field_config *ioconfig_alloc_field(input_port_config &port, int type, input_port_value defval, input_port_value mask, const char *name = NULL);
input_field_config *ioconfig_alloc_onoff(input_port_config &port, const char *name, input_port_value defval, input_port_value mask, const char *diplocation, astring &errorbuf);
input_setting_config *ioconfig_alloc_setting(input_field_config &field, input_port_value value, const char *name);
void ioconfig_field_add_char(input_field_config &field, unicode_char ch, astring &errorbuf);
void ioconfig_add_code(input_field_config &field, int which, input_code code);

#endif	/* __INPTPORT_H__ */
