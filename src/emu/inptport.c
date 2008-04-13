/***************************************************************************

    inptport.c

    Input port handling.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Theory of operation

    ------------
    OSD controls
    ------------

    There are three types of controls that the OSD can provide as potential
    input devices: digital controls, absolute analog controls, and relative
    analog controls.

    Digital controls have only two states: on or off. They are generally
    mapped to buttons and digital joystick directions (like a gamepad or a
    joystick hat). The OSD layer must return either 0 (off) or 1 (on) for
    these types of controls.

    Absolute analog controls are analog in the sense that they return a
    range of values depending on how much a given control is moved, but they
    are physically bounded. This means that there is a minimum and maximum
    limit to how far the control can be moved. They are generally mapped to
    analog joystick axes, lightguns, most PC steering wheels, and pedals.
    The OSD layer must determine the minimum and maximum range of each
    analog device and scale that to a value between -65536 and +65536
    representing the position of the control. -65536 generally refers to
    the topmost or leftmost position, while +65536 refers to the bottommost
    or rightmost position. Note that pedals are a special case here, the
    OSD layer needs to return half axis as full -65536 to + 65536 range.

    Relative analog controls are analog as well, but are not physically
    bounded. They can be moved continually in one direction without limit.
    They are generally mapped to trackballs and mice. Because they are
    unbounded, the OSD layer can only return delta values since the last
    read. Because of this, it is difficult to scale appropriately. For
    MAME's purposes, when mapping a mouse devices to a relative analog
    control, one pixel of movement should correspond to 512 units. Other
    analog control types should be scaled to return values of a similar
    magnitude. Like absolute analog controls, negative values refer to
    upward or leftward movement, while positive values refer to downward
    or rightward movement.

    -------------
    Game controls
    -------------

    Similarly, the types of controls used by arcade games fall into the same
    three categories: digital, absolute analog, and relative analog. The
    tricky part is how to map any arbitrary type of OSD control to an
    arbitrary type of game control.

    Digital controls: used for game buttons and standard 4/8-way joysticks,
    as well as many other types of game controls. Mapping an OSD digital
    control to a game's OSD control is trivial. For OSD analog controls,
    the MAME core does not directly support mapping any OSD analog devices
    to digital controls. However, the OSD layer is free to enumerate digital
    equivalents for analog devices. For example, each analog axis in the
    Windows OSD code enumerates to two digital controls, one for the
    negative direction (up/left) and one for the position direction
    (down/right). When these "digital" inputs are queried, the OSD layer
    checks the axis position against the center, adding in a dead zone,
    and returns 0 or 1 to indicate its position.

    Absolute analog controls: used for analog joysticks, lightguns, pedals,
    and wheel controls. Mapping an OSD absolute analog control to this type
    is easy. OSD relative analog controls can be mapped here as well by
    accumulating the deltas and bounding the results. OSD digital controls
    are mapped to these types of controls in pairs, one for a decrement and
    one for an increment, but apart from that, operate the same as the OSD
    relative analog controls by accumulating deltas and applying bounds.
    The speed of the digital delta is user-configurable per analog input.
    In addition, most absolute analog control types have an autocentering
    feature that is activated when using the digital increment/decrement
    sequences, which returns the control back to the center at a user-
    controllable speed if no digital sequences are pressed.

    Relative analog controls: used for trackballs and dial controls. Again,
    mapping an OSD relative analog control to this type is straightforward.
    OSD absolute analog controls can't map directly to these, but if the OSD
    layer provides a digital equivalent for each direction, it can be done.
    OSD digital controls map just like they do for absolute analog controls,
    except that the accumulated deltas are not bounded, but rather wrap.

***************************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "config.h"
#include "xmlfile.h"
#include "profiler.h"
#include "inputseq.h"
#include "ui.h"
#include "deprecat.h"
#include <ctype.h>
#include <time.h>

#ifdef MESS
#include "inputx.h"
#endif


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DIGITAL_JOYSTICKS_PER_PLAYER	3

/* these constants must match the order of the joystick directions in the IPT definition */
#define JOYDIR_UP			0
#define JOYDIR_DOWN			1
#define JOYDIR_LEFT			2
#define JOYDIR_RIGHT		3

#define JOYDIR_UP_BIT		(1 << JOYDIR_UP)
#define JOYDIR_DOWN_BIT		(1 << JOYDIR_DOWN)
#define JOYDIR_LEFT_BIT		(1 << JOYDIR_LEFT)
#define JOYDIR_RIGHT_BIT	(1 << JOYDIR_RIGHT)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _analog_port_info analog_port_info;
struct _analog_port_info
{
	analog_port_info *	next;				/* linked list */
	input_port_entry *	portentry;			/* pointer to the input port entry referenced */
	INT32				accum;				/* accumulated value (including relative adjustments) */
	INT32				previous;			/* previous adjusted value */
	INT32				previousanalog;		/* previous analog value */
	INT32				minimum;			/* minimum adjusted value */
	INT32				maximum;			/* maximum adjusted value */
	INT32				center;				/* center adjusted value for autocentering */
	INT32				reverse_val;		/* value where we subtract from to reverse directions */
	INT64				scalepos;			/* scale factor to apply to positive adjusted values */
	INT64				scaleneg;			/* scale factor to apply to negative adjusted values */
	INT64				keyscalepos;		/* scale factor to apply to the key delta field when pos */
	INT64				keyscaleneg;		/* scale factor to apply to the key delta field when neg */
	INT64				positionalscale;	/* scale factor to divide a joystick into positions */
	UINT8				shift;				/* left shift to apply to the final result */
	UINT8				bits;				/* how many bits of resolution are expected? */
	UINT8				absolute;			/* is this an absolute or relative input? */
	UINT8				wraps;				/* does the control wrap around? */
	UINT8				one_of_x;			/* is this a 1 of X positional input? */
	UINT8				autocenter;			/* autocenter this input? */
	UINT8				single_scale;		/* scale joystick differently if default is between min/max */
	UINT8				interpolate;		/* should we do linear interpolation for mid-frame reads? */
	UINT8				lastdigital;		/* was the last modification caused by a digital form? */
};


typedef struct _custom_port_info custom_port_info;
struct _custom_port_info
{
	custom_port_info *	next;				/* linked list */
	input_port_entry *	portentry;			/* pointer to the input port entry referenced */
	UINT8				shift;				/* left shift to apply to the final result */
};


typedef struct _changed_port_info changed_port_info;
struct _changed_port_info
{
	changed_port_info *	next;				/* linked list */
	input_port_entry *	portentry;			/* pointer to the input port entry referenced */
	UINT8				shift;				/* right shift to apply before calling callback */
};


typedef struct _input_bit_info input_bit_info;
struct _input_bit_info
{
	input_port_entry *	portentry;			/* port entry for this input */
	UINT8				impulse;			/* counter for impulse controls */
	UINT8				last;				/* were we pressed last time? */
};


typedef struct _input_port_info input_port_info;
struct _input_port_info
{
	const char *		tag;				/* tag for this port */
	UINT32				defvalue;			/* default value of all the bits */
	UINT32				digital;			/* value from digital inputs */
	UINT32				vblank;				/* value of all IPT_VBLANK bits */
	input_bit_info 		bit[MAX_BITS_PER_PORT]; /* info about each bit in the port */
	analog_port_info *	analoginfo;			/* pointer to linked list of analog port info */
	custom_port_info *	custominfo;			/* pointer to linked list of custom port info */
	changed_port_info *	changedinfo;		/* pointer to linked list of changed port info */
};


typedef struct _digital_joystick_info digital_joystick_info;
struct _digital_joystick_info
{
	input_port_entry *	portentry[4];		/* port entry for up,down,left,right respectively */
	UINT8				inuse;				/* is this joystick used? */
	UINT8				current;			/* current value */
	UINT8				current4way;		/* current 4-way value */
	UINT8				previous;			/* previous value */
};


struct _input_port_init_params
{
	input_port_entry *	ports;				/* base of the port array */
	int					max_ports;			/* maximum number of ports we can support */
	int					current_port;		/* current port index */
};



/***************************************************************************
    MACROS
***************************************************************************/

#define IS_ANALOG(in)				((in)->type >= __ipt_analog_start && (in)->type <= __ipt_analog_end)
#define IS_DIGITAL_JOYSTICK(in)		((in)->type >= __ipt_digital_joystick_start && (in)->type <= __ipt_digital_joystick_end)
#define JOYSTICK_INFO_FOR_PORT(in)	(&joystick_info[(in)->player][((in)->type - __ipt_digital_joystick_start) / 4])
#define JOYSTICK_DIR_FOR_PORT(in)	(((in)->type - __ipt_digital_joystick_start) % 4)

#define APPLY_SENSITIVITY(x,s)		(((INT64)(x) * (s)) / 100)
#define APPLY_INVERSE_SENSITIVITY(x,s) (((INT64)(x) * 100) / (s))

#define COMPUTE_SCALE(num,den)		(((INT64)(num) << 24) / (den))
#define RECIP_SCALE(s)				(((INT64)1 << 48) / (s))
#define APPLY_SCALE(x,s)			(((INT64)(x) * (s)) >> 24)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* current value of all the ports */
static input_port_info port_info[MAX_INPUT_PORTS];

/* additiona tracking information for special types of controls */
static digital_joystick_info joystick_info[MAX_PLAYERS][DIGITAL_JOYSTICKS_PER_PLAYER];

/* memory for UI keys */
static UINT8 ui_memory[__ipt_max];

/* XML attributes for the different types */
static const char *const seqtypestrings[] = { "standard", "decrement", "increment" };

/* original input_ports without modifications */
static input_port_entry *input_ports_default;

/* for scaling analog ports */
static attotime last_update;
static attoseconds_t last_delta;

/* playback information */
static UINT64 playback_accumulated_speed;
static UINT32 playback_accumulated_frames;



/***************************************************************************
    PORT HANDLER TABLES
***************************************************************************/

static const read8_machine_func port_handler8[] =
{
	input_port_0_r,			input_port_1_r,			input_port_2_r,			input_port_3_r,
	input_port_4_r,			input_port_5_r,			input_port_6_r,			input_port_7_r,
	input_port_8_r,			input_port_9_r,			input_port_10_r,		input_port_11_r,
	input_port_12_r,		input_port_13_r,		input_port_14_r,		input_port_15_r,
	input_port_16_r,		input_port_17_r,		input_port_18_r,		input_port_19_r,
	input_port_20_r,		input_port_21_r,		input_port_22_r,		input_port_23_r,
	input_port_24_r,		input_port_25_r,		input_port_26_r,		input_port_27_r,
	input_port_28_r,		input_port_29_r,		input_port_30_r,		input_port_31_r
};


static const read16_machine_func port_handler16[] =
{
	input_port_0_word_r,	input_port_1_word_r,	input_port_2_word_r,	input_port_3_word_r,
	input_port_4_word_r,	input_port_5_word_r,	input_port_6_word_r,	input_port_7_word_r,
	input_port_8_word_r,	input_port_9_word_r,	input_port_10_word_r,	input_port_11_word_r,
	input_port_12_word_r,	input_port_13_word_r,	input_port_14_word_r,	input_port_15_word_r,
	input_port_16_word_r,	input_port_17_word_r,	input_port_18_word_r,	input_port_19_word_r,
	input_port_20_word_r,	input_port_21_word_r,	input_port_22_word_r,	input_port_23_word_r,
	input_port_24_word_r,	input_port_25_word_r,	input_port_26_word_r,	input_port_27_word_r,
	input_port_28_word_r,	input_port_29_word_r,	input_port_30_word_r,	input_port_31_word_r
};


static const read32_machine_func port_handler32[] =
{
	input_port_0_dword_r,	input_port_1_dword_r,	input_port_2_dword_r,	input_port_3_dword_r,
	input_port_4_dword_r,	input_port_5_dword_r,	input_port_6_dword_r,	input_port_7_dword_r,
	input_port_8_dword_r,	input_port_9_dword_r,	input_port_10_dword_r,	input_port_11_dword_r,
	input_port_12_dword_r,	input_port_13_dword_r,	input_port_14_dword_r,	input_port_15_dword_r,
	input_port_16_dword_r,	input_port_17_dword_r,	input_port_18_dword_r,	input_port_19_dword_r,
	input_port_20_dword_r,	input_port_21_dword_r,	input_port_22_dword_r,	input_port_23_dword_r,
	input_port_24_dword_r,	input_port_25_dword_r,	input_port_26_dword_r,	input_port_27_dword_r,
	input_port_28_dword_r,	input_port_29_dword_r,	input_port_30_dword_r,	input_port_31_dword_r
};



/***************************************************************************
    COMMON SHARED STRINGS
***************************************************************************/

static const struct
{
	UINT32 id;
	const char *string;
} input_port_default_strings[] =
{
	{ INPUT_STRING_Off, "Off" },
	{ INPUT_STRING_On, "On" },
	{ INPUT_STRING_No, "No" },
	{ INPUT_STRING_Yes, "Yes" },
	{ INPUT_STRING_Lives, "Lives" },
	{ INPUT_STRING_Bonus_Life, "Bonus Life" },
	{ INPUT_STRING_Difficulty, "Difficulty" },
	{ INPUT_STRING_Demo_Sounds, "Demo Sounds" },
	{ INPUT_STRING_Coinage, "Coinage" },
	{ INPUT_STRING_Coin_A, "Coin A" },
	{ INPUT_STRING_Coin_B, "Coin B" },
	{ INPUT_STRING_9C_1C, "9 Coins/1 Credit" },
	{ INPUT_STRING_8C_1C, "8 Coins/1 Credit" },
	{ INPUT_STRING_7C_1C, "7 Coins/1 Credit" },
	{ INPUT_STRING_6C_1C, "6 Coins/1 Credit" },
	{ INPUT_STRING_5C_1C, "5 Coins/1 Credit" },
	{ INPUT_STRING_4C_1C, "4 Coins/1 Credit" },
	{ INPUT_STRING_3C_1C, "3 Coins/1 Credit" },
	{ INPUT_STRING_8C_3C, "8 Coins/3 Credits" },
	{ INPUT_STRING_4C_2C, "4 Coins/2 Credits" },
	{ INPUT_STRING_2C_1C, "2 Coins/1 Credit" },
	{ INPUT_STRING_5C_3C, "5 Coins/3 Credits" },
	{ INPUT_STRING_3C_2C, "3 Coins/2 Credits" },
	{ INPUT_STRING_4C_3C, "4 Coins/3 Credits" },
	{ INPUT_STRING_4C_4C, "4 Coins/4 Credits" },
	{ INPUT_STRING_3C_3C, "3 Coins/3 Credits" },
	{ INPUT_STRING_2C_2C, "2 Coins/2 Credits" },
	{ INPUT_STRING_1C_1C, "1 Coin/1 Credit" },
	{ INPUT_STRING_4C_5C, "4 Coins/5 Credits" },
	{ INPUT_STRING_3C_4C, "3 Coins/4 Credits" },
	{ INPUT_STRING_2C_3C, "2 Coins/3 Credits" },
	{ INPUT_STRING_4C_7C, "4 Coins/7 Credits" },
	{ INPUT_STRING_2C_4C, "2 Coins/4 Credits" },
	{ INPUT_STRING_1C_2C, "1 Coin/2 Credits" },
	{ INPUT_STRING_2C_5C, "2 Coins/5 Credits" },
	{ INPUT_STRING_2C_6C, "2 Coins/6 Credits" },
	{ INPUT_STRING_1C_3C, "1 Coin/3 Credits" },
	{ INPUT_STRING_2C_7C, "2 Coins/7 Credits" },
	{ INPUT_STRING_2C_8C, "2 Coins/8 Credits" },
	{ INPUT_STRING_1C_4C, "1 Coin/4 Credits" },
	{ INPUT_STRING_1C_5C, "1 Coin/5 Credits" },
	{ INPUT_STRING_1C_6C, "1 Coin/6 Credits" },
	{ INPUT_STRING_1C_7C, "1 Coin/7 Credits" },
	{ INPUT_STRING_1C_8C, "1 Coin/8 Credits" },
	{ INPUT_STRING_1C_9C, "1 Coin/9 Credits" },
	{ INPUT_STRING_Free_Play, "Free Play" },
	{ INPUT_STRING_Cabinet, "Cabinet" },
	{ INPUT_STRING_Upright, "Upright" },
	{ INPUT_STRING_Cocktail, "Cocktail" },
	{ INPUT_STRING_Flip_Screen, "Flip Screen" },
	{ INPUT_STRING_Service_Mode, "Service Mode" },
	{ INPUT_STRING_Pause, "Pause" },
	{ INPUT_STRING_Test, "Test" },
	{ INPUT_STRING_Tilt, "Tilt" },
	{ INPUT_STRING_Version, "Version" },
	{ INPUT_STRING_Region, "Region" },
	{ INPUT_STRING_International, "International" },
	{ INPUT_STRING_Japan, "Japan" },
	{ INPUT_STRING_USA, "USA" },
	{ INPUT_STRING_Europe, "Europe" },
	{ INPUT_STRING_Asia, "Asia" },
	{ INPUT_STRING_World, "World" },
	{ INPUT_STRING_Hispanic, "Hispanic" },
	{ INPUT_STRING_Language, "Language" },
	{ INPUT_STRING_English, "English" },
	{ INPUT_STRING_Japanese, "Japanese" },
	{ INPUT_STRING_German, "German" },
	{ INPUT_STRING_French, "French" },
	{ INPUT_STRING_Italian, "Italian" },
	{ INPUT_STRING_Spanish, "Spanish" },
	{ INPUT_STRING_Very_Easy, "Very Easy" },
	{ INPUT_STRING_Easiest, "Easiest" },
	{ INPUT_STRING_Easier, "Easier" },
	{ INPUT_STRING_Easy, "Easy" },
	{ INPUT_STRING_Normal, "Normal" },
	{ INPUT_STRING_Medium, "Medium" },
	{ INPUT_STRING_Hard, "Hard" },
	{ INPUT_STRING_Harder, "Harder" },
	{ INPUT_STRING_Hardest, "Hardest" },
	{ INPUT_STRING_Very_Hard, "Very Hard" },
	{ INPUT_STRING_Very_Low, "Very Low" },
	{ INPUT_STRING_Low, "Low" },
	{ INPUT_STRING_High, "High" },
	{ INPUT_STRING_Higher, "Higher" },
	{ INPUT_STRING_Highest, "Highest" },
	{ INPUT_STRING_Very_High, "Very High" },
	{ INPUT_STRING_Players, "Players" },
	{ INPUT_STRING_Controls, "Controls" },
	{ INPUT_STRING_Dual, "Dual" },
	{ INPUT_STRING_Single, "Single" },
	{ INPUT_STRING_Game_Time, "Game Time" },
	{ INPUT_STRING_Continue_Price, "Continue Price" },
	{ INPUT_STRING_Controller, "Controller" },
	{ INPUT_STRING_Light_Gun, "Light Gun" },
	{ INPUT_STRING_Joystick, "Joystick" },
	{ INPUT_STRING_Trackball, "Trackball" },
	{ INPUT_STRING_Continues, "Continues" },
	{ INPUT_STRING_Allow_Continue, "Allow Continue" },
	{ INPUT_STRING_Level_Select, "Level Select" },
	{ INPUT_STRING_Infinite, "Infinite" },
	{ INPUT_STRING_Stereo, "Stereo" },
	{ INPUT_STRING_Mono, "Mono" },
	{ INPUT_STRING_Unused, "Unused" },
	{ INPUT_STRING_Unknown, "Unknown" },
	{ INPUT_STRING_Standard, "Standard" },
	{ INPUT_STRING_Reverse, "Reverse" },
	{ INPUT_STRING_Alternate, "Alternate" },
	{ INPUT_STRING_None, "None" }
};



/***************************************************************************
    DEFAULT INPUT PORTS
***************************************************************************/

#define INPUT_PORT_DIGITAL_DEF(player_,group_,type_,name_,seq_) \
	{ IPT_##type_, group_, (player_ == 0) ? player_ : (player_) - 1, (player_ == 0) ? #type_ : ("P" #player_ "_" #type_), name_, seq_, SEQ_DEF_0, SEQ_DEF_0 },

#define INPUT_PORT_ANALOG_DEF(player_,group_,type_,name_,seq_,decseq_,incseq_) \
	{ IPT_##type_, group_, (player_ == 0) ? player_ : (player_) - 1, (player_ == 0) ? #type_ : ("P" #player_ "_" #type_), name_, seq_, incseq_, decseq_ },

#define INDEXED(a,b)	INPUT_CODE_SET_DEVINDEX(a,b)

static const input_port_default_entry default_ports_builtin[] =
{
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICK_UP,		"P1 Up",				SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICK_DOWN,      "P1 Down",				SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICK_LEFT,      "P1 Left",    			SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICK_RIGHT,     "P1 Right",   			SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKRIGHT_UP,   "P1 Right/Up",			SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKRIGHT_DOWN, "P1 Right/Down",		SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKRIGHT_LEFT, "P1 Right/Left",		SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKRIGHT_RIGHT,"P1 Right/Right",		SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_BUTTON4, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKLEFT_UP,    "P1 Left/Up", 			SEQ_DEF_3(KEYCODE_E, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKLEFT_DOWN,  "P1 Left/Down",			SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKLEFT_LEFT,  "P1 Left/Left", 		SEQ_DEF_3(KEYCODE_S, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	JOYSTICKLEFT_RIGHT, "P1 Left/Right",		SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON1,			"P1 Button 1",			SEQ_DEF_7(KEYCODE_LCONTROL, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 0), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON1, 0), SEQCODE_OR, INDEXED(GUNCODE_BUTTON1, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON2,			"P1 Button 2",			SEQ_DEF_7(KEYCODE_LALT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 0), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON3, 0), SEQCODE_OR, INDEXED(GUNCODE_BUTTON2, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON3,			"P1 Button 3",			SEQ_DEF_5(KEYCODE_SPACE, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 0), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON2, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON4,			"P1 Button 4",			SEQ_DEF_3(KEYCODE_LSHIFT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON4, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON5,			"P1 Button 5",			SEQ_DEF_3(KEYCODE_Z, SEQCODE_OR, INDEXED(JOYCODE_BUTTON5, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON6,			"P1 Button 6",			SEQ_DEF_3(KEYCODE_X, SEQCODE_OR, INDEXED(JOYCODE_BUTTON6, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON7,			"P1 Button 7",			SEQ_DEF_3(KEYCODE_C, SEQCODE_OR, INDEXED(JOYCODE_BUTTON7, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON8,			"P1 Button 8",			SEQ_DEF_3(KEYCODE_V, SEQCODE_OR, INDEXED(JOYCODE_BUTTON8, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON9,			"P1 Button 9",			SEQ_DEF_3(KEYCODE_B, SEQCODE_OR, INDEXED(JOYCODE_BUTTON9, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON10,			"P1 Button 10",			SEQ_DEF_3(KEYCODE_N, SEQCODE_OR, INDEXED(JOYCODE_BUTTON10, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON11,			"P1 Button 11",			SEQ_DEF_3(KEYCODE_M, SEQCODE_OR, INDEXED(JOYCODE_BUTTON11, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON12,			"P1 Button 12",			SEQ_DEF_3(KEYCODE_COMMA, SEQCODE_OR, INDEXED(JOYCODE_BUTTON12, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON13,			"P1 Button 13",			SEQ_DEF_3(KEYCODE_STOP, SEQCODE_OR, INDEXED(JOYCODE_BUTTON13, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON14,			"P1 Button 14",			SEQ_DEF_3(KEYCODE_SLASH, SEQCODE_OR, INDEXED(JOYCODE_BUTTON14, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON15,			"P1 Button 15",			SEQ_DEF_3(KEYCODE_RSHIFT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON15, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	BUTTON16,			"P1 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1, START,				"P1 Start",				SEQ_DEF_3(KEYCODE_1, SEQCODE_OR, INDEXED(JOYCODE_START, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1, SELECT,				"P1 Select",			SEQ_DEF_3(KEYCODE_5, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 0)) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_A,          "P1 Mahjong A",			SEQ_DEF_1(KEYCODE_A) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_B,          "P1 Mahjong B",			SEQ_DEF_1(KEYCODE_B) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_C,          "P1 Mahjong C",			SEQ_DEF_1(KEYCODE_C) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_D,          "P1 Mahjong D",			SEQ_DEF_1(KEYCODE_D) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_E,          "P1 Mahjong E",			SEQ_DEF_1(KEYCODE_E) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_F,          "P1 Mahjong F",			SEQ_DEF_1(KEYCODE_F) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_G,          "P1 Mahjong G",			SEQ_DEF_1(KEYCODE_G) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_H,          "P1 Mahjong H",			SEQ_DEF_1(KEYCODE_H) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_I,          "P1 Mahjong I",			SEQ_DEF_1(KEYCODE_I) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_J,          "P1 Mahjong J",			SEQ_DEF_1(KEYCODE_J) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_K,          "P1 Mahjong K",			SEQ_DEF_1(KEYCODE_K) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_L,          "P1 Mahjong L",			SEQ_DEF_1(KEYCODE_L) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_M,          "P1 Mahjong M",			SEQ_DEF_1(KEYCODE_M) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_N,          "P1 Mahjong N",			SEQ_DEF_1(KEYCODE_N) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_O,          "P1 Mahjong O",			SEQ_DEF_1(KEYCODE_O) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_P,          "P1 Mahjong P",			SEQ_DEF_1(KEYCODE_COLON) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_Q,          "P1 Mahjong Q",			SEQ_DEF_1(KEYCODE_Q) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_KAN,        "P1 Mahjong Kan",		SEQ_DEF_1(KEYCODE_LCONTROL) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_PON,        "P1 Mahjong Pon",		SEQ_DEF_1(KEYCODE_LALT) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_CHI,        "P1 Mahjong Chi",		SEQ_DEF_1(KEYCODE_SPACE) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_REACH,      "P1 Mahjong Reach",		SEQ_DEF_1(KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_RON,        "P1 Mahjong Ron",		SEQ_DEF_1(KEYCODE_Z) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_BET,        "P1 Mahjong Bet",		SEQ_DEF_1(KEYCODE_2) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_LAST_CHANCE,"P1 Mahjong Last Chance",SEQ_DEF_1(KEYCODE_RALT) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_SCORE,      "P1 Mahjong Score",		SEQ_DEF_1(KEYCODE_RCONTROL) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_DOUBLE_UP,  "P1 Mahjong Double Up",	SEQ_DEF_1(KEYCODE_RSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_FLIP_FLOP,  "P1 Mahjong Flip Flop",	SEQ_DEF_1(KEYCODE_Y) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_BIG,        "P1 Mahjong Big",       SEQ_DEF_1(KEYCODE_ENTER) )
	INPUT_PORT_DIGITAL_DEF( 1, IPG_PLAYER1,	MAHJONG_SMALL,      "P1 Mahjong Small",     SEQ_DEF_1(KEYCODE_BACKSPACE) )

	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICK_UP,		"P2 Up",				SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICK_DOWN,      "P2 Down",    			SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICK_LEFT,      "P2 Left",    			SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICK_RIGHT,     "P2 Right",   			SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKRIGHT_UP,   "P2 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKRIGHT_DOWN, "P2 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKRIGHT_LEFT, "P2 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKRIGHT_RIGHT,"P2 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKLEFT_UP,    "P2 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKLEFT_DOWN,  "P2 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKLEFT_LEFT,  "P2 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	JOYSTICKLEFT_RIGHT, "P2 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON1,			"P2 Button 1",			SEQ_DEF_7(KEYCODE_A, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 1), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON1, 1), SEQCODE_OR, INDEXED(GUNCODE_BUTTON1, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON2,			"P2 Button 2",			SEQ_DEF_7(KEYCODE_S, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 1), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON3, 1), SEQCODE_OR, INDEXED(GUNCODE_BUTTON2, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON3,			"P2 Button 3",			SEQ_DEF_5(KEYCODE_Q, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 1), SEQCODE_OR, INDEXED(MOUSECODE_BUTTON2, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON4,			"P2 Button 4",			SEQ_DEF_3(KEYCODE_W, SEQCODE_OR, INDEXED(JOYCODE_BUTTON4, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON5,			"P2 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON6,			"P2 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON7,			"P2 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON8,			"P2 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON9,			"P2 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON10,			"P2 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON11,			"P2 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON12,			"P2 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON13,			"P2 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON14,			"P2 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON15,			"P2 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	BUTTON16,			"P2 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2, START,				"P2 Start",				SEQ_DEF_3(KEYCODE_2, SEQCODE_OR, INDEXED(JOYCODE_START, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2, SELECT,				"P2 Select",			SEQ_DEF_3(KEYCODE_6, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 1)) )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_A,          "P2 Mahjong A",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_B,          "P2 Mahjong B",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_C,          "P2 Mahjong C",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_D,          "P2 Mahjong D",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_E,          "P2 Mahjong E",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_F,          "P2 Mahjong F",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_G,          "P2 Mahjong G",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_H,          "P2 Mahjong H",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_I,          "P2 Mahjong I",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_J,          "P2 Mahjong J",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_K,          "P2 Mahjong K",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_L,          "P2 Mahjong L",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_M,          "P2 Mahjong M",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_N,          "P2 Mahjong N",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_O,          "P2 Mahjong O",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_P,          "P2 Mahjong P",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_Q,          "P2 Mahjong Q",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_KAN,        "P2 Mahjong Kan",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_PON,        "P2 Mahjong Pon",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_CHI,        "P2 Mahjong Chi",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_REACH,      "P2 Mahjong Reach",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_RON,        "P2 Mahjong Ron",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_BET,        "P2 Mahjong Bet",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_LAST_CHANCE,"P2 Mahjong Last Chance",SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_SCORE,      "P2 Mahjong Score",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_DOUBLE_UP,  "P2 Mahjong Double Up",	SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_FLIP_FLOP,  "P2 Mahjong Flip Flop",	SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_BIG,        "P2 Mahjong Big",       SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 2, IPG_PLAYER2,	MAHJONG_SMALL,      "P2 Mahjong Small",     SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICK_UP,		"P3 Up",				SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICK_DOWN,      "P3 Down",    			SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICK_LEFT,      "P3 Left",    			SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICK_RIGHT,     "P3 Right",   			SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKRIGHT_UP,   "P3 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKRIGHT_DOWN, "P3 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKRIGHT_LEFT, "P3 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKRIGHT_RIGHT,"P3 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKLEFT_UP,    "P3 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKLEFT_DOWN,  "P3 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKLEFT_LEFT,  "P3 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	JOYSTICKLEFT_RIGHT, "P3 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON1,			"P3 Button 1",			SEQ_DEF_5(KEYCODE_RCONTROL, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 2), SEQCODE_OR, INDEXED(GUNCODE_BUTTON1, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON2,			"P3 Button 2",			SEQ_DEF_5(KEYCODE_RSHIFT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 2), SEQCODE_OR, INDEXED(GUNCODE_BUTTON2, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON3,			"P3 Button 3",			SEQ_DEF_3(KEYCODE_ENTER, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON4,			"P3 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON5,			"P3 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON6,			"P3 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON7,			"P3 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON8,			"P3 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON9,			"P3 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON10,			"P3 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON11,			"P3 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON12,			"P3 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON13,			"P3 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON14,			"P3 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON15,			"P3 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3,	BUTTON16,			"P3 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3, START,				"P3 Start",				SEQ_DEF_3(KEYCODE_3, SEQCODE_OR, INDEXED(JOYCODE_START, 2)) )
	INPUT_PORT_DIGITAL_DEF( 3, IPG_PLAYER3, SELECT,				"P3 Select",			SEQ_DEF_3(KEYCODE_7, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 2)) )

	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICK_UP,		"P4 Up",				SEQ_DEF_3(KEYCODE_8_PAD, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICK_DOWN,      "P4 Down",    			SEQ_DEF_3(KEYCODE_2_PAD, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICK_LEFT,      "P4 Left",    			SEQ_DEF_3(KEYCODE_4_PAD, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICK_RIGHT,     "P4 Right",   			SEQ_DEF_3(KEYCODE_6_PAD, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKRIGHT_UP,   "P4 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKRIGHT_DOWN, "P4 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKRIGHT_LEFT, "P4 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKRIGHT_RIGHT,"P4 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKLEFT_UP,    "P4 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKLEFT_DOWN,  "P4 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKLEFT_LEFT,  "P4 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	JOYSTICKLEFT_RIGHT, "P4 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON1,			"P4 Button 1",			SEQ_DEF_3(KEYCODE_0_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON2,			"P4 Button 2",			SEQ_DEF_3(KEYCODE_DEL_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON3,			"P4 Button 3",			SEQ_DEF_3(KEYCODE_ENTER_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON4,			"P4 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON5,			"P4 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON6,			"P4 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON7,			"P4 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON8,			"P4 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON9,			"P4 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON10,			"P4 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON11,			"P4 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON12,			"P4 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON13,			"P4 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON14,			"P4 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON15,			"P4 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4,	BUTTON16,			"P4 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4, START,				"P4 Start",				SEQ_DEF_3(KEYCODE_4, SEQCODE_OR, INDEXED(JOYCODE_START, 3)) )
	INPUT_PORT_DIGITAL_DEF( 4, IPG_PLAYER4, SELECT,				"P4 Select",			SEQ_DEF_3(KEYCODE_8, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 3)) )

	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICK_UP,		"P5 Up",				SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICK_DOWN,      "P5 Down",    			SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICK_LEFT,      "P5 Left",    			SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICK_RIGHT,     "P5 Right",   			SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKRIGHT_UP,   "P5 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKRIGHT_DOWN, "P5 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKRIGHT_LEFT, "P5 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKRIGHT_RIGHT,"P5 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKLEFT_UP,    "P5 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKLEFT_DOWN,  "P5 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKLEFT_LEFT,  "P5 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	JOYSTICKLEFT_RIGHT, "P5 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON1,			"P5 Button 1",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON2,			"P5 Button 2",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON3,			"P5 Button 3",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON4,			"P5 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON5,			"P5 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON6,			"P5 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON7,			"P5 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON8,			"P5 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON9,			"P5 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON10,			"P5 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON11,			"P5 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON12,			"P5 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON13,			"P5 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON14,			"P5 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON15,			"P5 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5,	BUTTON16,			"P5 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 4)) )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5, START,				"P5 Start",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 5, IPG_PLAYER5, SELECT,				"P5 Select",			SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICK_UP,		"P6 Up",				SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICK_DOWN,      "P6 Down",    			SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICK_LEFT,      "P6 Left",    			SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICK_RIGHT,     "P6 Right",   			SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKRIGHT_UP,   "P6 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKRIGHT_DOWN, "P6 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKRIGHT_LEFT, "P6 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKRIGHT_RIGHT,"P6 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKLEFT_UP,    "P6 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKLEFT_DOWN,  "P6 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKLEFT_LEFT,  "P6 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	JOYSTICKLEFT_RIGHT, "P6 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON1,			"P6 Button 1",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON2,			"P6 Button 2",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON3,			"P6 Button 3",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON4,			"P6 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON5,			"P6 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON6,			"P6 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON7,			"P6 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON8,			"P6 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON9,			"P6 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON10,			"P6 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON11,			"P6 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON12,			"P6 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON13,			"P6 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON14,			"P6 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON15,			"P6 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6,	BUTTON16,			"P6 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 5)) )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6, START,				"P6 Start",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 6, IPG_PLAYER6, SELECT,				"P6 Select",			SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICK_UP,		"P7 Up",				SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICK_DOWN,      "P7 Down",    			SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICK_LEFT,      "P7 Left",    			SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICK_RIGHT,     "P7 Right",   			SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKRIGHT_UP,   "P7 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKRIGHT_DOWN, "P7 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKRIGHT_LEFT, "P7 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKRIGHT_RIGHT,"P7 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKLEFT_UP,    "P7 Left/Up", 			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKLEFT_DOWN,  "P7 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKLEFT_LEFT,  "P7 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	JOYSTICKLEFT_RIGHT, "P7 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON1,			"P7 Button 1",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON2,			"P7 Button 2",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON3,			"P7 Button 3",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON4,			"P7 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON5,			"P7 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON6,			"P7 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON7,			"P7 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON8,			"P7 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON9,			"P7 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON10,			"P7 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON11,			"P7 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON12,			"P7 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON13,			"P7 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON14,			"P7 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON15,			"P7 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7,	BUTTON16,			"P7 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 6)) )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7, START,				"P7 Start",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 7, IPG_PLAYER7, SELECT,				"P7 Select",			SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICK_UP,		"P8 Up",				SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICK_DOWN,      "P8 Down",				SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICK_LEFT,      "P8 Left",				SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICK_RIGHT,     "P8 Right",				SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKRIGHT_UP,   "P8 Right/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKRIGHT_DOWN, "P8 Right/Down",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKRIGHT_LEFT, "P8 Right/Left",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKRIGHT_RIGHT,"P8 Right/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKLEFT_UP,    "P8 Left/Up",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKLEFT_DOWN,  "P8 Left/Down",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKLEFT_LEFT,  "P8 Left/Left",			SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	JOYSTICKLEFT_RIGHT, "P8 Left/Right",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON1,			"P8 Button 1",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON2,			"P8 Button 2",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON3,			"P8 Button 3",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON4,			"P8 Button 4",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON4, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON5,			"P8 Button 5",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON5, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON6,			"P8 Button 6",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON6, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON7,			"P8 Button 7",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON7, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON8,			"P8 Button 8",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON8, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON9,			"P8 Button 9",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON9, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON10,			"P8 Button 10",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON10, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON11,			"P8 Button 11",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON11, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON12,			"P8 Button 12",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON12, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON13,			"P8 Button 13",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON13, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON14,			"P8 Button 14",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON14, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON15,			"P8 Button 15",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON15, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8,	BUTTON16,			"P8 Button 16",			SEQ_DEF_1(INDEXED(JOYCODE_BUTTON16, 7)) )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8, START,				"P8 Start",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 8, IPG_PLAYER8, SELECT,				"P8 Select",			SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START1,				"1 Player Start",		SEQ_DEF_3(KEYCODE_1, SEQCODE_OR, INDEXED(JOYCODE_START, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START2,				"2 Players Start",		SEQ_DEF_3(KEYCODE_2, SEQCODE_OR, INDEXED(JOYCODE_START, 1)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START3,				"3 Players Start",		SEQ_DEF_3(KEYCODE_3, SEQCODE_OR, INDEXED(JOYCODE_START, 2)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START4,				"4 Players Start",		SEQ_DEF_3(KEYCODE_4, SEQCODE_OR, INDEXED(JOYCODE_START, 3)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START5,				"5 Players Start",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START6,				"6 Players Start",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START7,				"7 Players Start",		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   START8,				"8 Players Start",		SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN1,				"Coin 1",				SEQ_DEF_3(KEYCODE_5, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN2,				"Coin 2",				SEQ_DEF_3(KEYCODE_6, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 1)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN3,				"Coin 3",				SEQ_DEF_3(KEYCODE_7, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 2)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN4,				"Coin 4",				SEQ_DEF_3(KEYCODE_8, SEQCODE_OR, INDEXED(JOYCODE_SELECT, 3)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN5,				"Coin 5",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN6,				"Coin 6",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN7,				"Coin 7",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   COIN8,				"Coin 8",				SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   BILL1,				"Bill 1",				SEQ_DEF_1(KEYCODE_BACKSPACE) )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   SERVICE1,			"Service 1",    		SEQ_DEF_1(KEYCODE_9) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   SERVICE2,			"Service 2",    		SEQ_DEF_1(KEYCODE_0) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   SERVICE3,			"Service 3",     		SEQ_DEF_1(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   SERVICE4,			"Service 4",     		SEQ_DEF_1(KEYCODE_EQUALS) )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   SERVICE,			"Service",	     		SEQ_DEF_1(KEYCODE_F2) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   TILT, 				"Tilt",			   		SEQ_DEF_1(KEYCODE_T) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   INTERLOCK,			"Door Interlock",  		SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   VOLUME_DOWN,		"Volume Down",     		SEQ_DEF_1(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   VOLUME_UP,			"Volume Up",     		SEQ_DEF_1(KEYCODE_EQUALS) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	PEDAL,				"P1 Pedal 1",     		SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 0)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_LCONTROL, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	PEDAL,				"P2 Pedal 1", 			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 1)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_A, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	PEDAL,				"P3 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 2)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_RCONTROL, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	PEDAL,				"P4 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 3)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_0_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	PEDAL,				"P5 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 4)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	PEDAL,				"P6 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 5)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	PEDAL,				"P7 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 6)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	PEDAL,				"P8 Pedal 1",			SEQ_DEF_1(INDEXED(JOYCODE_Y_NEG_ABSOLUTE, 7)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON1, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	PEDAL2,				"P1 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 0)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_LALT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	PEDAL2,				"P2 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 1)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_S, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	PEDAL2,				"P3 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 2)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_RSHIFT, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	PEDAL2,				"P4 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 3)), SEQ_DEF_0, SEQ_DEF_3(KEYCODE_DEL_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON2, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	PEDAL2,				"P5 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 4)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	PEDAL2,				"P6 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 5)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	PEDAL2,				"P7 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 6)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	PEDAL2,				"P8 Pedal 2",			SEQ_DEF_1(INDEXED(JOYCODE_Y_POS_ABSOLUTE, 7)), SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON2, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	PEDAL3,				"P1 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_3(KEYCODE_SPACE, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	PEDAL3,				"P2 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_3(KEYCODE_Q, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	PEDAL3,				"P3 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_3(KEYCODE_ENTER, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	PEDAL3,				"P4 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_3(KEYCODE_ENTER_PAD, SEQCODE_OR, INDEXED(JOYCODE_BUTTON3, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	PEDAL3,				"P5 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	PEDAL3,				"P6 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	PEDAL3,				"P7 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	PEDAL3,				"P8 Pedal 3",			SEQ_DEF_0, SEQ_DEF_0, SEQ_DEF_1(INDEXED(JOYCODE_BUTTON3, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	PADDLE,				"Paddle",   	    	SEQ_DEF_3(INDEXED(MOUSECODE_X, 0), SEQCODE_OR, INDEXED(JOYCODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	PADDLE,				"Paddle 2",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 1), SEQCODE_OR, INDEXED(JOYCODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	PADDLE,				"Paddle 3",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 2), SEQCODE_OR, INDEXED(JOYCODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	PADDLE,				"Paddle 4",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 3), SEQCODE_OR, INDEXED(JOYCODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	PADDLE,				"Paddle 5",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 4), SEQCODE_OR, INDEXED(JOYCODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	PADDLE,				"Paddle 6",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 5), SEQCODE_OR, INDEXED(JOYCODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	PADDLE,				"Paddle 7",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 6), SEQCODE_OR, INDEXED(JOYCODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	PADDLE,				"Paddle 8",      		SEQ_DEF_3(INDEXED(MOUSECODE_X, 7), SEQCODE_OR, INDEXED(JOYCODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	PADDLE_V,			"Paddle V",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 0), SEQCODE_OR, INDEXED(JOYCODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	PADDLE_V,			"Paddle V 2",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 1), SEQCODE_OR, INDEXED(JOYCODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	PADDLE_V,			"Paddle V 3",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 2), SEQCODE_OR, INDEXED(JOYCODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	PADDLE_V,			"Paddle V 4",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 3), SEQCODE_OR, INDEXED(JOYCODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	PADDLE_V,			"Paddle V 5",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 4), SEQCODE_OR, INDEXED(JOYCODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	PADDLE_V,			"Paddle V 6",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 5), SEQCODE_OR, INDEXED(JOYCODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	PADDLE_V,			"Paddle V 7",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 6), SEQCODE_OR, INDEXED(JOYCODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	PADDLE_V,			"Paddle V 8",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 7), SEQCODE_OR, INDEXED(JOYCODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	POSITIONAL,			"Positional",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 0), SEQCODE_OR, INDEXED(JOYCODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	POSITIONAL,			"Positional 2",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 1), SEQCODE_OR, INDEXED(JOYCODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	POSITIONAL,			"Positional 3", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 2), SEQCODE_OR, INDEXED(JOYCODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	POSITIONAL,			"Positional 4", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 3), SEQCODE_OR, INDEXED(JOYCODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	POSITIONAL,			"Positional 5", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 4), SEQCODE_OR, INDEXED(JOYCODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	POSITIONAL,			"Positional 6", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 5), SEQCODE_OR, INDEXED(JOYCODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	POSITIONAL,			"Positional 7", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 6), SEQCODE_OR, INDEXED(JOYCODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	POSITIONAL,			"Positional 8", 		SEQ_DEF_3(INDEXED(MOUSECODE_X, 7), SEQCODE_OR, INDEXED(JOYCODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	POSITIONAL_V,		"Positional V",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 0), SEQCODE_OR, INDEXED(JOYCODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	POSITIONAL_V,		"Positional V 2",		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 1), SEQCODE_OR, INDEXED(JOYCODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	POSITIONAL_V,		"Positional V 3", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 2), SEQCODE_OR, INDEXED(JOYCODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	POSITIONAL_V,		"Positional V 4", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 3), SEQCODE_OR, INDEXED(JOYCODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	POSITIONAL_V,		"Positional V 5", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 4), SEQCODE_OR, INDEXED(JOYCODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	POSITIONAL_V,		"Positional V 6", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 5), SEQCODE_OR, INDEXED(JOYCODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	POSITIONAL_V,		"Positional V 7", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 6), SEQCODE_OR, INDEXED(JOYCODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	POSITIONAL_V,		"Positional V 8", 		SEQ_DEF_3(INDEXED(MOUSECODE_Y, 7), SEQCODE_OR, INDEXED(JOYCODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	DIAL,				"Dial",					SEQ_DEF_3(INDEXED(MOUSECODE_X, 0), SEQCODE_OR, INDEXED(JOYCODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	DIAL,				"Dial 2",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 1), SEQCODE_OR, INDEXED(JOYCODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	DIAL,				"Dial 3",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 2), SEQCODE_OR, INDEXED(JOYCODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	DIAL,				"Dial 4",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 3), SEQCODE_OR, INDEXED(JOYCODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	DIAL,				"Dial 5",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 4), SEQCODE_OR, INDEXED(JOYCODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	DIAL,				"Dial 6",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 5), SEQCODE_OR, INDEXED(JOYCODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	DIAL,				"Dial 7",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 6), SEQCODE_OR, INDEXED(JOYCODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	DIAL,				"Dial 8",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 7), SEQCODE_OR, INDEXED(JOYCODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	DIAL_V,				"Dial V",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 0), SEQCODE_OR, INDEXED(JOYCODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	DIAL_V,				"Dial V 2",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 1), SEQCODE_OR, INDEXED(JOYCODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	DIAL_V,				"Dial V 3",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 2), SEQCODE_OR, INDEXED(JOYCODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	DIAL_V,				"Dial V 4",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 3), SEQCODE_OR, INDEXED(JOYCODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	DIAL_V,				"Dial V 5",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 4), SEQCODE_OR, INDEXED(JOYCODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	DIAL_V,				"Dial V 6",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 5), SEQCODE_OR, INDEXED(JOYCODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	DIAL_V,				"Dial V 7",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 6), SEQCODE_OR, INDEXED(JOYCODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	DIAL_V,				"Dial V 8",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 7), SEQCODE_OR, INDEXED(JOYCODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	TRACKBALL_X,		"Track X",				SEQ_DEF_3(INDEXED(MOUSECODE_X, 0), SEQCODE_OR, INDEXED(JOYCODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	TRACKBALL_X,		"Track X 2",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 1), SEQCODE_OR, INDEXED(JOYCODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	TRACKBALL_X,		"Track X 3",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 2), SEQCODE_OR, INDEXED(JOYCODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	TRACKBALL_X,		"Track X 4",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 3), SEQCODE_OR, INDEXED(JOYCODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	TRACKBALL_X,		"Track X 5",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 4), SEQCODE_OR, INDEXED(JOYCODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	TRACKBALL_X,		"Track X 6",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 5), SEQCODE_OR, INDEXED(JOYCODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	TRACKBALL_X,		"Track X 7",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 6), SEQCODE_OR, INDEXED(JOYCODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	TRACKBALL_X,		"Track X 8",			SEQ_DEF_3(INDEXED(MOUSECODE_X, 7), SEQCODE_OR, INDEXED(JOYCODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	TRACKBALL_Y,		"Track Y",				SEQ_DEF_3(INDEXED(MOUSECODE_Y, 0), SEQCODE_OR, INDEXED(JOYCODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	TRACKBALL_Y,		"Track Y 2",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 1), SEQCODE_OR, INDEXED(JOYCODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	TRACKBALL_Y,		"Track Y 3",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 2), SEQCODE_OR, INDEXED(JOYCODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	TRACKBALL_Y,		"Track Y 4",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 3), SEQCODE_OR, INDEXED(JOYCODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	TRACKBALL_Y,		"Track Y 5",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 4), SEQCODE_OR, INDEXED(JOYCODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	TRACKBALL_Y,		"Track Y 6",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 5), SEQCODE_OR, INDEXED(JOYCODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	TRACKBALL_Y,		"Track Y 7",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 6), SEQCODE_OR, INDEXED(JOYCODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	TRACKBALL_Y,		"Track Y 8",			SEQ_DEF_3(INDEXED(MOUSECODE_Y, 7), SEQCODE_OR, INDEXED(JOYCODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	AD_STICK_X,			"AD Stick X",			SEQ_DEF_3(INDEXED(JOYCODE_X, 0), SEQCODE_OR, INDEXED(MOUSECODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	AD_STICK_X,			"AD Stick X 2",			SEQ_DEF_3(INDEXED(JOYCODE_X, 1), SEQCODE_OR, INDEXED(MOUSECODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	AD_STICK_X,			"AD Stick X 3",			SEQ_DEF_3(INDEXED(JOYCODE_X, 2), SEQCODE_OR, INDEXED(MOUSECODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	AD_STICK_X,			"AD Stick X 4",			SEQ_DEF_3(INDEXED(JOYCODE_X, 3), SEQCODE_OR, INDEXED(MOUSECODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	AD_STICK_X,			"AD Stick X 5",			SEQ_DEF_3(INDEXED(JOYCODE_X, 4), SEQCODE_OR, INDEXED(MOUSECODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	AD_STICK_X,			"AD Stick X 6",			SEQ_DEF_3(INDEXED(JOYCODE_X, 5), SEQCODE_OR, INDEXED(MOUSECODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	AD_STICK_X,			"AD Stick X 7",			SEQ_DEF_3(INDEXED(JOYCODE_X, 6), SEQCODE_OR, INDEXED(MOUSECODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	AD_STICK_X,			"AD Stick X 8",			SEQ_DEF_3(INDEXED(JOYCODE_X, 7), SEQCODE_OR, INDEXED(MOUSECODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	AD_STICK_Y,			"AD Stick Y",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 0), SEQCODE_OR, INDEXED(MOUSECODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	AD_STICK_Y,			"AD Stick Y 2",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 1), SEQCODE_OR, INDEXED(MOUSECODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	AD_STICK_Y,			"AD Stick Y 3",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 2), SEQCODE_OR, INDEXED(MOUSECODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	AD_STICK_Y,			"AD Stick Y 4",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 3), SEQCODE_OR, INDEXED(MOUSECODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	AD_STICK_Y,			"AD Stick Y 5",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 4), SEQCODE_OR, INDEXED(MOUSECODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	AD_STICK_Y,			"AD Stick Y 6",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 5), SEQCODE_OR, INDEXED(MOUSECODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	AD_STICK_Y,			"AD Stick Y 7",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 6), SEQCODE_OR, INDEXED(MOUSECODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	AD_STICK_Y,			"AD Stick Y 8",			SEQ_DEF_3(INDEXED(JOYCODE_Y, 7), SEQCODE_OR, INDEXED(MOUSECODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	AD_STICK_Z,			"AD Stick Z",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 0)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	AD_STICK_Z,			"AD Stick Z 2",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 1)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	AD_STICK_Z,			"AD Stick Z 3",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 2)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	AD_STICK_Z,			"AD Stick Z 4",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 3)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	AD_STICK_Z,			"AD Stick Z 5",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 4)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	AD_STICK_Z,			"AD Stick Z 6",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 5)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	AD_STICK_Z,			"AD Stick Z 7",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 6)), SEQ_DEF_0, SEQ_DEF_0 )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	AD_STICK_Z,			"AD Stick Z 8",			SEQ_DEF_1(INDEXED(JOYCODE_Z, 7)), SEQ_DEF_0, SEQ_DEF_0 )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	LIGHTGUN_X,			"Lightgun X",			SEQ_DEF_5(INDEXED(GUNCODE_X, 0), SEQCODE_OR, INDEXED(MOUSECODE_X, 0), SEQCODE_OR, INDEXED(JOYCODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	LIGHTGUN_X,			"Lightgun X 2",			SEQ_DEF_5(INDEXED(GUNCODE_X, 1), SEQCODE_OR, INDEXED(MOUSECODE_X, 1), SEQCODE_OR, INDEXED(JOYCODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	LIGHTGUN_X,			"Lightgun X 3",			SEQ_DEF_5(INDEXED(GUNCODE_X, 2), SEQCODE_OR, INDEXED(MOUSECODE_X, 2), SEQCODE_OR, INDEXED(JOYCODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	LIGHTGUN_X,			"Lightgun X 4",			SEQ_DEF_5(INDEXED(GUNCODE_X, 3), SEQCODE_OR, INDEXED(MOUSECODE_X, 3), SEQCODE_OR, INDEXED(JOYCODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	LIGHTGUN_X,			"Lightgun X 5",			SEQ_DEF_5(INDEXED(GUNCODE_X, 4), SEQCODE_OR, INDEXED(MOUSECODE_X, 4), SEQCODE_OR, INDEXED(JOYCODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	LIGHTGUN_X,			"Lightgun X 6",			SEQ_DEF_5(INDEXED(GUNCODE_X, 5), SEQCODE_OR, INDEXED(MOUSECODE_X, 5), SEQCODE_OR, INDEXED(JOYCODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	LIGHTGUN_X,			"Lightgun X 7",			SEQ_DEF_5(INDEXED(GUNCODE_X, 6), SEQCODE_OR, INDEXED(MOUSECODE_X, 6), SEQCODE_OR, INDEXED(JOYCODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	LIGHTGUN_X,			"Lightgun X 8",			SEQ_DEF_5(INDEXED(GUNCODE_X, 7), SEQCODE_OR, INDEXED(MOUSECODE_X, 7), SEQCODE_OR, INDEXED(JOYCODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	LIGHTGUN_Y,			"Lightgun Y",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 0), SEQCODE_OR, INDEXED(MOUSECODE_Y, 0), SEQCODE_OR, INDEXED(JOYCODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	LIGHTGUN_Y,			"Lightgun Y 2",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 1), SEQCODE_OR, INDEXED(MOUSECODE_Y, 1), SEQCODE_OR, INDEXED(JOYCODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	LIGHTGUN_Y,			"Lightgun Y 3",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 2), SEQCODE_OR, INDEXED(MOUSECODE_Y, 2), SEQCODE_OR, INDEXED(JOYCODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	LIGHTGUN_Y,			"Lightgun Y 4",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 3), SEQCODE_OR, INDEXED(MOUSECODE_Y, 3), SEQCODE_OR, INDEXED(JOYCODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	LIGHTGUN_Y,			"Lightgun Y 5",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 4), SEQCODE_OR, INDEXED(MOUSECODE_Y, 4), SEQCODE_OR, INDEXED(JOYCODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	LIGHTGUN_Y,			"Lightgun Y 6",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 5), SEQCODE_OR, INDEXED(MOUSECODE_Y, 5), SEQCODE_OR, INDEXED(JOYCODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	LIGHTGUN_Y,			"Lightgun Y 7",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 6), SEQCODE_OR, INDEXED(MOUSECODE_Y, 6), SEQCODE_OR, INDEXED(JOYCODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	LIGHTGUN_Y,			"Lightgun Y 8",			SEQ_DEF_5(INDEXED(GUNCODE_Y, 7), SEQCODE_OR, INDEXED(MOUSECODE_Y, 7), SEQCODE_OR, INDEXED(JOYCODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	MOUSE_X,			"Mouse X",				SEQ_DEF_1(INDEXED(MOUSECODE_X, 0)), SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)), SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	MOUSE_X,			"Mouse X 2",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 1)), SEQ_DEF_3(KEYCODE_D, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 1)), SEQ_DEF_3(KEYCODE_G, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	MOUSE_X,			"Mouse X 3",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 2)), SEQ_DEF_3(KEYCODE_J, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 2)), SEQ_DEF_3(KEYCODE_L, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	MOUSE_X,			"Mouse X 4",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	MOUSE_X,			"Mouse X 5",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	MOUSE_X,			"Mouse X 6",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	MOUSE_X,			"Mouse X 7",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	MOUSE_X,			"Mouse X 8",			SEQ_DEF_1(INDEXED(MOUSECODE_X, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_LEFT_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_X_RIGHT_SWITCH, 7)) )

	INPUT_PORT_ANALOG_DEF ( 1, IPG_PLAYER1,	MOUSE_Y,			"Mouse Y",				SEQ_DEF_1(INDEXED(MOUSECODE_Y, 0)), SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)), SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_ANALOG_DEF ( 2, IPG_PLAYER2,	MOUSE_Y,			"Mouse Y 2",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 1)), SEQ_DEF_3(KEYCODE_R, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 1)), SEQ_DEF_3(KEYCODE_F, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 1)) )
	INPUT_PORT_ANALOG_DEF ( 3, IPG_PLAYER3,	MOUSE_Y,			"Mouse Y 3",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 2)), SEQ_DEF_3(KEYCODE_I, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 2)), SEQ_DEF_3(KEYCODE_K, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 2)) )
	INPUT_PORT_ANALOG_DEF ( 4, IPG_PLAYER4,	MOUSE_Y,			"Mouse Y 4",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 3)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 3)) )
	INPUT_PORT_ANALOG_DEF ( 5, IPG_PLAYER5,	MOUSE_Y,			"Mouse Y 5",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 4)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 4)) )
	INPUT_PORT_ANALOG_DEF ( 6, IPG_PLAYER6,	MOUSE_Y,			"Mouse Y 6",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 5)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 5)) )
	INPUT_PORT_ANALOG_DEF ( 7, IPG_PLAYER7,	MOUSE_Y,			"Mouse Y 7",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 6)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 6)) )
	INPUT_PORT_ANALOG_DEF ( 8, IPG_PLAYER8,	MOUSE_Y,			"Mouse Y 8",			SEQ_DEF_1(INDEXED(MOUSECODE_Y, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_UP_SWITCH, 7)), SEQ_DEF_1(INDEXED(JOYCODE_Y_DOWN_SWITCH, 7)) )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_OTHER,   KEYBOARD, 			"Keyboard",		   		SEQ_DEF_0 )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_ON_SCREEN_DISPLAY,"On Screen Display",	SEQ_DEF_1(KEYCODE_TILDE) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_DEBUG_BREAK,      "Break in Debugger",	SEQ_DEF_1(KEYCODE_TILDE) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_CONFIGURE,        "Config Menu",			SEQ_DEF_1(KEYCODE_TAB) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_PAUSE,            "Pause",				SEQ_DEF_1(KEYCODE_P) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_RESET_MACHINE,    "Reset Game",			SEQ_DEF_2(KEYCODE_F3, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SOFT_RESET,       "Soft Reset",			SEQ_DEF_3(KEYCODE_F3, SEQCODE_NOT, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SHOW_GFX,         "Show Gfx",			SEQ_DEF_1(KEYCODE_F4) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_FRAMESKIP_DEC,    "Frameskip Dec",		SEQ_DEF_1(KEYCODE_F8) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_FRAMESKIP_INC,    "Frameskip Inc",		SEQ_DEF_1(KEYCODE_F9) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_THROTTLE,         "Throttle",			SEQ_DEF_1(KEYCODE_F10) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_FAST_FORWARD,     "Fast Forward",		SEQ_DEF_1(KEYCODE_INSERT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SHOW_FPS,         "Show FPS",			SEQ_DEF_5(KEYCODE_F11, SEQCODE_NOT, KEYCODE_LCONTROL, SEQCODE_NOT, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SNAPSHOT,         "Save Snapshot",		SEQ_DEF_3(KEYCODE_F12, SEQCODE_NOT, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_RECORD_MOVIE,     "Record Movie",		SEQ_DEF_2(KEYCODE_F12, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_TOGGLE_CHEAT,     "Toggle Cheat",		SEQ_DEF_1(KEYCODE_F6) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_UP,               "UI Up",				SEQ_DEF_3(KEYCODE_UP, SEQCODE_OR, INDEXED(JOYCODE_Y_UP_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_DOWN,             "UI Down",				SEQ_DEF_3(KEYCODE_DOWN, SEQCODE_OR, INDEXED(JOYCODE_Y_DOWN_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_LEFT,             "UI Left",				SEQ_DEF_3(KEYCODE_LEFT, SEQCODE_OR, INDEXED(JOYCODE_X_LEFT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_RIGHT,            "UI Right",			SEQ_DEF_3(KEYCODE_RIGHT, SEQCODE_OR, INDEXED(JOYCODE_X_RIGHT_SWITCH, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_HOME,             "UI Home",				SEQ_DEF_1(KEYCODE_HOME) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_END,              "UI End",				SEQ_DEF_1(KEYCODE_END) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_PAGE_UP,          "UI Page Up",			SEQ_DEF_1(KEYCODE_PGUP) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_PAGE_DOWN,        "UI Page Down",		SEQ_DEF_1(KEYCODE_PGDN) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SELECT,           "UI Select",			SEQ_DEF_3(KEYCODE_ENTER, SEQCODE_OR, INDEXED(JOYCODE_BUTTON1, 0)) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_CANCEL,           "UI Cancel",			SEQ_DEF_1(KEYCODE_ESC) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_CLEAR,            "UI Clear",			SEQ_DEF_1(KEYCODE_DEL) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_ZOOM_IN,          "UI Zoom In",			SEQ_DEF_1(KEYCODE_EQUALS) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_ZOOM_OUT,         "UI Zoom Out",			SEQ_DEF_1(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_PREV_GROUP,       "UI Previous Group",	SEQ_DEF_1(KEYCODE_OPENBRACE) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_NEXT_GROUP,       "UI Next Group",		SEQ_DEF_1(KEYCODE_CLOSEBRACE) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_ROTATE,           "UI Rotate",			SEQ_DEF_1(KEYCODE_R) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SHOW_PROFILER,    "Show Profiler",		SEQ_DEF_2(KEYCODE_F11, KEYCODE_LSHIFT) )
#ifdef MESS
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_TOGGLE_UI,        "UI Toggle",			SEQ_DEF_1(KEYCODE_SCRLOCK) )
#endif
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_TOGGLE_DEBUG,     "Toggle Debugger",		SEQ_DEF_1(KEYCODE_F5) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SAVE_STATE,       "Save State",			SEQ_DEF_2(KEYCODE_F7, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_LOAD_STATE,       "Load State",			SEQ_DEF_3(KEYCODE_F7, SEQCODE_NOT, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_ADD_CHEAT,        "Add Cheat",			SEQ_DEF_1(KEYCODE_A) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_DELETE_CHEAT,     "Delete Cheat",		SEQ_DEF_1(KEYCODE_D) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_SAVE_CHEAT,       "Save Cheat",			SEQ_DEF_1(KEYCODE_S) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_WATCH_VALUE,      "Watch Value",			SEQ_DEF_1(KEYCODE_W) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_EDIT_CHEAT,       "Edit Cheat",			SEQ_DEF_1(KEYCODE_E) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_RELOAD_CHEAT,     "Reload Database",		SEQ_DEF_1(KEYCODE_L) )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      UI_TOGGLE_CROSSHAIR, "Toggle Crosshair",	SEQ_DEF_1(KEYCODE_F1) )

	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_1,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_2,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_3,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_4,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_5,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_6,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_7,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_8,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_9,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_10,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_11,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_12,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_13,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_14,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_15,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_UI,      OSD_16,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, UNKNOWN,			NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, SPECIAL,			NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, OTHER,				NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, DIPSWITCH_NAME,		NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, CONFIG_NAME,		NULL,					SEQ_DEF_0 )
	INPUT_PORT_DIGITAL_DEF( 0, IPG_INVALID, END,				NULL,					SEQ_DEF_0 )
};


static input_port_default_entry default_ports[ARRAY_LENGTH(default_ports_builtin)];
static input_port_default_entry default_ports_backup[ARRAY_LENGTH(default_ports_builtin)];
static const int input_port_count = ARRAY_LENGTH(default_ports_builtin);
static int default_ports_lookup[__ipt_max][MAX_PLAYERS];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void input_port_exit(running_machine *machine);
static void input_port_frame(running_machine *machine);
static void input_port_frame_update(running_machine *machine);
static void input_port_load(int config_type, xml_data_node *parentnode);
static void input_port_save(int config_type, xml_data_node *parentnode);
static void update_digital_joysticks(running_machine *machine);
static void update_analog_port(int port);
static void autoselect_device(const input_port_entry *ipt, int type1, int type2, int type3, const char *option, const char *ananame);

/* playback and record */
static time_t playback_init(running_machine *machine);
static void record_init(running_machine *machine);
static void playback_end(running_machine *machine, const char *message);
static void record_end(running_machine *machine, const char *message);
static void playback_frame(running_machine *machine, attotime curtime);
static void record_frame(running_machine *machine, attotime curtime);
static void playback_port(running_machine *machine, input_port_info *portinfo);
static void record_port(running_machine *machine, input_port_info *portinfo);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  Input port initialize
 *
 *************************************/

time_t input_port_init(running_machine *machine, const input_port_token *ipt)
{
	int ipnum, player;
	time_t basetime;

	/* add an exit callback */
	add_exit_callback(machine, input_port_exit);
	add_frame_callback(machine, input_port_frame);

	/* reset the pointers */
	memset(&joystick_info, 0, sizeof(joystick_info));

	/* start with the raw defaults and ask the OSD to customize them in the backup array */
	memcpy(default_ports_backup, default_ports_builtin, sizeof(default_ports_backup));
	osd_customize_inputport_list(default_ports_backup);

	/* propogate these changes forward to the final input list */
	memcpy(default_ports, default_ports_backup, sizeof(default_ports));

	/* make a lookup table mapping type/player to the default port list entry */
	for (ipnum = 0; ipnum < __ipt_max; ipnum++)
		for (player = 0; player < MAX_PLAYERS; player++)
			default_ports_lookup[ipnum][player] = -1;
	for (ipnum = 0; default_ports[ipnum].type != IPT_END; ipnum++)
		default_ports_lookup[default_ports[ipnum].type][default_ports[ipnum].player] = ipnum;

	/* reset the port info */
	memset(port_info, 0, sizeof(port_info));

	/* if we have inputs, process them now */
	if (ipt != NULL)
	{
		const char *joystick_map_default = options_get_string(mame_options(), OPTION_JOYSTICK_MAP);
		input_port_entry *portentry;
		int portnum;

		/* allocate input ports */
		machine->input_ports = input_port_allocate(ipt, NULL);

		/* allocate default input ports */
		input_ports_default = input_port_allocate(ipt, NULL);

		/* handle autoselection of devices */
		autoselect_device(machine->input_ports, IPT_PADDLE,      IPT_PADDLE_V,     0,              OPTION_PADDLE_DEVICE,     "paddle");
		autoselect_device(machine->input_ports, IPT_AD_STICK_X,  IPT_AD_STICK_Y,   IPT_AD_STICK_Z, OPTION_ADSTICK_DEVICE,    "analog joystick");
		autoselect_device(machine->input_ports, IPT_LIGHTGUN_X,  IPT_LIGHTGUN_Y,   0,              OPTION_LIGHTGUN_DEVICE,   "lightgun");
		autoselect_device(machine->input_ports, IPT_PEDAL,       IPT_PEDAL2,       IPT_PEDAL3,     OPTION_PEDAL_DEVICE,      "pedal");
		autoselect_device(machine->input_ports, IPT_DIAL,        IPT_DIAL_V,       0,              OPTION_DIAL_DEVICE,       "dial");
		autoselect_device(machine->input_ports, IPT_TRACKBALL_X, IPT_TRACKBALL_Y,  0,              OPTION_TRACKBALL_DEVICE,  "trackball");
		autoselect_device(machine->input_ports, IPT_POSITIONAL,  IPT_POSITIONAL_V, 0,              OPTION_POSITIONAL_DEVICE, "positional");
		autoselect_device(machine->input_ports, IPT_MOUSE_X,     IPT_MOUSE_Y,      0,              OPTION_MOUSE_DEVICE,      "mouse");

		/* look for 4-way joysticks and change the default map if we find any */
		if (joystick_map_default[0] == 0 || strcmp(joystick_map_default, "auto") == 0)
			for (portentry = machine->input_ports; portentry->type != IPT_END; portentry++)
				if (IS_DIGITAL_JOYSTICK(portentry) && portentry->way == 4)
				{
					input_device_set_joystick_map(-1, portentry->rotated ? joystick_map_4way_diagonal : joystick_map_4way_sticky);
					break;
				}

		/* identify all the tagged ports up front so the memory system can access them */
		portnum = 0;
		for (portentry = machine->input_ports; portentry->type != IPT_END; portentry++)
			if (portentry->type == IPT_PORT)
				port_info[portnum++].tag = portentry->start.tag;

		/* look up all the tags referenced in conditions */
		for (portentry = machine->input_ports; portentry->type != IPT_END; portentry++)
			if (portentry->condition.tag)
			{
				int tag = port_tag_to_index(portentry->condition.tag);
				if (tag == -1)
					fatalerror("Conditional port references invalid tag '%s'", portentry->condition.tag);
				portentry->condition.portnum = tag;
			}
	}

	/* register callbacks for when we load configurations */
	config_register("input", input_port_load, input_port_save);

	/* open playback and record files if specified */
	basetime = playback_init(machine);
	record_init(machine);

	return basetime;
}



/*************************************
 *
 *  Input port exit
 *
 *************************************/

void input_port_exit(running_machine *machine)
{
	/* close any playback or recording files */
	playback_end(machine, NULL);
	record_end(machine, NULL);
}



/*************************************
 *
 *  Input port initialization
 *
 *************************************/

static void input_port_postload(void)
{
	input_port_entry *portentry;
	int portnum, bitnum;
	UINT32 mask;

	/* loop over the ports and identify all the analog inputs */
	portnum = -1;
	bitnum = 0;
	for (portentry = Machine->input_ports; portentry->type != IPT_END; portentry++)
	{
		/* if this is IPT_PORT, increment the port number */
		if (portentry->type == IPT_PORT)
		{
			portnum++;
			bitnum = 0;
		}

		/* if this is not a DIP setting or config setting, add it to the list */
		else if (portentry->type != IPT_DIPSWITCH_SETTING && portentry->type != IPT_CONFIG_SETTING)
		{
			/* fatal error if we didn't hit an IPT_PORT */
			if (portnum < 0)
				fatalerror("Error in InputPort definition: expecting PORT_START");

			/* fatal error if too many bits */
			if (bitnum >= MAX_BITS_PER_PORT)
				fatalerror("Error in InputPort definition: too many bits for a port (%d max)", MAX_BITS_PER_PORT);

			/* fill in the bit info */
			port_info[portnum].bit[bitnum].portentry = portentry;
			port_info[portnum].bit[bitnum].impulse = 0;
			port_info[portnum].bit[bitnum++].last = 0;

			/* if this is a custom input, add it to the list */
			if (portentry->custom != NULL)
			{
				custom_port_info *info;

				/* allocate memory */
				info = auto_malloc(sizeof(*info));
				memset(info, 0, sizeof(*info));

				/* fill in the data */
				info->portentry = portentry;
				for (mask = portentry->mask; !(mask & 1); mask >>= 1)
					info->shift++;

				/* hook in the list */
				info->next = port_info[portnum].custominfo;
				port_info[portnum].custominfo = info;
			}

			/* if this is a changed input, add it to the list */
			else if (portentry->changed != NULL)
			{
				changed_port_info *info;

				/* allocate memory */
				info = auto_malloc(sizeof(*info));
				memset(info, 0, sizeof(*info));

				/* fill in the data */
				info->portentry = portentry;
				for (mask = portentry->mask; !(mask & 1); mask >>= 1)
					info->shift++;

				/* hook in the list */
				info->next = port_info[portnum].changedinfo;
				port_info[portnum].changedinfo = info;
			}

			/* if this is an analog port, create an info struct for it */
			else if (IS_ANALOG(portentry))
			{
				analog_port_info *info;

				/* allocate memory */
				info = auto_malloc(sizeof(*info));
				memset(info, 0, sizeof(*info));

				/* fill in the data */
				info->portentry = portentry;
				for (mask = portentry->mask; !(mask & 1); mask >>= 1)
					info->shift++;
				for ( ; mask & 1; mask >>= 1)
					info->bits++;

				/* based on the port type determine if we need absolute or relative coordinates */
				info->minimum = INPUT_ABSOLUTE_MIN;
				info->maximum = INPUT_ABSOLUTE_MAX;
				info->interpolate = 1;

				/* adjust default, min, and max so they fall in the bitmask range */
				portentry->default_value = (portentry->default_value & portentry->mask) >> info->shift;
				if (portentry->type != IPT_POSITIONAL && portentry->type != IPT_POSITIONAL_V)
				{
					portentry->analog.min = (portentry->analog.min & portentry->mask) >> info->shift;
					portentry->analog.max = (portentry->analog.max & portentry->mask) >> info->shift;
				}

				switch (portentry->type)
				{
					/* pedals start at and autocenter to the min range*/
					case IPT_PEDAL:
					case IPT_PEDAL2:
					case IPT_PEDAL3:
						info->center = INPUT_ABSOLUTE_MIN;
						/* force pedals to start at their scaled minimum */
						info->accum = APPLY_INVERSE_SENSITIVITY(info->center, portentry->analog.sensitivity);
						/* fall through to complete setup */

					/* pedals, paddles and analog joysticks are absolute and autocenter */
					case IPT_AD_STICK_X:
					case IPT_AD_STICK_Y:
					case IPT_AD_STICK_Z:
					case IPT_PADDLE:
					case IPT_PADDLE_V:
						info->absolute = 1;
						info->autocenter = 1;
						break;

					/* lightguns are absolute as well, but don't autocenter and don't interpolate their values */
					case IPT_LIGHTGUN_X:
					case IPT_LIGHTGUN_Y:
						info->absolute = 1;
						info->interpolate = 0;
						break;

					/* dials, mice and trackballs are relative devices */
					/* these have fixed "min" and "max" values based on how many bits are in the port */
					/* in addition, we set the wrap around min/max values to 512 * the min/max values */
					/* this takes into account the mapping that one mouse unit ~= 512 analog units */
					case IPT_DIAL:
					case IPT_DIAL_V:
					case IPT_MOUSE_X:
					case IPT_MOUSE_Y:
					case IPT_TRACKBALL_X:
					case IPT_TRACKBALL_Y:
						info->absolute = 0;
						info->wraps = 1;
						break;

					/* positional devices are abolute, but can also wrap like relative devices */
					/* set each position to be 512 units */
					case IPT_POSITIONAL:
					case IPT_POSITIONAL_V:
						info->positionalscale = COMPUTE_SCALE(portentry->analog.max, INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);
						/* force to only use PORT_POSITIONS data */
						portentry->analog.min = 0;
						portentry->analog.max--;
						info->autocenter = !portentry->analog.wraps;
						info->wraps = portentry->analog.wraps;
						break;

					default:
						fatalerror("Unknown analog port type -- don't know if it is absolute or not");
						break;
				}

				if (info->absolute)
				{
					/* if we are receiving data from the OSD input as absolute,
                     * and the emulated port type is absolute,
                     * and the default port value is between min/max,
                     * we need to scale differently for the +/- directions.
                     * All other absolute types use a 1:1 scale */
					info->single_scale = (portentry->default_value == portentry->analog.min) || (portentry->default_value == portentry->analog.max);

					if (!info->single_scale)
					{
						/* axis moves in both directions from the default value */

						/* unsigned */
						info->scalepos = COMPUTE_SCALE(portentry->analog.max - portentry->default_value, INPUT_ABSOLUTE_MAX - 0);
						info->scaleneg = COMPUTE_SCALE(portentry->default_value - portentry->analog.min, 0 - INPUT_ABSOLUTE_MIN);

						if (portentry->analog.min > portentry->analog.max)
						{
							/* signed */
							info->scaleneg *= -1;
						}

						/* reverse from center */
						info->reverse_val = 0;
					}
					else
					{
						/* single axis that increases from default */
						info->scalepos = COMPUTE_SCALE(portentry->analog.max - portentry->analog.min, INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);

						/* move from default */
						if (portentry->default_value == portentry->analog.max)
							info->scalepos *= -1;

						/* make the scaling the same for easier coding when we need to scale */
						info->scaleneg = info->scalepos;

						/* reverse from max */
						info->reverse_val = info->maximum;
					}
				}

				/* relative and positional controls all map directly with a 512x scale factor */
				else
				{
					/* The relative code is set up to allow specifing PORT_MINMAX and default values. */
					/* The validity checks are purposely set up to not allow you to use anything other */
					/* a default of 0 and PORT_MINMAX(0,mask).  This is in case the need arises to use */
					/* this feature in the future.  Keeping the code in does not hurt anything. */
					if (portentry->analog.min > portentry->analog.max)
						/* adjust for signed */
						portentry->analog.min *= -1;

					info->minimum = (portentry->analog.min - portentry->default_value) * 512;
					info->maximum = (portentry->analog.max - portentry->default_value) * 512;

					/* make the scaling the same for easier coding when we need to scale */
					info->scaleneg = info->scalepos = COMPUTE_SCALE(1, 512);

					if (portentry->analog.reset)
						/* delta values reverse from center */
						info->reverse_val = 0;
					else
					{
						/* positional controls reverse from their max range */
						info->reverse_val = info->maximum + info->minimum;

						/* relative controls reverse from 1 past their max range */
						if (info->positionalscale == 0)
							info->reverse_val += 512;
					}
				}

				/* compute scale for keypresses */
				info->keyscalepos = RECIP_SCALE(info->scalepos);
				info->keyscaleneg = RECIP_SCALE(info->scaleneg);

				/* hook in the list */
				info->next = port_info[portnum].analoginfo;
				port_info[portnum].analoginfo = info;
			}

			/* if this is a digital joystick port, update info on it */
			else if (IS_DIGITAL_JOYSTICK(portentry))
			{
				digital_joystick_info *info = JOYSTICK_INFO_FOR_PORT(portentry);
				info->portentry[JOYSTICK_DIR_FOR_PORT(portentry)] = portentry;
				info->inuse = 1;
			}
		}
	}

	/* run an initial update */
	input_port_frame_update(Machine);
}



/*************************************
 *
 *  Identifies which port types are
 *  saved/loaded
 *
 *************************************/

static int save_this_port_type(int type)
{
	switch (type)
	{
		case IPT_UNUSED:
		case IPT_END:
		case IPT_PORT:
		case IPT_DIPSWITCH_SETTING:
		case IPT_CONFIG_SETTING:
		case IPT_CATEGORY_SETTING:
		case IPT_VBLANK:
		case IPT_UNKNOWN:
			return 0;
	}
	return 1;
}



/*************************************
 *
 *  Input port configuration read
 *
 *************************************/

#ifdef UNUSED_FUNCTION
INLINE input_code get_default_code(int config_type, int type)
{
	switch (type)
	{
		case IPT_DIPSWITCH_NAME:
		case IPT_CATEGORY_NAME:
			return SEQCODE_END;

		default:
			if (config_type != CONFIG_TYPE_GAME)
				return SEQCODE_END;
			else
				return SEQCODE_DEFAULT;
	}
	return SEQCODE_END;
}
#endif


INLINE int string_to_seq_index(const char *string)
{
	int seqindex;

	for (seqindex = 0; seqindex < ARRAY_LENGTH(seqtypestrings); seqindex++)
		if (!mame_stricmp(string, seqtypestrings[seqindex]))
			return seqindex;

	return -1;
}


static void apply_remaps(int count, const input_code *oldtable, const input_code *newtable)
{
	int remapnum;

	/* loop over the remapping table, operating only if something was specified */
	for (remapnum = 0; remapnum < count; remapnum++)
	{
		input_code oldcode = oldtable[remapnum];
		input_code newcode = newtable[remapnum];
		int portnum;

		/* loop over all default ports, remapping the requested keys */
		for (portnum = 0; default_ports[portnum].type != IPT_END; portnum++)
		{
			input_port_default_entry *defport = &default_ports[portnum];
			int seqnum;

			/* remap anything in the default sequences */
			for (seqnum = 0; seqnum < ARRAY_LENGTH(defport->defaultseq.code); seqnum++)
				if (defport->defaultseq.code[seqnum] == oldcode)
					defport->defaultseq.code[seqnum] = newcode;
			for (seqnum = 0; seqnum < ARRAY_LENGTH(defport->defaultdecseq.code); seqnum++)
				if (defport->defaultdecseq.code[seqnum] == oldcode)
					defport->defaultdecseq.code[seqnum] = newcode;
			for (seqnum = 0; seqnum < ARRAY_LENGTH(defport->defaultincseq.code); seqnum++)
				if (defport->defaultincseq.code[seqnum] == oldcode)
					defport->defaultincseq.code[seqnum] = newcode;
		}
	}
}



static int apply_config_to_default(xml_data_node *portnode, int type, int player, input_seq *newseq)
{
	int portnum;

	/* find a matching port in the list */
	for (portnum = 0; portnum < ARRAY_LENGTH(default_ports_backup); portnum++)
	{
		input_port_default_entry *updateport = &default_ports[portnum];
		if (updateport->type == type && updateport->player == player)
		{
			/* copy the sequence(s) that were specified */
			if (input_seq_get_1(&newseq[0]) != INPUT_CODE_INVALID)
				updateport->defaultseq = newseq[0];
			if (input_seq_get_1(&newseq[1]) != INPUT_CODE_INVALID)
				updateport->defaultdecseq = newseq[1];
			if (input_seq_get_1(&newseq[2]) != INPUT_CODE_INVALID)
				updateport->defaultincseq = newseq[2];
			return 1;
		}
	}
	return 0;
}


static int apply_config_to_current(xml_data_node *portnode, int type, int player, input_seq *newseq)
{
	input_port_entry *updateport;
	int mask, defvalue, index;

	/* read the mask, index, and defvalue attributes */
	index = xml_get_attribute_int(portnode, "index", 0);
	mask = xml_get_attribute_int(portnode, "mask", 0);
	defvalue = xml_get_attribute_int(portnode, "defvalue", 0);

	/* find the indexed port; we scan the array to make sure we don't read past the end */
	for (updateport = input_ports_default; updateport->type != IPT_END; updateport++)
		if (index-- == 0)
			break;

	/* verify that it matches */
	if (updateport->type == type && updateport->player == player &&
		updateport->mask == mask && (updateport->default_value & mask) == (defvalue & mask))
	{
		const char *revstring;

		/* point to the real port */
		updateport = Machine->input_ports + (updateport - input_ports_default);

		/* fill in the data from the attributes */
		if (!port_type_is_analog(updateport->type))
			updateport->default_value = xml_get_attribute_int(portnode, "value", updateport->default_value);
		updateport->analog.delta = xml_get_attribute_int(portnode, "keydelta", updateport->analog.delta);
		updateport->analog.centerdelta = xml_get_attribute_int(portnode, "centerdelta", updateport->analog.centerdelta);
		updateport->analog.sensitivity = xml_get_attribute_int(portnode, "sensitivity", updateport->analog.sensitivity);
		revstring = xml_get_attribute_string(portnode, "reverse", NULL);
		if (revstring)
			updateport->analog.reverse = (strcmp(revstring, "yes") == 0);

		/* copy the sequence(s) that were specified */
		if (input_seq_get_1(&newseq[0]) != INPUT_CODE_INVALID)
			updateport->seq = newseq[0];
		if (input_seq_get_1(&newseq[1]) != INPUT_CODE_INVALID)
			updateport->analog.decseq = newseq[1];
		if (input_seq_get_1(&newseq[2]) != INPUT_CODE_INVALID)
			updateport->analog.incseq = newseq[2];
		return 1;
	}
	return 0;
}


static void input_port_load(int config_type, xml_data_node *parentnode)
{
	xml_data_node *portnode;
	int seqnum;

	/* in the completion phase, we finish the initialization with the final ports */
	if (config_type == CONFIG_TYPE_FINAL)
		input_port_postload();

	/* early exit if no data to parse */
	if (!parentnode)
		return;

	/* iterate over all the remap nodes for controller configs only */
	if (config_type == CONFIG_TYPE_CONTROLLER)
	{
		input_code *oldtable, *newtable;
		xml_data_node *remapnode;
		int count;

		/* count items first so we can allocate */
		count = 0;
		for (remapnode = xml_get_sibling(parentnode->child, "remap"); remapnode; remapnode = xml_get_sibling(remapnode->next, "remap"))
			count++;

		/* if we have some, deal with them */
		if (count > 0)
		{
			/* allocate tables */
			oldtable = malloc_or_die(count * sizeof(*oldtable));
			newtable = malloc_or_die(count * sizeof(*newtable));

			/* build up the remap table */
			count = 0;
			for (remapnode = xml_get_sibling(parentnode->child, "remap"); remapnode; remapnode = xml_get_sibling(remapnode->next, "remap"))
			{
				input_code origcode = input_code_from_token(xml_get_attribute_string(remapnode, "origcode", ""));
				input_code newcode = input_code_from_token(xml_get_attribute_string(remapnode, "newcode", ""));
				if (origcode != INPUT_CODE_INVALID && newcode != INPUT_CODE_INVALID)
				{
					oldtable[count] = origcode;
					newtable[count] = newcode;
					count++;
				}
			}

			/* apply it then free the tables */
			apply_remaps(count, oldtable, newtable);
			free(oldtable);
			free(newtable);
		}
	}

	/* iterate over all the port nodes */
	for (portnode = xml_get_sibling(parentnode->child, "port"); portnode; portnode = xml_get_sibling(portnode->next, "port"))
	{
		input_seq newseq[3], tempseq;
		xml_data_node *seqnode;
		int type, player;

		/* get the basic port info from the attributes */
		type = token_to_port_type(xml_get_attribute_string(portnode, "type", ""), &player);

		/* initialize sequences to invalid defaults */
		for (seqnum = 0; seqnum < 3; seqnum++)
			input_seq_set_1(&newseq[seqnum], INPUT_CODE_INVALID);

		/* loop over new sequences */
		for (seqnode = xml_get_sibling(portnode->child, "newseq"); seqnode; seqnode = xml_get_sibling(seqnode->next, "newseq"))
		{
			/* with a valid type, parse out the new sequence */
			seqnum = string_to_seq_index(xml_get_attribute_string(seqnode, "type", ""));
			if (seqnum != -1)
				if (seqnode->value != NULL)
				{
					if (strcmp(seqnode->value, "NONE") == 0)
						input_seq_set_0(&newseq[seqnum]);
					else if (input_seq_from_tokens(seqnode->value, &tempseq) != 0)
						newseq[seqnum] = tempseq;
				}
		}

		/* if we're loading default ports, apply to the default_ports */
		if (config_type != CONFIG_TYPE_GAME)
			apply_config_to_default(portnode, type, player, newseq);
		else
			apply_config_to_current(portnode, type, player, newseq);
	}

	/* after applying the controller config, push that back into the backup, since that is */
	/* what we will diff against */
	if (config_type == CONFIG_TYPE_CONTROLLER)
		memcpy(default_ports_backup, default_ports, sizeof(default_ports_backup));
}



/*************************************
 *
 *  Input port configuration write
 *
 *************************************/

static void add_sequence(xml_data_node *parentnode, int type, int porttype, const input_seq *seq)
{
	astring *seqstring = astring_alloc();
	xml_data_node *seqnode;

	/* get the string for the sequence */
	if (input_seq_get_1(seq) == SEQCODE_END)
		astring_cpyc(seqstring, "NONE");
	else
		input_seq_to_tokens(seqstring, seq);

	/* add the new node */
	seqnode = xml_add_child(parentnode, "newseq", astring_c(seqstring));
	if (seqnode)
		xml_set_attribute(seqnode, "type", seqtypestrings[type]);
	astring_free(seqstring);
}


static void save_default_inputs(xml_data_node *parentnode)
{
	int portnum;

	/* iterate over ports */
	for (portnum = 0; portnum < ARRAY_LENGTH(default_ports_backup); portnum++)
	{
		input_port_default_entry *defport = &default_ports_backup[portnum];
		input_port_default_entry *curport = &default_ports[portnum];

		/* only save if something has changed and this port is a type we save */
		if (save_this_port_type(defport->type) &&
			(input_seq_cmp(&defport->defaultseq, &curport->defaultseq) != 0 ||
			 input_seq_cmp(&defport->defaultdecseq, &curport->defaultdecseq) != 0 ||
			 input_seq_cmp(&defport->defaultincseq, &curport->defaultincseq) != 0))
		{
			/* add a new port node */
			xml_data_node *portnode = xml_add_child(parentnode, "port", NULL);
			if (portnode)
			{
				/* add the port information and attributes */
				xml_set_attribute(portnode, "type", port_type_to_token(defport->type, defport->player));

				/* add only the sequences that have changed from the defaults */
				if (input_seq_cmp(&defport->defaultseq, &curport->defaultseq) != 0)
					add_sequence(portnode, 0, defport->type, &curport->defaultseq);
				if (input_seq_cmp(&defport->defaultdecseq, &curport->defaultdecseq) != 0)
					add_sequence(portnode, 1, defport->type, &curport->defaultdecseq);
				if (input_seq_cmp(&defport->defaultincseq, &curport->defaultincseq) != 0)
					add_sequence(portnode, 2, defport->type, &curport->defaultincseq);
			}
		}
	}
}


static void save_game_inputs(xml_data_node *parentnode)
{
	int portnum;

	/* iterate over ports */
	for (portnum = 0; input_ports_default[portnum].type != IPT_END; portnum++)
	{
		input_port_entry *defport = &input_ports_default[portnum];
		input_port_entry *curport = &Machine->input_ports[portnum];

		/* only save if something has changed and this port is a type we save */
		if (save_this_port_type(defport->type) &&
			((!port_type_is_analog(defport->type) && (defport->default_value & defport->mask) != (curport->default_value & defport->mask)) ||
			 defport->analog.delta != curport->analog.delta ||
			 defport->analog.centerdelta != curport->analog.centerdelta ||
			 defport->analog.sensitivity != curport->analog.sensitivity ||
			 defport->analog.reverse != curport->analog.reverse ||
			 input_seq_cmp(&defport->seq, &curport->seq) != 0 ||
			 input_seq_cmp(&defport->analog.decseq, &curport->analog.decseq) != 0 ||
			 input_seq_cmp(&defport->analog.incseq, &curport->analog.incseq) != 0))
		{
			/* add a new port node */
			xml_data_node *portnode = xml_add_child(parentnode, "port", NULL);
			if (portnode)
			{
				/* add the port information and attributes */
				xml_set_attribute(portnode, "type", port_type_to_token(defport->type, defport->player));
				xml_set_attribute_int(portnode, "mask", defport->mask);
				xml_set_attribute_int(portnode, "index", portnum);
				xml_set_attribute_int(portnode, "defvalue", defport->default_value & defport->mask);

				/* if the value has changed, add it as well */
				if (!port_type_is_analog(defport->type) && defport->default_value != curport->default_value)
					xml_set_attribute_int(portnode, "value", curport->default_value & defport->mask);

				/* add analog-specific attributes if they have changed */
				if (port_type_is_analog(defport->type))
				{
					if (defport->analog.delta != curport->analog.delta)
						xml_set_attribute_int(portnode, "keydelta", curport->analog.delta);
					if (defport->analog.centerdelta != curport->analog.centerdelta)
						xml_set_attribute_int(portnode, "centerdelta", curport->analog.centerdelta);
					if (defport->analog.sensitivity != curport->analog.sensitivity)
						xml_set_attribute_int(portnode, "sensitivity", curport->analog.sensitivity);
					if (defport->analog.reverse != curport->analog.reverse)
						xml_set_attribute(portnode, "reverse", curport->analog.reverse ? "yes" : "no");
				}

				/* add only the sequences that have changed from the defaults */
				if (input_seq_cmp(&defport->seq, &curport->seq) != 0)
					add_sequence(portnode, 0, defport->type, &curport->seq);
				if (input_seq_cmp(&defport->analog.decseq, &curport->analog.decseq) != 0)
					add_sequence(portnode, 1, defport->type, &curport->analog.decseq);
				if (input_seq_cmp(&defport->analog.incseq, &curport->analog.incseq) != 0)
					add_sequence(portnode, 2, defport->type, &curport->analog.incseq);
			}
		}
	}
}


static void input_port_save(int config_type, xml_data_node *parentnode)
{
	if (parentnode)
	{
		/* default ports save differently */
		if (config_type == CONFIG_TYPE_DEFAULT)
			save_default_inputs(parentnode);
		else
			save_game_inputs(parentnode);
	}
}



/*************************************
 *
 *  Token to default string
 *
 *************************************/

const char *input_port_string_from_token(const input_port_token token)
{
	int index;

	if (token.i == 0)
		return NULL;
	if (token.i >= INPUT_STRING_COUNT)
		return token.stringptr;
	for (index = 0; index < ARRAY_LENGTH(input_port_default_strings); index++)
		if (input_port_default_strings[index].id == token.i)
			return input_port_default_strings[index].string;
	return "(Unknown Default)";
}



/*************************************
 *
 *  Input port detokenizer
 *
 *************************************/

static void input_port_detokenize(input_port_init_params *param, const input_port_token *ipt)
{
	UINT32 entrytype = INPUT_TOKEN_INVALID;
	input_port_entry *portentry = NULL;
	const char *modify_tag = NULL;
	int seq_index[3] = {0};

	/* loop over tokens until we hit the end */
	while (entrytype != INPUT_TOKEN_END)
	{
		UINT32 mask, defval, type, val;
		input_port_token temptoken;
		int hasdiploc, strindex;
#ifdef MESS
		UINT16 category;
#endif /* MESS */

		/* unpack the token from the first entry */
		TOKEN_GET_UINT32_UNPACK1(ipt, entrytype, 8);
		switch (entrytype)
		{
			/* end */
			case INPUT_TOKEN_END:
				break;

			/* including */
			case INPUT_TOKEN_INCLUDE:
				input_port_detokenize(param, TOKEN_GET_PTR(ipt, tokenptr));
				break;

			/* start of a new input port */
			case INPUT_TOKEN_START:
			case INPUT_TOKEN_START_TAG:
				modify_tag = NULL;
				portentry = input_port_initialize(param, IPT_PORT, NULL, 0, 0);
				if (entrytype == INPUT_TOKEN_START_TAG)
					portentry->start.tag = TOKEN_GET_STRING(ipt);
				break;

			/* modify an existing port */
			case INPUT_TOKEN_MODIFY:
				modify_tag = TOKEN_GET_STRING(ipt);
				break;

			/* input bit definition */
			case INPUT_TOKEN_BIT:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, type, 24);
				TOKEN_GET_UINT64_UNPACK2(ipt, mask, 32, defval, 32);
				portentry = input_port_initialize(param, type, modify_tag, mask, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				break;

			case INPUT_TOKEN_SPECIAL_ONOFF:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK3(ipt, entrytype, 8, hasdiploc, 1, strindex, 23);
				TOKEN_GET_UINT64_UNPACK2(ipt, mask, 32, defval, 32);

				portentry = input_port_initialize(param, IPT_DIPSWITCH_NAME, modify_tag, mask, mask & defval);
				temptoken.i = strindex;
				portentry->name = input_port_string_from_token(temptoken);
				if (strindex == INPUT_STRING_Service_Mode)
				{
					portentry->toggle = TRUE;
					portentry->seq.code[0] = KEYCODE_F2;
				}
				if (hasdiploc)
					input_port_parse_diplocation(portentry, TOKEN_GET_STRING(ipt));

				portentry = input_port_initialize(param, IPT_DIPSWITCH_SETTING, modify_tag, 0, mask & defval);
				temptoken.i = INPUT_STRING_Off;
				portentry->name = input_port_string_from_token(temptoken);

				portentry = input_port_initialize(param, IPT_DIPSWITCH_SETTING, modify_tag, 0, mask & ~defval);
				temptoken.i = INPUT_STRING_On;
				portentry->name = input_port_string_from_token(temptoken);
				break;

			/* append a code */
			case INPUT_TOKEN_CODE:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, val, 32);
				if (seq_index[0] > 0)
					portentry->seq.code[seq_index[0]++] = SEQCODE_OR;
				portentry->seq.code[seq_index[0]++] = val;
				break;

			case INPUT_TOKEN_CODE_DEC:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, val, 32);
				if (seq_index[1] > 0)
					portentry->analog.decseq.code[seq_index[1]++] = SEQCODE_OR;
				portentry->analog.decseq.code[seq_index[1]++] = val;
				break;

			case INPUT_TOKEN_CODE_INC:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, val, 32);
				if (seq_index[2] > 0)
					portentry->analog.incseq.code[seq_index[2]++] = SEQCODE_OR;
				portentry->analog.incseq.code[seq_index[2]++] = val;
				break;

			/* joystick flags */
			case INPUT_TOKEN_2WAY:
			case INPUT_TOKEN_4WAY:
			case INPUT_TOKEN_8WAY:
			case INPUT_TOKEN_16WAY:
				portentry->way = 2 << (entrytype - INPUT_TOKEN_2WAY);
				break;

			case INPUT_TOKEN_ROTATED:
				portentry->rotated = TRUE;
				break;

			/* general flags */
			case INPUT_TOKEN_NAME:
				portentry->name = input_port_string_from_token(*ipt++);
				break;

			case INPUT_TOKEN_PLAYER1:
			case INPUT_TOKEN_PLAYER2:
			case INPUT_TOKEN_PLAYER3:
			case INPUT_TOKEN_PLAYER4:
			case INPUT_TOKEN_PLAYER5:
			case INPUT_TOKEN_PLAYER6:
			case INPUT_TOKEN_PLAYER7:
			case INPUT_TOKEN_PLAYER8:
				portentry->player = entrytype - INPUT_TOKEN_PLAYER1;
				break;

			case INPUT_TOKEN_COCKTAIL:
				portentry->cocktail = TRUE;
				portentry->player = 1;
				break;

			case INPUT_TOKEN_TOGGLE:
				portentry->toggle = TRUE;
				break;

			case INPUT_TOKEN_IMPULSE:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->impulse, 24);
				break;

			case INPUT_TOKEN_REVERSE:
				portentry->analog.reverse = TRUE;
				break;

			case INPUT_TOKEN_RESET:
				portentry->analog.reset = TRUE;
				break;

			case INPUT_TOKEN_UNUSED:
				portentry->unused = TRUE;
				break;

			/* analog settings */
			case INPUT_TOKEN_MINMAX:
				TOKEN_GET_UINT64_UNPACK2(ipt, portentry->analog.min, 32, portentry->analog.max, 32);
				break;

			case INPUT_TOKEN_SENSITIVITY:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->analog.sensitivity, 24);
				break;

			case INPUT_TOKEN_KEYDELTA:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->analog.delta, -24);
				portentry->analog.centerdelta = portentry->analog.delta;
				break;

			case INPUT_TOKEN_CENTERDELTA:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->analog.centerdelta, -24);
				break;

			case INPUT_TOKEN_CROSSHAIR:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK3(ipt, entrytype, 8, portentry->analog.crossaxis, 4, portentry->analog.crossaltaxis, -20);
				TOKEN_GET_UINT64_UNPACK2(ipt, portentry->analog.crossscale, -32, portentry->analog.crossoffset, -32);
				portentry->analog.crossaltaxis *= 1.0f / 65536.0f;
				portentry->analog.crossscale *= 1.0f / 65536.0f;
				portentry->analog.crossoffset *= 1.0f / 65536.0f;
				break;

			case INPUT_TOKEN_FULL_TURN_COUNT:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->analog.full_turn_count, 24);
				break;

			case INPUT_TOKEN_POSITIONS:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->analog.max, 24);
				break;

			case INPUT_TOKEN_WRAPS:
				portentry->analog.wraps = TRUE;
				break;

			case INPUT_TOKEN_REMAP_TABLE:
				portentry->analog.remap_table = TOKEN_GET_PTR(ipt, ui32ptr);
				break;

			case INPUT_TOKEN_INVERT:
				portentry->analog.invert = TRUE;
				break;

			/* custom callbacks */
			case INPUT_TOKEN_CUSTOM:
				portentry->custom = TOKEN_GET_PTR(ipt, customptr);
				portentry->custom_param = (void *)TOKEN_GET_PTR(ipt, voidptr);
				break;

			/* changed callbacks */
			case INPUT_TOKEN_CHANGED:
				portentry->changed = TOKEN_GET_PTR(ipt, changedptr);
				portentry->changed_param = (void *)TOKEN_GET_PTR(ipt, voidptr);
				break;

			/* dip switch definition */
			case INPUT_TOKEN_DIPNAME:
				TOKEN_GET_UINT64_UNPACK2(ipt, mask, 32, defval, 32);
				portentry = input_port_initialize(param, IPT_DIPSWITCH_NAME, modify_tag, mask, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				break;

			case INPUT_TOKEN_DIPSETTING:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, defval, 32);
				portentry = input_port_initialize(param, IPT_DIPSWITCH_SETTING, modify_tag, 0, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				break;

			/* physical location */
			case INPUT_TOKEN_DIPLOCATION:
				input_port_parse_diplocation(portentry, TOKEN_GET_STRING(ipt));
				break;

			/* conditionals for dip switch settings */
			case INPUT_TOKEN_CONDITION:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->condition.condition, 24);
				TOKEN_GET_UINT64_UNPACK2(ipt, portentry->condition.mask, 32, portentry->condition.value, 32);
				portentry->condition.tag = TOKEN_GET_STRING(ipt);
				break;

			/* analog adjuster definition */
			case INPUT_TOKEN_ADJUSTER:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, defval, 32);
				portentry = input_port_initialize(param, IPT_ADJUSTER, modify_tag, 0xff, defval | (defval << 8));
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = TOKEN_GET_STRING(ipt);
				break;

			/* configuration definition */
			case INPUT_TOKEN_CONFNAME:
				TOKEN_GET_UINT64_UNPACK2(ipt, mask, 32, defval, 32);
				portentry = input_port_initialize(param, IPT_CONFIG_NAME, modify_tag, mask, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				break;

			case INPUT_TOKEN_CONFSETTING:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK2(ipt, entrytype, 8, defval, 32);
				portentry = input_port_initialize(param, IPT_CONFIG_SETTING, modify_tag, 0, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				break;

#ifdef MESS
			case INPUT_TOKEN_CHAR:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, val, 24);
				{
					int ch;
					for (ch = 0; portentry->keyboard.chars[ch] != 0; ch++)
						;
					portentry->keyboard.chars[ch] = (unicode_char) val;
				}
				break;

			/* category definition */
			case INPUT_TOKEN_CATEGORY:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT32_UNPACK2(ipt, entrytype, 8, portentry->category, 24);
				break;

			case INPUT_TOKEN_CATEGORY_NAME:
				TOKEN_GET_UINT64_UNPACK2(ipt, mask, 32, defval, 32);
				portentry = input_port_initialize(param, IPT_CATEGORY_NAME, modify_tag, mask, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				break;

			case INPUT_TOKEN_CATEGORY_SETTING:
				TOKEN_UNGET_UINT32(ipt);
				TOKEN_GET_UINT64_UNPACK3(ipt, entrytype, 8, defval, 32, category, 16);
				portentry = input_port_initialize(param, IPT_CATEGORY_SETTING, modify_tag, 0, defval);
				seq_index[0] = seq_index[1] = seq_index[2] = 0;
				portentry->name = input_port_string_from_token(*ipt++);
				portentry->category = category;
				break;
#endif /* MESS */

			default:
				fatalerror("Invalid token %d in input ports\n", entrytype);
				break;
		}
	}
}



/*************************************
 *
 *  Input port construction
 *
 *************************************/

input_port_entry *input_port_initialize(input_port_init_params *iip, UINT32 type, const char *tag, UINT32 mask, UINT32 defval)
{
	/* this function is used within an INPUT_PORT callback to set up a single port */
	input_port_entry *portentry;
	input_code code;

	/* are we modifying an existing port? */
	if (tag != NULL)
	{
		int portnum, deleting;

		/* find the matching port */
		for (portnum = 0; portnum < iip->current_port; portnum++)
			if (iip->ports[portnum].type == IPT_PORT && iip->ports[portnum].start.tag != NULL && !strcmp(iip->ports[portnum].start.tag, tag))
				break;
		if (portnum >= iip->current_port)
			fatalerror("Could not find port to modify: '%s'", tag);

		/* nuke any matching masks */
		for (portnum++, deleting = 0; portnum < iip->current_port && iip->ports[portnum].type != IPT_PORT; portnum++)
		{
			deleting = (iip->ports[portnum].mask & mask) || (deleting && iip->ports[portnum].mask == 0);
			if (deleting)
			{
				iip->current_port--;
				memmove(&iip->ports[portnum], &iip->ports[portnum + 1], (iip->current_port - portnum) * sizeof(iip->ports[0]));
				portnum--;
			}
		}

		/* allocate space for a new port at the end of this entry */
		if (iip->current_port >= iip->max_ports)
			fatalerror("Too many input ports");
		if (portnum < iip->current_port)
		{
			memmove(&iip->ports[portnum + 1], &iip->ports[portnum], (iip->current_port - portnum) * sizeof(iip->ports[0]));
			iip->current_port++;
			portentry = &iip->ports[portnum];
		}
		else
			portentry = &iip->ports[iip->current_port++];
	}

	/* otherwise, just allocate a new one from the end */
	else
	{
		if (iip->current_port >= iip->max_ports)
			fatalerror("Too many input ports");
		portentry = &iip->ports[iip->current_port++];
	}

	/* set up defaults */
	memset(portentry, 0, sizeof(*portentry));
	portentry->name = IP_NAME_DEFAULT;
	portentry->type = type;
	portentry->mask = mask;
	portentry->default_value = defval;

	/* default min to 0 and max to the mask value */
	/* they can be overwritten by PORT_MINMAX */
	if (port_type_is_analog(portentry->type) && !portentry->analog.min && !portentry->analog.max)
		portentry->analog.max = portentry->mask;

	/* sets up default port codes */
	switch (portentry->type)
	{
		case IPT_DIPSWITCH_NAME:
		case IPT_DIPSWITCH_SETTING:
		case IPT_CONFIG_NAME:
		case IPT_CONFIG_SETTING:
			code = SEQCODE_END;
			break;

		default:
			code = SEQCODE_DEFAULT;
			break;
	}

	/* set the default codes */
	input_seq_set_1(&portentry->seq, code);
	input_seq_set_1(&portentry->analog.incseq, code);
	input_seq_set_1(&portentry->analog.decseq, code);
	return portentry;
}


input_port_entry *input_port_allocate(const input_port_token *ipt, input_port_entry *memory)
{
	input_port_init_params iip;

	/* set up the port parameter structure */
	iip.max_ports = MAX_INPUT_PORTS * MAX_BITS_PER_PORT;
	iip.current_port = 0;

	/* allocate memory for the input ports */
	if (memory == NULL)
		iip.ports = (input_port_entry *)auto_malloc(iip.max_ports * sizeof(*iip.ports));
	else
		iip.ports = memory;
	memset(iip.ports, 0, iip.max_ports * sizeof(*iip.ports));

	/* construct the ports */
	input_port_detokenize(&iip, ipt);

	/* append final IPT_END */
	input_port_initialize(&iip, IPT_END, NULL, 0, 0);

#ifdef MESS
	/* process MESS specific extensions to the port */
	inputx_handle_mess_extensions(iip.ports);
#endif

	return iip.ports;
}


void input_port_parse_diplocation(input_port_entry *in, const char *location)
{
	char *curname = NULL, tempbuf[100];
	const char *entry;
	int index, val, bits;
	UINT32 temp;

	/* if nothing present, bail */
	if (!location)
		return;
	memset(in->diploc, 0, sizeof(in->diploc));

	/* parse the string */
	for (index = 0, entry = location; *entry && index < ARRAY_LENGTH(in->diploc); index++)
	{
		const char *comma, *colon, *number;

		/* find the end of this entry */
		comma = strchr(entry, ',');
		if (comma == NULL)
			comma = entry + strlen(entry);

		/* extract it to tempbuf */
		strncpy(tempbuf, entry, comma - entry);
		tempbuf[comma - entry] = 0;

		/* first extract the switch name if present */
		number = tempbuf;
		colon = strchr(tempbuf, ':');
		if (colon != NULL)
		{
			curname = auto_malloc(colon - tempbuf + 1);
			strncpy(curname, tempbuf, colon - tempbuf);
			curname[colon - tempbuf] = 0;
			number = colon + 1;
		}

		/* if we don't have a name by now, we're screwed */
		if (curname == NULL)
			fatalerror("Switch location '%s' missing switch name!", location);

		/* if the number is preceded by a '!' it's active high */
		if (*number == '!')
		{
			in->diploc[index].invert = 1;
			number++;
		}
		else
			in->diploc[index].invert = 0;

		/* now scan the switch number */
		if (sscanf(number, "%d", &val) != 1)
			fatalerror("Switch location '%s' has invalid format!", location);

		/* fill the entry and bump the index */
		in->diploc[index].swname = curname;
		in->diploc[index].swnum = val;

		/* advance to the next item */
		entry = comma;
		if (*entry)
			entry++;
	}

	/* then verify the number of bits in the mask matches */
	for (bits = 0, temp = in->mask; temp && bits < 32; bits++)
		temp &= temp - 1;
	if (bits != index)
		fatalerror("Switch location '%s' does not describe enough bits for mask %X\n", location, in->mask);
}



/*************************************
 *
 *  List access
 *
 *************************************/

input_port_default_entry *get_input_port_list(void)
{
	return default_ports;
}


const input_port_default_entry *get_input_port_list_defaults(void)
{
	return default_ports_backup;
}



/*************************************
 *
 *  Input port tokens
 *
 *************************************/

const char *port_type_to_token(int type, int player)
{
	static char tempbuf[32];
	int defindex;

	/* look up the port and return the token */
	defindex = default_ports_lookup[type][player];
	if (defindex != -1)
		return default_ports[defindex].token;

	/* if that fails, carry on */
	sprintf(tempbuf, "TYPE_OTHER(%d,%d)", type, player);
	return tempbuf;
}


int token_to_port_type(const char *string, int *player)
{
	int ipnum;

	/* check for our failsafe case first */
	if (sscanf(string, "TYPE_OTHER(%d,%d)", &ipnum, player) == 2)
		return ipnum;

	/* find the token in the list */
	for (ipnum = 0; ipnum < input_port_count; ipnum++)
		if (default_ports[ipnum].token != NULL && !strcmp(default_ports[ipnum].token, string))
		{
			*player = default_ports[ipnum].player;
			return default_ports[ipnum].type;
		}

	/* if we fail, return IPT_UNKNOWN */
	*player = 0;
	return IPT_UNKNOWN;
}



/*************************************
 *
 *  Input port getters
 *
 *************************************/

int input_port_active(const input_port_entry *portentry)
{
	return (input_port_name(portentry) != NULL && !portentry->unused);
}


int port_type_is_analog(int type)
{
	return (type >= __ipt_analog_start && type <= __ipt_analog_end);
}


int port_type_is_analog_absolute(int type)
{
	return (type >= __ipt_analog_absolute_start && type <= __ipt_analog_absolute_end);
}


int port_type_in_use(int type)
{
	input_port_entry *portentry;
	for (portentry = Machine->input_ports; portentry->type != IPT_END; portentry++)
		if (portentry->type == type)
			return 1;
	return 0;
}


int port_type_to_group(int type, int player)
{
	int defindex = default_ports_lookup[type][player];
	if (defindex != -1)
		return default_ports[defindex].group;
	return IPG_INVALID;
}


int port_tag_to_index(const char *tag)
{
	int portnum;

	/* find the matching tag */
	for (portnum = 0; portnum < MAX_INPUT_PORTS; portnum++)
		if (port_info[portnum].tag != NULL && !strcmp(port_info[portnum].tag, tag))
			return portnum;
	return -1;
}


read8_machine_func port_tag_to_handler8(const char *tag)
{
	int portnum = port_tag_to_index(tag);
	return (portnum == -1) ? SMH_NOP : port_handler8[portnum];
}


read16_machine_func port_tag_to_handler16(const char *tag)
{
	int portnum = port_tag_to_index(tag);
	return (portnum == -1) ? SMH_NOP : port_handler16[portnum];
}


read32_machine_func port_tag_to_handler32(const char *tag)
{
	int portnum = port_tag_to_index(tag);
	return (portnum == -1) ? SMH_NOP : port_handler32[portnum];
}


read64_machine_func port_tag_to_handler64(const char *tag)
{
	return SMH_NOP;
}


const char *input_port_name(const input_port_entry *portentry)
{
	int defindex;

	/* if we have a non-default name, use that */
	if (portentry->name != IP_NAME_DEFAULT)
		return portentry->name;

	/* if the port exists, return the default name */
	defindex = default_ports_lookup[portentry->type][portentry->player];
	if (defindex != -1)
		return default_ports[defindex].name;

	/* should never get here */
	return NULL;
}


const input_seq *input_port_seq(input_port_entry *portentry, input_seq_type seqtype)
{
	static const input_seq ip_none = SEQ_DEF_0;
	input_seq *portseq;

	/* if port is disabled, return no key */
	if (portentry->unused)
		return &ip_none;

	/* handle the various seq types */
	switch (seqtype)
	{
		case SEQ_TYPE_STANDARD:
			portseq = &portentry->seq;
			break;

		case SEQ_TYPE_INCREMENT:
			if (!IS_ANALOG(portentry))
				return &ip_none;
			portseq = &portentry->analog.incseq;
			break;

		case SEQ_TYPE_DECREMENT:
			if (!IS_ANALOG(portentry))
				return &ip_none;
			portseq = &portentry->analog.decseq;
			break;

		default:
			return &ip_none;
	}

	/* does this override the default? if so, return it directly */
	if (input_seq_get_1(portseq) != SEQCODE_DEFAULT)
		return portseq;

	/* otherwise find the default setting */
	return input_port_default_seq(portentry->type, portentry->player, seqtype);
}


const input_seq *input_port_default_seq(int type, int player, input_seq_type seqtype)
{
	static const input_seq ip_none = SEQ_DEF_0;

	/* find the default setting */
	int defindex = default_ports_lookup[type][player];
	if (defindex != -1)
	{
		switch (seqtype)
		{
			case SEQ_TYPE_STANDARD:
				return &default_ports[defindex].defaultseq;
			case SEQ_TYPE_INCREMENT:
				return &default_ports[defindex].defaultincseq;
			case SEQ_TYPE_DECREMENT:
				return &default_ports[defindex].defaultdecseq;
		}
	}
	return &ip_none;
}


int input_port_condition(const input_port_entry *in)
{
	switch (in->condition.condition)
	{
		case PORTCOND_EQUALS:
			return ((input_port_read_indexed(Machine, in->condition.portnum) & in->condition.mask) == in->condition.value);
		case PORTCOND_NOTEQUALS:
			return ((input_port_read_indexed(Machine, in->condition.portnum) & in->condition.mask) != in->condition.value);
	}
	return 1;
}



/*************************************
 *
 *  Key sequence handlers
 *
 *************************************/

int input_port_type_pressed(int type, int player)
{
	int defindex = default_ports_lookup[type][player];
	if (defindex != -1)
		return input_seq_pressed(&default_ports[defindex].defaultseq);

	return 0;
}


int input_ui_pressed(int code)
{
	int pressed;

profiler_mark(PROFILER_INPUT);

	/* get the status of this key (assumed to be only in the defaults) */
	pressed = input_seq_pressed(input_port_default_seq(code, 0, SEQ_TYPE_STANDARD));

	/* if pressed, handle it specially */
	if (pressed)
	{
		/* if this is the first press, leave pressed = 1 */
		if (ui_memory[code] == 0)
			ui_memory[code] = 1;

		/* otherwise, reset pressed = 0 */
		else
			pressed = 0;
	}

	/* if we're not pressed, reset the memory field */
	else
		ui_memory[code] = 0;

profiler_mark(PROFILER_END);

	return pressed;
}


int input_ui_pressed_repeat(int code, int speed)
{
	static osd_ticks_t lastdown;
	static osd_ticks_t keydelay;
	int pressed;

profiler_mark(PROFILER_INPUT);

	/* get the status of this key (assumed to be only in the defaults) */
	pressed = input_seq_pressed(input_port_default_seq(code, 0, SEQ_TYPE_STANDARD));

	/* if so, handle it specially */
	if (pressed)
	{
		/* if this is the first press, set a 3x delay and leave pressed = 1 */
		if (ui_memory[code] == 0)
		{
			ui_memory[code] = 1;
			lastdown = osd_ticks();
			keydelay = 3 * speed * osd_ticks_per_second() / 60;
		}

		/* if this is an autorepeat case, set a 1x delay and leave pressed = 1 */
		else if (osd_ticks() - lastdown >= keydelay)
		{
			lastdown += keydelay;
			keydelay = 1 * speed * osd_ticks_per_second() / 60;
		}

		/* otherwise, reset pressed = 0 */
		else
			pressed = 0;
	}

	/* if we're not pressed, reset the memory field */
	else
		ui_memory[code] = 0;

profiler_mark(PROFILER_END);

	return pressed;
}



/*************************************
 *
 *  Update default ports
 *
 *************************************/

void input_port_update_defaults(void)
{
	int loopnum, portnum;

	/* two passes to catch conditionals properly */
	for (loopnum = 0; loopnum < 2; loopnum++)

		/* loop over all input ports */
		for (portnum = 0; portnum < MAX_INPUT_PORTS; portnum++)
		{
			input_port_info *portinfo = &port_info[portnum];
			input_bit_info *info;
			int bitnum;

			/* only clear on the first pass */
			if (loopnum == 0)
				portinfo->defvalue = 0;

			/* first compute the default value for the entire port */
			for (bitnum = 0, info = &portinfo->bit[0]; bitnum < MAX_BITS_PER_PORT && info->portentry; bitnum++, info++)
				if (input_port_condition(info->portentry))
					portinfo->defvalue = (portinfo->defvalue & ~info->portentry->mask) | (info->portentry->default_value & info->portentry->mask);
		}
}



/*************************************
 *
 *  VBLANK start routine
 *
 *************************************/

static void input_port_frame(running_machine *machine)
{
	/* if we're paused, don't do anything */
	if (mame_is_paused(machine))
		return;

	/* otherwise, use the common code */
	input_port_frame_update(machine);
}


static void input_port_frame_update(running_machine *machine)
{
	int ui_visible = ui_is_menu_active() || ui_is_slider_active();
	attotime curtime = timer_get_time();
	int portnum, bitnum;

profiler_mark(PROFILER_INPUT);

	/* record/playback information about the current frame */
	playback_frame(machine, curtime);
	record_frame(machine, curtime);

	/* track the duration of the previous frame */
	last_delta = attotime_to_attoseconds(attotime_sub(curtime, last_update)) / ATTOSECONDS_PER_SECOND_SQRT;
	last_update = curtime;

	/* update all analog ports if the UI isn't visible */
	if (!ui_visible)
		for (portnum = 0; portnum < MAX_INPUT_PORTS; portnum++)
			update_analog_port(portnum);

	/* update the digital joysticks first */
	update_digital_joysticks(machine);

	/* compute default values for all the ports */
	input_port_update_defaults();

	/* loop over all input ports */
	for (portnum = 0; portnum < MAX_INPUT_PORTS; portnum++)
	{
		input_port_info *portinfo = &port_info[portnum];
		input_bit_info *info;
		changed_port_info *changed;

		/* compute the VBLANK mask */
		portinfo->vblank = 0;
		for (bitnum = 0, info = &portinfo->bit[0]; bitnum < MAX_BITS_PER_PORT && info->portentry; bitnum++, info++)
			if (input_port_condition(info->portentry) && info->portentry->type == IPT_VBLANK)
				portinfo->vblank ^= info->portentry->mask;

		/* now loop back and modify based on the inputs */
		portinfo->digital = 0;
		for (bitnum = 0, info = &portinfo->bit[0]; bitnum < MAX_BITS_PER_PORT && info->portentry; bitnum++, info++)
			if (input_port_condition(info->portentry))
			{
				input_port_entry *portentry = info->portentry;

				/* handle non-analog types, but only when the UI isn't visible */
				if (portentry->type != IPT_VBLANK && !IS_ANALOG(portentry) && !ui_visible)
				{
					/* if the sequence for this port is currently pressed.... */
					if (input_seq_pressed(input_port_seq(portentry, SEQ_TYPE_STANDARD)))
					{
#ifdef MESS
						/* (MESS-specific) check for disabled keyboard */
						if (portentry->type == IPT_KEYBOARD && osd_keyboard_disabled())
							continue;
#endif
						/* skip locked-out coin inputs */
						if (portentry->type >= IPT_COIN1 && portentry->type <= IPT_COIN8 && coinlockedout[portentry->type - IPT_COIN1])
						{
							ui_popup_time(3, "Coinlock disabled %s.", input_port_name(portentry));
							continue;
						}
						if (portentry->type >= IPT_SERVICE1 && portentry->type <= IPT_SERVICE4 && servicecoinlockedout[portentry->type - IPT_SERVICE1])
						{
							ui_popup_time(3, "Coinlock disabled %s.", input_port_name(portentry));
							continue;
						}
						/* if this is a downward press and we're an impulse control, reset the count */
						if (portentry->impulse)
						{
							if (info->last == 0)
								info->impulse = portentry->impulse;
						}

						/* if this is a downward press and we're a toggle control, toggle the value */
						else if (portentry->toggle)
						{
							if (info->last == 0)
							{
								portentry->default_value ^= portentry->mask;
								portinfo->digital ^= portentry->mask;
							}
						}

						/* if this is a digital joystick type, apply either standard or 4-way rules */
						else if (IS_DIGITAL_JOYSTICK(portentry))
						{
							digital_joystick_info *joyinfo = JOYSTICK_INFO_FOR_PORT(portentry);
							UINT8 mask;
							switch( portentry->way )
							{
								case 4:
									mask = joyinfo->current4way;
									break;
								case 16:
									mask = 0xff;
									break;
								default:
									mask = joyinfo->current;
									break;
							}
							if ((mask >> JOYSTICK_DIR_FOR_PORT(portentry)) & 1)
								portinfo->digital ^= portentry->mask;
						}

						/* otherwise, just set it raw */
						else
							portinfo->digital ^= portentry->mask;

						/* track the last value */
						info->last = 1;
					}
					else
						info->last = 0;

					/* handle the impulse countdown */
					if (portentry->impulse && info->impulse > 0)
					{
						info->impulse--;
						info->last = 1;
						portinfo->digital ^= portentry->mask;
					}
				}

				/* note that analog ports are handled instantaneously at port read time */
			}

		/* call changed handlers */
		for (changed = portinfo->changedinfo; changed; changed = changed->next)
			if (input_port_condition(changed->portentry))
			{
				input_port_entry *portentry = changed->portentry;

				UINT32 new_unmasked_value = input_port_read_indexed(machine, portnum);
				UINT32 newval = (new_unmasked_value       & portentry->mask) >> changed->shift;
				UINT32 oldval = (portentry->changed_last_value & portentry->mask) >> changed->shift;

				if (newval != oldval)
					(*portentry->changed)(machine, portentry->changed_param, oldval, newval);

				portentry->changed_last_value = new_unmasked_value;
			}
	}

#ifdef MESS
	/* less MESS to MESSy things */
	inputx_update();
#endif

	/* handle playback/record */
	for (portnum = 0; portnum < MAX_INPUT_PORTS; portnum++)
	{
		playback_port(machine, &port_info[portnum]);
		record_port(machine, &port_info[portnum]);
	}

profiler_mark(PROFILER_END);
}



/*************************************
 *
 *  Digital joystick updating
 *
 *************************************/

static void update_digital_joysticks(running_machine *machine)
{
	int player, joyindex;

	/* loop over all the joysticks */
	for (player = 0; player < MAX_PLAYERS; player++)
		for (joyindex = 0; joyindex < DIGITAL_JOYSTICKS_PER_PLAYER; joyindex++)
		{
			digital_joystick_info *info = &joystick_info[player][joyindex];
			if (info->inuse)
			{
				info->previous = info->current;
				info->current = 0;

				/* read all the associated ports */
				if (info->portentry[JOYDIR_UP] != NULL && input_seq_pressed(input_port_seq(info->portentry[JOYDIR_UP], SEQ_TYPE_STANDARD)))
					info->current |= JOYDIR_UP_BIT;
				if (info->portentry[JOYDIR_DOWN] != NULL && input_seq_pressed(input_port_seq(info->portentry[JOYDIR_DOWN], SEQ_TYPE_STANDARD)))
					info->current |= JOYDIR_DOWN_BIT;
				if (info->portentry[JOYDIR_LEFT] != NULL && input_seq_pressed(input_port_seq(info->portentry[JOYDIR_LEFT], SEQ_TYPE_STANDARD)))
					info->current |= JOYDIR_LEFT_BIT;
				if (info->portentry[JOYDIR_RIGHT] != NULL && input_seq_pressed(input_port_seq(info->portentry[JOYDIR_RIGHT], SEQ_TYPE_STANDARD)))
					info->current |= JOYDIR_RIGHT_BIT;

				/* lock out opposing directions (left + right or up + down) */
				if ((info->current & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) == (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT))
					info->current &= ~(JOYDIR_UP_BIT | JOYDIR_DOWN_BIT);
				if ((info->current & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)) == (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT))
					info->current &= ~(JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT);

				/* only update 4-way case if joystick has moved */
				if (info->current != info->previous)
				{
					info->current4way = info->current;

					/*
                        If joystick is pointing at a diagonal, acknowledge that the player moved
                        the joystick by favoring a direction change.  This minimizes frustration
                        when using a keyboard for input, and maximizes responsiveness.

                        For example, if you are holding "left" then switch to "up" (where both left
                        and up are briefly pressed at the same time), we'll transition immediately
                        to "up."

                        Zero any switches that didn't change from the previous to current state.
                     */
					if ((info->current4way & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) &&
						(info->current4way & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)))
					{
						info->current4way ^= info->current4way & info->previous;
					}

					/*
                        If we are still pointing at a diagonal, we are in an indeterminant state.

                        This could happen if the player moved the joystick from the idle position directly
                        to a diagonal, or from one diagonal directly to an extreme diagonal.

                        The chances of this happening with a keyboard are slim, but we still need to
                        constrain this case.

                        For now, just resolve randomly.
                     */
					if ((info->current4way & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) &&
						(info->current4way & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)))
					{
						if (mame_rand(machine) & 1)
							info->current4way &= ~(JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT);
						else
							info->current4way &= ~(JOYDIR_UP_BIT | JOYDIR_DOWN_BIT);
					}
				}
			}
		}
}



/*************************************
 *
 *  Analog minimum/maximum clamping
 *
 *************************************/

INLINE INT32 apply_analog_min_max(const analog_port_info *info, INT32 value)
{
	const input_port_entry *portentry = info->portentry;
	INT32 adjmax, adjmin, adj1, adjdif;

	/* take the analog minimum and maximum values and apply the inverse of the */
	/* sensitivity so that we can clamp against them before applying sensitivity */
	adjmin = APPLY_INVERSE_SENSITIVITY(info->minimum, portentry->analog.sensitivity);
	adjmax = APPLY_INVERSE_SENSITIVITY(info->maximum, portentry->analog.sensitivity);
	adj1   = APPLY_INVERSE_SENSITIVITY(512, portentry->analog.sensitivity);

	/* for absolute devices, clamp to the bounds absolutely */
	if (!info->wraps)
	{
		if (value > adjmax)
			value = adjmax;
		else if (value < adjmin)
			value = adjmin;
	}

	/* for relative devices, wrap around when we go past the edge */
	else
	{
		adjdif = adjmax - adjmin + adj1;
		if (portentry->analog.reverse)
		{
			while (value <= adjmin - adj1)
				value += adjdif;
			while (value > adjmax)
				value -= adjdif;
		}
		else
		{
			while (value >= adjmax + adj1)
				value -= adjdif;
			while (value < adjmin)
				value += adjdif;
		}
	}

	return value;
}



/*************************************
 *
 *  Analog port updating
 *
 *************************************/

static void update_analog_port(int portnum)
{
	analog_port_info *info;

	/* loop over all analog ports in this port number */
	for (info = port_info[portnum].analoginfo; info != NULL; info = info->next)
	{
		input_port_entry *portentry = info->portentry;
		INT32 rawvalue;
		INT32 delta = 0;
		INT64 keyscale;
		int keypressed = FALSE;
		input_item_class itemclass;

		/* clamp the previous value to the min/max range and remember it */
		info->previous = info->accum = apply_analog_min_max(info, info->accum);

		/* get the new raw analog value and its type */
		rawvalue = input_seq_axis_value(input_port_seq(portentry, SEQ_TYPE_STANDARD), &itemclass);

		/* if we got an absolute input, it overrides everything else */
		if (itemclass == ITEM_CLASS_ABSOLUTE)
		{
			if (info->previousanalog != rawvalue)
			{
				/* only update if analog value changed */
				info->previousanalog = rawvalue;

				/* apply the inverse of the sensitivity to the raw value so that */
				/* it will still cover the full min->max range requested after */
				/* we apply the sensitivity adjustment */
				if (info->absolute || portentry->analog.reset)
				{
					/* if port is absolute, then just return the absolute data supplied */
					info->accum = APPLY_INVERSE_SENSITIVITY(rawvalue, portentry->analog.sensitivity);
				}
				else if (info->positionalscale != 0)
				{
					/* if port is positional, we will take the full analog control and divide it */
					/* into positions, that way as the control is moved full scale, */
					/* it moves through all the positions */
					rawvalue = APPLY_SCALE(rawvalue - INPUT_ABSOLUTE_MIN, info->positionalscale) * 512 + info->minimum;

					/* clamp the high value so it does not roll over */
					rawvalue = MIN(rawvalue, info->maximum);
					info->accum = APPLY_INVERSE_SENSITIVITY(rawvalue, portentry->analog.sensitivity);
				}
				else
					/* if port is relative, we use the value to simulate the speed of relative movement */
					/* sensitivity adjustment is allowed for this mode */
					info->accum += rawvalue;

				info->lastdigital = FALSE;
				/* do not bother with other control types if the analog data is changing */
				continue;
			}
			else
			{
				/* we still have to update fake relative from joystick control */
				if (!info->absolute && info->positionalscale == 0)
					info->accum += rawvalue;
			}
		}

		/* if we got it from a relative device, use that as the starting delta */
		/* also note that the last input was not a digital one */
		if (itemclass == ITEM_CLASS_RELATIVE && rawvalue != 0)
		{
			delta = rawvalue;
			info->lastdigital = 0;
		}

		keyscale = (info->accum >= 0) ? info->keyscalepos : info->keyscaleneg;

		/* if the decrement code sequence is pressed, add the key delta to */
		/* the accumulated delta; also note that the last input was a digital one */
		if (input_seq_pressed(input_port_seq(info->portentry, SEQ_TYPE_DECREMENT)))
		{
			keypressed = TRUE;
			if (portentry->analog.delta)
				delta -= APPLY_SCALE(portentry->analog.delta, keyscale);
			else if (info->lastdigital != 1)
				/* decrement only once when first pressed */
				delta -= APPLY_SCALE(1, keyscale);
			info->lastdigital = 1;
		}

		/* same for the increment code sequence */
		if (input_seq_pressed(input_port_seq(info->portentry, SEQ_TYPE_INCREMENT)))
		{
			keypressed = TRUE;
			if (portentry->analog.delta)
				delta += APPLY_SCALE(portentry->analog.delta, keyscale);
			else if (info->lastdigital != 2)
				/* increment only once when first pressed */
				delta += APPLY_SCALE(1, keyscale);
			info->lastdigital = 2;
		}

		/* if resetting is requested, clear the accumulated position to 0 before */
		/* applying the deltas so that we only return this frame's delta */
		/* note that centering only works for relative controls */
		/* no need to check if absolute here because it is checked by the validity tests */
		if (portentry->analog.reset)
			info->accum = 0;

		/* apply the delta to the accumulated value */
		info->accum += delta;

		/* if our last movement was due to a digital input, and if this control */
		/* type autocenters, and if neither the increment nor the decrement seq */
		/* was pressed, apply autocentering */
		if (info->autocenter)
		{
			INT32 center = APPLY_INVERSE_SENSITIVITY(info->center, portentry->analog.sensitivity);
			if (info->lastdigital && !keypressed)
			{
				/* autocenter from positive values */
				if (info->accum >= center)
				{
					info->accum -= APPLY_SCALE(portentry->analog.centerdelta, info->keyscalepos);
					if (info->accum < center)
					{
						info->accum = center;
						info->lastdigital = 0;
					}
				}

				/* autocenter from negative values */
				else
				{
					info->accum += APPLY_SCALE(portentry->analog.centerdelta, info->keyscaleneg);
					if (info->accum > center)
					{
						info->accum = center;
						info->lastdigital = 0;
					}
				}
			}
		}
		else if (!keypressed)
			info->lastdigital = 0;
	}
}



/*************************************
 *
 *  Apply analogue port settings
 *
 *************************************/

INT32 apply_analogue_settings(INT32 current, analog_port_info *info)
{
	INT32 value;
	input_port_entry *portentry = info->portentry;

	/* apply the min/max and then the sensitivity */
	current = apply_analog_min_max(info, current);
	current = APPLY_SENSITIVITY(current, portentry->analog.sensitivity);

	/* apply reversal if needed */
	if (portentry->analog.reverse)
		current = info->reverse_val - current;
	else if (info->single_scale)
		/* it's a pedal or the default value is equal to min/max */
		/* so we need to adjust the center to the minimum */
		current -= INPUT_ABSOLUTE_MIN;

	/* map differently for positive and negative values */
	if (current >= 0)
		value = APPLY_SCALE(current, info->scalepos);
	else
		value = APPLY_SCALE(current, info->scaleneg);
	value += portentry->default_value;

	return value;
}



/*************************************
 *
 *  Analog port interpolation
 *
 *************************************/

static UINT32 merge_interpolated_analog_data(int portnum, UINT32 digital)
{
	analog_port_info *info;
	UINT32 result = digital;

profiler_mark(PROFILER_INPUT);

	/* loop over all analog ports in this port number */
	for (info = port_info[portnum].analoginfo; info != NULL; info = info->next)
	{
		input_port_entry *portentry = info->portentry;

		if (input_port_condition(portentry))
		{
			INT32 current;
			INT32 value;

			/* interpolate or not */
			if (info->interpolate && !portentry->analog.reset && last_delta != 0)
			{
				attoseconds_t time_since_last = attotime_to_attoseconds(attotime_sub(timer_get_time(), last_update)) / ATTOSECONDS_PER_SECOND_SQRT;
				current = info->previous + ((INT64)(info->accum - info->previous) * time_since_last / last_delta);
			}
			else
				current = info->accum;

			value = apply_analogue_settings(current, info);

			/* remap the value if needed */
			if (portentry->analog.remap_table)
				value = info->portentry->analog.remap_table[value];

			/* invert bits if needed */
			if (portentry->analog.invert)
				value = ~value;

			/* insert into the port */
			result = (result & ~portentry->mask) | ((value << info->shift) & portentry->mask);
		}
	}

profiler_mark(PROFILER_END);
	return result;
}



/*************************************
 *
 *  Input port reading
 *
 *************************************/

UINT32 input_port_read_indexed(running_machine *machine, int portnum)
{
	input_port_info *portinfo = &port_info[portnum];
	custom_port_info *custom;
	UINT32 result;

	/* start with the digital */
	result = portinfo->defvalue ^ portinfo->digital;

	/* merge in analog portions */
	result = merge_interpolated_analog_data(portnum, result);

	/* update custom values */
	for (custom = portinfo->custominfo; custom; custom = custom->next)
		if (input_port_condition(custom->portentry))
		{
			/* replace the bits with bits from the custom routine */
			input_port_entry *portentry = custom->portentry;
			result &= ~portentry->mask;
			result |= ((*portentry->custom)(machine, portentry->custom_param) << custom->shift) & portentry->mask;
		}

	/* handle VBLANK bits */
	if (portinfo->vblank)
	{
		/* reset the VBLANK bits to their default value, regardless of inputs */
		result = (result & ~portinfo->vblank) | (portinfo->defvalue & portinfo->vblank);

		/* toggle VBLANK if we're in a VBLANK state */
		if (video_screen_get_vblank(machine->primary_screen))
			result ^= portinfo->vblank;
	}
	return result;
}


UINT32 input_port_read(running_machine *machine, const char *tag)
{
	int portnum = port_tag_to_index(tag);
	if (portnum != -1)
		return input_port_read_indexed(machine, portnum);

	/* otherwise fail horribly */
	fatalerror("Unable to locate input port '%s'", tag);
	return -1;
}


UINT32 input_port_read_safe(running_machine *machine, const char *tag, UINT32 defvalue)
{
	int portnum = port_tag_to_index(tag);
	if (portnum != -1)
		return input_port_read_indexed(machine, portnum);
	return defvalue;
}



/*************************************
 *
 *  Return position of crosshair axis
 *
 *************************************/

UINT32 get_crosshair_pos(int port_num, UINT8 player, UINT8 axis)
{
	input_port_info *portinfo = &port_info[port_num];
	analog_port_info *info;
	input_port_entry *portentry;
	UINT32 result = 0;

	for (info = portinfo->analoginfo; info; info = info->next)
	{
		portentry = info->portentry;
		if (portentry->player == player && portentry->analog.crossaxis == axis && input_port_condition(portentry))
		{
			result = apply_analogue_settings(info->accum, info) & (portentry->mask >> info->shift);
			break;
		}
	}

	return result;
}


/*-------------------------------------------------
    autoselect_device - autoselect a single
    device based on the input port list passed
    in and the corresponding option
-------------------------------------------------*/

static void autoselect_device(const input_port_entry *ipt, int type1, int type2, int type3, const char *option, const char *ananame)
{
	const char *stemp = options_get_string(mame_options(), option);
	input_device_class autoenable = DEVICE_CLASS_KEYBOARD;
	const char *autostring = "keyboard";

	/* if nothing specified, ignore the option */
	if (stemp[0] == 0)
		return;

	/* extract valid strings */
	if (strcmp(stemp, "mouse") == 0)
	{
		autoenable = DEVICE_CLASS_MOUSE;
		autostring = "mouse";
	}
	else if (strcmp(stemp, "joystick") == 0)
	{
		autoenable = DEVICE_CLASS_JOYSTICK;
		autostring = "joystick";
	}
	else if (strcmp(stemp, "lightgun") == 0)
	{
		autoenable = DEVICE_CLASS_LIGHTGUN;
		autostring = "lightgun";
	}
	else if (strcmp(stemp, "none") == 0)
	{
		/* nothing specified */
		return;
	}
	else if (strcmp(stemp, "keyboard") != 0)
		mame_printf_error("Invalid %s value %s; reverting to keyboard\n", option, stemp);

	/* only scan the list if we haven't already enabled this class of control */
	if (!input_device_class_enabled(autoenable))
		for ( ; ipt->type != IPT_END; ipt++)

			/* if this port type is in use, apply the autoselect criteria */
			if ((type1 != 0 && ipt->type == type1) ||
				(type2 != 0 && ipt->type == type2) ||
				(type3 != 0 && ipt->type == type3))
			{
				mame_printf_verbose("Input: Autoenabling %s due to presence of a %s\n", autostring, ananame);
				input_device_class_enable(autoenable, TRUE);
				break;
			}
}



/***************************************************************************
    PLAYBACK AND RECORD
***************************************************************************/

/*-------------------------------------------------
    playback_read_uint8 - read an 8-bit value
    from the playback file
-------------------------------------------------*/

static UINT8 playback_read_uint8(running_machine *machine)
{
	UINT8 result;

	/* protect against NULL handles if previous reads fail */
	if (machine->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (mame_fread(machine->playback_file, &result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return result;
}


/*-------------------------------------------------
    record_write_uint8 - write an 8-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint8(running_machine *machine, UINT8 data)
{
	UINT8 result = data;

	/* protect against NULL handles if previous reads fail */
	if (machine->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (mame_fwrite(machine->record_file, &result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    playback_read_uint32 - read a 32-bit value
    from the playback file
-------------------------------------------------*/

static UINT32 playback_read_uint32(running_machine *machine)
{
	UINT32 result;

	/* protect against NULL handles if previous reads fail */
	if (machine->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (mame_fread(machine->playback_file, &result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return LITTLE_ENDIANIZE_INT32(result);
}


/*-------------------------------------------------
    record_write_uint32 - write a 32-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint32(running_machine *machine, UINT32 data)
{
	UINT32 result = LITTLE_ENDIANIZE_INT32(data);

	/* protect against NULL handles if previous reads fail */
	if (machine->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (mame_fwrite(machine->record_file, &result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    playback_read_uint64 - read a 64-bit value
    from the playback file
-------------------------------------------------*/

static UINT64 playback_read_uint64(running_machine *machine)
{
	UINT64 result;

	/* protect against NULL handles if previous reads fail */
	if (machine->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (mame_fread(machine->playback_file, &result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return LITTLE_ENDIANIZE_INT64(result);
}


/*-------------------------------------------------
    record_write_uint64 - write a 64-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint64(running_machine *machine, UINT64 data)
{
	UINT64 result = LITTLE_ENDIANIZE_INT64(data);

	/* protect against NULL handles if previous reads fail */
	if (machine->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (mame_fwrite(machine->record_file, &result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    playback_init - initialize INP playback
-------------------------------------------------*/

static time_t playback_init(running_machine *machine)
{
	const char *filename = options_get_string(mame_options(), OPTION_PLAYBACK);
	UINT8 header[INP_HEADER_SIZE];
	file_error filerr;
	time_t basetime;

	/* if no file, nothing to do */
	if (filename[0] == 0)
		return 0;

	/* open the playback file */
	filerr = mame_fopen(SEARCHPATH_INPUTLOG, filename, OPEN_FLAG_READ, &machine->playback_file);
	assert_always(filerr == FILERR_NONE, "Failed to open file for playback");

	/* read the header and verify that it is a modern version; if not, print an error */
	if (mame_fread(machine->playback_file, header, sizeof(header)) != sizeof(header))
		fatalerror("Input file is corrupt or invalid (missing header)");
	if (memcmp(header, "MAMEINP\0", 8) != 0)
		fatalerror("Input file invalid or in an older, unsupported format");
	if (header[0x10] != INP_HEADER_MAJVERSION)
		fatalerror("Input file format version mismatch");

	/* output info to console */
	mame_printf_info("Input file: %s\n", filename);
	mame_printf_info("INP version %d.%d\n", header[0x10], header[0x11]);
	basetime = header[0x08] | (header[0x09] << 8) | (header[0x0a] << 16) | (header[0x0b] << 24) |
				((UINT64)header[0x0c] << 32) | ((UINT64)header[0x0d] << 40) | ((UINT64)header[0x0e] << 48) | ((UINT64)header[0x0f] << 56);
	mame_printf_info("Created %s", ctime(&basetime));
	mame_printf_info("Recorded using %s\n", header + 0x20);

	/* verify the header against the current game */
	if (memcmp(machine->gamedrv->name, header + 0x14, strlen(machine->gamedrv->name) + 1) != 0)
		fatalerror("Input file is for " GAMENOUN " '%s', not for current " GAMENOUN " '%s'\n", header + 0x14, machine->gamedrv->name);

	return basetime;
}


/*-------------------------------------------------
    record_init - initialize INP recording
-------------------------------------------------*/

static void record_init(running_machine *machine)
{
	const char *filename = options_get_string(mame_options(), OPTION_RECORD);
	UINT8 header[INP_HEADER_SIZE];
	mame_system_time systime;
	file_error filerr;

	/* if no file, nothing to do */
	if (filename[0] == 0)
		return;

	/* open the record file  */
	filerr = mame_fopen(SEARCHPATH_INPUTLOG, filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &machine->record_file);
	assert_always(filerr == FILERR_NONE, "Failed to open file for recording");

	/* get the base time */
	mame_get_base_datetime(machine, &systime);

	/* fill in the header */
	memset(header, 0, sizeof(header));
	memcpy(header, "MAMEINP\0", 8);
	header[0x08] = systime.time >> 0;
	header[0x09] = systime.time >> 8;
	header[0x0a] = systime.time >> 16;
	header[0x0b] = systime.time >> 24;
	header[0x0c] = systime.time >> 32;
	header[0x0d] = systime.time >> 40;
	header[0x0e] = systime.time >> 48;
	header[0x0f] = systime.time >> 56;
	header[0x10] = INP_HEADER_MAJVERSION;
	header[0x11] = INP_HEADER_MINVERSION;
	strcpy((char *)header + 0x14, machine->gamedrv->name);
	sprintf((char *)header + 0x20, APPNAME " %s", build_version);

	/* write it */
	mame_fwrite(machine->record_file, header, sizeof(header));
}


/*-------------------------------------------------
    playback_end - end INP playback
-------------------------------------------------*/

static void playback_end(running_machine *machine, const char *message)
{
	/* only applies if we have a live file */
	if (machine->playback_file != NULL)
	{
		/* close the file */
		mame_fclose(machine->playback_file);
		machine->playback_file = NULL;

		/* pop a message */
		if (message != NULL)
			popmessage("Playback Ended\nReason: %s", message);

		/* display speed stats */
		playback_accumulated_speed /= playback_accumulated_frames;
		mame_printf_info("Total playback frames: %d\n", (UINT32)playback_accumulated_frames);
		mame_printf_info("Average recorded speed: %d%%\n", (UINT32)((playback_accumulated_speed * 200 + 1) >> 21));
	}
}


/*-------------------------------------------------
    record_end - end INP recording
-------------------------------------------------*/

static void record_end(running_machine *machine, const char *message)
{
	/* only applies if we have a live file */
	if (machine->record_file != NULL)
	{
		/* close the file */
		mame_fclose(machine->record_file);
		machine->record_file = NULL;

		/* pop a message */
		if (message != NULL)
			popmessage("Recording Ended\nReason: %s", message);
	}
}


/*-------------------------------------------------
    playback_frame - start of frame callback for
    playback
-------------------------------------------------*/

static void playback_frame(running_machine *machine, attotime curtime)
{
	/* if playing back, fetch the information and verify */
	if (machine->playback_file != NULL)
	{
		attotime readtime;

		/* first the absolute time */
		readtime.seconds = playback_read_uint32(machine);
		readtime.attoseconds = playback_read_uint64(machine);
		if (attotime_compare(readtime, curtime) != 0)
			playback_end(machine, "Out of sync");

		/* then the speed */
		playback_accumulated_speed += playback_read_uint32(machine);
		playback_accumulated_frames++;
	}
}


/*-------------------------------------------------
    record_frame - start of frame callback for
    recording
-------------------------------------------------*/

static void record_frame(running_machine *machine, attotime curtime)
{
	/* if recording, record information about the current frame */
	if (machine->record_file != NULL)
	{
		/* first the absolute time */
		record_write_uint32(machine, curtime.seconds);
		record_write_uint64(machine, curtime.attoseconds);

		/* then the current speed */
		record_write_uint32(machine, video_get_speed_percent(machine) * (double)(1 << 20));
	}
}


/*-------------------------------------------------
    playback_port - per-port callback for playback
-------------------------------------------------*/

static void playback_port(running_machine *machine, input_port_info *portinfo)
{
	/* if playing back, fetch information about this port */
	if (machine->playback_file != NULL)
	{
		analog_port_info *analog;

		/* read the digital value */
		portinfo->digital = playback_read_uint32(machine);

		/* loop over analog ports and save their data */
		for (analog = portinfo->analoginfo; analog != NULL; analog = analog->next)
		{
			/* read current and previous values */
			analog->accum = playback_read_uint32(machine);
			analog->previous = playback_read_uint32(machine);

			/* read configuration information */
			analog->portentry->analog.sensitivity = playback_read_uint32(machine);
			analog->portentry->analog.reverse = playback_read_uint8(machine);
		}
	}
}


/*-------------------------------------------------
    record_port - per-port callback for record
-------------------------------------------------*/

static void record_port(running_machine *machine, input_port_info *portinfo)
{
	/* if recording, store information about this port */
	if (machine->record_file != NULL)
	{
		analog_port_info *analog;

		/* store the digital value */
		record_write_uint32(machine, portinfo->digital);

		/* loop over analog ports and save their data */
		for (analog = portinfo->analoginfo; analog != NULL; analog = analog->next)
		{
			/* store current and previous values */
			record_write_uint32(machine, analog->accum);
			record_write_uint32(machine, analog->previous);

			/* store configuration information */
			record_write_uint32(machine, analog->portentry->analog.sensitivity);
			record_write_uint8(machine, analog->portentry->analog.reverse);
		}
	}
}
