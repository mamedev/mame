/***************************************************************************

    inptport.h

    Handle input ports and mappings.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __INPTPORT_H__
#define __INPTPORT_H__

#include "memory.h"
#include "inputseq.h"

#ifdef MESS
#include "unicode.h"
#endif



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_INPUT_PORTS		32
#define MAX_PLAYERS			8
#define MAX_BITS_PER_PORT	32

#define IP_ACTIVE_HIGH		0x00000000
#define IP_ACTIVE_LOW		0xffffffff


/* sequence types for input_port_seq() call */
enum _input_seq_type
{
	SEQ_TYPE_STANDARD = 0,
	SEQ_TYPE_INCREMENT = 1,
	SEQ_TYPE_DECREMENT = 2
};
typedef enum _input_seq_type input_seq_type;

/* conditions for DIP switches */
enum
{
	PORTCOND_ALWAYS = 0,
	PORTCOND_EQUALS,
	PORTCOND_NOTEQUALS
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
	IPT_DIPSWITCH_NAME,
	IPT_DIPSWITCH_SETTING,
	IPT_VBLANK,
	IPT_CONFIG_NAME,			/* MESS only */
	IPT_CONFIG_SETTING,			/* MESS only */
	IPT_CATEGORY_NAME,			/* MESS only */
	IPT_CATEGORY_SETTING,		/* MESS only */

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
	IPT_BILL1,

	/* service coin */
	IPT_SERVICE1,
	IPT_SERVICE2,
	IPT_SERVICE3,
	IPT_SERVICE4,

	/* misc other digital inputs */
	IPT_SERVICE,
	IPT_TILT,
	IPT_INTERLOCK,
	IPT_VOLUME_UP,
	IPT_VOLUME_DOWN,
	IPT_START,					/* MESS only */
	IPT_SELECT,					/* MESS only */
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
	IPT_UI_CLEAR,
	IPT_UI_ZOOM_IN,
	IPT_UI_ZOOM_OUT,
	IPT_UI_PREV_GROUP,
	IPT_UI_NEXT_GROUP,
	IPT_UI_ROTATE,
	IPT_UI_SHOW_PROFILER,
	IPT_UI_TOGGLE_UI,
	IPT_UI_TOGGLE_DEBUG,
	IPT_UI_SAVE_STATE,
	IPT_UI_LOAD_STATE,
	IPT_UI_ADD_CHEAT,
	IPT_UI_DELETE_CHEAT,
	IPT_UI_SAVE_CHEAT,
	IPT_UI_WATCH_VALUE,
	IPT_UI_EDIT_CHEAT,
	IPT_UI_RELOAD_CHEAT,
	IPT_UI_TOGGLE_CROSSHAIR,

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

	/* other meaning not mapped to standard defaults */
	IPT_OTHER,

	/* special meaning handled by custom code */
	IPT_SPECIAL,

	__ipt_max
};


/* token types */
enum
{
	INPUT_TOKEN_INVALID,
	INPUT_TOKEN_END,
	INPUT_TOKEN_INCLUDE,
	INPUT_TOKEN_START,
	INPUT_TOKEN_START_TAG,
	INPUT_TOKEN_MODIFY,
	INPUT_TOKEN_BIT,
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
	INPUT_TOKEN_FULL_TURN_COUNT,
	INPUT_TOKEN_POSITIONS,
	INPUT_TOKEN_WRAPS,
	INPUT_TOKEN_REMAP_TABLE,
	INPUT_TOKEN_INVERT,
	INPUT_TOKEN_UNUSED,
	INPUT_TOKEN_CUSTOM,
	INPUT_TOKEN_DIPNAME,
	INPUT_TOKEN_DIPSETTING,
	INPUT_TOKEN_DIPLOCATION,
	INPUT_TOKEN_CONDITION,
	INPUT_TOKEN_ADJUSTER,
	INPUT_TOKEN_CONFNAME,
	INPUT_TOKEN_CONFSETTING,
#ifdef MESS
	INPUT_TOKEN_CHAR,
	INPUT_TOKEN_CATEGORY,
	INPUT_TOKEN_CATEGORY_NAME,
	INPUT_TOKEN_CATEGORY_SETTING,
#endif /* MESS */
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
	INPUT_STRING_Normal,
	INPUT_STRING_Medium,
	INPUT_STRING_Hard,
	INPUT_STRING_Harder,
	INPUT_STRING_Hardest,
	INPUT_STRING_Very_Hard,
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



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* this is an opaque type */
typedef struct _input_port_init_params input_port_init_params;


/* this type is used to encode input port definitions */
typedef struct _input_port_token *input_port_token;


/* In mamecore.h: typedef struct _input_port_default_entry input_port_default_entry; */
struct _input_port_default_entry
{
	UINT32		type;			/* type of port; see enum above */
	UINT8		group;			/* which group the port belongs to */
	UINT8		player;			/* player number (0 is player 1) */
	const char *token;			/* token used to store settings */
	const char *name;			/* user-friendly name */
	input_seq	defaultseq;		/* default input sequence */
	input_seq	defaultincseq;	/* default input sequence to increment (analog ports only) */
	input_seq	defaultdecseq;	/* default input sequence to decrement (analog ports only) */
};


/* In mamecore.h: typedef struct _input_port_entry input_port_entry; */
struct _input_port_entry
{
	UINT32		mask;			/* bits affected */
	UINT32		default_value;	/* default value for the bits affected */
								/* you can also use one of the IP_ACTIVE defines above */
	UINT32		type;			/* see enum above */
	UINT8		unused;			/* The bit is not used by this game, but is used */
								/* by other games running on the same hardware. */
								/* This is different from IPT_UNUSED, which marks */
								/* bits not connected to anything. */
	UINT8		cocktail;		/* the bit is used in cocktail mode only */
	UINT8		player;			/* the player associated with this port; note that */
								/* player 1 is '0' */
	UINT8		toggle;			/* When this is set, the key acts as a toggle - press */
								/* it once and it goes on, press it again and it goes off. */
								/* useful e.g. for some Test Mode dip switches. */
	UINT8		impulse;		/* When this is set, when the key corresponding to */
								/* the input bit is pressed it will be reported as */
								/* pressed for a certain number of video frames and */
								/* then released, regardless of the real status of */
								/* the key. This is useful e.g. for some coin inputs. */
								/* The number of frames the signal should stay active */
								/* is specified in the "arg" field. */
	UINT8		way;			/* Joystick modes of operation. 8WAY is the default, */
								/* it prevents left/right or up/down to be pressed at */
								/* the same time. 4WAY prevents diagonal directions. */
								/* 2WAY should be used for joysticks wich move only */
								/* on one axis (e.g. Battle Zone) */
	UINT8		rotated;		/* Indicates the control is rotated 45 degrees. This */
								/* is used as a hint for joystick mapping. */
	UINT16		category;		/* (MESS-specific) category */
	const char *name;			/* user-friendly name to display */
	input_seq	seq;			/* input sequence affecting the input bits */
	UINT32		(*custom)(void *);/* custom callback routine */
	void *		custom_param;	/* parameter for callback routine */

	/* valid if type is between __ipt_analog_start and __ipt_analog_end */
	struct
	{
		INT32	min;				/* minimum value for absolute axes */
		INT32	max;				/* maximum value for absolute axes */
		INT32	sensitivity;		/* sensitivity (100=normal) */
		INT32	delta;				/* delta to apply each frame a digital inc/dec key is pressed */
		INT32	centerdelta;		/* delta to apply each frame no digital inputs are pressed */
		UINT8	reverse;			/* reverse the sense of the analog axis */
		UINT8	reset;				/* always preload in->default for relative axes, returning only deltas */
		UINT8	crossaxis;			/* crosshair axis */
		float	crossscale;			/* crosshair scale */
		float	crossoffset;		/* crosshair offset */
		float	crossaltaxis;		/* crosshair alternate axis value */
		input_seq incseq;			/* increment sequence */
		input_seq decseq;			/* decrement sequence */
		UINT8	wraps;				/* positional count wraps around */
		UINT8	invert;				/* bitwise invert bits */
		UINT16	full_turn_count;	/* number of optical counts for 1 full turn of the original control */
		const UINT32 *remap_table;	/* pointer to an array that remaps the port value */
	} analog;

	/* valid if type is IPT_PORT */
	struct
	{
		const char *tag;		/* used to tag PORT_START declarations */
	} start;

	/* valid for most types */
	struct
	{
		const char *tag;		/* port tag to use for condition */
		UINT8	portnum;		/* port number for condition */
		UINT8	condition;		/* condition to use */
		UINT32	mask;			/* mask to apply to the port */
		UINT32	value;			/* value to compare against */
	} condition;

	/* valid for IPT_DIPNAME */
	struct
	{
		const char *swname;		/* name of the physical DIP switch */
		UINT8	swnum;			/* physical switch number */
		UINT8	invert;			/* is this an active-high DIP? */
	} diploc[8];

	/* valid if type is IPT_KEYBOARD */
#ifdef MESS
	struct
	{
		unicode_char chars[3];/* (MESS-specific) unicode key data */
	} keyboard;
#endif /* MESS */
};


typedef struct _inp_header inp_header;
struct _inp_header
{
	char name[9];      /* 8 bytes for game->name + NUL */
	char version[3];   /* byte[0] = 0, byte[1] = version byte[2] = beta_version */
	char reserved[20]; /* for future use, possible store game options? */
};



/***************************************************************************
    MACROS FOR BUILDING INPUT PORTS
***************************************************************************/

#define IP_NAME_DEFAULT 				NULL

/* single pointers and UINT32s are just encoded straight */
#define INPUT_PORT_PTR(x)				((input_port_token)(x))
#define INPUT_PORT_UINT32(x)			((input_port_token)(FPTR)(UINT32)(FPTR)(x))

#ifdef PTR64
/* on 64-bit platforms, pairs of UINT32s are encoded into a single 64-bit pointer */
#define INPUT_PORT_UINT32_PAIR(x,y)		((input_port_token)((UINT32)(x) | ((UINT64)(y) << 32)))
#define INPUT_PORT_PAIR_ITEM(vptr,x)	((UINT32)((FPTR)*(vptr) >> (32 * (x))))
#define INPUT_PORT_PAIR_TOKENS			1
#else
/* on 32-bit platforms, pairs of UINT32s are encoded in two consecutive 32-bit pointers */
#define INPUT_PORT_UINT32_PAIR(x,y)		INPUT_PORT_UINT32(x), INPUT_PORT_UINT32(y)
#define INPUT_PORT_PAIR_ITEM(vptr,x)	((UINT32)((vptr)[x]))
#define INPUT_PORT_PAIR_TOKENS			2
#endif

/* start of table */
#define INPUT_PORTS_START(name) \
	const input_port_token ipt_##name[] = {

/* end of table */
#define INPUT_PORTS_END \
	INPUT_PORT_UINT32(INPUT_TOKEN_END) };

/* aliasing */
#define INPUT_PORTS_EXTERN(name) \
	extern const input_port_token ipt_##name[]

/* including */
#define PORT_INCLUDE(name) \
	INPUT_PORT_UINT32(INPUT_TOKEN_INCLUDE), INPUT_PORT_PTR(&ipt_##name[0]),

/* start of a new input port */
#define PORT_START \
	INPUT_PORT_UINT32(INPUT_TOKEN_START),

/* start of a new input port (with included tag) */
#define PORT_START_TAG(tag_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_START_TAG), INPUT_PORT_PTR(tag_),

/* modify an existing port */
#define PORT_MODIFY(tag_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_MODIFY), INPUT_PORT_PTR(tag_),

/* input bit definition */
#define PORT_BIT(mask_,default_,type_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_BIT, type_), INPUT_PORT_UINT32_PAIR(mask_, default_),

/* append a code */
#define PORT_CODE(code) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CODE, code),

#define PORT_CODE_DEC(code) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CODE_DEC, code),

#define PORT_CODE_INC(code) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CODE_INC, code),

/* joystick flags */
#define PORT_2WAY \
	INPUT_PORT_UINT32(INPUT_TOKEN_2WAY),

#define PORT_4WAY \
	INPUT_PORT_UINT32(INPUT_TOKEN_4WAY),

#define PORT_8WAY \
	INPUT_PORT_UINT32(INPUT_TOKEN_8WAY),

#define PORT_16WAY \
	INPUT_PORT_UINT32(INPUT_TOKEN_16WAY),

#define PORT_ROTATED \
	INPUT_PORT_UINT32(INPUT_TOKEN_ROTATED),

/* general flags */
#define PORT_NAME(name_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_NAME), INPUT_PORT_PTR(name_),

#define PORT_PLAYER(player_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_PLAYER1 + (((player_) - 1) % MAX_PLAYERS)),

#define PORT_COCKTAIL \
	INPUT_PORT_UINT32(INPUT_TOKEN_COCKTAIL),

#define PORT_TOGGLE \
	INPUT_PORT_UINT32(INPUT_TOKEN_TOGGLE),

#define PORT_IMPULSE(duration_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_IMPULSE, duration_),

#define PORT_REVERSE \
	INPUT_PORT_UINT32(INPUT_TOKEN_REVERSE),

#define PORT_RESET \
	INPUT_PORT_UINT32(INPUT_TOKEN_RESET),

#define PORT_UNUSED \
	INPUT_PORT_UINT32(INPUT_TOKEN_UNUSED),

/* analog settings */
/* if this macro is not used, the minimum defaluts to 0 and maximum defaluts to the mask value */
#define PORT_MINMAX(min_,max_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_MINMAX), INPUT_PORT_UINT32_PAIR(min_, max_),

#define PORT_SENSITIVITY(sensitivity_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_SENSITIVITY, sensitivity_),

#define PORT_KEYDELTA(delta_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_KEYDELTA, delta_),

/* note that PORT_CENTERDELTA must appear after PORT_KEYDELTA */
#define PORT_CENTERDELTA(delta_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CENTERDELTA, delta_),

#define PORT_CROSSHAIR(axis, scale, offset, altaxis) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CROSSHAIR, CROSSHAIR_AXIS_##axis | ((INT32)((altaxis) * 65536.0f) << 8)), \
	INPUT_PORT_UINT32_PAIR((INT32)((scale) * 65536.0f), (INT32)((offset) * 65536.0f)),

/* how many optical counts for 1 full turn of the control */
#define PORT_FULL_TURN_COUNT(count_)								\
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_FULL_TURN_COUNT, count_),

/* positional controls can be binary or 1 of X */
/* 1 of X not completed yet */
/* if it is specified as PORT_REMAP_TABLE then it is binary, but remapped */
/* otherwise it is binary */
#define PORT_POSITIONS(positions_)									\
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_POSITIONS, positions_),

/* positional control wraps at min/max */
#define PORT_WRAPS								\
	INPUT_PORT_UINT32(INPUT_TOKEN_WRAPS),

/* positional control uses this remap table */
#define PORT_REMAP_TABLE(table_)										\
	INPUT_PORT_UINT32(INPUT_TOKEN_REMAP_TABLE), INPUT_PORT_PTR(table_),

/* positional control bits are active low */
#define PORT_INVERT													\
	INPUT_PORT_UINT32(INPUT_TOKEN_INVERT),

/* custom callbacks */
#define PORT_CUSTOM(callback_, param_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_CUSTOM), INPUT_PORT_PTR(callback_), INPUT_PORT_PTR(param_),

/* dip switch definition */
#define PORT_DIPNAME(mask,default,name) \
	INPUT_PORT_UINT32(INPUT_TOKEN_DIPNAME), INPUT_PORT_UINT32_PAIR(mask, default), INPUT_PORT_PTR(name),

#define PORT_DIPSETTING(default,name) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_DIPSETTING, default), INPUT_PORT_PTR(name),

/* physical location, of the form: name:[!]sw,[name:][!]sw,... */
/* note that these are specified LSB-first */
#define PORT_DIPLOCATION(location_) \
	INPUT_PORT_UINT32(INPUT_TOKEN_DIPLOCATION), INPUT_PORT_PTR(location_),

/* conditionals for dip switch settings */
#define PORT_CONDITION(tag_,mask_,condition_,value_) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CONDITION, condition_), INPUT_PORT_UINT32_PAIR(mask_, value_), INPUT_PORT_PTR(tag_),

/* analog adjuster definition */
#define PORT_ADJUSTER(default,name) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_ADJUSTER, default), INPUT_PORT_PTR(name),

/* config definition */
#define PORT_CONFNAME(mask,default,name) \
	INPUT_PORT_UINT32(INPUT_TOKEN_CONFNAME), INPUT_PORT_UINT32_PAIR(mask, default), INPUT_PORT_PTR(name),

#define PORT_CONFSETTING(default,name) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CONFSETTING, default), INPUT_PORT_PTR(name),

#ifdef MESS
/* keyboard chars */
#define PORT_CHAR(ch)	\
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CHAR, ch),

/* categories */
#define PORT_CATEGORY(category) \
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CATEGORY, category),

#define PORT_CATEGORY_CLASS(mask,default,name) 						\
	INPUT_PORT_UINT32(INPUT_TOKEN_CATEGORY_NAME), INPUT_PORT_UINT32_PAIR(mask, default), INPUT_PORT_PTR(name),

#define PORT_CATEGORY_ITEM(default,name,category) 					\
	INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CATEGORY_SETTING, default), INPUT_PORT_PTR(name), INPUT_PORT_UINT32_PAIR(INPUT_TOKEN_CATEGORY, category),
#endif /* MESS */



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define PORT_SERVICE_DIPLOC(mask,default,loc)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_DIPLOCATION(loc)	\
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )

#define PORT_SERVICE(mask,default)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE	\
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )

#define PORT_SERVICE_NO_TOGGLE(mask,default)	\
	PORT_BIT(    mask, mask & default, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode ))

#define PORT_DIPUNUSED_DIPLOC(mask,default,loc)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Unused )) PORT_DIPLOCATION(loc)	\
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )

#define PORT_DIPUNUSED(mask,default)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Unused )) \
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )

#define PORT_DIPUNKNOWN_DIPLOC(mask,default,loc)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Unknown )) PORT_DIPLOCATION(loc)	\
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )

#define PORT_DIPUNKNOWN(mask,default)	\
	PORT_BIT(    mask, mask & default, IPT_DIPSWITCH_NAME ) PORT_NAME( DEF_STR( Unknown )) \
	PORT_DIPSETTING(    mask & default, DEF_STR( Off ) )	\
	PORT_DIPSETTING(    mask &~default, DEF_STR( On ) )



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

#define DEF_STR(str_num) ((const char *)INPUT_STRING_##str_num)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void input_port_init(running_machine *machine, const input_port_token *ipt);
const char *input_port_string_from_token(const input_port_token token);

input_port_entry *input_port_initialize(input_port_init_params *params, UINT32 type, const char *tag, UINT32 mask, UINT32 defval);
input_port_entry *input_port_allocate(const input_port_token *ipt, input_port_entry *memory);
void input_port_parse_diplocation(input_port_entry *in, const char *location);

input_port_default_entry *get_input_port_list(void);
const input_port_default_entry *get_input_port_list_defaults(void);

int input_port_active(const input_port_entry *in);
int port_type_is_analog(int type);
int port_type_is_analog_absolute(int type);
int port_type_in_use(int type);
int port_type_to_group(int type, int player);
int port_tag_to_index(const char *tag);
read8_handler port_tag_to_handler8(const char *tag);
read16_handler port_tag_to_handler16(const char *tag);
read32_handler port_tag_to_handler32(const char *tag);
read64_handler port_tag_to_handler64(const char *tag);
const char *input_port_name(const input_port_entry *in);
const input_seq *input_port_seq(input_port_entry *in, input_seq_type seqtype);
const input_seq *input_port_default_seq(int type, int player, input_seq_type seqtype);
int input_port_condition(const input_port_entry *in);
void input_port_set_changed_callback(int port, UINT32 mask, void (*callback)(void *, UINT32, UINT32), void *param);

const char *port_type_to_token(int type, int player);
int token_to_port_type(const char *string, int *player);

int input_port_type_pressed(int type, int player);
int input_ui_pressed(int code);
int input_ui_pressed_repeat(int code, int speed);

void input_port_update_defaults(void);
void input_port_vblank_start(void);	/* called by cpuintrf.c - not for external use */
void input_port_vblank_end(void);	/* called by cpuintrf.c - not for external use */

void input_port_set_digital_value(int port, UINT32 value, UINT32 mask);

UINT32 get_crosshair_pos(int port_num, UINT8 player, UINT8 axis);

UINT32 readinputport(int port);
UINT32 readinputportbytag(const char *tag);
UINT32 readinputportbytag_safe(const char *tag, UINT32 defvalue);

#endif	/* __INPTPORT_H__ */
