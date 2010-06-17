/***************************************************************************

    inptport.h

    Handle input ports and mappings.

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
enum
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
	IPT_CATEGORY,				/* MESS only */

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
	IPT_MAHJONG_BET,
	IPT_MAHJONG_LAST_CHANCE,
	IPT_MAHJONG_SCORE,
	IPT_MAHJONG_DOUBLE_UP,
	IPT_MAHJONG_FLIP_FLOP,
	IPT_MAHJONG_BIG,
	IPT_MAHJONG_SMALL,

	/* hanafuda inputs */
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

	/* gambling inputs */
	IPT_GAMBLE_HIGH,
	IPT_GAMBLE_LOW,
	IPT_GAMBLE_HALF,
	IPT_GAMBLE_DEAL,
	IPT_GAMBLE_D_UP,
	IPT_GAMBLE_TAKE,
	IPT_GAMBLE_STAND,
	IPT_GAMBLE_BET,
	IPT_GAMBLE_KEYIN,
	IPT_GAMBLE_KEYOUT,
	IPT_GAMBLE_PAYOUT,
	IPT_GAMBLE_DOOR,
	IPT_GAMBLE_SERVICE,
	IPT_GAMBLE_BOOK,

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

	/* analog inputs */
#define __ipt_analog_start IPT_PADDLE
#define __ipt_analog_absolute_start IPT_PADDLE
	IPT_PADDLE,			/* absolute */
	IPT_PADDLE_V,		/* absolute */
	IPT_AD_STICK_X,		/* absolute */
	IPT_AD_STICK_Y,		/* absolute */
	IPT_AD_STICK_Z,		/* absolute */
	IPT_LIGHTGUN_X,		/* absolute */
	IPT_LIGHTGUN_Y,		/* absolute */
	IPT_PEDAL,			/* absolute */
	IPT_PEDAL2,			/* absolute */
	IPT_PEDAL3,			/* absolute */
	IPT_POSITIONAL,		/* absolute */
	IPT_POSITIONAL_V,	/* absolute */
#define __ipt_analog_absolute_end IPT_POSITIONAL_V

	IPT_DIAL,			/* relative */
	IPT_DIAL_V,			/* relative */
	IPT_TRACKBALL_X,	/* relative */
	IPT_TRACKBALL_Y,	/* relative */
	IPT_MOUSE_X,		/* relative */
	IPT_MOUSE_Y,		/* relative */
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


/* token types */
enum
{
	INPUT_TOKEN_INVALID,
	INPUT_TOKEN_END,
	INPUT_TOKEN_INCLUDE,
	INPUT_TOKEN_START,
	INPUT_TOKEN_MODIFY,
	INPUT_TOKEN_FIELD,
	INPUT_TOKEN_SPECIAL_ONOFF,
	INPUT_TOKEN_CODE,
	INPUT_TOKEN_CODE_DEC,
	INPUT_TOKEN_CODE_INC,
	INPUT_TOKEN_2WAY,
	INPUT_TOKEN_4WAY,
	INPUT_TOKEN_8WAY,
	INPUT_TOKEN_16WAY,
	INPUT_TOKEN_ROTATED,
	INPUT_TOKEN_PLAYER1,
	INPUT_TOKEN_PLAYER2,
	INPUT_TOKEN_PLAYER3,
	INPUT_TOKEN_PLAYER4,
	INPUT_TOKEN_PLAYER5,
	INPUT_TOKEN_PLAYER6,
	INPUT_TOKEN_PLAYER7,
	INPUT_TOKEN_PLAYER8,
	INPUT_TOKEN_COCKTAIL,
	INPUT_TOKEN_TOGGLE,
	INPUT_TOKEN_NAME,
	INPUT_TOKEN_IMPULSE,
	INPUT_TOKEN_REVERSE,
	INPUT_TOKEN_RESET,
	INPUT_TOKEN_MINMAX,
	INPUT_TOKEN_SENSITIVITY,
	INPUT_TOKEN_KEYDELTA,
	INPUT_TOKEN_CENTERDELTA,
	INPUT_TOKEN_CROSSHAIR,
	INPUT_TOKEN_CROSSHAIR_MAPPER,
	INPUT_TOKEN_FULL_TURN_COUNT,
	INPUT_TOKEN_POSITIONS,
	INPUT_TOKEN_WRAPS,
	INPUT_TOKEN_REMAP_TABLE,
	INPUT_TOKEN_INVERT,
	INPUT_TOKEN_UNUSED,
	INPUT_TOKEN_CUSTOM,
	INPUT_TOKEN_CHANGED,
	INPUT_TOKEN_DIPNAME,
	INPUT_TOKEN_DIPSETTING,
	INPUT_TOKEN_DIPLOCATION,
	INPUT_TOKEN_CONDITION,
	INPUT_TOKEN_ADJUSTER,
	INPUT_TOKEN_CONFNAME,
	INPUT_TOKEN_CONFSETTING,
	INPUT_TOKEN_CHAR,
	INPUT_TOKEN_CATEGORY,
	INPUT_TOKEN_CATEGORY_NAME,
	INPUT_TOKEN_CATEGORY_SETTING,
	INPUT_TOKEN_READ_LINE_DEVICE,
	INPUT_TOKEN_WRITE_LINE_DEVICE,
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
	INPUT_STRING_9C_1C,
	INPUT_STRING_8C_1C,
	INPUT_STRING_7C_1C,
	INPUT_STRING_6C_1C,
	INPUT_STRING_5C_1C,
	INPUT_STRING_4C_1C,
	INPUT_STRING_3C_1C,
	INPUT_STRING_8C_3C,
	INPUT_STRING_4C_2C,
	INPUT_STRING_2C_1C,
	INPUT_STRING_5C_3C,
	INPUT_STRING_3C_2C,
	INPUT_STRING_4C_3C,
	INPUT_STRING_4C_4C,
	INPUT_STRING_3C_3C,
	INPUT_STRING_2C_2C,
	INPUT_STRING_1C_1C,
	INPUT_STRING_4C_5C,
	INPUT_STRING_3C_4C,
	INPUT_STRING_2C_3C,
	INPUT_STRING_4C_7C,
	INPUT_STRING_2C_4C,
	INPUT_STRING_1C_2C,
	INPUT_STRING_2C_5C,
	INPUT_STRING_2C_6C,
	INPUT_STRING_1C_3C,
	INPUT_STRING_2C_7C,
	INPUT_STRING_2C_8C,
	INPUT_STRING_1C_4C,
	INPUT_STRING_1C_5C,
	INPUT_STRING_1C_6C,
	INPUT_STRING_1C_7C,
	INPUT_STRING_1C_8C,
	INPUT_STRING_1C_9C,
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
	INPUT_STRING_World,
	INPUT_STRING_Hispanic,
	INPUT_STRING_Language,
	INPUT_STRING_English,
	INPUT_STRING_Japanese,
	INPUT_STRING_German,
	INPUT_STRING_French,
	INPUT_STRING_Italian,
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
	INPUT_STRING_More_Difficult,
	INPUT_STRING_Most_Difficult,
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
	INPUT_STRING_Infinite,
	INPUT_STRING_Stereo,
	INPUT_STRING_Mono,
	INPUT_STRING_Unused,
	INPUT_STRING_Unknown,
	INPUT_STRING_Standard,
	INPUT_STRING_Reverse,
	INPUT_STRING_Alternate,
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
	INPUT_CLASS_CATEGORIZED,
	INPUT_CLASS_MISC
};

#define UCHAR_PRIVATE		(0x100000)
#define UCHAR_SHIFT_1		(UCHAR_PRIVATE + 0)
#define UCHAR_SHIFT_2		(UCHAR_PRIVATE + 1)
#define UCHAR_MAMEKEY_BEGIN	(UCHAR_PRIVATE + 2)
#define UCHAR_MAMEKEY_END	(UCHAR_MAMEKEY_BEGIN + __code_key_last)
#define UCHAR_MAMEKEY(code)	(UCHAR_MAMEKEY_BEGIN + KEYCODE_##code)

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
typedef struct _input_field_config input_field_config;


/* template specializations */
typedef tagged_list<input_port_config> ioport_list;


/* custom input port callback function */
typedef UINT32 (*input_field_custom_func)(const input_field_config *field, void *param);

/* input port changed callback function */
typedef void (*input_field_changed_func)(const input_field_config *field, void *param, UINT32 oldval, UINT32 newval);

/* crosshair mapping function */
typedef float (*input_field_crossmap_func)(const input_field_config *field, float linear_value);


/* this type is used to encode input port definitions */
typedef union _input_port_token input_port_token;
union _input_port_token
{
	TOKEN_COMMON_FIELDS
	const input_port_token *	tokenptr;
	input_field_custom_func 	customptr;
	input_field_changed_func	changedptr;
	input_field_crossmap_func	crossmapptr;
	read_line_device_func		read_line_device;
	write_line_device_func		write_line_device;
};


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
typedef struct _input_setting_config input_setting_config;
struct _input_setting_config
{
	const input_setting_config *next;			/* pointer to next setting in sequence */
	const input_field_config *	field;			/* pointer back to the field that owns us */
	input_port_value			value;			/* value of the bits in this setting */
	input_condition				condition;		/* condition under which this setting is valid */
	const char *				name;			/* user-friendly name to display */
	UINT16						category;		/* (MESS-specific) category */
};


/* a mapping from a bit to a physical DIP switch description */
typedef struct _input_field_diplocation input_field_diplocation;
struct _input_field_diplocation
{
	input_field_diplocation *	next;			/* pointer to the next bit */
	const char *				swname;			/* name of the physical DIP switch */
	UINT8						swnum;			/* physical switch number */
	UINT8						invert;			/* is this an active-high DIP? */
};


/* a single bitfield within an input port */
struct _input_field_config
{
	/* generally-applicable data */
	const input_field_config *	next;			/* pointer to next field in sequence */
	const input_port_config *	port;			/* pointer back to the port that owns us */
	input_port_value			mask;			/* mask of bits belonging to the field */
	input_port_value			defvalue;		/* default value of these bits */
	input_condition				condition;		/* condition under which this field is relevant */
	UINT32						type;			/* IPT_* type for this port */
	UINT8						player;			/* player number (0-based) */
	UINT16						category;		/* (MESS-specific) category */
	UINT32						flags;			/* combination of FIELD_FLAG_* and ANALOG_FLAG_* above */
	UINT8						impulse;		/* number of frames before reverting to defvalue */
	const char *				name;			/* user-friendly name to display */
	input_seq					seq[SEQ_TYPE_TOTAL];/* sequences of all types */
	read_line_device_func		read_line_device;	/* input device handler */
	const char *				read_device_name;	/* input device name */
	write_line_device_func		write_line_device;	/* output device handler */
	const char *				write_device_name;	/* input device name */
	input_field_custom_func		custom;			/* custom callback routine */
	void *						custom_param;	/* parameter for custom callback routine */
	input_field_changed_func	changed;		/* changed callback routine */
	void *						changed_param;	/* parameter for changed callback routine */

	/* data relevant to analog control types */
	INT32						min;			/* minimum value for absolute axes */
	INT32						max;			/* maximum value for absolute axes */
	INT32						sensitivity;	/* sensitivity (100=normal) */
	INT32						delta;			/* delta to apply each frame a digital inc/dec key is pressed */
	INT32						centerdelta;	/* delta to apply each frame no digital inputs are pressed */
	UINT8						crossaxis;		/* crosshair axis */
	float						crossscale;		/* crosshair scale */
	float						crossoffset;	/* crosshair offset */
	float						crossaltaxis;	/* crosshair alternate axis value */
	input_field_crossmap_func	crossmapper;	/* crosshair mapping function */
	UINT16						full_turn_count;/* number of optical counts for 1 full turn of the original control */
	const input_port_value *	remap_table;	/* pointer to an array that remaps the port value */

	/* data relevant to other specific types */
	const input_setting_config *settinglist;	/* list of input_setting_configs */
	const input_field_diplocation *diploclist;	/* list of locations for various bits */
	UINT8						way;			/* digital joystick 2/4/8-way descriptions */
	unicode_char				chars[3];		/* (MESS-specific) unicode key data */

	/* this field is only valid if the device is live */
	input_field_state *			state;			/* live state of field (NULL if not live) */
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


/* a single input port configuration */
class input_port_config
{
	DISABLE_COPYING(input_port_config);

public:
	input_port_config(const char *tag);
	~input_port_config();

	input_port_config *next() const { return m_next; }

	input_port_config *			m_next;			/* pointer to next port */
	const char *				tag;			/* pointer to this port's tag */
	const input_field_config *	fieldlist;		/* list of input_field_configs */

	/* these two fields are only valid if the port is live */
	input_port_state *			state;			/* live state of port (NULL if not live) */
	running_machine *			machine;		/* machine if port is live */
};


/* describes a fundamental input type, including default input sequences */
class input_type_desc
{
public:
	input_type_desc *			next;			/* next description in the list */
	UINT32						type;			/* IPT_* for this entry */
	UINT8						group;			/* which group the port belongs to */
	UINT8						player;			/* player number (0 is player 1) */
	const char *				token;			/* token used to store settings */
	const char *				name;			/* user-friendly name */
	input_seq					seq[SEQ_TYPE_TOTAL];/* default input sequence */
};


/* heaeder at the front of INP files */
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



/***************************************************************************
    MACROS
***************************************************************************/

/* macro for a custom callback functions (PORT_CUSTOM) */
#define CUSTOM_INPUT(name)	UINT32 name(const input_field_config *field, void *param)

/* macro for port changed callback functions (PORT_CHANGED) */
#define INPUT_CHANGED(name)	void name(const input_field_config *field, void *param, UINT32 oldval, UINT32 newval)

/* macro for port changed callback functions (PORT_CROSSHAIR_MAPPER) */
#define CROSSHAIR_MAPPER(name)	float name(const input_field_config *field, float linear_value)

/* macro for wrapping a default string */
#define DEF_STR(str_num) ((const char *)INPUT_STRING_##str_num)



/***************************************************************************
    MACROS FOR BUILDING INPUT PORTS
***************************************************************************/

/* so that "0" can be used for unneeded input ports */
#define ipt_0 NULL

/* name of table */
#define INPUT_PORTS_NAME(_name) ipt_##_name

/* start of table */
#define INPUT_PORTS_START(_name) \
	const input_port_token INPUT_PORTS_NAME(_name)[] = {

/* end of table */
#define INPUT_PORTS_END \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_END, 8) };

/* aliasing */
#define INPUT_PORTS_EXTERN(_name) \
	extern const input_port_token INPUT_PORTS_NAME(_name)[]

/* including */
#define PORT_INCLUDE(_name) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_INCLUDE, 8), \
	TOKEN_PTR(tokenptr, &INPUT_PORTS_NAME(_name)[0]),

/* start of a new input port (with included tag) */
#define PORT_START(_tag) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_START, 8), \
	TOKEN_STRING(_tag),

/* modify an existing port */
#define PORT_MODIFY(_tag) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_MODIFY, 8), \
	TOKEN_STRING(_tag),

/* input bit definition */
#define PORT_BIT(_mask, _default, _type) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_FIELD, 8, _type, 24), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32),

#define PORT_SPECIAL_ONOFF(_mask, _default, _strindex) \
	TOKEN_UINT32_PACK3(INPUT_TOKEN_SPECIAL_ONOFF, 8, FALSE, 1, INPUT_STRING_##_strindex, 23), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32),

#define PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, _diploc) \
	TOKEN_UINT32_PACK3(INPUT_TOKEN_SPECIAL_ONOFF, 8, TRUE, 1, INPUT_STRING_##_strindex, 23), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32), \
	TOKEN_STRING(_diploc),

/* append a code */
#define PORT_CODE(_code) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_CODE, 8, _code, 32),

#define PORT_CODE_DEC(_code) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_CODE_DEC, 8, _code, 32),

#define PORT_CODE_INC(_code) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_CODE_INC, 8, _code, 32),

/* joystick flags */
#define PORT_2WAY \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_2WAY, 8),

#define PORT_4WAY \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_4WAY, 8),

#define PORT_8WAY \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_8WAY, 8),

#define PORT_16WAY \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_16WAY, 8),

#define PORT_ROTATED \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_ROTATED, 8),

/* general flags */
#define PORT_NAME(_name) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_NAME, 8), \
	TOKEN_STRING(_name),

#define PORT_PLAYER(player_) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_PLAYER1 + (((player_) - 1) % MAX_PLAYERS), 8),

#define PORT_COCKTAIL \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_COCKTAIL, 8),

#define PORT_TOGGLE \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_TOGGLE, 8),

#define PORT_IMPULSE(_duration) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_IMPULSE, 8, _duration, 24),

#define PORT_REVERSE \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_REVERSE, 8),

#define PORT_RESET \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_RESET, 8),

#define PORT_UNUSED \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_UNUSED, 8),

/* analog settings */
/* if this macro is not used, the minimum defaluts to 0 and maximum defaults to the mask value */
#define PORT_MINMAX(_min, _max) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_MINMAX, 8), \
	TOKEN_UINT64_PACK2(_min, 32, _max, 32),

#define PORT_SENSITIVITY(_sensitivity) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_SENSITIVITY, 8, _sensitivity, 24),

#define PORT_KEYDELTA(_delta) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_KEYDELTA, 8, _delta, 24),

/* note that PORT_CENTERDELTA must appear after PORT_KEYDELTA */
#define PORT_CENTERDELTA(_delta) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_CENTERDELTA, 8, _delta, 24),

#define PORT_CROSSHAIR(axis, scale, offset, altaxis) \
	TOKEN_UINT32_PACK3(INPUT_TOKEN_CROSSHAIR, 8, CROSSHAIR_AXIS_##axis, 4, (INT32)((altaxis) * 65536.0f), 20), \
	TOKEN_UINT64_PACK2((INT32)((scale) * 65536.0f), 32, (INT32)((offset) * 65536.0f), 32),

#define PORT_CROSSHAIR_MAPPER(_callback) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_CROSSHAIR_MAPPER, 8), \
	TOKEN_PTR(crossmapptr, _callback),

/* how many optical counts for 1 full turn of the control */
#define PORT_FULL_TURN_COUNT(_count) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_FULL_TURN_COUNT, 8, _count, 24),

/* positional controls can be binary or 1 of X */
/* 1 of X not completed yet */
/* if it is specified as PORT_REMAP_TABLE then it is binary, but remapped */
/* otherwise it is binary */
#define PORT_POSITIONS(_positions) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_POSITIONS, 8, _positions, 24),

/* positional control wraps at min/max */
#define PORT_WRAPS \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_WRAPS, 8),

/* positional control uses this remap table */
#define PORT_REMAP_TABLE(_table) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_REMAP_TABLE, 8), \
	TOKEN_PTR(ui32ptr, _table),

/* positional control bits are active low */
#define PORT_INVERT \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_INVERT, 8),

/* custom callbacks */
#define PORT_CUSTOM(_callback, _param) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_CUSTOM, 8), \
	TOKEN_PTR(customptr, _callback), \
	TOKEN_PTR(voidptr, _param),

/* changed callbacks */
#define PORT_CHANGED(_callback, _param) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_CHANGED, 8), \
	TOKEN_PTR(changedptr, _callback), \
	TOKEN_PTR(voidptr, _param),

/* input device handler */
#define PORT_READ_LINE_DEVICE(_device, _read_line_device) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_READ_LINE_DEVICE, 8), \
	TOKEN_STRING(_device), \
	TOKEN_PTR(read_line_device, _read_line_device),

/* output device handler */
#define PORT_WRITE_LINE_DEVICE(_device, _write_line_device) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_WRITE_LINE_DEVICE, 8), \
	TOKEN_STRING(_device), \
	TOKEN_PTR(write_line_device, _write_line_device),

/* dip switch definition */
#define PORT_DIPNAME(_mask, _default, _name) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_DIPNAME, 8), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32), \
	TOKEN_STRING(_name),

#define PORT_DIPSETTING(_default, _name) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_DIPSETTING, 8, _default, 32), \
	TOKEN_STRING(_name),

/* physical location, of the form: name:[!]sw,[name:][!]sw,... */
/* note that these are specified LSB-first */
#define PORT_DIPLOCATION(_location) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_DIPLOCATION, 8), \
	TOKEN_STRING(_location),

/* conditionals for dip switch settings */
#define PORT_CONDITION(_tag, _mask, _condition, _value) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_CONDITION, 8, _condition, 24), \
	TOKEN_UINT64_PACK2(_mask, 32, _value, 32), \
	TOKEN_STRING(_tag),

/* analog adjuster definition */
#define PORT_ADJUSTER(_default, _name) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_ADJUSTER, 8, _default, 32), \
	TOKEN_STRING(_name),

/* config definition */
#define PORT_CONFNAME(_mask, _default, _name) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_CONFNAME, 8), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32), \
	TOKEN_STRING(_name),

#define PORT_CONFSETTING(_default, _name) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_CONFSETTING, 8, _default, 32), \
	TOKEN_STRING(_name),

/* keyboard chars */
#define PORT_CHAR(_ch) \
	TOKEN_UINT64_PACK2(INPUT_TOKEN_CHAR, 8, _ch, 32), \

/* categories */
#define PORT_CATEGORY(_category) \
	TOKEN_UINT32_PACK2(INPUT_TOKEN_CATEGORY, 8, _category, 24),

#define PORT_CATEGORY_CLASS(_mask, _default, _name) \
	TOKEN_UINT32_PACK1(INPUT_TOKEN_CATEGORY_NAME, 8), \
	TOKEN_UINT64_PACK2(_mask, 32, _default, 32), \
	TOKEN_STRING(_name),

#define PORT_CATEGORY_ITEM(_default, _name, _category) \
	TOKEN_UINT64_PACK3(INPUT_TOKEN_CATEGORY_SETTING, 8, _default, 32, _category, 16), \
	TOKEN_STRING(_name),



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


/* ----- core system management ----- */

/* initialize the input ports, processing the given token list */
time_t input_port_init(running_machine *machine, const input_port_token *tokens);



/* ----- port configurations ----- */

/* initialize an input port list structure and allocate ports according to the given tokens */
void input_port_list_init(ioport_list &portlist, const input_port_token *tokens, char *errorbuf, int errorbuflen, int allocmap);

/* return the field that matches the given tag and mask */
const input_field_config *input_field_by_tag_and_mask(const ioport_list &portlist, const char *tag, input_port_value mask);



/* ----- accessors for input types ----- */

/* return TRUE if the given type represents an analog control */
int input_type_is_analog(int type);

/* return the name for the given type/player */
const char *input_type_name(running_machine *machine, int type, int player);

/* return the group for the given type/player */
int input_type_group(running_machine *machine, int type, int player);

/* return the global input mapping sequence for the given type/player */
const input_seq *input_type_seq(running_machine *machine, int type, int player, input_seq_type seqtype);

/* change the global input sequence for the given type/player */
void input_type_set_seq(running_machine *machine, int type, int player, input_seq_type seqtype, const input_seq *newseq);

/* return TRUE if the sequence for the given input type/player is pressed */
int input_type_pressed(running_machine *machine, int type, int player);

/* return the list of default mappings */
const input_type_desc *input_type_list(running_machine *machine);



/* ----- accessors for input fields ----- */

/* return the expanded string name of the field */
const char *input_field_name(const input_field_config *field);

/* return the input sequence for the given input field */
const input_seq *input_field_seq(const input_field_config *field, input_seq_type seqtype);

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



/* ----- port reading ----- */

/* return the value of an input port */
input_port_value input_port_read_direct(const input_port_config *port);

/* return the value of an input port specified by tag */
input_port_value input_port_read(running_machine *machine, const char *tag);

/* return the value of an input port specified by tag, or a default value if the port does not exist */
input_port_value input_port_read_safe(running_machine *machine, const char *tag, input_port_value defvalue);

/* return the extracted crosshair values for the given player */
int input_port_get_crosshair_position(running_machine *machine, int player, float *x, float *y);

/* force an update to the input port values based on current conditions */
void input_port_update_defaults(running_machine *machine);



/* ----- port writing ----- */

/* write a value to a port */
void input_port_write_direct(const input_port_config *port, input_port_value value, input_port_value mask);

/* write a value to a port specified by tag */
void input_port_write(running_machine *machine, const char *tag, input_port_value value, input_port_value mask);

/* write a value to a port, ignore if the port does not exist */
void input_port_write_safe(running_machine *machine, const char *tag, input_port_value value, input_port_value mask);



/* ----- misc helper functions ----- */

/* return the TRUE if the given condition attached is true */
int input_condition_true(running_machine *machine, const input_condition *condition);

/* convert an input_port_token to a default string */
const char *input_port_string_from_token(const input_port_token token);

/* return TRUE if machine use full keyboard emulation */
int input_machine_has_keyboard(running_machine *machine);

/* these are called by the core; they should not be called from FEs */
void inputx_init(running_machine *machine);

/* called by drivers to setup natural keyboard support */
void inputx_setup_natural_keyboard(
	int (*queue_chars)(const unicode_char *text, size_t text_len),
	int (*accept_char)(unicode_char ch),
	int (*charqueue_empty)(void));

/* validity checks */
int validate_natural_keyboard_statics(void);

/* these can be called from FEs */
int inputx_can_post(running_machine *machine);

/* various posting functions; can be called from FEs */
void inputx_postc(running_machine *machine, unicode_char ch);
void inputx_post_utf8(running_machine *machine, const char *text);
void inputx_post_utf8_rate(running_machine *machine, const char *text, attotime rate);
int inputx_is_posting(running_machine *machine);

/* miscellaneous functions */
int input_classify_port(const input_field_config *field);
int input_has_input_class(running_machine *machine, int inputclass);
int input_player_number(const input_field_config *field);
int input_count_players(running_machine *machine);
int input_category_active(running_machine *machine, int category);

#endif	/* __INPTPORT_H__ */
