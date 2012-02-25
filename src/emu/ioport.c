/***************************************************************************

    ioport.c

    Input/output port handling.

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

#include "emu.h"
#include "emuopts.h"
#include "config.h"
#include "xmlfile.h"
#include "profiler.h"
#include "ui.h"
#include "uiinput.h"
#include "debug/debugcon.h"

#include <ctype.h>
#include <time.h>

/* temporary: set this to 1 to enable the originally defined behavior that
   a field specified via PORT_MODIFY which intersects a previously-defined
   field completely wipes out the previous definition */
#define INPUT_PORT_OVERRIDE_FULLY_NUKES_PREVIOUS	1


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

#define NUM_SIMUL_KEYS	(UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1)
#define LOG_INPUTX		0
#define SPACE_COUNT		3
#define INVALID_CHAR	'?'
#define IP_NAME_DEFAULT	NULL


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live analog field information */
typedef struct _analog_field_state analog_field_state;
struct _analog_field_state
{
	analog_field_state *		next;				/* link to the next analog state for this port */
	const input_field_config *	field;				/* pointer to the input field referenced */

	/* adjusted values (right-justified and tweaked) */
	UINT8						shift;				/* shift to align final value in the port */
	INT32						adjdefvalue;		/* adjusted default value from the config */
	INT32						adjmin;				/* adjusted minimum value from the config */
	INT32						adjmax;				/* adjusted maximum value from the config */

	/* live values of configurable parameters */
	INT32						sensitivity;		/* current live sensitivity (100=normal) */
	UINT8						reverse;			/* current live reverse flag */
	INT32						delta;				/* current live delta to apply each frame a digital inc/dec key is pressed */
	INT32						centerdelta;		/* current live delta to apply each frame no digital inputs are pressed */

	/* live analog value tracking */
	INT32						accum;				/* accumulated value (including relative adjustments) */
	INT32						previous;			/* previous adjusted value */
	INT32						previousanalog;		/* previous analog value */

	/* parameters for modifying live values */
	INT32						minimum;			/* minimum adjusted value */
	INT32						maximum;			/* maximum adjusted value */
	INT32						center;				/* center adjusted value for autocentering */
	INT32						reverse_val;		/* value where we subtract from to reverse directions */

	/* scaling factors */
	INT64						scalepos;			/* scale factor to apply to positive adjusted values */
	INT64						scaleneg;			/* scale factor to apply to negative adjusted values */
	INT64						keyscalepos;		/* scale factor to apply to the key delta field when pos */
	INT64						keyscaleneg;		/* scale factor to apply to the key delta field when neg */
	INT64						positionalscale;	/* scale factor to divide a joystick into positions */

	/* misc flags */
	UINT8						absolute;			/* is this an absolute or relative input? */
	UINT8						wraps;				/* does the control wrap around? */
	UINT8						autocenter;			/* autocenter this input? */
	UINT8						single_scale;		/* scale joystick differently if default is between min/max */
	UINT8						interpolate;		/* should we do linear interpolation for mid-frame reads? */
	UINT8						lastdigital;		/* was the last modification caused by a digital form? */
};


/* shared digital joystick state */
typedef struct _digital_joystick_state digital_joystick_state;
struct _digital_joystick_state
{
	const input_field_config *	field[4];			/* input field for up, down, left, right respectively */
	UINT8						inuse;				/* is this joystick used? */
	UINT8						current;			/* current value */
	UINT8						current4way;		/* current 4-way value */
	UINT8						previous;			/* previous value */
};


/* live device field information */
typedef struct _device_field_info device_field_info;
struct _device_field_info
{
	device_field_info *			next;				/* linked list of info for this port */
	const input_field_config *	field;				/* pointer to the input field referenced */
	device_t *					device;				/* device */
	UINT8						shift;				/* shift to apply to the final result */
	input_port_value			oldval;				/* last value */
};


/* internal live state of an input field */
struct _input_field_state
{
	analog_field_state *		analog;				/* pointer to live analog data if this is an analog field */
	digital_joystick_state *	joystick;			/* pointer to digital joystick information */
	input_seq					seq[SEQ_TYPE_TOTAL];/* currently configured input sequences */
	input_port_value			value;				/* current value of this port */
	UINT8						impulse;			/* counter for impulse controls */
	UINT8						last;				/* were we pressed last time? */
	UINT8						joydir;				/* digital joystick direction index */
	char *						name;				/* overridden name */
};


/* internal live state of an input port */
struct _input_port_state
{
	analog_field_state *		analoglist;			/* pointer to list of analog port info */
	device_field_info *			readdevicelist;		/* pointer to list of input device info */
	device_field_info *			writedevicelist;	/* pointer to list of output device info */
	input_port_value			defvalue;			/* combined default value across the port */
	input_port_value			digital;			/* current value from all digital inputs */
	input_port_value			vblank;				/* value of all IPT_VBLANK bits */
	input_port_value			outputvalue;		/* current value for outputs */
};


typedef struct _inputx_code inputx_code;
struct _inputx_code
{
	unicode_char ch;
	const input_field_config * field[NUM_SIMUL_KEYS];
};

#define KEY_BUFFER_SIZE	4096

typedef struct _key_buffer key_buffer;
struct _key_buffer
{
	int begin_pos;
	int end_pos;
	unsigned int status_keydown : 1;
	int size;
	unicode_char *buffer;
};

typedef struct _char_info char_info;
struct _char_info
{
	unicode_char ch;
	const char *name;
	const char *alternate;	/* alternative string, in UTF-8 */
};


/* private input port state */
struct _input_port_private
{
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
};


/***************************************************************************
    MACROS
***************************************************************************/

#define APPLY_SENSITIVITY(x,s)		(((INT64)(x) * (s)) / 100.0 + 0.5)
#define APPLY_INVERSE_SENSITIVITY(x,s) (((INT64)(x) * 100) / (s))

#define COMPUTE_SCALE(num,den)		(((INT64)(num) << 24) / (den))
#define RECIP_SCALE(s)				(((INT64)1 << 48) / (s))
#define APPLY_SCALE(x,s)			(((INT64)(x) * (s)) >> 24)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* XML attributes for the different types */
static const char *const seqtypestrings[] = { "standard", "increment", "decrement" };


static const char_info charinfo[] =
{
	{ 0x0008,					"Backspace",	NULL },		/* Backspace */
	{ 0x0009,					"Tab",			"    " },	/* Tab */
	{ 0x000c,					"Clear",		NULL },		/* Clear */
	{ 0x000d,					"Enter",		NULL },		/* Enter */
	{ 0x001a,					"Esc",			NULL },		/* Esc */
	{ 0x0020,					"Space",		" " },		/* Space */
	{ 0x0061,					NULL,			"A" },		/* a */
	{ 0x0062,					NULL,			"B" },		/* b */
	{ 0x0063,					NULL,			"C" },		/* c */
	{ 0x0064,					NULL,			"D" },		/* d */
	{ 0x0065,					NULL,			"E" },		/* e */
	{ 0x0066,					NULL,			"F" },		/* f */
	{ 0x0067,					NULL,			"G" },		/* g */
	{ 0x0068,					NULL,			"H" },		/* h */
	{ 0x0069,					NULL,			"I" },		/* i */
	{ 0x006a,					NULL,			"J" },		/* j */
	{ 0x006b,					NULL,			"K" },		/* k */
	{ 0x006c,					NULL,			"L" },		/* l */
	{ 0x006d,					NULL,			"M" },		/* m */
	{ 0x006e,					NULL,			"N" },		/* n */
	{ 0x006f,					NULL,			"O" },		/* o */
	{ 0x0070,					NULL,			"P" },		/* p */
	{ 0x0071,					NULL,			"Q" },		/* q */
	{ 0x0072,					NULL,			"R" },		/* r */
	{ 0x0073,					NULL,			"S" },		/* s */
	{ 0x0074,					NULL,			"T" },		/* t */
	{ 0x0075,					NULL,			"U" },		/* u */
	{ 0x0076,					NULL,			"V" },		/* v */
	{ 0x0077,					NULL,			"W" },		/* w */
	{ 0x0078,					NULL,			"X" },		/* x */
	{ 0x0079,					NULL,			"Y" },		/* y */
	{ 0x007a,					NULL,			"Z" },		/* z */
	{ 0x00a0,					NULL,			" " },		/* non breaking space */
	{ 0x00a1,					NULL,			"!" },		/* inverted exclaimation mark */
	{ 0x00a6,					NULL,			"|" },		/* broken bar */
	{ 0x00a9,					NULL,			"(c)" },	/* copyright sign */
	{ 0x00ab,					NULL,			"<<" },		/* left pointing double angle */
	{ 0x00ae,					NULL,			"(r)" },	/* registered sign */
	{ 0x00bb,					NULL,			">>" },		/* right pointing double angle */
	{ 0x00bc,					NULL,			"1/4" },	/* vulgar fraction one quarter */
	{ 0x00bd,					NULL,			"1/2" },	/* vulgar fraction one half */
	{ 0x00be,					NULL,			"3/4" },	/* vulgar fraction three quarters */
	{ 0x00bf,					NULL,			"?" },		/* inverted question mark */
	{ 0x00c0,					NULL,			"A" },		/* 'A' grave */
	{ 0x00c1,					NULL,			"A" },		/* 'A' acute */
	{ 0x00c2,					NULL,			"A" },		/* 'A' circumflex */
	{ 0x00c3,					NULL,			"A" },		/* 'A' tilde */
	{ 0x00c4,					NULL,			"A" },		/* 'A' diaeresis */
	{ 0x00c5,					NULL,			"A" },		/* 'A' ring above */
	{ 0x00c6,					NULL,			"AE" },		/* 'AE' ligature */
	{ 0x00c7,					NULL,			"C" },		/* 'C' cedilla */
	{ 0x00c8,					NULL,			"E" },		/* 'E' grave */
	{ 0x00c9,					NULL,			"E" },		/* 'E' acute */
	{ 0x00ca,					NULL,			"E" },		/* 'E' circumflex */
	{ 0x00cb,					NULL,			"E" },		/* 'E' diaeresis */
	{ 0x00cc,					NULL,			"I" },		/* 'I' grave */
	{ 0x00cd,					NULL,			"I" },		/* 'I' acute */
	{ 0x00ce,					NULL,			"I" },		/* 'I' circumflex */
	{ 0x00cf,					NULL,			"I" },		/* 'I' diaeresis */
	{ 0x00d0,					NULL,			"D" },		/* 'ETH' */
	{ 0x00d1,					NULL,			"N" },		/* 'N' tilde */
	{ 0x00d2,					NULL,			"O" },		/* 'O' grave */
	{ 0x00d3,					NULL,			"O" },		/* 'O' acute */
	{ 0x00d4,					NULL,			"O" },		/* 'O' circumflex */
	{ 0x00d5,					NULL,			"O" },		/* 'O' tilde */
	{ 0x00d6,					NULL,			"O" },		/* 'O' diaeresis */
	{ 0x00d7,					NULL,			"X" },		/* multiplication sign */
	{ 0x00d8,					NULL,			"O" },		/* 'O' stroke */
	{ 0x00d9,					NULL,			"U" },		/* 'U' grave */
	{ 0x00da,					NULL,			"U" },		/* 'U' acute */
	{ 0x00db,					NULL,			"U" },		/* 'U' circumflex */
	{ 0x00dc,					NULL,			"U" },		/* 'U' diaeresis */
	{ 0x00dd,					NULL,			"Y" },		/* 'Y' acute */
	{ 0x00df,					NULL,			"SS" },		/* sharp S */
	{ 0x00e0,					NULL,			"a" },		/* 'a' grave */
	{ 0x00e1,					NULL,			"a" },		/* 'a' acute */
	{ 0x00e2,					NULL,			"a" },		/* 'a' circumflex */
	{ 0x00e3,					NULL,			"a" },		/* 'a' tilde */
	{ 0x00e4,					NULL,			"a" },		/* 'a' diaeresis */
	{ 0x00e5,					NULL,			"a" },		/* 'a' ring above */
	{ 0x00e6,					NULL,			"ae" },		/* 'ae' ligature */
	{ 0x00e7,					NULL,			"c" },		/* 'c' cedilla */
	{ 0x00e8,					NULL,			"e" },		/* 'e' grave */
	{ 0x00e9,					NULL,			"e" },		/* 'e' acute */
	{ 0x00ea,					NULL,			"e" },		/* 'e' circumflex */
	{ 0x00eb,					NULL,			"e" },		/* 'e' diaeresis */
	{ 0x00ec,					NULL,			"i" },		/* 'i' grave */
	{ 0x00ed,					NULL,			"i" },		/* 'i' acute */
	{ 0x00ee,					NULL,			"i" },		/* 'i' circumflex */
	{ 0x00ef,					NULL,			"i" },		/* 'i' diaeresis */
	{ 0x00f0,					NULL,			"d" },		/* 'eth' */
	{ 0x00f1,					NULL,			"n" },		/* 'n' tilde */
	{ 0x00f2,					NULL,			"o" },		/* 'o' grave */
	{ 0x00f3,					NULL,			"o" },		/* 'o' acute */
	{ 0x00f4,					NULL,			"o" },		/* 'o' circumflex */
	{ 0x00f5,					NULL,			"o" },		/* 'o' tilde */
	{ 0x00f6,					NULL,			"o" },		/* 'o' diaeresis */
	{ 0x00f8,					NULL,			"o" },		/* 'o' stroke */
	{ 0x00f9,					NULL,			"u" },		/* 'u' grave */
	{ 0x00fa,					NULL,			"u" },		/* 'u' acute */
	{ 0x00fb,					NULL,			"u" },		/* 'u' circumflex */
	{ 0x00fc,					NULL,			"u" },		/* 'u' diaeresis */
	{ 0x00fd,					NULL,			"y" },		/* 'y' acute */
	{ 0x00ff,					NULL,			"y" },		/* 'y' diaeresis */
	{ 0x2010,					NULL,			"-" },		/* hyphen */
	{ 0x2011,					NULL,			"-" },		/* non-breaking hyphen */
	{ 0x2012,					NULL,			"-" },		/* figure dash */
	{ 0x2013,					NULL,			"-" },		/* en dash */
	{ 0x2014,					NULL,			"-" },		/* em dash */
	{ 0x2015,					NULL,			"-" },		/* horizontal dash */
	{ 0x2018,					NULL,			"\'" },		/* left single quotation mark */
	{ 0x2019,					NULL,			"\'" },		/* right single quotation mark */
	{ 0x201a,					NULL,			"\'" },		/* single low quotation mark */
	{ 0x201b,					NULL,			"\'" },		/* single high reversed quotation mark */
	{ 0x201c,					NULL,			"\"" },		/* left double quotation mark */
	{ 0x201d,					NULL,			"\"" },		/* right double quotation mark */
	{ 0x201e,					NULL,			"\"" },		/* double low quotation mark */
	{ 0x201f,					NULL,			"\"" },		/* double high reversed quotation mark */
	{ 0x2024,					NULL,			"." },		/* one dot leader */
	{ 0x2025,					NULL,			".." },		/* two dot leader */
	{ 0x2026,					NULL,			"..." },	/* horizontal ellipsis */
	{ 0x2047,					NULL,			"??" },		/* double question mark */
	{ 0x2048,					NULL,			"?!" },		/* question exclamation mark */
	{ 0x2049,					NULL,			"!?" },		/* exclamation question mark */
	{ 0xff01,					NULL,			"!" },		/* fullwidth exclamation point */
	{ 0xff02,					NULL,			"\"" },		/* fullwidth quotation mark */
	{ 0xff03,					NULL,			"#" },		/* fullwidth number sign */
	{ 0xff04,					NULL,			"$" },		/* fullwidth dollar sign */
	{ 0xff05,					NULL,			"%" },		/* fullwidth percent sign */
	{ 0xff06,					NULL,			"&" },		/* fullwidth ampersand */
	{ 0xff07,					NULL,			"\'" },		/* fullwidth apostrophe */
	{ 0xff08,					NULL,			"(" },		/* fullwidth left parenthesis */
	{ 0xff09,					NULL,			")" },		/* fullwidth right parenthesis */
	{ 0xff0a,					NULL,			"*" },		/* fullwidth asterisk */
	{ 0xff0b,					NULL,			"+" },		/* fullwidth plus */
	{ 0xff0c,					NULL,			"," },		/* fullwidth comma */
	{ 0xff0d,					NULL,			"-" },		/* fullwidth minus */
	{ 0xff0e,					NULL,			"." },		/* fullwidth period */
	{ 0xff0f,					NULL,			"/" },		/* fullwidth slash */
	{ 0xff10,					NULL,			"0" },		/* fullwidth zero */
	{ 0xff11,					NULL,			"1" },		/* fullwidth one */
	{ 0xff12,					NULL,			"2" },		/* fullwidth two */
	{ 0xff13,					NULL,			"3" },		/* fullwidth three */
	{ 0xff14,					NULL,			"4" },		/* fullwidth four */
	{ 0xff15,					NULL,			"5" },		/* fullwidth five */
	{ 0xff16,					NULL,			"6" },		/* fullwidth six */
	{ 0xff17,					NULL,			"7" },		/* fullwidth seven */
	{ 0xff18,					NULL,			"8" },		/* fullwidth eight */
	{ 0xff19,					NULL,			"9" },		/* fullwidth nine */
	{ 0xff1a,					NULL,			":" },		/* fullwidth colon */
	{ 0xff1b,					NULL,			";" },		/* fullwidth semicolon */
	{ 0xff1c,					NULL,			"<" },		/* fullwidth less than sign */
	{ 0xff1d,					NULL,			"=" },		/* fullwidth equals sign */
	{ 0xff1e,					NULL,			">" },		/* fullwidth greater than sign */
	{ 0xff1f,					NULL,			"?" },		/* fullwidth question mark */
	{ 0xff20,					NULL,			"@" },		/* fullwidth at sign */
	{ 0xff21,					NULL,			"A" },		/* fullwidth 'A' */
	{ 0xff22,					NULL,			"B" },		/* fullwidth 'B' */
	{ 0xff23,					NULL,			"C" },		/* fullwidth 'C' */
	{ 0xff24,					NULL,			"D" },		/* fullwidth 'D' */
	{ 0xff25,					NULL,			"E" },		/* fullwidth 'E' */
	{ 0xff26,					NULL,			"F" },		/* fullwidth 'F' */
	{ 0xff27,					NULL,			"G" },		/* fullwidth 'G' */
	{ 0xff28,					NULL,			"H" },		/* fullwidth 'H' */
	{ 0xff29,					NULL,			"I" },		/* fullwidth 'I' */
	{ 0xff2a,					NULL,			"J" },		/* fullwidth 'J' */
	{ 0xff2b,					NULL,			"K" },		/* fullwidth 'K' */
	{ 0xff2c,					NULL,			"L" },		/* fullwidth 'L' */
	{ 0xff2d,					NULL,			"M" },		/* fullwidth 'M' */
	{ 0xff2e,					NULL,			"N" },		/* fullwidth 'N' */
	{ 0xff2f,					NULL,			"O" },		/* fullwidth 'O' */
	{ 0xff30,					NULL,			"P" },		/* fullwidth 'P' */
	{ 0xff31,					NULL,			"Q" },		/* fullwidth 'Q' */
	{ 0xff32,					NULL,			"R" },		/* fullwidth 'R' */
	{ 0xff33,					NULL,			"S" },		/* fullwidth 'S' */
	{ 0xff34,					NULL,			"T" },		/* fullwidth 'T' */
	{ 0xff35,					NULL,			"U" },		/* fullwidth 'U' */
	{ 0xff36,					NULL,			"V" },		/* fullwidth 'V' */
	{ 0xff37,					NULL,			"W" },		/* fullwidth 'W' */
	{ 0xff38,					NULL,			"X" },		/* fullwidth 'X' */
	{ 0xff39,					NULL,			"Y" },		/* fullwidth 'Y' */
	{ 0xff3a,					NULL,			"Z" },		/* fullwidth 'Z' */
	{ 0xff3b,					NULL,			"[" },		/* fullwidth left bracket */
	{ 0xff3c,					NULL,			"\\" },		/* fullwidth backslash */
	{ 0xff3d,					NULL,			"]"	},		/* fullwidth right bracket */
	{ 0xff3e,					NULL,			"^" },		/* fullwidth caret */
	{ 0xff3f,					NULL,			"_" },		/* fullwidth underscore */
	{ 0xff40,					NULL,			"`" },		/* fullwidth backquote */
	{ 0xff41,					NULL,			"a" },		/* fullwidth 'a' */
	{ 0xff42,					NULL,			"b" },		/* fullwidth 'b' */
	{ 0xff43,					NULL,			"c" },		/* fullwidth 'c' */
	{ 0xff44,					NULL,			"d" },		/* fullwidth 'd' */
	{ 0xff45,					NULL,			"e" },		/* fullwidth 'e' */
	{ 0xff46,					NULL,			"f" },		/* fullwidth 'f' */
	{ 0xff47,					NULL,			"g" },		/* fullwidth 'g' */
	{ 0xff48,					NULL,			"h" },		/* fullwidth 'h' */
	{ 0xff49,					NULL,			"i" },		/* fullwidth 'i' */
	{ 0xff4a,					NULL,			"j" },		/* fullwidth 'j' */
	{ 0xff4b,					NULL,			"k" },		/* fullwidth 'k' */
	{ 0xff4c,					NULL,			"l" },		/* fullwidth 'l' */
	{ 0xff4d,					NULL,			"m" },		/* fullwidth 'm' */
	{ 0xff4e,					NULL,			"n" },		/* fullwidth 'n' */
	{ 0xff4f,					NULL,			"o" },		/* fullwidth 'o' */
	{ 0xff50,					NULL,			"p" },		/* fullwidth 'p' */
	{ 0xff51,					NULL,			"q" },		/* fullwidth 'q' */
	{ 0xff52,					NULL,			"r" },		/* fullwidth 'r' */
	{ 0xff53,					NULL,			"s" },		/* fullwidth 's' */
	{ 0xff54,					NULL,			"t" },		/* fullwidth 't' */
	{ 0xff55,					NULL,			"u" },		/* fullwidth 'u' */
	{ 0xff56,					NULL,			"v" },		/* fullwidth 'v' */
	{ 0xff57,					NULL,			"w" },		/* fullwidth 'w' */
	{ 0xff58,					NULL,			"x" },		/* fullwidth 'x' */
	{ 0xff59,					NULL,			"y" },		/* fullwidth 'y' */
	{ 0xff5a,					NULL,			"z" },		/* fullwidth 'z' */
	{ 0xff5b,					NULL,			"{" },		/* fullwidth left brace */
	{ 0xff5c,					NULL,			"|" },		/* fullwidth vertical bar */
	{ 0xff5d,					NULL,			"}" },		/* fullwidth right brace */
	{ 0xff5e,					NULL,			"~" },		/* fullwidth tilde */
	{ 0xff5f,					NULL,			"((" },		/* fullwidth double left parenthesis */
	{ 0xff60,					NULL,			"))" },		/* fullwidth double right parenthesis */
	{ 0xffe0,					NULL,			"\xC2\xA2" },		/* fullwidth cent sign */
	{ 0xffe1,					NULL,			"\xC2\xA3" },		/* fullwidth pound sign */
	{ 0xffe4,					NULL,			"\xC2\xA4" },		/* fullwidth broken bar */
	{ 0xffe5,					NULL,			"\xC2\xA5" },		/* fullwidth yen sign */
	{ 0xffe6,					NULL,			"\xE2\x82\xA9" },	/* fullwidth won sign */
	{ 0xffe9,					NULL,			"\xE2\x86\x90" },	/* fullwidth left arrow */
	{ 0xffea,					NULL,			"\xE2\x86\x91" },	/* fullwidth up arrow */
	{ 0xffeb,					NULL,			"\xE2\x86\x92" },	/* fullwidth right arrow */
	{ 0xffec,					NULL,			"\xE2\x86\x93" },	/* fullwidth down arrow */
	{ 0xffed,					NULL,			"\xE2\x96\xAA" },	/* fullwidth solid box */
	{ 0xffee,					NULL,			"\xE2\x97\xA6" },	/* fullwidth open circle */
	{ UCHAR_SHIFT_1,			"Shift",		NULL },		/* Shift key */
	{ UCHAR_SHIFT_2,			"Ctrl",			NULL },		/* Ctrl key */
	{ UCHAR_MAMEKEY(F1),		"F1",			NULL },		/* F1 function key */
	{ UCHAR_MAMEKEY(F2),		"F2",			NULL },		/* F2 function key */
	{ UCHAR_MAMEKEY(F3),		"F3",			NULL },		/* F3 function key */
	{ UCHAR_MAMEKEY(F4),		"F4",			NULL },		/* F4 function key */
	{ UCHAR_MAMEKEY(F5),		"F5",			NULL },		/* F5 function key */
	{ UCHAR_MAMEKEY(F6),		"F6",			NULL },		/* F6 function key */
	{ UCHAR_MAMEKEY(F7),		"F7",			NULL },		/* F7 function key */
	{ UCHAR_MAMEKEY(F8),		"F8",			NULL },		/* F8 function key */
	{ UCHAR_MAMEKEY(F9),		"F9",			NULL },		/* F9 function key */
	{ UCHAR_MAMEKEY(F10),		"F10",			NULL },		/* F10 function key */
	{ UCHAR_MAMEKEY(F11),		"F11",			NULL },		/* F11 function key */
	{ UCHAR_MAMEKEY(F12),		"F12",			NULL },		/* F12 function key */
	{ UCHAR_MAMEKEY(F13),		"F13",			NULL },		/* F13 function key */
	{ UCHAR_MAMEKEY(F14),		"F14",			NULL },		/* F14 function key */
	{ UCHAR_MAMEKEY(F15),		"F15",			NULL },		/* F15 function key */
	{ UCHAR_MAMEKEY(ESC),		"Esc",			"\033" },	/* Esc key */
	{ UCHAR_MAMEKEY(INSERT),	"Insert",		NULL },		/* Insert key */
	{ UCHAR_MAMEKEY(DEL),		"Delete",		"\010" },	/* Delete key */
	{ UCHAR_MAMEKEY(HOME),		"Home",			"\014" },	/* Home key */
	{ UCHAR_MAMEKEY(END),		"End",			NULL },		/* End key */
	{ UCHAR_MAMEKEY(PGUP),		"Page Up",		NULL },		/* Page Up key */
	{ UCHAR_MAMEKEY(PGDN),		"Page Down",	NULL },		/* Page Down key */
	{ UCHAR_MAMEKEY(LEFT),		"Cursor Left",	NULL },		/* Cursor Left */
	{ UCHAR_MAMEKEY(RIGHT),		"Cursor Right",	NULL },		/* Cursor Right */
	{ UCHAR_MAMEKEY(UP),		"Cursor Up",	NULL },		/* Cursor Up */
	{ UCHAR_MAMEKEY(DOWN),		"Cursor Down",	NULL },		/* Cursor Down */
	{ UCHAR_MAMEKEY(0_PAD),		"Keypad 0",		NULL },		/* 0 on the numeric keypad */
	{ UCHAR_MAMEKEY(1_PAD),		"Keypad 1",		NULL },		/* 1 on the numeric keypad */
	{ UCHAR_MAMEKEY(2_PAD),		"Keypad 2",		NULL },		/* 2 on the numeric keypad */
	{ UCHAR_MAMEKEY(3_PAD),		"Keypad 3",		NULL },		/* 3 on the numeric keypad */
	{ UCHAR_MAMEKEY(4_PAD),		"Keypad 4",		NULL },		/* 4 on the numeric keypad */
	{ UCHAR_MAMEKEY(5_PAD),		"Keypad 5",		NULL },		/* 5 on the numeric keypad */
	{ UCHAR_MAMEKEY(6_PAD),		"Keypad 6",		NULL },		/* 6 on the numeric keypad */
	{ UCHAR_MAMEKEY(7_PAD),		"Keypad 7",		NULL },		/* 7 on the numeric keypad */
	{ UCHAR_MAMEKEY(8_PAD),		"Keypad 8",		NULL },		/* 8 on the numeric keypad */
	{ UCHAR_MAMEKEY(9_PAD),		"Keypad 9",		NULL },		/* 9 on the numeric keypad */
	{ UCHAR_MAMEKEY(SLASH_PAD),	"Keypad /",		NULL },		/* / on the numeric keypad */
	{ UCHAR_MAMEKEY(ASTERISK),	"Keypad *",		NULL },		/* * on the numeric keypad */
	{ UCHAR_MAMEKEY(MINUS_PAD),	"Keypad -",		NULL },		/* - on the numeric Keypad */
	{ UCHAR_MAMEKEY(PLUS_PAD),	"Keypad +",		NULL },		/* + on the numeric Keypad */
	{ UCHAR_MAMEKEY(DEL_PAD),	"Keypad .",		NULL },		/* . on the numeric keypad */
	{ UCHAR_MAMEKEY(ENTER_PAD),	"Keypad Enter",	NULL },		/* Enter on the numeric keypad */
	{ UCHAR_MAMEKEY(PRTSCR),	"Print Screen",	NULL },		/* Print Screen key */
	{ UCHAR_MAMEKEY(PAUSE),		"Pause",		NULL },		/* Pause key */
	{ UCHAR_MAMEKEY(LSHIFT),	"Left Shift",	NULL },		/* Left Shift key */
	{ UCHAR_MAMEKEY(RSHIFT),	"Right Shift",	NULL },		/* Right Shift key */
	{ UCHAR_MAMEKEY(LCONTROL),	"Left Ctrl",	NULL },		/* Left Control key */
	{ UCHAR_MAMEKEY(RCONTROL),	"Right Ctrl",	NULL },		/* Right Control key */
	{ UCHAR_MAMEKEY(LALT),		"Left Alt",		NULL },		/* Left Alt key */
	{ UCHAR_MAMEKEY(RALT),		"Right Alt",	NULL },		/* Right Alt key */
	{ UCHAR_MAMEKEY(SCRLOCK),	"Scroll Lock",	NULL },		/* Scroll Lock key */
	{ UCHAR_MAMEKEY(NUMLOCK),	"Num Lock",		NULL },		/* Num Lock key */
	{ UCHAR_MAMEKEY(CAPSLOCK),	"Caps Lock",	NULL },		/* Caps Lock key */
	{ UCHAR_MAMEKEY(LWIN),		"Left Win",		NULL },		/* Left Win key */
	{ UCHAR_MAMEKEY(RWIN),		"Right Win",	NULL },		/* Right Win key */
	{ UCHAR_MAMEKEY(MENU),		"Menu",			NULL },		/* Menu key */
	{ UCHAR_MAMEKEY(CANCEL),	"Break",		NULL }		/* Break/Pause key */
};

static TIMER_CALLBACK(inputx_timerproc);


/*  Debugging commands and handlers. */
static void execute_input(running_machine &machine, int ref, int params, const char *param[]);
static void execute_dumpkbd(running_machine &machine, int ref, int params, const char *param[]);

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
	{ INPUT_STRING_China, "China" },
	{ INPUT_STRING_Hong_Kong, "Hong Kong" },
	{ INPUT_STRING_Korea, "Korea" },
	{ INPUT_STRING_Southeast_Asia, "Southeast Asia" },
	{ INPUT_STRING_Taiwan, "Taiwan" },
	{ INPUT_STRING_World, "World" },
	{ INPUT_STRING_Language, "Language" },
	{ INPUT_STRING_English, "English" },
	{ INPUT_STRING_Japanese, "Japanese" },
	{ INPUT_STRING_Chinese, "Chinese" },
	{ INPUT_STRING_French, "French" },
	{ INPUT_STRING_German, "German" },
	{ INPUT_STRING_Italian, "Italian" },
	{ INPUT_STRING_Korean, "Korean" },
	{ INPUT_STRING_Spanish, "Spanish" },
	{ INPUT_STRING_Very_Easy, "Very Easy" },
	{ INPUT_STRING_Easiest, "Easiest" },
	{ INPUT_STRING_Easier, "Easier" },
	{ INPUT_STRING_Easy, "Easy" },
	{ INPUT_STRING_Medium_Easy, "Medium Easy" },
	{ INPUT_STRING_Normal, "Normal" },
	{ INPUT_STRING_Medium, "Medium" },
	{ INPUT_STRING_Medium_Hard, "Medium Hard" },
	{ INPUT_STRING_Hard, "Hard" },
	{ INPUT_STRING_Harder, "Harder" },
	{ INPUT_STRING_Hardest, "Hardest" },
	{ INPUT_STRING_Very_Hard, "Very Hard" },
	{ INPUT_STRING_Medium_Difficult, "Medium Difficult" },
	{ INPUT_STRING_Difficult, "Difficult" },
	{ INPUT_STRING_Very_Difficult, "Very Difficult" },
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
	{ INPUT_STRING_None, "None" },
};



/***************************************************************************
    BUILT-IN CORE MAPPINGS
***************************************************************************/

#include "inpttype.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core system management */
static void input_port_exit(running_machine &machine);

/* port reading */
static INT32 apply_analog_settings(INT32 current, analog_field_state *analog);

/* initialization helpers */
static void init_port_types(running_machine &machine);
static void init_port_state(running_machine &machine);
static void init_autoselect_devices(running_machine &machine, int type1, int type2, int type3, const char *option, const char *ananame);
static device_field_info *init_field_device_info(const input_field_config *field,const char *device_name);
static analog_field_state *init_field_analog_state(const input_field_config *field);

/* once-per-frame updates */
static void frame_update_callback(running_machine &machine);
static void frame_update(running_machine &machine);
static void frame_update_digital_joysticks(running_machine &machine);
static void frame_update_analog_field(running_machine &machine, analog_field_state *analog);
static int frame_get_digital_field_state(const input_field_config *field, int mouse_down);

/* tokenization helpers */
static int token_to_input_field_type(running_machine &machine, const char *string, int *player);
static const char *input_field_type_to_token(running_machine &machine, int type, int player);
static int token_to_seq_type(const char *string);

/* settings load */
static void load_config_callback(running_machine &machine, int config_type, xml_data_node *parentnode);
static void load_remap_table(running_machine &machine, xml_data_node *parentnode);
static int load_default_config(running_machine &machine, xml_data_node *portnode, int type, int player, const input_seq *newseq);
static int load_game_config(running_machine &machine, xml_data_node *portnode, int type, int player, const input_seq *newseq);

/* settings save */
static void save_config_callback(running_machine &machine, int config_type, xml_data_node *parentnode);
static void save_sequence(running_machine &machine, xml_data_node *parentnode, int type, int porttype, const input_seq &seq);
static int save_this_input_field_type(int type);
static void save_default_inputs(running_machine &machine, xml_data_node *parentnode);
static void save_game_inputs(running_machine &machine, xml_data_node *parentnode);

/* input playback */
static time_t playback_init(running_machine &machine);
static void playback_end(running_machine &machine, const char *message);
static void playback_frame(running_machine &machine, attotime curtime);
static void playback_port(const input_port_config *port);

/* input recording */
static void record_init(running_machine &machine);
static void record_end(running_machine &machine, const char *message);
static void record_frame(running_machine &machine, attotime curtime);
static void record_port(const input_port_config *port);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    apply_analog_min_max - clamp the given input
    value to the appropriate min/max for the
    analog control
-------------------------------------------------*/

INLINE INT32 apply_analog_min_max(const analog_field_state *analog, INT32 value)
{
	/* take the analog minimum and maximum values and apply the inverse of the */
	/* sensitivity so that we can clamp against them before applying sensitivity */
	INT32 adjmin = APPLY_INVERSE_SENSITIVITY(analog->minimum, analog->sensitivity);
	INT32 adjmax = APPLY_INVERSE_SENSITIVITY(analog->maximum, analog->sensitivity);

	/* for absolute devices, clamp to the bounds absolutely */
	if (!analog->wraps)
	{
		if (value > adjmax)
			value = adjmax;
		else if (value < adjmin)
			value = adjmin;
	}

	/* for relative devices, wrap around when we go past the edge */
	else
	{
		INT32 range = adjmax - adjmin;
		/* rolls to other end when 1 position past end. */
		value = (value - adjmin) % range;
		if (value < 0)
			value += range;
		value += adjmin;
	}

	return value;
}


/*-------------------------------------------------
    get_port_tag - return a guaranteed tag for
    a port
-------------------------------------------------*/

INLINE const char *get_port_tag(const input_port_config *port, char *tempbuffer)
{
	return port->tag();
}


/*-------------------------------------------------
    condition_equal - TRUE if two conditions are
    equivalent
-------------------------------------------------*/

INLINE int condition_equal(const input_condition *cond1, const input_condition *cond2)
{
	return (cond1->mask == cond2->mask && cond1->value == cond2->value && cond1->condition == cond2->condition && strcmp(cond1->tag, cond2->tag) == 0);
}



/***************************************************************************
    CORE SYSTEM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    input_port_init - initialize the input port
    system
-------------------------------------------------*/

time_t input_port_init(running_machine &machine)
{
	//input_port_private *portdata;
	time_t basetime;

	/* allocate memory for our data structure */
	machine.input_port_data = auto_alloc_clear(machine, input_port_private);
	//portdata = machine.input_port_data;

	/* add an exit callback and a frame callback */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(input_port_exit), &machine));
	machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(FUNC(frame_update_callback), &machine));

	/* initialize the default port info from the OSD */
	init_port_types(machine);

	/* if we have a token list, proceed */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
	{
		astring errors;
		input_port_list_init(*device, machine.m_portlist, errors);
		if (errors)
			mame_printf_error("Input port errors:\n%s", errors.cstr());
	}

	init_port_state(machine);
	/* register callbacks for when we load configurations */
	config_register(machine, "input", config_saveload_delegate(FUNC(load_config_callback), &machine), config_saveload_delegate(FUNC(save_config_callback), &machine));

	/* open playback and record files if specified */
	basetime = playback_init(machine);
	record_init(machine);

	return basetime;
}


/*-------------------------------------------------
    input_port_exit - exit callback to ensure
    we clean up and close our files
-------------------------------------------------*/

static void input_port_exit(running_machine &machine)
{
	/* close any playback or recording files */
	playback_end(machine, NULL);
	record_end(machine, NULL);
}



/***************************************************************************
    PORT CONFIGURATIONS
***************************************************************************/

/*-------------------------------------------------
    input_port_list_init - initialize an input
    port list structure and allocate ports
    according to the given tokens
-------------------------------------------------*/

void input_port_list_init(device_t &device, ioport_list &portlist, astring &errorbuf)
{
	/* no constructor, no list */
	ioport_constructor constructor = device.input_ports();
	if (constructor == NULL)
		return;

	/* reset error buffer */
	errorbuf.reset();

	/* detokenize into the list */
	(*constructor)(device, portlist, errorbuf);

	// collapse fields and sort the list
	for (input_port_config *port = portlist.first(); port != NULL; port = port->next())
		port->collapse_fields(errorbuf);
}


/*-------------------------------------------------
    input_field_by_tag_and_mask - return a pointer
    to the first field that intersects the given
    mask on the tagged port
-------------------------------------------------*/

const input_field_config *input_field_by_tag_and_mask(running_machine &machine, const char *tag, input_port_value mask)
{
	const input_port_config *port = machine.port(tag);

	/* if we got the port, look for the field */
	if (port != NULL)
		for (const input_field_config *field = port->first_field(); field != NULL; field = field->next())
			if ((field->mask & mask) != 0)
				return field;

	return NULL;
}



/***************************************************************************
    ACCESSORS FOR INPUT FIELDS
***************************************************************************/

/*-------------------------------------------------
    input_field_name - return the field name for
    a given input field
-------------------------------------------------*/

const char *input_field_name(const input_field_config *field)
{
	/* if we have a non-default name, use that */
	if ((field->state != NULL) && (field->state->name != NULL))
		return field->state->name;
	if (field->name != NULL)
		return field->name;

	/* otherwise, return the name associated with the type */
	return input_type_name(field->machine(), field->type, field->player);
}


/*-------------------------------------------------
    input_field_seq - return the input sequence
    for the given input field
-------------------------------------------------*/

const input_seq &input_field_seq(const input_field_config *field, input_seq_type seqtype)
{
	/* if the field is disabled, return no key */
	if (field->flags & FIELD_FLAG_UNUSED)
		return input_seq::empty_seq;

	/* select either the live or config state depending on whether we have live state */
	const input_seq &portseq = (field->state == NULL) ? field->seq[seqtype] : field->state->seq[seqtype];

	/* if the portseq is the special default code, return the expanded default value */
	if (portseq.is_default())
		return input_type_seq(field->machine(), field->type, field->player, seqtype);

	/* otherwise, return the sequence as-is */
	return portseq;
}


/*-------------------------------------------------
    input_field_get_user_settings - return the current
    settings for the given input field
-------------------------------------------------*/

void input_field_get_user_settings(const input_field_config *field, input_field_user_settings *settings)
{
	int seqtype;

	/* zap the entire structure */
	memset(settings, 0, sizeof(*settings));

	/* copy the basics */
	for (seqtype = 0; seqtype < ARRAY_LENGTH(settings->seq); seqtype++)
		settings->seq[seqtype] = field->state->seq[seqtype];

	/* if there's a list of settings or we're an adjuster, copy the current value */
	if (field->settinglist().count() != 0 || field->type == IPT_ADJUSTER)
		settings->value = field->state->value;

	/* if there's analog data, extract the analog settings */
	if (field->state->analog != NULL)
	{
		settings->sensitivity = field->state->analog->sensitivity;
		settings->delta = field->state->analog->delta;
		settings->centerdelta = field->state->analog->centerdelta;
		settings->reverse = field->state->analog->reverse;
	}
}


/*-------------------------------------------------
    input_field_set_user_settings - modify the current
    settings for the given input field
-------------------------------------------------*/

void input_field_set_user_settings(const input_field_config *field, const input_field_user_settings *settings)
{
	int seqtype;

	/* copy the basics */
	for (seqtype = 0; seqtype < ARRAY_LENGTH(settings->seq); seqtype++)
	{
		const input_seq &defseq = input_type_seq(field->machine(), field->type, field->player, (input_seq_type)seqtype);
		if (defseq == settings->seq[seqtype])
			field->state->seq[seqtype].set_default();
		else
			field->state->seq[seqtype] = settings->seq[seqtype];
	}

	/* if there's a list of settings or we're an adjuster, copy the current value */
	if (field->settinglist().count() != 0 || field->type == IPT_ADJUSTER)
		field->state->value = settings->value;

	/* if there's analog data, extract the analog settings */
	if (field->state->analog != NULL)
	{
		field->state->analog->sensitivity = settings->sensitivity;
		field->state->analog->delta = settings->delta;
		field->state->analog->centerdelta = settings->centerdelta;
		field->state->analog->reverse = settings->reverse;
	}
}


/*-------------------------------------------------
    input_field_setting_name - return the expanded
    setting name for a field
-------------------------------------------------*/

const char *input_field_setting_name(const input_field_config *field)
{
	const input_setting_config *setting;

	/* only makes sense if we have settings */
	assert(field->settinglist().count() != 0);

	/* scan the list of settings looking for a match on the current value */
	for (setting = field->settinglist().first(); setting != NULL; setting = setting->next())
		if (input_condition_true(field->machine(), &setting->condition, field->port().owner()))
			if (setting->value == field->state->value)
				return setting->name;

	return "INVALID";
}


/*-------------------------------------------------
    input_field_has_previous_setting - return TRUE
    if the given field has a "previous" setting
-------------------------------------------------*/

int input_field_has_previous_setting(const input_field_config *field)
{
	const input_setting_config *setting;

	/* only makes sense if we have settings */
	assert(field->settinglist().count() != 0);

	/* scan the list of settings looking for a match on the current value */
	for (setting = field->settinglist().first(); setting != NULL; setting = setting->next())
		if (input_condition_true(field->machine(), &setting->condition, field->port().owner()))
			return (setting->value != field->state->value);

	return FALSE;
}


/*-------------------------------------------------
    input_field_select_previous_setting - select
    the previous item for a DIP switch or
    configuration field
-------------------------------------------------*/

void input_field_select_previous_setting(const input_field_config *field)
{
	const input_setting_config *setting, *prevsetting;
	int found_match = FALSE;

	/* only makes sense if we have settings */
	assert(field->settinglist().count() != 0);

	/* scan the list of settings looking for a match on the current value */
	prevsetting = NULL;
	for (setting = field->settinglist().first(); setting != NULL; setting = setting->next())
		if (input_condition_true(field->machine(), &setting->condition, field->port().owner()))
		{
			if (setting->value == field->state->value)
			{
				found_match = TRUE;
				if (prevsetting != NULL)
					break;
			}
			prevsetting = setting;
		}

	/* if we didn't find a matching value, select the first */
	if (!found_match)
	{
		for (prevsetting = field->settinglist().first(); prevsetting != NULL; prevsetting = prevsetting->next())
			if (input_condition_true(field->machine(), &prevsetting->condition, field->port().owner()))
				break;
	}

	/* update the value to the previous one */
	if (prevsetting != NULL)
		field->state->value = prevsetting->value;
}


/*-------------------------------------------------
    input_field_has_next_setting - return TRUE
    if the given field has a "next" setting
-------------------------------------------------*/

int input_field_has_next_setting(const input_field_config *field)
{
	const input_setting_config *setting;
	int found = FALSE;

	/* only makes sense if we have settings */
	assert(field->settinglist().count() != 0);

	/* scan the list of settings looking for a match on the current value */
	for (setting = field->settinglist().first(); setting != NULL; setting = setting->next())
		if (input_condition_true(field->machine(), &setting->condition, field->port().owner()))
		{
			if (found)
				return TRUE;
			if (setting->value == field->state->value)
				found = TRUE;
		}

	return FALSE;
}


/*-------------------------------------------------
    input_field_select_next_setting - select the
    next item for a DIP switch or
    configuration field
-------------------------------------------------*/

void input_field_select_next_setting(const input_field_config *field)
{
	const input_setting_config *setting, *nextsetting;

	/* only makes sense if we have settings */
	assert(field->settinglist().count() != 0);

	/* scan the list of settings looking for a match on the current value */
	nextsetting = NULL;
	for (setting = field->settinglist().first(); setting != NULL; setting = setting->next())
		if (input_condition_true(field->machine(), &setting->condition, field->port().owner()))
			if (setting->value == field->state->value)
				break;

	/* if we found one, scan forward for the next valid one */
	if (setting != NULL)
		for (nextsetting = setting->next(); nextsetting != NULL; nextsetting = nextsetting->next())
			if (input_condition_true(field->machine(), &nextsetting->condition, field->port().owner()))
				break;

	/* if we hit the end, search from the beginning */
	if (nextsetting == NULL)
		for (nextsetting = field->settinglist().first(); nextsetting != NULL; nextsetting = nextsetting->next())
			if (input_condition_true(field->machine(), &nextsetting->condition, field->port().owner()))
				break;

	/* update the value to the previous one */
	if (nextsetting != NULL)
		field->state->value = nextsetting->value;
}



/***************************************************************************
    ACCESSORS FOR INPUT TYPES
***************************************************************************/

/*-------------------------------------------------
    input_type_is_analog - return TRUE if
    the given type represents an analog control
-------------------------------------------------*/

int input_type_is_analog(int type)
{
	return (type >= __ipt_analog_start && type <= __ipt_analog_end);
}


/*-------------------------------------------------
    input_type_name - return the name
    for the given type/player
-------------------------------------------------*/

const char *input_type_name(running_machine &machine, int type, int player)
{
	/* if we have a machine, use the live state and quick lookup */
	input_port_private *portdata = machine.input_port_data;
	input_type_entry *entry = portdata->type_to_entry[type][player];
	if (entry != NULL)
		return entry->name;

	/* if we find nothing, return an invalid group */
	return "???";
}


/*-------------------------------------------------
    input_type_group - return the group
    for the given type/player
-------------------------------------------------*/

int input_type_group(running_machine &machine, int type, int player)
{
	input_port_private *portdata = machine.input_port_data;
	input_type_entry *entry = portdata->type_to_entry[type][player];
	if (entry != NULL)
		return entry->group;

	/* if we find nothing, return an invalid group */
	return IPG_INVALID;
}


/*-------------------------------------------------
    input_type_seq - return the input
    sequence for the given type/player
-------------------------------------------------*/

const input_seq &input_type_seq(running_machine &machine, int type, int player, input_seq_type seqtype)
{
	assert(type >= 0 && type < __ipt_max);
	assert(player >= 0 && player < MAX_PLAYERS);

	/* if we have a machine, use the live state and quick lookup */
	input_port_private *portdata = machine.input_port_data;
	input_type_entry *entry = portdata->type_to_entry[type][player];
	if (entry != NULL)
		return entry->seq[seqtype];

	/* if we find nothing, return an empty sequence */
	return input_seq::empty_seq;
}


/*-------------------------------------------------
    input_type_set_seq - change the input
    sequence for the given type/player
-------------------------------------------------*/

void input_type_set_seq(running_machine &machine, int type, int player, input_seq_type seqtype, const input_seq *newseq)
{
	input_port_private *portdata = machine.input_port_data;
	input_type_entry *entry = portdata->type_to_entry[type][player];
	if (entry != NULL)
		entry->seq[seqtype] = *newseq;
}


/*-------------------------------------------------
    input_type_pressed - return TRUE if
    the sequence for the given input type/player
    is pressed
-------------------------------------------------*/

int input_type_pressed(running_machine &machine, int type, int player)
{
	return machine.input().seq_pressed(input_type_seq(machine, type, player, SEQ_TYPE_STANDARD));
}


/*-------------------------------------------------
    input_type_list - return the list of types
-------------------------------------------------*/

const simple_list<input_type_entry> &input_type_list(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	return portdata->typelist;
}



/***************************************************************************
    PORT CHECKING
***************************************************************************/


/*-------------------------------------------------
    input_port_exists - return whether an input
    port exists
-------------------------------------------------*/

bool input_port_exists(running_machine &machine, const char *tag)
{
	return machine.port(tag) != 0;
}


/*-------------------------------------------------
    input_port_active - return a bitmask of which
    bits of an input port are active (i.e. not
    unused or unknown)
-------------------------------------------------*/

input_port_value input_port_active(running_machine &machine, const char *tag)
{
	const input_port_config *port = machine.port(tag);
	if (port == NULL)
		fatalerror("Unable to locate input port '%s'", tag);
	return port->active;
}


/*-------------------------------------------------
    input_port_active_safe - return a bitmask of
    which bits of an input port are active (i.e.
    not unused or unknown), or a default value if
    the port does not exist
-------------------------------------------------*/

input_port_value input_port_active_safe(running_machine &machine, const char *tag, input_port_value defvalue)
{
	const input_port_config *port = machine.port(tag);
	return port == NULL ? defvalue : port->active;
}



/***************************************************************************
    PORT READING
***************************************************************************/

/*-------------------------------------------------
    input_port_read_direct - return the value of
    an input port
-------------------------------------------------*/

input_port_value input_port_read_direct(const input_port_config *port)
{
	assert(port != NULL);

	input_port_private *portdata = port->machine().input_port_data;
	analog_field_state *analog;
	device_field_info *device_field;
	input_port_value result;

	assert_always(portdata->safe_to_read, "Input ports cannot be read at init time!");

	/* start with the digital */
	result = port->state->digital;

	/* update read values */
	for (device_field = port->state->readdevicelist; device_field != NULL; device_field = device_field->next)
		if (input_condition_true(port->machine(), &device_field->field->condition, port->owner()))
		{
			/* replace the bits with bits from the device */
			input_port_value newval = device_field->field->read(*device_field->field, device_field->field->read_param);
			device_field->oldval = newval;
			result = (result & ~device_field->field->mask) | ((newval << device_field->shift) & device_field->field->mask);
		}

	/* update VBLANK bits */
	if (port->state->vblank != 0)
	{
		if (port->machine().primary_screen->vblank())
			result |= port->state->vblank;
		else
			result &= ~port->state->vblank;
	}

	/* apply active high/low state to digital, read, and VBLANK inputs */
	result ^= port->state->defvalue;

	/* merge in analog portions */
	for (analog = port->state->analoglist; analog != NULL; analog = analog->next)
		if (input_condition_true(port->machine(), &analog->field->condition, port->owner()))
		{
			/* start with the raw value */
			INT32 value = analog->accum;

			/* interpolate if appropriate and if time has passed since the last update */
			if (analog->interpolate && !(analog->field->flags & ANALOG_FLAG_RESET) && portdata->last_delta_nsec != 0)
			{
				attoseconds_t nsec_since_last = (port->machine().time() - portdata->last_frame_time).as_attoseconds() / ATTOSECONDS_PER_NANOSECOND;
				value = analog->previous + ((INT64)(analog->accum - analog->previous) * nsec_since_last / portdata->last_delta_nsec);
			}

			/* apply standard analog settings */
			value = apply_analog_settings(value, analog);

			/* remap the value if needed */
			if (analog->field->remap_table != NULL)
				value = analog->field->remap_table[value];

			/* invert bits if needed */
			if (analog->field->flags & ANALOG_FLAG_INVERT)
				value = ~value;

			/* insert into the port */
			result = (result & ~analog->field->mask) | ((value << analog->shift) & analog->field->mask);
		}

	return result;
}


/*-------------------------------------------------
    input_port_read - return the value of
    an input port specified by tag
-------------------------------------------------*/

input_port_value input_port_read(running_machine &machine, const char *tag)
{
	const input_port_config *port = machine.port(tag);
	if (port == NULL)
		fatalerror("Unable to locate input port '%s'", tag);
	return input_port_read_direct(port);
}


/*-------------------------------------------------
    input_port_read - return the value of
    a device input port specified by tag
-------------------------------------------------*/

input_port_value input_port_read(device_t &device, const char *tag)
{
	astring fullpath;
	const input_port_config *port = device.machine().port(device.subtag(fullpath, tag));
	if (port == NULL)
		fatalerror("Unable to locate input port '%s'", tag);
	return input_port_read_direct(port);
}


/*-------------------------------------------------
    input_port_read_safe - return the value of
    an input port specified by tag, or a default
    value if the port does not exist
-------------------------------------------------*/

input_port_value input_port_read_safe(running_machine &machine, const char *tag, UINT32 defvalue)
{
	const input_port_config *port = machine.port(tag);
	return (port == NULL) ? defvalue : input_port_read_direct(port);
}


/*-------------------------------------------------
    input_port_read_crosshair - return the
    extracted crosshair values for the given
    player
-------------------------------------------------*/

int input_port_get_crosshair_position(running_machine &machine, int player, float *x, float *y)
{
	const input_port_config *port;
	const input_field_config *field;
	int gotx = FALSE, goty = FALSE;

	/* read all the lightgun values */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (field->player == player && field->crossaxis != CROSSHAIR_AXIS_NONE)
				if (input_condition_true(machine, &field->condition, port->owner()))
				{
					analog_field_state *analog = field->state->analog;
					INT32 rawvalue = apply_analog_settings(analog->accum, analog) & (analog->field->mask >> analog->shift);
					float value = (float)(rawvalue - field->state->analog->adjmin) / (float)(field->state->analog->adjmax - field->state->analog->adjmin);

					/* apply the scale and offset */
					if (field->crossscale < 0)
						value = -(1.0 - value) * field->crossscale;
					else
						value *= field->crossscale;
					value += field->crossoffset;

					/* apply custom mapping if necessary */
					if (!field->crossmapper.isnull())
						value = field->crossmapper(*field, value);

					/* handle X axis */
					if (field->crossaxis == CROSSHAIR_AXIS_X)
					{
						*x = value;
						gotx = TRUE;
						if (field->crossaltaxis != 0)
						{
							*y = field->crossaltaxis;
							goty = TRUE;
						}
					}

					/* handle Y axis */
					else
					{
						*y = value;
						goty = TRUE;
						if (field->crossaltaxis != 0)
						{
							*x = field->crossaltaxis;
							gotx = TRUE;
						}
					}

					/* if we got both, stop */
					if (gotx && goty)
						break;
				}

	return (gotx && goty);
}


/*-------------------------------------------------
    input_port_update_defaults - force an update
    to the input port values based on current
    conditions
-------------------------------------------------*/

void input_port_update_defaults(running_machine &machine)
{
	int loopnum;

	/* two passes to catch conditionals properly */
	for (loopnum = 0; loopnum < 2; loopnum++)
	{
		const input_port_config *port;

		/* loop over all input ports */
		for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		{
			const input_field_config *field;

			/* only clear on the first pass */
			if (loopnum == 0)
				port->state->defvalue = 0;

			/* first compute the default value for the entire port */
			for (field = port->first_field(); field != NULL; field = field->next())
				if (input_condition_true(machine, &field->condition, port->owner()))
					port->state->defvalue = (port->state->defvalue & ~field->mask) | (field->state->value & field->mask);
		}
	}
}


/*-------------------------------------------------
    apply_analog_settings - return the value of
    an input port
-------------------------------------------------*/

static INT32 apply_analog_settings(INT32 value, analog_field_state *analog)
{
	/* apply the min/max and then the sensitivity */
	value = apply_analog_min_max(analog, value);
	value = APPLY_SENSITIVITY(value, analog->sensitivity);

	/* apply reversal if needed */
	if (analog->reverse)
		value = analog->reverse_val - value;
	else if (analog->single_scale)
		/* it's a pedal or the default value is equal to min/max */
		/* so we need to adjust the center to the minimum */
		value -= INPUT_ABSOLUTE_MIN;

	/* map differently for positive and negative values */
	if (value >= 0)
		value = APPLY_SCALE(value, analog->scalepos);
	else
		value = APPLY_SCALE(value, analog->scaleneg);
	value += analog->adjdefvalue;

	return value;
}


/***************************************************************************
    PORT WRITING
***************************************************************************/

/*-------------------------------------------------
    input_port_write_direct - write a value
    to a port
-------------------------------------------------*/

void input_port_write_direct(const input_port_config *port, input_port_value data, input_port_value mem_mask)
{
	/* call device line write handlers */
	device_field_info *device_field;

	COMBINE_DATA(&port->state->outputvalue);

	for (device_field = port->state->writedevicelist; device_field; device_field = device_field->next)
		if (device_field->field->type == IPT_OUTPUT && input_condition_true(port->machine(), &device_field->field->condition, port->owner()))
		{
			input_port_value newval = ( (port->state->outputvalue ^ device_field->field->defvalue ) & device_field->field->mask) >> device_field->shift;

			/* if the bits have write, call the handler */
			if (device_field->oldval != newval)
			{
				device_field->field->write(*device_field->field, device_field->field->write_param, device_field->oldval, newval);

				device_field->oldval = newval;
			}
		}
}


/*-------------------------------------------------
    input_port_write - write a value to a
    port specified by tag
-------------------------------------------------*/

void input_port_write(running_machine &machine, const char *tag, input_port_value value, input_port_value mask)
{
	const input_port_config *port = machine.port(tag);
	if (port == NULL)
		fatalerror("Unable to locate input port '%s'", tag);
	input_port_write_direct(port, value, mask);
}


/*-------------------------------------------------
    input_port_write_safe - write a value to
    a port, ignore if the port does not exist
-------------------------------------------------*/

void input_port_write_safe(running_machine &machine, const char *tag, input_port_value value, input_port_value mask)
{
	const input_port_config *port = machine.port(tag);
	if (port != NULL)
		input_port_write_direct(port, value, mask);
}



/***************************************************************************
    MISC HELPER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_condition_true - return the TRUE
    if the given condition attached is true
-------------------------------------------------*/

int input_condition_true(running_machine &machine, const input_condition *condition, device_t &owner)
{
	input_port_value condvalue;

	/* always condition is always true */
	if (condition->condition == PORTCOND_ALWAYS)
		return TRUE;

	/* otherwise, read the referenced port */
	astring conditiontag;
	owner.subtag(conditiontag, condition->tag);
	condvalue = input_port_read(machine, conditiontag.cstr());

	/* based on the condition encoded, determine truth */
	switch (condition->condition)
	{
		case PORTCOND_EQUALS:
			return ((condvalue & condition->mask) == condition->value);

		case PORTCOND_NOTEQUALS:
			return ((condvalue & condition->mask) != condition->value);

		case PORTCOND_GREATERTHAN:
			return ((condvalue & condition->mask) > condition->value);

		case PORTCOND_NOTGREATERTHAN:
			return ((condvalue & condition->mask) <= condition->value);

		case PORTCOND_LESSTHAN:
			return ((condvalue & condition->mask) < condition->value);

		case PORTCOND_NOTLESSTHAN:
			return ((condvalue & condition->mask) >= condition->value);
	}
	return TRUE;
}


/*-------------------------------------------------
    input_port_string_from_token - convert an
    input_port_token to a default string
-------------------------------------------------*/

const char *input_port_string_from_token(const char *string)
{
	/* 0 is an invalid index */
	if (string == NULL)
		return NULL;

	/* if the index is greater than the count, assume it to be a pointer */
	if (FPTR(string) >= INPUT_STRING_COUNT)
		return string;

#if FALSE // Set TRUE, If you want to take care missing-token or wrong-sorting

	/* otherwise, scan the list for a matching string and return it */
	{
	int index;
	for (index = 0; index < ARRAY_LENGTH(input_port_default_strings); index++)
		if (input_port_default_strings[index].id == FPTR(string))
			return input_port_default_strings[index].string;
	}
	return "(Unknown Default)";

#else

	return input_port_default_strings[FPTR(string)-1].string;

#endif
}



/***************************************************************************
    INITIALIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    init_port_types - initialize the default
    type list
-------------------------------------------------*/

static void init_port_types(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;

	/* convert the array into a list of type states that can be modified */
	construct_core_types(portdata->typelist);

	/* ask the OSD to customize the list */
	machine.osd().customize_input_type_list(portdata->typelist);

	/* now iterate over the OSD-modified types */
	for (input_type_entry *curtype = portdata->typelist.first(); curtype != NULL; curtype = curtype->next())
	{
		/* first copy all the OSD-updated sequences into our current state */
		for (int seqtype = 0; seqtype < ARRAY_LENGTH(curtype->seq); seqtype++)
			curtype->seq[seqtype] = curtype->defseq[seqtype];

		/* also make a lookup table mapping type/player to the appropriate type list entry */
		portdata->type_to_entry[curtype->type][curtype->player] = curtype;
	}
}


/*-------------------------------------------------
    get_keyboard_code - accesses a particular
    keyboard code
-------------------------------------------------*/

static unicode_char get_keyboard_code(const input_field_config *field, int i)
{
	unicode_char ch = field->chars[i];

	/* special hack to allow for PORT_CODE('\xA3') */
	if ((ch >= 0xFFFFFF80) && (ch <= 0xFFFFFFFF))
		ch &= 0xFF;
	return ch;
}


/***************************************************************************
    MISCELLANEOUS
***************************************************************************/

/*-------------------------------------------------
    find_charinfo - looks up information about a
    particular character
-------------------------------------------------*/

static const char_info *find_charinfo(unicode_char target_char)
{
	int low = 0;
	int high = ARRAY_LENGTH(charinfo);
	int i;
	unicode_char ch;

	/* perform a simple binary search to find the proper alternate */
	while(high > low)
	{
		i = (high + low) / 2;
		ch = charinfo[i].ch;
		if (ch < target_char)
			low = i + 1;
		else if (ch > target_char)
			high = i;
		else
			return &charinfo[i];
	}
	return NULL;
}

/*-------------------------------------------------
    inputx_key_name - returns the name of a
    specific key
-------------------------------------------------*/

static const char *inputx_key_name(unicode_char ch)
{
	static char buf[UTF8_CHAR_MAX + 1];
	const char_info *ci;
	const char *result;
	int pos;

	ci = find_charinfo(ch);
	result = ci ? ci->name : NULL;

	if (ci && ci->name)
	{
		result = ci->name;
	}
	else
	{
		if ((ch > 0x7F) || isprint(ch))
		{
			pos = utf8_from_uchar(buf, ARRAY_LENGTH(buf), ch);
			buf[pos] = '\0';
			result = buf;
		}
		else
			result = "???";
	}
	return result;
}

/*-------------------------------------------------
    get_keyboard_key_name - builds the name of
    a key based on natural keyboard characters
-------------------------------------------------*/

static astring &get_keyboard_key_name(astring &name, const input_field_config *field)
{
	int i;
	unicode_char ch;

	name.reset();
	/* loop through each character on the field*/
	for (i = 0; i < ARRAY_LENGTH(field->chars) && (field->chars[i] != '\0'); i++)
	{
		ch = get_keyboard_code(field, i);
		name.catprintf("%-*s ", MAX(SPACE_COUNT - 1, 0), inputx_key_name(ch));
	}

	/* trim extra spaces */
	name.trimspace();

	/* special case */
	if (name.len() == 0)
		name.cpy("Unnamed Key");

	return name;
}

/*-------------------------------------------------
    init_port_state - initialize the live port
    states based on the tokens
-------------------------------------------------*/

static void init_port_state(running_machine &machine)
{
	const char *joystick_map_default = machine.options().joystick_map();
	input_port_private *portdata = machine.input_port_data;
	input_field_config *field;
	input_port_config *port;

	/* allocate live structures to mirror the configuration */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
	{
		analog_field_state **analogstatetail;
		device_field_info **readdevicetail;
		device_field_info **writedevicetail;
		input_port_state *portstate;

		/* allocate a new input_port_info structure */
		portstate = auto_alloc_clear(machine, input_port_state);
		((input_port_config *)port)->state = portstate;

		/* start with tail pointers to all the data */
		analogstatetail = &portstate->analoglist;
		readdevicetail = &portstate->readdevicelist;
		writedevicetail = &portstate->writedevicelist;

		/* iterate over fields */
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			input_field_state *fieldstate;
			int seqtype;

			/* allocate a new input_field_info structure */
			fieldstate = auto_alloc_clear(machine, input_field_state);
			((input_field_config *)field)->state = fieldstate;

			/* fill in the basic values */
			for (seqtype = 0; seqtype < ARRAY_LENGTH(fieldstate->seq); seqtype++)
				fieldstate->seq[seqtype] = field->seq[seqtype];
			fieldstate->value = field->defvalue;

			/* if this is an analog field, allocate memory for the analog data */
			if (field->type >= __ipt_analog_start && field->type <= __ipt_analog_end)
			{
				*analogstatetail = fieldstate->analog = init_field_analog_state(field);
				analogstatetail = &(*analogstatetail)->next;
			}

			/* if this is a digital joystick field, make a note of it */
			if (field->type >= __ipt_digital_joystick_start && field->type <= __ipt_digital_joystick_end)
			{
				fieldstate->joystick = &portdata->joystick_info[field->player][(field->type - __ipt_digital_joystick_start) / 4];
				fieldstate->joydir = (field->type - __ipt_digital_joystick_start) % 4;
				fieldstate->joystick->field[fieldstate->joydir] = field;
				fieldstate->joystick->inuse = TRUE;
			}

			/* if this entry has device input, allocate memory for the tracking structure */
			astring devicetag;
			if (!field->read.isnull())
			{
				*readdevicetail = init_field_device_info(field, port->owner().subtag(devicetag, field->read_device));
				field->read.late_bind(*(*readdevicetail)->device);
				if (!field->read.has_object())
					fatalerror("Input port %s, unable to find valid device with tag '%s'", port->tag(), devicetag.cstr());
				readdevicetail = &(*readdevicetail)->next;
			}

			/* if this entry has device output, allocate memory for the tracking structure */
			if (!field->write.isnull())
			{
				*writedevicetail = init_field_device_info(field, port->owner().subtag(devicetag, field->write_device));
				field->write.late_bind(*(*writedevicetail)->device);
				if (!field->write.has_object())
					fatalerror("Input port %s, unable to find valid device with tag '%s'", port->tag(), devicetag.cstr());
				writedevicetail = &(*writedevicetail)->next;
			}

			/* if this entry has device output, allocate memory for the tracking structure */
			if (!field->crossmapper.isnull())
			{
				device_t *device = machine.device(port->owner().subtag(devicetag, field->crossmapper_device));
				field->crossmapper.late_bind(*device);
			}

			/* Name keyboard key names */
			if ((field->type == IPT_KEYBOARD || field->type == IPT_KEYPAD) && (field->name == NULL))
			{
				astring name;
				field->state->name = auto_strdup(machine, get_keyboard_key_name(name, field));
			}
		}
	}

	/* handle autoselection of devices */
	init_autoselect_devices(machine, IPT_AD_STICK_X,  IPT_AD_STICK_Y,   IPT_AD_STICK_Z, OPTION_ADSTICK_DEVICE,    "analog joystick");
	init_autoselect_devices(machine, IPT_PADDLE,      IPT_PADDLE_V,     0,              OPTION_PADDLE_DEVICE,     "paddle");
	init_autoselect_devices(machine, IPT_PEDAL,       IPT_PEDAL2,       IPT_PEDAL3,     OPTION_PEDAL_DEVICE,      "pedal");
	init_autoselect_devices(machine, IPT_LIGHTGUN_X,  IPT_LIGHTGUN_Y,   0,              OPTION_LIGHTGUN_DEVICE,   "lightgun");
	init_autoselect_devices(machine, IPT_POSITIONAL,  IPT_POSITIONAL_V, 0,              OPTION_POSITIONAL_DEVICE, "positional");
	init_autoselect_devices(machine, IPT_DIAL,        IPT_DIAL_V,       0,              OPTION_DIAL_DEVICE,       "dial");
	init_autoselect_devices(machine, IPT_TRACKBALL_X, IPT_TRACKBALL_Y,  0,              OPTION_TRACKBALL_DEVICE,  "trackball");
	init_autoselect_devices(machine, IPT_MOUSE_X,     IPT_MOUSE_Y,      0,              OPTION_MOUSE_DEVICE,      "mouse");

	/* look for 4-way joysticks and change the default map if we find any */
	if (joystick_map_default[0] == 0 || strcmp(joystick_map_default, "auto") == 0)
		for (port = machine.m_portlist.first(); port != NULL; port = port->next())
			for (field = port->first_field(); field != NULL; field = field->next())
				if (field->state->joystick != NULL && field->way == 4)
				{
					machine.input().set_global_joystick_map((field->flags & FIELD_FLAG_ROTATED) ? joystick_map_4way_diagonal : joystick_map_4way_sticky);
					break;
				}
}


/*-------------------------------------------------
    init_autoselect_devices - autoselect a single
    device based on the input port list passed
    in and the corresponding option
-------------------------------------------------*/

static void init_autoselect_devices(running_machine &machine, int type1, int type2, int type3, const char *option, const char *ananame)
{
	const ioport_list &portlist = machine.m_portlist;
	const char *stemp = machine.options().value(option);
	input_device_class autoenable = DEVICE_CLASS_KEYBOARD;
	const char *autostring = "keyboard";
	const input_field_config *field;
	const input_port_config *port;

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
	if (portlist.first() != NULL && !machine.input().device_class(autoenable).enabled())
		for (port = portlist.first(); port != NULL; port = port->next())
			for (field = port->first_field(); field != NULL; field = field->next())

				/* if this port type is in use, apply the autoselect criteria */
				if ((type1 != 0 && field->type == type1) ||
					(type2 != 0 && field->type == type2) ||
					(type3 != 0 && field->type == type3))
				{
					mame_printf_verbose("Input: Autoenabling %s due to presence of a %s\n", autostring, ananame);
					machine.input().device_class(autoenable).enable();
					break;
				}
}


/*-------------------------------------------------
    init_field_device_info - allocate and populate
    information about a device callback
-------------------------------------------------*/

static device_field_info *init_field_device_info(const input_field_config *field, const char *device_name)
{
	device_field_info *info;
	input_port_value mask;

	/* allocate memory */
	info = auto_alloc_clear(field->machine(), device_field_info);

	/* fill in the data */
	info->field = field;
	for (mask = field->mask; !(mask & 1); mask >>= 1)
		info->shift++;

	info->device = (device_name != NULL) ? field->machine().device(device_name) : &field->port().owner();

	info->oldval = field->defvalue >> info->shift;
	return info;
}


/*-------------------------------------------------
    init_field_analog_state - allocate and populate
    information about an analog port
-------------------------------------------------*/

static analog_field_state *init_field_analog_state(const input_field_config *field)
{
	analog_field_state *state;
	input_port_value mask;

	/* allocate memory */
	state = auto_alloc_clear(field->machine(), analog_field_state);

	/* compute the shift amount and number of bits */
	for (mask = field->mask; !(mask & 1); mask >>= 1)
		state->shift++;

	/* initialize core data */
	state->field = field;
	state->adjdefvalue = (field->defvalue & field->mask) >> state->shift;
	state->adjmin = (field->min & field->mask) >> state->shift;
	state->adjmax = (field->max & field->mask) >> state->shift;
	state->sensitivity = field->sensitivity;
	state->reverse = ((field->flags & ANALOG_FLAG_REVERSE) != 0);
	state->delta = field->delta;
	state->centerdelta = field->centerdelta;
	state->minimum = INPUT_ABSOLUTE_MIN;
	state->maximum = INPUT_ABSOLUTE_MAX;

	/* set basic parameters based on the configured type */
	switch (field->type)
	{
		/* paddles and analog joysticks are absolute and autocenter */
		case IPT_AD_STICK_X:
		case IPT_AD_STICK_Y:
		case IPT_AD_STICK_Z:
		case IPT_PADDLE:
		case IPT_PADDLE_V:
			state->absolute = TRUE;
			state->autocenter = TRUE;
			state->interpolate = TRUE;
			break;

		/* pedals start at and autocenter to the min range */
		case IPT_PEDAL:
		case IPT_PEDAL2:
		case IPT_PEDAL3:
			state->center = INPUT_ABSOLUTE_MIN;
			state->accum = APPLY_INVERSE_SENSITIVITY(state->center, state->sensitivity);
			state->absolute = TRUE;
			state->autocenter = TRUE;
			state->interpolate = TRUE;
			break;

		/* lightguns are absolute as well, but don't autocenter and don't interpolate their values */
		case IPT_LIGHTGUN_X:
		case IPT_LIGHTGUN_Y:
			state->absolute = TRUE;
			state->autocenter = FALSE;
			state->interpolate = FALSE;
			break;

		/* positional devices are absolute, but can also wrap like relative devices */
		/* set each position to be 512 units */
		case IPT_POSITIONAL:
		case IPT_POSITIONAL_V:
			state->positionalscale = COMPUTE_SCALE(field->max, INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);
			state->adjmin = 0;
			state->adjmax = field->max - 1;
			state->wraps = ((field->flags & ANALOG_FLAG_WRAPS) != 0);
			state->autocenter = !state->wraps;
			break;

		/* dials, mice and trackballs are relative devices */
		/* these have fixed "min" and "max" values based on how many bits are in the port */
		/* in addition, we set the wrap around min/max values to 512 * the min/max values */
		/* this takes into account the mapping that one mouse unit ~= 512 analog units */
		case IPT_DIAL:
		case IPT_DIAL_V:
		case IPT_TRACKBALL_X:
		case IPT_TRACKBALL_Y:
		case IPT_MOUSE_X:
		case IPT_MOUSE_Y:
			state->absolute = FALSE;
			state->wraps = TRUE;
			state->interpolate = TRUE;
			break;

		default:
			fatalerror("Unknown analog port type -- don't know if it is absolute or not");
			break;
	}

	/* further processing for absolute controls */
	if (state->absolute)
	{
		/* if the default value is pegged at the min or max, use a single scale value for the whole axis */
		state->single_scale = (state->adjdefvalue == state->adjmin) || (state->adjdefvalue == state->adjmax);

		/* if not "single scale", compute separate scales for each side of the default */
		if (!state->single_scale)
		{
			/* unsigned */
			state->scalepos = COMPUTE_SCALE(state->adjmax - state->adjdefvalue, INPUT_ABSOLUTE_MAX - 0);
			state->scaleneg = COMPUTE_SCALE(state->adjdefvalue - state->adjmin, 0 - INPUT_ABSOLUTE_MIN);

			if (state->adjmin > state->adjmax)
				state->scaleneg = -state->scaleneg;

			/* reverse point is at center */
			state->reverse_val = 0;
		}
		else
		{
			/* single axis that increases from default */
			state->scalepos = COMPUTE_SCALE(state->adjmax - state->adjmin, INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);

			/* move from default */
			if (state->adjdefvalue == state->adjmax)
				state->scalepos = -state->scalepos;

			/* make the scaling the same for easier coding when we need to scale */
			state->scaleneg = state->scalepos;

			/* reverse point is at max */
			state->reverse_val = state->maximum;
		}
	}

	/* relative and positional controls all map directly with a 512x scale factor */
	else
	{
		/* The relative code is set up to allow specifing PORT_MINMAX and default values. */
		/* The validity checks are purposely set up to not allow you to use anything other */
		/* a default of 0 and PORT_MINMAX(0,mask).  This is in case the need arises to use */
		/* this feature in the future.  Keeping the code in does not hurt anything. */
		if (state->adjmin > state->adjmax)
			/* adjust for signed */
			state->adjmin = -state->adjmin;

		if (state->wraps)
			state->adjmax++;

		state->minimum = (state->adjmin - state->adjdefvalue) * INPUT_RELATIVE_PER_PIXEL;
		state->maximum = (state->adjmax - state->adjdefvalue) * INPUT_RELATIVE_PER_PIXEL;

		/* make the scaling the same for easier coding when we need to scale */
		state->scaleneg = state->scalepos = COMPUTE_SCALE(1, INPUT_RELATIVE_PER_PIXEL);

		if (field->flags & ANALOG_FLAG_RESET)
			/* delta values reverse from center */
			state->reverse_val = 0;
		else
		{
			/* positional controls reverse from their max range */
			state->reverse_val = state->maximum + state->minimum;

			/* relative controls reverse from 1 past their max range */
			if (state->wraps)
				state->reverse_val -= INPUT_RELATIVE_PER_PIXEL;
		}
	}

	/* compute scale for keypresses */
	state->keyscalepos = RECIP_SCALE(state->scalepos);
	state->keyscaleneg = RECIP_SCALE(state->scaleneg);

	return state;
}



/***************************************************************************
    ONCE-PER-FRAME UPDATES
***************************************************************************/

/*-------------------------------------------------
    frame_update_callback - system-wide callback to
    update the input ports once per frame, but
    only if we are not paused
-------------------------------------------------*/

static void frame_update_callback(running_machine &machine)
{
	/* if we're paused, don't do anything */
	if (machine.paused())
		return;

	/* otherwise, use the common code */
	frame_update(machine);
}

static key_buffer *get_buffer(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	assert(inputx_can_post(machine));
	return (key_buffer *)&portdata->keybuffer;
}



static const inputx_code *find_code(inputx_code *codes, unicode_char ch)
{
	int i;

	assert(codes);
	for (i = 0; codes[i].ch; i++)
	{
		if (codes[i].ch == ch)
			return &codes[i];
	}
	return NULL;
}

/*-------------------------------------------------
    input_port_update_hook - hook function
    called from core to allow for natural keyboard
-------------------------------------------------*/

static void input_port_update_hook(running_machine &machine, const input_port_config *port, input_port_value *digital)
{
	input_port_private *portdata = machine.input_port_data;
	const key_buffer *keybuf;
	const inputx_code *code;
	unicode_char ch;
	int i;
	UINT32 value;

	if (inputx_can_post(machine))
	{
		keybuf = get_buffer(machine);

		/* is the key down right now? */
		if (keybuf && keybuf->status_keydown && (keybuf->begin_pos != keybuf->end_pos))
		{
			/* identify the character that is down right now, and its component codes */
			ch = keybuf->buffer[keybuf->begin_pos];
			code = find_code(portdata->codes, ch);

			/* loop through this character's component codes */
			if (code != NULL)
			{
				for (i = 0; i < ARRAY_LENGTH(code->field) && (code->field[i] != NULL); i++)
				{
					if (&code->field[i]->port() == port)
					{
						value = code->field[i]->mask;
						*digital |= value;
					}
				}
			}
		}
	}
}


/*-------------------------------------------------
    frame_update - core logic for per-frame input
    port updating
-------------------------------------------------*/

static void frame_update(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	const input_field_config *mouse_field = NULL;
	int ui_visible = ui_is_menu_active();
	attotime curtime = machine.time();
	const input_port_config *port;
	render_target *mouse_target;
	INT32 mouse_target_x;
	INT32 mouse_target_y;
	int mouse_button;

g_profiler.start(PROFILER_INPUT);

	/* record/playback information about the current frame */
	playback_frame(machine, curtime);
	record_frame(machine, curtime);

	/* track the duration of the previous frame */
	portdata->last_delta_nsec = (curtime - portdata->last_frame_time).as_attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	portdata->last_frame_time = curtime;

	/* update the digital joysticks */
	frame_update_digital_joysticks(machine);

	/* compute default values for all the ports */
	input_port_update_defaults(machine);

	/* perform the mouse hit test */
	mouse_target = ui_input_find_mouse(machine, &mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_button && mouse_target)
	{
		const char *tag = NULL;
		input_port_value mask;
		float x, y;
		if (mouse_target->map_point_input(mouse_target_x, mouse_target_y, tag, mask, x, y))
			mouse_field = input_field_by_tag_and_mask(machine, tag, mask);
	}

	/* loop over all input ports */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
	{
		const input_field_config *field;
		device_field_info *device_field;
		input_port_value newvalue;

		/* start with 0 values for the digital and VBLANK bits */
		port->state->digital = 0;
		port->state->vblank = 0;

		/* now loop back and modify based on the inputs */
		for (field = port->first_field(); field != NULL; field = field->next())
			if (input_condition_true(port->machine(), &field->condition, port->owner()))
			{
				/* accumulate VBLANK bits */
				if (field->type == IPT_VBLANK)
					port->state->vblank ^= field->mask;

				/* handle analog inputs */
				else if (field->state->analog != NULL)
					frame_update_analog_field(machine, field->state->analog);

				/* handle non-analog types, but only when the UI isn't visible */
				else if (!ui_visible && frame_get_digital_field_state(field, field == mouse_field))
					port->state->digital |= field->mask;
			}

		/* hook for MESS's natural keyboard support */
		input_port_update_hook(machine, port, &port->state->digital);

		/* handle playback/record */
		playback_port(port);
		record_port(port);

		/* call device line write handlers */
		newvalue = input_port_read_direct(port);
		for (device_field = port->state->writedevicelist; device_field; device_field = device_field->next)
			if (device_field->field->type != IPT_OUTPUT && input_condition_true(port->machine(), &device_field->field->condition, port->owner()))
			{
				input_port_value newval = (newvalue & device_field->field->mask) >> device_field->shift;

				/* if the bits have  write, call the handler */
				if (device_field->oldval != newval)
				{
					device_field->field->write(*device_field->field, device_field->field->write_param, device_field->oldval, newval);

					device_field->oldval = newval;
				}
			}
	}

g_profiler.stop();
}


/*-------------------------------------------------
    frame_update_digital_joysticks - update the
    state of digital joysticks prior to
    accumulating the results in a port
-------------------------------------------------*/

static void frame_update_digital_joysticks(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	int player, joyindex;

	/* loop over all the joysticks */
	for (player = 0; player < MAX_PLAYERS; player++)
		for (joyindex = 0; joyindex < DIGITAL_JOYSTICKS_PER_PLAYER; joyindex++)
		{
			digital_joystick_state *joystick = &portdata->joystick_info[player][joyindex];
			if (joystick->inuse)
			{
				joystick->previous = joystick->current;
				joystick->current = 0;

				/* read all the associated ports */
				if (joystick->field[JOYDIR_UP] != NULL && machine.input().seq_pressed(input_field_seq(joystick->field[JOYDIR_UP], SEQ_TYPE_STANDARD)))
					joystick->current |= JOYDIR_UP_BIT;
				if (joystick->field[JOYDIR_DOWN] != NULL && machine.input().seq_pressed(input_field_seq(joystick->field[JOYDIR_DOWN], SEQ_TYPE_STANDARD)))
					joystick->current |= JOYDIR_DOWN_BIT;
				if (joystick->field[JOYDIR_LEFT] != NULL && machine.input().seq_pressed(input_field_seq(joystick->field[JOYDIR_LEFT], SEQ_TYPE_STANDARD)))
					joystick->current |= JOYDIR_LEFT_BIT;
				if (joystick->field[JOYDIR_RIGHT] != NULL && machine.input().seq_pressed(input_field_seq(joystick->field[JOYDIR_RIGHT], SEQ_TYPE_STANDARD)))
					joystick->current |= JOYDIR_RIGHT_BIT;

				/* lock out opposing directions (left + right or up + down) */
				if ((joystick->current & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) == (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT))
					joystick->current &= ~(JOYDIR_UP_BIT | JOYDIR_DOWN_BIT);
				if ((joystick->current & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)) == (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT))
					joystick->current &= ~(JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT);

				/* only update 4-way case if joystick has moved */
				if (joystick->current != joystick->previous)
				{
					joystick->current4way = joystick->current;

					/*
                        If joystick is pointing at a diagonal, acknowledge that the player moved
                        the joystick by favoring a direction change.  This minimizes frustration
                        when using a keyboard for input, and maximizes responsiveness.

                        For example, if you are holding "left" then switch to "up" (where both left
                        and up are briefly pressed at the same time), we'll transition immediately
                        to "up."

                        Zero any switches that didn't change from the previous to current state.
                     */
					if ((joystick->current4way & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) &&
						(joystick->current4way & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)))
					{
						joystick->current4way ^= joystick->current4way & joystick->previous;
					}

					/*
                        If we are still pointing at a diagonal, we are in an indeterminant state.

                        This could happen if the player moved the joystick from the idle position directly
                        to a diagonal, or from one diagonal directly to an extreme diagonal.

                        The chances of this happening with a keyboard are slim, but we still need to
                        constrain this case.

                        For now, just resolve randomly.
                     */
					if ((joystick->current4way & (JOYDIR_UP_BIT | JOYDIR_DOWN_BIT)) &&
						(joystick->current4way & (JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT)))
					{
						if (machine.rand() & 1)
							joystick->current4way &= ~(JOYDIR_LEFT_BIT | JOYDIR_RIGHT_BIT);
						else
							joystick->current4way &= ~(JOYDIR_UP_BIT | JOYDIR_DOWN_BIT);
					}
				}
			}
		}
}


/*-------------------------------------------------
    frame_update_analog_field - update the
    internals of a single analog field
-------------------------------------------------*/

static void frame_update_analog_field(running_machine &machine, analog_field_state *analog)
{
	input_item_class itemclass;
	int keypressed = FALSE;
	INT64 keyscale;
	INT32 rawvalue;
	INT32 delta = 0;

	/* clamp the previous value to the min/max range and remember it */
	analog->previous = analog->accum = apply_analog_min_max(analog, analog->accum);

	/* get the new raw analog value and its type */
	rawvalue = machine.input().seq_axis_value(input_field_seq(analog->field, SEQ_TYPE_STANDARD), itemclass);

	/* if we got an absolute input, it overrides everything else */
	if (itemclass == ITEM_CLASS_ABSOLUTE)
	{
		if (analog->previousanalog != rawvalue)
		{
			/* only update if analog value changed */
			analog->previousanalog = rawvalue;

			/* apply the inverse of the sensitivity to the raw value so that */
			/* it will still cover the full min->max range requested after */
			/* we apply the sensitivity adjustment */
			if (analog->absolute || (analog->field->flags & ANALOG_FLAG_RESET))
			{
				/* if port is absolute, then just return the absolute data supplied */
				analog->accum = APPLY_INVERSE_SENSITIVITY(rawvalue, analog->sensitivity);
			}
			else if (analog->positionalscale != 0)
			{
				/* if port is positional, we will take the full analog control and divide it */
				/* into positions, that way as the control is moved full scale, */
				/* it moves through all the positions */
				rawvalue = APPLY_SCALE(rawvalue - INPUT_ABSOLUTE_MIN, analog->positionalscale) * INPUT_RELATIVE_PER_PIXEL + analog->minimum;

				/* clamp the high value so it does not roll over */
				rawvalue = MIN(rawvalue, analog->maximum);
				analog->accum = APPLY_INVERSE_SENSITIVITY(rawvalue, analog->sensitivity);
			}
			else
				/* if port is relative, we use the value to simulate the speed of relative movement */
				/* sensitivity adjustment is allowed for this mode */
				analog->accum += rawvalue;

			analog->lastdigital = FALSE;
			/* do not bother with other control types if the analog data is changing */
			return;
		}
		else
		{
			/* we still have to update fake relative from joystick control */
			if (!analog->absolute && analog->positionalscale == 0)
				analog->accum += rawvalue;
		}
	}

	/* if we got it from a relative device, use that as the starting delta */
	/* also note that the last input was not a digital one */
	if (itemclass == ITEM_CLASS_RELATIVE && rawvalue != 0)
	{
		delta = rawvalue;
		analog->lastdigital = FALSE;
	}

	keyscale = (analog->accum >= 0) ? analog->keyscalepos : analog->keyscaleneg;

	/* if the decrement code sequence is pressed, add the key delta to */
	/* the accumulated delta; also note that the last input was a digital one */
	if (machine.input().seq_pressed(input_field_seq(analog->field, SEQ_TYPE_DECREMENT)))
	{
		keypressed = TRUE;
		if (analog->delta != 0)
			delta -= APPLY_SCALE(analog->delta, keyscale);
		else if (!analog->lastdigital)
			/* decrement only once when first pressed */
			delta -= APPLY_SCALE(1, keyscale);
		analog->lastdigital = TRUE;
	}

	/* same for the increment code sequence */
	if (machine.input().seq_pressed(input_field_seq(analog->field, SEQ_TYPE_INCREMENT)))
	{
		keypressed = TRUE;
		if (analog->delta)
			delta += APPLY_SCALE(analog->delta, keyscale);
		else if (!analog->lastdigital)
			/* increment only once when first pressed */
			delta += APPLY_SCALE(1, keyscale);
		analog->lastdigital = TRUE;
	}

	/* if resetting is requested, clear the accumulated position to 0 before */
	/* applying the deltas so that we only return this frame's delta */
	/* note that centering only works for relative controls */
	/* no need to check if absolute here because it is checked by the validity tests */
	if (analog->field->flags & ANALOG_FLAG_RESET)
		analog->accum = 0;

	/* apply the delta to the accumulated value */
	analog->accum += delta;

	/* if our last movement was due to a digital input, and if this control */
	/* type autocenters, and if neither the increment nor the decrement seq */
	/* was pressed, apply autocentering */
	if (analog->autocenter)
	{
		INT32 center = APPLY_INVERSE_SENSITIVITY(analog->center, analog->sensitivity);
		if (analog->lastdigital && !keypressed)
		{
			/* autocenter from positive values */
			if (analog->accum >= center)
			{
				analog->accum -= APPLY_SCALE(analog->centerdelta, analog->keyscalepos);
				if (analog->accum < center)
				{
					analog->accum = center;
					analog->lastdigital = FALSE;
				}
			}

			/* autocenter from negative values */
			else
			{
				analog->accum += APPLY_SCALE(analog->centerdelta, analog->keyscaleneg);
				if (analog->accum > center)
				{
					analog->accum = center;
					analog->lastdigital = FALSE;
				}
			}
		}
	}
	else if (!keypressed)
		analog->lastdigital = FALSE;
}


/*-------------------------------------------------
    frame_get_digital_field_state - get the state
    of a digital field
-------------------------------------------------*/

static int frame_get_digital_field_state(const input_field_config *field, int mouse_down)
{
	int curstate = mouse_down || field->machine().input().seq_pressed(input_field_seq(field, SEQ_TYPE_STANDARD));
	int changed = FALSE;
	int temp_field_impulse;

	/* if the state changed, look for switch down/switch up */
	if (curstate != field->state->last)
	{
		field->state->last = curstate;
		changed = TRUE;
	}

	if (field->type == IPT_KEYBOARD && ui_get_use_natural_keyboard(field->machine()))
		return FALSE;

	/* coin impulse option */
	{
		int temp_option_impulse = field->machine().options().coin_impulse();
		temp_field_impulse = field->impulse;
		if ( temp_option_impulse != 0)
		{
			if (temp_option_impulse < 0)
				temp_field_impulse = 0;
			else if ((field->type >= IPT_COIN1 && field->type <= IPT_COIN12) || field->impulse != 0)
				temp_field_impulse = temp_option_impulse;
		}
	}

	/* if this is a switch-down event, handle impulse and toggle */
	if (changed && curstate)
	{
		/* impluse controls: reset the impulse counter */
		if (temp_field_impulse != 0 && field->state->impulse == 0)
			field->state->impulse = temp_field_impulse;

		/* toggle controls: flip the toggle state or advance to the next setting */
		if (field->flags & FIELD_FLAG_TOGGLE)
		{
			if (field->settinglist().count() == 0)
				field->state->value ^= field->mask;
			else
				input_field_select_next_setting(field);
		}
	}

	/* update the current state with the impulse state */
	if (temp_field_impulse != 0)
	{
		if (field->state->impulse != 0)
		{
			field->state->impulse--;
			curstate = TRUE;
		}
		else
			curstate = FALSE;
	}

	/* for toggle switches, the current value is folded into the port's default value */
	/* so we always return FALSE here */
	if (field->flags & FIELD_FLAG_TOGGLE)
		curstate = FALSE;

	/* additional logic to restrict digital joysticks */
	if (curstate && !mouse_down && field->state->joystick != NULL && field->way != 16 && !field->machine().options().joystick_contradictory())
	{
		UINT8 mask = (field->way == 4) ? field->state->joystick->current4way : field->state->joystick->current;
		if (!(mask & (1 << field->state->joydir)))
			curstate = FALSE;
	}

	/* skip locked-out coin inputs */
	if (curstate && field->type >= IPT_COIN1 && field->type <= IPT_COIN12 && coin_lockout_get_state(field->machine(), field->type - IPT_COIN1))
	{
		int verbose = field->machine().options().verbose();
#ifdef MAME_DEBUG
		verbose = 1;
#endif
		if (field->machine().options().coin_lockout())
		{
			if (verbose)
				ui_popup_time(3, "Coinlock disabled %s.", input_field_name(field));
			return FALSE; /* curstate = FALSE; */
		}
		else
			if (verbose)
				ui_popup_time(3, "Coinlock disabled, but broken through %s.", input_field_name(field));
	}

	return curstate;
}



/***************************************************************************
    PORT CONFIGURATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    port_default_value - updates default value
    of port settings according to device settings
-------------------------------------------------*/

UINT32 port_default_value(const char *fulltag, UINT32 mask, UINT32 defval, device_t &owner)
{
	const input_device_default *def = owner.input_ports_defaults();
	if (def != NULL)
	{
		astring fullpath;
		for ( ; def->tag != NULL; def++)
			if (owner.subtag(fullpath, def->tag) == fulltag && def->mask == mask)
				return def->defvalue;
	}
	return defval;
}


/*-------------------------------------------------
    input_port_config - constructor for an
    I/O port configuration object
-------------------------------------------------*/

input_port_config::input_port_config(device_t &owner, const char *tag)
	: state(NULL),
	  active(0),
	  m_next(NULL),
	  m_owner(owner),
	  m_tag(tag),
	  m_modcount(0)
{
}


running_machine &input_port_config::machine() const
{
	return m_owner.machine();
}


/*-------------------------------------------------
    field_config_alloc - allocate a new input
    port field config
-------------------------------------------------*/

input_field_config::input_field_config(input_port_config &port, int _type, input_port_value _defvalue, input_port_value _maskbits, const char *_name)
	: mask(_maskbits),
	  defvalue(_defvalue & _maskbits),
	  type(_type),
	  player(0),
	  flags(0),
	  impulse(0),
	  name(_name),
	  read_param(NULL),
	  read_device(DEVICE_SELF),
	  write_param(NULL),
	  write_device(DEVICE_SELF),
	  min(0),
	  max(_maskbits),
	  sensitivity(0),
	  delta(0),
	  centerdelta(0),
	  crossaxis(0),
	  crossscale(0),
	  crossoffset(0),
	  crossaltaxis(0),
	  crossmapper_device(DEVICE_SELF),
	  full_turn_count(0),
	  remap_table(NULL),
	  way(0),
	  state(NULL),
	  m_next(NULL),
	  m_port(port),
	  m_modcount(port.modcount())
{
	memset(&condition, 0, sizeof(condition));
	for (int seqtype = 0; seqtype < ARRAY_LENGTH(seq); seqtype++)
		seq[seqtype].set_default();
	chars[0] = chars[1] = chars[2] = (unicode_char) 0;
}


input_setting_config::input_setting_config(input_field_config &field, input_port_value _value, const char *_name)
	: value(_value),
	  name(_name),
	  m_field(field),
	  m_next(NULL)
{
	memset(&condition, 0, sizeof(condition));
}

input_field_diplocation::input_field_diplocation(const char *string, UINT8 _swnum, bool _invert)
	: swname(string),
	  swnum(_swnum),
	  invert(_invert)
{
}

/*-------------------------------------------------
    field_config_insert - insert an allocated
    input port field config, replacing any
    intersecting fields already present and
    inserting at the correct sorted location
-------------------------------------------------*/

void input_port_config::collapse_fields(astring &errorbuf)
{
	input_field_config *list = m_fieldlist.detach_all();
	input_port_value maskbits = 0;
	int lastmodcount = -1;
	while (list != NULL)
	{
		if (list->modcount() != lastmodcount)
		{
			lastmodcount = list->modcount();
			maskbits = 0;
		}
		input_field_config *current = list;
		list = list->next();
		field_config_insert(*current, maskbits, errorbuf);
	}
}

void field_config_insert(input_field_config &newfield, input_port_value &disallowedbits, astring &errorbuf)
{
	input_port_value lowbit;

	/* verify against the disallowed bits, but only if we are condition-free */
	if (newfield.condition.condition == PORTCOND_ALWAYS)
	{
		if ((newfield.mask & disallowedbits) != 0)
			errorbuf.catprintf("INPUT_TOKEN_FIELD specifies duplicate port bits (port=%s mask=%X)\n", newfield.port().tag(), newfield.mask);
		disallowedbits |= newfield.mask;
	}

	/* first modify/nuke any entries that intersect our maskbits */
	input_field_config *nextfield;
	for (input_field_config *field = newfield.port().fieldlist().first(); field != NULL; field = nextfield)
	{
		nextfield = field->next();
		if ((field->mask & newfield.mask) != 0 && (newfield.condition.condition == PORTCOND_ALWAYS ||
		                                           field->condition.condition == PORTCOND_ALWAYS ||
		                                           condition_equal(&field->condition, &newfield.condition)))
		{
			/* reduce the mask of the field we found */
			field->mask &= ~newfield.mask;

			/* if the new entry fully overrides the previous one, we nuke */
			if (INPUT_PORT_OVERRIDE_FULLY_NUKES_PREVIOUS || field->mask == 0)
				newfield.port().fieldlist().remove(*field);
		}
	}

	/* make a mask of just the low bit */
	lowbit = (newfield.mask ^ (newfield.mask - 1)) & newfield.mask;

	/* scan forward to find where to insert ourselves */
	input_field_config *field;
	for (field = newfield.port().fieldlist().first(); field != NULL; field = field->next())
		if (field->mask > lowbit)
			break;

	/* insert it into the list */
	newfield.port().fieldlist().insert_before(newfield, field);
}


/*-------------------------------------------------
    diplocation_expand - expand a string-based
    DIP location into a linked list of
    descriptions
-------------------------------------------------*/

void diplocation_list_alloc(input_field_config &field, const char *location, astring &errorbuf)
{
	/* if nothing present, bail */
	if (location == NULL)
		return;

	field.diploclist().reset();

	/* parse the string */
	astring name; // Don't move this variable inside the loop, lastname's lifetime depends on it being outside
	const char *lastname = NULL;
	const char *curentry = location;
	int entries = 0;
	while (*curentry != 0)
	{
		/* find the end of this entry */
		const char *comma = strchr(curentry, ',');
		if (comma == NULL)
			comma = curentry + strlen(curentry);

		/* extract it to tempbuf */
		astring tempstr;
		tempstr.cpy(curentry, comma - curentry);

		/* first extract the switch name if present */
		const char *number = tempstr;
		const char *colon = strchr(tempstr, ':');

		/* allocate and copy the name if it is present */
		if (colon != NULL)
		{
			lastname = name.cpy(number, colon - number);
			number = colon + 1;
		}

		/* otherwise, just copy the last name */
		else
		{
			if (lastname == NULL)
			{
				errorbuf.catprintf("Switch location '%s' missing switch name!\n", location);
				lastname = (char *)"UNK";
			}
			name.cpy(lastname);
		}

		/* if the number is preceded by a '!' it's active high */
		bool invert = false;
		if (*number == '!')
		{
			invert = true;
			number++;
		}

		/* now scan the switch number */
		int swnum = -1;
		if (sscanf(number, "%d", &swnum) != 1)
			errorbuf.catprintf("Switch location '%s' has invalid format!\n", location);

		/* allocate a new entry */
		field.diploclist().append(*global_alloc(input_field_diplocation(name, swnum, invert)));
		entries++;

		/* advance to the next item */
		curentry = comma;
		if (*curentry != 0)
			curentry++;
	}

	/* then verify the number of bits in the mask matches */
	input_port_value temp;
	int bits;
	for (bits = 0, temp = field.mask; temp != 0 && bits < 32; bits++)
		temp &= temp - 1;
	if (bits != entries)
		errorbuf.catprintf("Switch location '%s' does not describe enough bits for mask %X\n", location, field.mask);
}



/***************************************************************************
    TOKENIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    token_to_input_field_type - convert a string
    token to an input field type and player
-------------------------------------------------*/

static int token_to_input_field_type(running_machine &machine, const char *string, int *player)
{
	input_port_private *portdata = machine.input_port_data;
	int ipnum;

	/* check for our failsafe case first */
	if (sscanf(string, "TYPE_OTHER(%d,%d)", &ipnum, player) == 2)
		return ipnum;

	/* find the token in the list */
	for (input_type_entry *entry = portdata->typelist.first(); entry != NULL; entry = entry->next())
		if (entry->token != NULL && !strcmp(entry->token, string))
		{
			*player = entry->player;
			return entry->type;
		}

	/* if we fail, return IPT_UNKNOWN */
	*player = 0;
	return IPT_UNKNOWN;
}


/*-------------------------------------------------
    input_field_type_to_token - convert an input
    field type and player to a string token
-------------------------------------------------*/

static const char *input_field_type_to_token(running_machine &machine, int type, int player)
{
	input_port_private *portdata = machine.input_port_data;
	static char tempbuf[32];

	/* look up the port and return the token */
	input_type_entry *entry = portdata->type_to_entry[type][player];
	if (entry != NULL)
		return entry->token;

	/* if that fails, carry on */
	sprintf(tempbuf, "TYPE_OTHER(%d,%d)", type, player);
	return tempbuf;
}


/*-------------------------------------------------
    token_to_seq_type - convert a string to
    a sequence type
-------------------------------------------------*/

static int token_to_seq_type(const char *string)
{
	int seqindex;

	/* look up the string in the table of possible sequence types and return the index */
	for (seqindex = 0; seqindex < ARRAY_LENGTH(seqtypestrings); seqindex++)
		if (!mame_stricmp(string, seqtypestrings[seqindex]))
			return seqindex;

	return -1;
}



/***************************************************************************
    SETTINGS LOAD
***************************************************************************/

/*-------------------------------------------------
    load_config_callback - callback to extract
    configuration data from the XML nodes
-------------------------------------------------*/

static void load_config_callback(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	input_port_private *portdata = machine.input_port_data;
	xml_data_node *portnode;
	int seqtype;

	/* in the completion phase, we finish the initialization with the final ports */
	if (config_type == CONFIG_TYPE_FINAL)
	{
		portdata->safe_to_read = TRUE;
		frame_update(machine);
	}

	/* early exit if no data to parse */
	if (parentnode == NULL)
		return;

	/* iterate over all the remap nodes for controller configs only */
	if (config_type == CONFIG_TYPE_CONTROLLER)
		load_remap_table(machine, parentnode);

	/* iterate over all the port nodes */
	for (portnode = xml_get_sibling(parentnode->child, "port"); portnode; portnode = xml_get_sibling(portnode->next, "port"))
	{
		input_seq newseq[SEQ_TYPE_TOTAL], tempseq;
		xml_data_node *seqnode;
		int type, player;

		/* get the basic port info from the attributes */
		type = token_to_input_field_type(machine, xml_get_attribute_string(portnode, "type", ""), &player);

		/* initialize sequences to invalid defaults */
		for (seqtype = 0; seqtype < ARRAY_LENGTH(newseq); seqtype++)
			newseq[seqtype].set(INPUT_CODE_INVALID);

		/* loop over new sequences */
		for (seqnode = xml_get_sibling(portnode->child, "newseq"); seqnode; seqnode = xml_get_sibling(seqnode->next, "newseq"))
		{
			/* with a valid type, parse out the new sequence */
			seqtype = token_to_seq_type(xml_get_attribute_string(seqnode, "type", ""));
			if (seqtype != -1 && seqnode->value != NULL)
			{
				if (strcmp(seqnode->value, "NONE") == 0)
					newseq[seqtype].set();
				else
					machine.input().seq_from_tokens(newseq[seqtype], seqnode->value);
			}
		}

		/* if we're loading default ports, apply to the defaults */
		if (config_type != CONFIG_TYPE_GAME)
			load_default_config(machine, portnode, type, player, newseq);
		else
			load_game_config(machine, portnode, type, player, newseq);
	}

	/* after applying the controller config, push that back into the backup, since that is */
	/* what we will diff against */
	if (config_type == CONFIG_TYPE_CONTROLLER)
		for (input_type_entry *entry = portdata->typelist.first(); entry != NULL; entry = entry->next())
			for (seqtype = 0; seqtype < ARRAY_LENGTH(entry->seq); seqtype++)
				entry->defseq[seqtype] = entry->seq[seqtype];
}


/*-------------------------------------------------
    load_remap_table - extract and apply the
    global remapping table
-------------------------------------------------*/

static void load_remap_table(running_machine &machine, xml_data_node *parentnode)
{
	input_port_private *portdata = machine.input_port_data;
	input_code *oldtable, *newtable;
	xml_data_node *remapnode;
	int count;

	/* count items first so we can allocate */
	count = 0;
	for (remapnode = xml_get_sibling(parentnode->child, "remap"); remapnode != NULL; remapnode = xml_get_sibling(remapnode->next, "remap"))
		count++;

	/* if we have some, deal with them */
	if (count > 0)
	{
		int remapnum;

		/* allocate tables */
		oldtable = global_alloc_array(input_code, count);
		newtable = global_alloc_array(input_code, count);

		/* build up the remap table */
		count = 0;
		for (remapnode = xml_get_sibling(parentnode->child, "remap"); remapnode != NULL; remapnode = xml_get_sibling(remapnode->next, "remap"))
		{
			input_code origcode = machine.input().code_from_token(xml_get_attribute_string(remapnode, "origcode", ""));
			input_code newcode = machine.input().code_from_token(xml_get_attribute_string(remapnode, "newcode", ""));
			if (origcode != INPUT_CODE_INVALID && newcode != INPUT_CODE_INVALID)
			{
				oldtable[count] = origcode;
				newtable[count] = newcode;
				count++;
			}
		}

		/* loop over the remapping table, operating only if something was specified */
		for (remapnum = 0; remapnum < count; remapnum++)
		{
			input_code oldcode = oldtable[remapnum];
			input_code newcode = newtable[remapnum];

			/* loop over all default ports, remapping the requested keys */
			for (input_type_entry *entry = portdata->typelist.first(); entry != NULL; entry = entry->next())
			{
				/* remap anything in the default sequences */
				for (int seqtype = 0; seqtype < ARRAY_LENGTH(entry->seq); seqtype++)
					entry->seq[seqtype].replace(oldcode, newcode);
			}
		}

		/* release the tables */
		global_free(oldtable);
		global_free(newtable);
	}
}


/*-------------------------------------------------
    load_default_config - apply configuration
    data to the default mappings
-------------------------------------------------*/

static int load_default_config(running_machine &machine, xml_data_node *portnode, int type, int player, const input_seq *newseq)
{
	input_port_private *portdata = machine.input_port_data;

	/* find a matching port in the list */
	for (input_type_entry *entry = portdata->typelist.first(); entry != NULL; entry = entry->next())
		if (entry->type == type && entry->player == player)
		{
			for (int seqtype = 0; seqtype < ARRAY_LENGTH(entry->seq); seqtype++)
				if (newseq[seqtype][0] != INPUT_CODE_INVALID)
					entry->seq[seqtype] = newseq[seqtype];
			return TRUE;
		}

	return FALSE;
}


/*-------------------------------------------------
    load_game_config - apply configuration
    data to the current set of input ports
-------------------------------------------------*/

static int load_game_config(running_machine &machine, xml_data_node *portnode, int type, int player, const input_seq *newseq)
{
	input_port_value mask, defvalue;
	const input_field_config *field;
	const input_port_config *port;
	char tempbuffer[20];
	const char *tag;

	/* read the mask, index, and defvalue attributes */
	tag = xml_get_attribute_string(portnode, "tag", NULL);
	mask = xml_get_attribute_int(portnode, "mask", 0);
	defvalue = xml_get_attribute_int(portnode, "defvalue", 0);

	/* find the port we want; if no tag, search them all */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		if (tag == NULL || strcmp(get_port_tag(port, tempbuffer), tag) == 0)
			for (field = port->first_field(); field != NULL; field = field->next())

				/* find the matching mask and defvalue */
				if (field->type == type && field->player == player &&
					field->mask == mask && (field->defvalue & mask) == (defvalue & mask))
				{
					const char *revstring;
					int seqtype;

					/* if a sequence was specified, copy it in */
					for (seqtype = 0; seqtype < ARRAY_LENGTH(field->state->seq); seqtype++)
						if (newseq[seqtype][0] != INPUT_CODE_INVALID)
							field->state->seq[seqtype] = newseq[seqtype];

					/* for non-analog fields, fetch the value */
					if (field->state->analog == NULL)
						field->state->value = xml_get_attribute_int(portnode, "value", field->defvalue);

					/* for analog fields, fetch configurable analog attributes */
					else
					{
						/* get base attributes */
						field->state->analog->delta = xml_get_attribute_int(portnode, "keydelta", field->delta);
						field->state->analog->centerdelta = xml_get_attribute_int(portnode, "centerdelta", field->centerdelta);
						field->state->analog->sensitivity = xml_get_attribute_int(portnode, "sensitivity", field->sensitivity);

						/* fetch yes/no for reverse setting */
						revstring = xml_get_attribute_string(portnode, "reverse", NULL);
						if (revstring != NULL)
							field->state->analog->reverse = (strcmp(revstring, "yes") == 0);
					}
					return TRUE;
				}

	return FALSE;
}



/***************************************************************************
    SETTINGS SAVE
***************************************************************************/

/*-------------------------------------------------
    save_config_callback - config callback for
    saving input port configuration
-------------------------------------------------*/

static void save_config_callback(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	/* if no parentnode, ignore */
	if (parentnode == NULL)
		return;

	/* default ports save differently */
	if (config_type == CONFIG_TYPE_DEFAULT)
		save_default_inputs(machine, parentnode);
	else
		save_game_inputs(machine, parentnode);
}


/*-------------------------------------------------
    save_sequence - add a node for an input
    sequence
-------------------------------------------------*/

static void save_sequence(running_machine &machine, xml_data_node *parentnode, int type, int porttype, const input_seq &seq)
{
	astring seqstring;
	xml_data_node *seqnode;

	/* get the string for the sequence */
	if (seq.length() == 0)
		seqstring.cpy("NONE");
	else
		machine.input().seq_to_tokens(seqstring, seq);

	/* add the new node */
	seqnode = xml_add_child(parentnode, "newseq", seqstring);
	if (seqnode != NULL)
		xml_set_attribute(seqnode, "type", seqtypestrings[type]);
}


/*-------------------------------------------------
    save_this_input_field_type - determine if the given
    port type is worth saving
-------------------------------------------------*/

static int save_this_input_field_type(int type)
{
	switch (type)
	{
		case IPT_UNUSED:
		case IPT_END:
		case IPT_PORT:
		case IPT_VBLANK:
		case IPT_UNKNOWN:
			return FALSE;
	}
	return TRUE;
}


/*-------------------------------------------------
    save_default_inputs - add nodes for any default
    mappings that have changed
-------------------------------------------------*/

static void save_default_inputs(running_machine &machine, xml_data_node *parentnode)
{
	input_port_private *portdata = machine.input_port_data;
	input_type_entry *entry;

	/* iterate over ports */
	for (entry = portdata->typelist.first(); entry != NULL; entry = entry->next())
	{
		/* only save if this port is a type we save */
		if (save_this_input_field_type(entry->type))
		{
			int seqtype;

			/* see if any of the sequences have changed */
			for (seqtype = 0; seqtype < ARRAY_LENGTH(entry->seq); seqtype++)
				if (entry->seq[seqtype] != entry->defseq[seqtype])
					break;

			/* if so, we need to add a node */
			if (seqtype < ARRAY_LENGTH(entry->seq))
			{
				/* add a new port node */
				xml_data_node *portnode = xml_add_child(parentnode, "port", NULL);
				if (portnode != NULL)
				{
					/* add the port information and attributes */
					xml_set_attribute(portnode, "type", input_field_type_to_token(machine, entry->type, entry->player));

					/* add only the sequences that have changed from the defaults */
					for (seqtype = 0; seqtype < ARRAY_LENGTH(entry->seq); seqtype++)
						if (entry->seq[seqtype] != entry->defseq[seqtype])
							save_sequence(machine, portnode, seqtype, entry->type, entry->seq[seqtype]);
				}
			}
		}
	}
}


/*-------------------------------------------------
    save_game_inputs - add nodes for any game
    mappings that have changed
-------------------------------------------------*/

static void save_game_inputs(running_machine &machine, xml_data_node *parentnode)
{
	const input_field_config *field;
	const input_port_config *port;

	/* iterate over ports */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (save_this_input_field_type(field->type))
			{
				bool changed = false;
				int seqtype;

				/* determine if we changed */
				for (seqtype = 0; seqtype < ARRAY_LENGTH(field->state->seq); seqtype++)
					changed |= (field->state->seq[seqtype] != field->seq[seqtype]);

				/* non-analog changes */
				if (field->state->analog == NULL)
					changed |= ((field->state->value & field->mask) != (field->defvalue & field->mask));

				/* analog changes */
				else
				{
					changed |= (field->state->analog->delta != field->delta);
					changed |= (field->state->analog->centerdelta != field->centerdelta);
					changed |= (field->state->analog->sensitivity != field->sensitivity);
					changed |= (field->state->analog->reverse != ((field->flags & ANALOG_FLAG_REVERSE) != 0));
				}

				/* if we did change, add a new node */
				if (changed)
				{
					/* add a new port node */
					xml_data_node *portnode = xml_add_child(parentnode, "port", NULL);
					if (portnode != NULL)
					{
						char tempbuffer[20];

						/* add the identifying information and attributes */
						xml_set_attribute(portnode, "tag", get_port_tag(port, tempbuffer));
						xml_set_attribute(portnode, "type", input_field_type_to_token(machine, field->type, field->player));
						xml_set_attribute_int(portnode, "mask", field->mask);
						xml_set_attribute_int(portnode, "defvalue", field->defvalue & field->mask);

						/* add sequences if changed */
						for (seqtype = 0; seqtype < ARRAY_LENGTH(field->state->seq); seqtype++)
							if (field->state->seq[seqtype] != field->seq[seqtype])
								save_sequence(machine, portnode, seqtype, field->type, field->state->seq[seqtype]);

						/* write out non-analog changes */
						if (field->state->analog == NULL)
						{
							if ((field->state->value & field->mask) != (field->defvalue & field->mask))
								xml_set_attribute_int(portnode, "value", field->state->value & field->mask);
						}

						/* write out analog changes */
						else
						{
							if (field->state->analog->delta != field->delta)
								xml_set_attribute_int(portnode, "keydelta", field->state->analog->delta);
							if (field->state->analog->centerdelta != field->centerdelta)
								xml_set_attribute_int(portnode, "centerdelta", field->state->analog->centerdelta);
							if (field->state->analog->sensitivity != field->sensitivity)
								xml_set_attribute_int(portnode, "sensitivity", field->state->analog->sensitivity);
							if (field->state->analog->reverse != ((field->flags & ANALOG_FLAG_REVERSE) != 0))
								xml_set_attribute(portnode, "reverse", field->state->analog->reverse ? "yes" : "no");
						}
					}
				}
			}
}



/***************************************************************************
    INPUT PLAYBACK
***************************************************************************/

/*-------------------------------------------------
    playback_read_uint8 - read an 8-bit value
    from the playback file
-------------------------------------------------*/

static UINT8 playback_read_uint8(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	UINT8 result;

	/* protect against NULL handles if previous reads fail */
	if (portdata->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (portdata->playback_file->read(&result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return result;
}


/*-------------------------------------------------
    playback_read_uint32 - read a 32-bit value
    from the playback file
-------------------------------------------------*/

static UINT32 playback_read_uint32(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	UINT32 result;

	/* protect against NULL handles if previous reads fail */
	if (portdata->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (portdata->playback_file->read(&result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return LITTLE_ENDIANIZE_INT32(result);
}


/*-------------------------------------------------
    playback_read_uint64 - read a 64-bit value
    from the playback file
-------------------------------------------------*/

static UINT64 playback_read_uint64(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	UINT64 result;

	/* protect against NULL handles if previous reads fail */
	if (portdata->playback_file == NULL)
		return 0;

	/* read the value; if we fail, end playback */
	if (portdata->playback_file->read(&result, sizeof(result)) != sizeof(result))
	{
		playback_end(machine, "End of file");
		return 0;
	}

	/* return the appropriate value */
	return LITTLE_ENDIANIZE_INT64(result);
}


/*-------------------------------------------------
    playback_init - initialize INP playback
-------------------------------------------------*/

static time_t playback_init(running_machine &machine)
{
	const char *filename = machine.options().playback();
	input_port_private *portdata = machine.input_port_data;
	UINT8 header[INP_HEADER_SIZE];
	time_t basetime;

	/* if no file, nothing to do */
	if (filename[0] == 0)
		return 0;

	/* open the playback file */
	portdata->playback_file = auto_alloc(machine, emu_file(machine.options().input_directory(), OPEN_FLAG_READ));
	file_error filerr = portdata->playback_file->open(filename);
	assert_always(filerr == FILERR_NONE, "Failed to open file for playback");

	/* read the header and verify that it is a modern version; if not, print an error */
	if (portdata->playback_file->read(header, sizeof(header)) != sizeof(header))
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
	if (memcmp(machine.system().name, header + 0x14, strlen(machine.system().name) + 1) != 0)
		mame_printf_info("Input file is for %s '%s', not for current %s '%s'\n", emulator_info::get_gamenoun(), header + 0x14, emulator_info::get_gamenoun(), machine.system().name);

	/* enable compression */
	portdata->playback_file->compress(FCOMPRESS_MEDIUM);

	return basetime;
}


/*-------------------------------------------------
    playback_end - end INP playback
-------------------------------------------------*/

static void playback_end(running_machine &machine, const char *message)
{
	input_port_private *portdata = machine.input_port_data;

	/* only applies if we have a live file */
	if (portdata->playback_file != NULL)
	{
		/* close the file */
		auto_free(machine, portdata->playback_file);
		portdata->playback_file = NULL;

		/* pop a message */
		if (message != NULL)
			popmessage("Playback Ended\nReason: %s", message);

		/* display speed stats */
		portdata->playback_accumulated_speed /= portdata->playback_accumulated_frames;
		mame_printf_info("Total playback frames: %d\n", (UINT32)portdata->playback_accumulated_frames);
		mame_printf_info("Average recorded speed: %d%%\n", (UINT32)((portdata->playback_accumulated_speed * 200 + 1) >> 21));
	}
}


/*-------------------------------------------------
    playback_frame - start of frame callback for
    playback
-------------------------------------------------*/

static void playback_frame(running_machine &machine, attotime curtime)
{
	input_port_private *portdata = machine.input_port_data;

	/* if playing back, fetch the information and verify */
	if (portdata->playback_file != NULL)
	{
		attotime readtime;

		/* first the absolute time */
		readtime.seconds = playback_read_uint32(machine);
		readtime.attoseconds = playback_read_uint64(machine);
		if (readtime != curtime)
			playback_end(machine, "Out of sync");

		/* then the speed */
		portdata->playback_accumulated_speed += playback_read_uint32(machine);
		portdata->playback_accumulated_frames++;
	}
}


/*-------------------------------------------------
    playback_port - per-port callback for playback
-------------------------------------------------*/

static void playback_port(const input_port_config *port)
{
	input_port_private *portdata = port->machine().input_port_data;

	/* if playing back, fetch information about this port */
	if (portdata->playback_file != NULL)
	{
		analog_field_state *analog;

		/* read the default value and the digital state */
		port->state->defvalue = playback_read_uint32(port->machine());
		port->state->digital = playback_read_uint32(port->machine());

		/* loop over analog ports and save their data */
		for (analog = port->state->analoglist; analog != NULL; analog = analog->next)
		{
			/* read current and previous values */
			analog->accum = playback_read_uint32(port->machine());
			analog->previous = playback_read_uint32(port->machine());

			/* read configuration information */
			analog->sensitivity = playback_read_uint32(port->machine());
			analog->reverse = playback_read_uint8(port->machine());
		}
	}
}



/***************************************************************************
    INPUT RECORDING
***************************************************************************/

/*-------------------------------------------------
    record_write_uint8 - write an 8-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint8(running_machine &machine, UINT8 data)
{
	input_port_private *portdata = machine.input_port_data;
	UINT8 result = data;

	/* protect against NULL handles if previous reads fail */
	if (portdata->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (portdata->record_file->write(&result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    record_write_uint32 - write a 32-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint32(running_machine &machine, UINT32 data)
{
	input_port_private *portdata = machine.input_port_data;
	UINT32 result = LITTLE_ENDIANIZE_INT32(data);

	/* protect against NULL handles if previous reads fail */
	if (portdata->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (portdata->record_file->write(&result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    record_write_uint64 - write a 64-bit value
    to the record file
-------------------------------------------------*/

static void record_write_uint64(running_machine &machine, UINT64 data)
{
	input_port_private *portdata = machine.input_port_data;
	UINT64 result = LITTLE_ENDIANIZE_INT64(data);

	/* protect against NULL handles if previous reads fail */
	if (portdata->record_file == NULL)
		return;

	/* read the value; if we fail, end playback */
	if (portdata->record_file->write(&result, sizeof(result)) != sizeof(result))
		record_end(machine, "Out of space");
}


/*-------------------------------------------------
    record_init - initialize INP recording
-------------------------------------------------*/

static void record_init(running_machine &machine)
{
	const char *filename = machine.options().record();
	input_port_private *portdata = machine.input_port_data;
	UINT8 header[INP_HEADER_SIZE];
	system_time systime;

	/* if no file, nothing to do */
	if (filename[0] == 0)
		return;

	/* open the record file  */
	portdata->record_file = auto_alloc(machine, emu_file(machine.options().input_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS));
	file_error filerr = portdata->record_file->open(filename);
	assert_always(filerr == FILERR_NONE, "Failed to open file for recording");

	/* get the base time */
	machine.base_datetime(systime);

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
	strcpy((char *)header + 0x14, machine.system().name);
	sprintf((char *)header + 0x20, "%s %s", emulator_info::get_appname(), build_version);

	/* write it */
	portdata->record_file->write(header, sizeof(header));

	/* enable compression */
	portdata->record_file->compress(FCOMPRESS_MEDIUM);
}


/*-------------------------------------------------
    record_end - end INP recording
-------------------------------------------------*/

static void record_end(running_machine &machine, const char *message)
{
	input_port_private *portdata = machine.input_port_data;

	/* only applies if we have a live file */
	if (portdata->record_file != NULL)
	{
		/* close the file */
		auto_free(machine, portdata->record_file);
		portdata->record_file = NULL;

		/* pop a message */
		if (message != NULL)
			popmessage("Recording Ended\nReason: %s", message);
	}
}


/*-------------------------------------------------
    record_frame - start of frame callback for
    recording
-------------------------------------------------*/

static void record_frame(running_machine &machine, attotime curtime)
{
	input_port_private *portdata = machine.input_port_data;

	/* if recording, record information about the current frame */
	if (portdata->record_file != NULL)
	{
		/* first the absolute time */
		record_write_uint32(machine, curtime.seconds);
		record_write_uint64(machine, curtime.attoseconds);

		/* then the current speed */
		record_write_uint32(machine, machine.video().speed_percent() * (double)(1 << 20));
	}
}


/*-------------------------------------------------
    record_port - per-port callback for record
-------------------------------------------------*/

static void record_port(const input_port_config *port)
{
	input_port_private *portdata = port->machine().input_port_data;

	/* if recording, store information about this port */
	if (portdata->record_file != NULL)
	{
		analog_field_state *analog;

		/* store the default value and digital state */
		record_write_uint32(port->machine(), port->state->defvalue);
		record_write_uint32(port->machine(), port->state->digital);

		/* loop over analog ports and save their data */
		for (analog = port->state->analoglist; analog != NULL; analog = analog->next)
		{
			/* store current and previous values */
			record_write_uint32(port->machine(), analog->accum);
			record_write_uint32(port->machine(), analog->previous);

			/* store configuration information */
			record_write_uint32(port->machine(), analog->sensitivity);
			record_write_uint8(port->machine(), analog->reverse);
		}
	}
}

int input_machine_has_keyboard(running_machine &machine)
{
	int have_keyboard = FALSE;
	const input_field_config *field;
	const input_port_config *port;
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
	{
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			// if we are at init, check IPT_KEYBOARD for inputx_init
			if (!port->machine().input_port_data->safe_to_read && field->type == IPT_KEYBOARD)
			{
				have_keyboard = TRUE;
				break;
			}

			// else, check if there is a keyboard and if such a keyboard is enabled
			if (field->type == IPT_KEYBOARD && input_condition_true(field->machine(), &field->condition, field->port().owner()))
			{
				have_keyboard = TRUE;
				break;
			}
		}
	}
	return have_keyboard;
}

/***************************************************************************
    CODE ASSEMBLING
***************************************************************************/

/*-------------------------------------------------
    code_point_string - obtain a string representation of a
    given code; used for logging and debugging
-------------------------------------------------*/

static const char *code_point_string(running_machine &machine, unicode_char ch)
{
	static char buf[16];
	const char *result = buf;

	switch(ch)
	{
		/* check some magic values */
		case '\0':	strcpy(buf, "\\0");		break;
		case '\r':	strcpy(buf, "\\r");		break;
		case '\n':	strcpy(buf, "\\n");		break;
		case '\t':	strcpy(buf, "\\t");		break;

		default:
			if ((ch >= 32) && (ch < 128))
			{
				/* seven bit ASCII is easy */
				buf[0] = (char) ch;
				buf[1] = '\0';
			}
			else if (ch >= UCHAR_MAMEKEY_BEGIN)
			{
				/* try to obtain a codename with code_name(); this can result in an empty string */
				input_code code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(ch - UCHAR_MAMEKEY_BEGIN));
				astring tempstr;
				snprintf(buf, ARRAY_LENGTH(buf), "%s", machine.input().code_name(tempstr, code));
			}
			else
			{
				/* empty string; resolve later */
				buf[0] = '\0';
			}

			/* did we fail to resolve? if so, we have a last resort */
			if (buf[0] == '\0')
				snprintf(buf, ARRAY_LENGTH(buf), "U+%04X", (unsigned) ch);
			break;
	}
	return result;
}


/*-------------------------------------------------
    scan_keys - scans through input ports and
    sets up natural keyboard input mapping
-------------------------------------------------*/

static int scan_keys(running_machine &machine, const input_port_config *portconfig, inputx_code *codes, const input_port_config * *ports, const input_field_config * *shift_ports, int keys, int shift)
{
	int code_count = 0;
	const input_port_config *port;
	const input_field_config *field;
	unicode_char code;

	assert(keys < NUM_SIMUL_KEYS);

	for (port = portconfig; port != NULL; port = port->next())
	{
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			if (field->type == IPT_KEYBOARD)
			{
				code = get_keyboard_code(field, shift);
				if (code != 0)
				{
					/* is this a shifter key? */
					if ((code >= UCHAR_SHIFT_BEGIN) && (code <= UCHAR_SHIFT_END))
					{
						shift_ports[keys] = field;
						code_count += scan_keys(machine,
							portconfig,
							codes ? &codes[code_count] : NULL,
							ports,
							shift_ports,
							keys+1,
							code - UCHAR_SHIFT_1 + 1);
					}
					else
					{
						/* not a shifter key; record normally */
						if (codes)
						{
							/* if we have a destination, record the codes used here */
							memcpy((void *) codes[code_count].field, shift_ports, sizeof(shift_ports[0]) * keys);
							codes[code_count].ch = code;
							codes[code_count].field[keys] = field;
						}

						/* increment the count */
						code_count++;

						if (LOG_INPUTX)
							logerror("inputx: code=%i (%s) port=%p field->name='%s'\n", (int) code, code_point_string(machine, code), port, field->name);
					}
				}
			}
		}
	}
	return code_count;
}



/*-------------------------------------------------
    build_codes - given an input port table, create
    a input code table useful for mapping unicode
    chars
-------------------------------------------------*/

static inputx_code *build_codes(running_machine &machine, const input_port_config *portconfig)
{
	inputx_code *codes = NULL;
	const input_port_config *ports[NUM_SIMUL_KEYS];
	const input_field_config *fields[NUM_SIMUL_KEYS];
	int code_count;

	/* first count the number of codes */
	code_count = scan_keys(machine, portconfig, NULL, ports, fields, 0, 0);
	if (code_count > 0)
	{
		/* allocate the codes */
		codes = auto_alloc_array_clear(machine, inputx_code, code_count + 1);

		/* and populate them */
		scan_keys(machine, portconfig, codes, ports, fields, 0, 0);
	}
	return codes;
}



/***************************************************************************
    VALIDITY CHECKS
***************************************************************************/

/*-------------------------------------------------
    validate_natural_keyboard_statics -
    validates natural keyboard static data
-------------------------------------------------*/

int validate_natural_keyboard_statics(void)
{
	int i;
	int error = FALSE;
	unicode_char last_char = 0;
	const char_info *ci;

	/* check to make sure that charinfo is in order */
	for (i = 0; i < ARRAY_LENGTH(charinfo); i++)
	{
		if (last_char >= charinfo[i].ch)
		{
			mame_printf_error("inputx: charinfo is out of order; 0x%08x should be higher than 0x%08x\n", charinfo[i].ch, last_char);
			error = TRUE;
		}
		last_char = charinfo[i].ch;
	}

	/* check to make sure that I can look up everything on alternate_charmap */
	for (i = 0; i < ARRAY_LENGTH(charinfo); i++)
	{
		ci = find_charinfo(charinfo[i].ch);
		if (ci != &charinfo[i])
		{
			mame_printf_error("inputx: expected find_charinfo(0x%08x) to work properly\n", charinfo[i].ch);
			error = TRUE;
		}
	}
	return error;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

static void clear_keybuffer(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	portdata->keybuffer.buffer = NULL;
	portdata->queue_chars = NULL;
	portdata->codes = NULL;
}



static void setup_keybuffer(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	portdata->inputx_timer = machine.scheduler().timer_alloc(FUNC(inputx_timerproc));
	portdata->keybuffer.begin_pos = 0;
	portdata->keybuffer.end_pos = 0;
	portdata->keybuffer.status_keydown = 0;
	portdata->keybuffer.size = KEY_BUFFER_SIZE;
	portdata->keybuffer.buffer = auto_alloc_array(machine, unicode_char, portdata->keybuffer.size);
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(clear_keybuffer), &machine));
}



void inputx_init(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	portdata->inputx_timer = NULL;
	portdata->accept_char = NULL;
	portdata->charqueue_empty = NULL;
	clear_keybuffer(machine);

	if (machine.debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine, "input", CMDFLAG_NONE, 0, 1, 1, execute_input);
		debug_console_register_command(machine, "dumpkbd", CMDFLAG_NONE, 0, 0, 1, execute_dumpkbd);
	}

	/* posting keys directly only makes sense for a computer */
	if (input_machine_has_keyboard(machine))
	{
		portdata->codes = build_codes(machine, machine.m_portlist.first());
		setup_keybuffer(machine);
	}
}



void inputx_setup_natural_keyboard(
	running_machine &machine,
	int (*queue_chars)(running_machine &machine, const unicode_char *text, size_t text_len),
	int (*accept_char)(running_machine &machine, unicode_char ch),
	int (*charqueue_empty)(running_machine &machine))
{
	input_port_private *portdata = machine.input_port_data;
	portdata->queue_chars = queue_chars;
	portdata->accept_char = accept_char;
	portdata->charqueue_empty = charqueue_empty;
}

int inputx_can_post(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	return portdata->queue_chars || portdata->codes;
}


static int can_post_key_directly(running_machine &machine, unicode_char ch)
{
	input_port_private *portdata = machine.input_port_data;
	int rc = FALSE;
	const inputx_code *code;

	if (portdata->queue_chars)
	{
		rc = portdata->accept_char ? (*portdata->accept_char)(machine, ch) : TRUE;
	}
	else
	{
		code = find_code(portdata->codes, ch);
		if (code)
			rc = code->field[0] != NULL;
	}
	return rc;
}



static int can_post_key_alternate(running_machine &machine, unicode_char ch)
{
	const char *s;
	const char_info *ci;
	unicode_char uchar;
	int rc;

	ci = find_charinfo(ch);
	s = ci ? ci->alternate : NULL;
	if (!s)
		return 0;

	while(*s)
	{
		rc = uchar_from_utf8(&uchar, s, strlen(s));
		if (rc <= 0)
			return 0;
		if (!can_post_key_directly(machine, uchar))
			return 0;
		s += rc;
	}
	return 1;
}

static attotime choose_delay(input_port_private *portdata, unicode_char ch)
{
	if (portdata->current_rate != attotime::zero)
		return portdata->current_rate;

	attotime delay = attotime::zero;
	if (portdata->queue_chars)
	{
		/* systems with queue_chars can afford a much smaller delay */
		delay = attotime::from_msec(10);
	}
	else
	{
		switch(ch) {
		case '\r':
			delay = attotime::from_msec(200);
			break;

		default:
			delay = attotime::from_msec(50);
			break;
		}
	}
	return delay;
}



static void internal_post_key(running_machine &machine, unicode_char ch)
{
	input_port_private *portdata = machine.input_port_data;
	key_buffer *keybuf;

	keybuf = get_buffer(machine);

	/* need to start up the timer? */
	if (keybuf->begin_pos == keybuf->end_pos)
	{
		portdata->inputx_timer->adjust(choose_delay(portdata, ch));
		keybuf->status_keydown = 0;
	}

	keybuf->buffer[keybuf->end_pos++] = ch;
	if ((keybuf->end_pos+1) % keybuf->size == keybuf->begin_pos)
	{
		// Buffer full
		unicode_char *old_buffer = keybuf->buffer;
		keybuf->size = keybuf->size + KEY_BUFFER_SIZE;
		keybuf->buffer = auto_alloc_array(machine, unicode_char, keybuf->size);
		for( int i = keybuf->begin_pos; i <= keybuf->end_pos; i++ )
		{
			keybuf->buffer[i] = old_buffer[i];
		}
		auto_free(machine, old_buffer);
	}
	keybuf->end_pos %= keybuf->size;
}



static int buffer_full(running_machine &machine)
{
	key_buffer *keybuf;
	keybuf = get_buffer(machine);
	return ((keybuf->end_pos + 1) % keybuf->size) == keybuf->begin_pos;
}



static void inputx_postn_rate(running_machine &machine, const unicode_char *text, size_t text_len, attotime rate)
{
	input_port_private *portdata = machine.input_port_data;
	int last_cr = 0;
	unicode_char ch;
	const char *s;
	const char_info *ci;
	const inputx_code *code;

	portdata->current_rate = rate;

	if (inputx_can_post(machine))
	{
		while((text_len > 0) && !buffer_full(machine))
		{
			ch = *(text++);
			text_len--;

			/* change all eolns to '\r' */
			if ((ch != '\n') || !last_cr)
			{
				if (ch == '\n')
					ch = '\r';
				else
					last_cr = (ch == '\r');

				if (LOG_INPUTX)
				{
					code = find_code(portdata->codes, ch);
					logerror("inputx_postn(): code=%i (%s) field->name='%s'\n", (int) ch, code_point_string(machine, ch), (code && code->field[0]) ? code->field[0]->name : "<null>");
				}

				if (can_post_key_directly(machine, ch))
				{
					/* we can post this key in the queue directly */
					internal_post_key(machine, ch);
				}
				else if (can_post_key_alternate(machine, ch))
				{
					/* we can post this key with an alternate representation */
					ci = find_charinfo(ch);
					assert(ci && ci->alternate);
					s = ci->alternate;
					while(*s)
					{
						s += uchar_from_utf8(&ch, s, strlen(s));
						internal_post_key(machine, ch);
					}
				}
			}
			else
			{
				last_cr = 0;
			}
		}
	}
}



static TIMER_CALLBACK(inputx_timerproc)
{
	input_port_private *portdata = machine.input_port_data;
	key_buffer *keybuf;
	attotime delay;

	keybuf = get_buffer(machine);

	if (portdata->queue_chars)
	{
		/* the driver has a queue_chars handler */
		while((keybuf->begin_pos != keybuf->end_pos) && (*portdata->queue_chars)(machine, &keybuf->buffer[keybuf->begin_pos], 1))
		{
			keybuf->begin_pos++;
			keybuf->begin_pos %= keybuf->size;

			if (portdata->current_rate != attotime::zero)
				break;
		}
	}
	else
	{
		/* the driver does not have a queue_chars handler */
		if (keybuf->status_keydown)
		{
			keybuf->status_keydown = FALSE;
			keybuf->begin_pos++;
			keybuf->begin_pos %= keybuf->size;
		}
		else
		{
			keybuf->status_keydown = TRUE;
		}
	}

	/* need to make sure timerproc is called again if buffer not empty */
	if (keybuf->begin_pos != keybuf->end_pos)
	{
		delay = choose_delay(portdata, keybuf->buffer[keybuf->begin_pos]);
		portdata->inputx_timer->adjust(delay);
	}
}

int inputx_is_posting(running_machine &machine)
{
	input_port_private *portdata = machine.input_port_data;
	const key_buffer *keybuf;
	keybuf = get_buffer(machine);
	return (keybuf->begin_pos != keybuf->end_pos) || (portdata->charqueue_empty && !(*portdata->charqueue_empty)(machine));
}

/***************************************************************************

    Coded input

***************************************************************************/
static void inputx_postc_rate(running_machine &machine, unicode_char ch, attotime rate);

static void inputx_postn_coded_rate(running_machine &machine, const char *text, size_t text_len, attotime rate)
{
	size_t i, j, key_len, increment;
	unicode_char ch;

	static const struct
	{
		const char *key;
		unicode_char code;
	} codes[] =
	{
		{ "BACKSPACE",	8 },
		{ "BS",			8 },
		{ "BKSP",		8 },
		{ "DEL",		UCHAR_MAMEKEY(DEL) },
		{ "DELETE",		UCHAR_MAMEKEY(DEL) },
		{ "END",		UCHAR_MAMEKEY(END) },
		{ "ENTER",		13 },
		{ "ESC",		'\033' },
		{ "HOME",		UCHAR_MAMEKEY(HOME) },
		{ "INS",		UCHAR_MAMEKEY(INSERT) },
		{ "INSERT",		UCHAR_MAMEKEY(INSERT) },
		{ "PGDN",		UCHAR_MAMEKEY(PGDN) },
		{ "PGUP",		UCHAR_MAMEKEY(PGUP) },
		{ "SPACE",		32 },
		{ "TAB",		9 },
		{ "F1",			UCHAR_MAMEKEY(F1) },
		{ "F2",			UCHAR_MAMEKEY(F2) },
		{ "F3",			UCHAR_MAMEKEY(F3) },
		{ "F4",			UCHAR_MAMEKEY(F4) },
		{ "F5",			UCHAR_MAMEKEY(F5) },
		{ "F6",			UCHAR_MAMEKEY(F6) },
		{ "F7",			UCHAR_MAMEKEY(F7) },
		{ "F8",			UCHAR_MAMEKEY(F8) },
		{ "F9",			UCHAR_MAMEKEY(F9) },
		{ "F10",		UCHAR_MAMEKEY(F10) },
		{ "F11",		UCHAR_MAMEKEY(F11) },
		{ "F12",		UCHAR_MAMEKEY(F12) },
		{ "QUOTE",		'\"' }
	};

	i = 0;
	while(i < text_len)
	{
		ch = text[i];
		increment = 1;

		if (ch == '{')
		{
			for (j = 0; j < ARRAY_LENGTH(codes); j++)
			{
				key_len = strlen(codes[j].key);
				if (i + key_len + 2 <= text_len)
				{
					if (!core_strnicmp(codes[j].key, &text[i + 1], key_len) && (text[i + key_len + 1] == '}'))
					{
						ch = codes[j].code;
						increment = key_len + 2;
					}
				}
			}
		}

		if (ch)
			inputx_postc_rate(machine, ch, rate);
		i += increment;
	}
}



/***************************************************************************

    Alternative calls

***************************************************************************/

static void inputx_postc_rate(running_machine &machine, unicode_char ch, attotime rate)
{
	inputx_postn_rate(machine, &ch, 1, rate);
}

void inputx_postc(running_machine &machine, unicode_char ch)
{
	inputx_postc_rate(machine, ch, attotime::zero);
}

static void inputx_postn_utf8_rate(running_machine &machine, const char *text, size_t text_len, attotime rate)
{
	size_t len = 0;
	unicode_char buf[256];
	unicode_char c;
	int rc;

	while(text_len > 0)
	{
		if (len == ARRAY_LENGTH(buf))
		{
			inputx_postn_rate(machine, buf, len, attotime::zero);
			len = 0;
		}

		rc = uchar_from_utf8(&c, text, text_len);
		if (rc < 0)
		{
			rc = 1;
			c = INVALID_CHAR;
		}
		text += rc;
		text_len -= rc;
		buf[len++] = c;
	}
	inputx_postn_rate(machine, buf, len, rate);
}

void inputx_post_utf8(running_machine &machine, const char *text)
{
	inputx_postn_utf8_rate(machine, text, strlen(text), attotime::zero);
}

void inputx_post_utf8_rate(running_machine &machine, const char *text, attotime rate)
{
	inputx_postn_utf8_rate(machine, text, strlen(text), rate);
}

/***************************************************************************

    Other stuff

    This stuff is here more out of convienience than anything else
***************************************************************************/

int input_classify_port(const input_field_config *field)
{
	int result;

	switch(field->type)
	{
		case IPT_JOYSTICK_UP:
		case IPT_JOYSTICK_DOWN:
		case IPT_JOYSTICK_LEFT:
		case IPT_JOYSTICK_RIGHT:
		case IPT_JOYSTICKLEFT_UP:
		case IPT_JOYSTICKLEFT_DOWN:
		case IPT_JOYSTICKLEFT_LEFT:
		case IPT_JOYSTICKLEFT_RIGHT:
		case IPT_JOYSTICKRIGHT_UP:
		case IPT_JOYSTICKRIGHT_DOWN:
		case IPT_JOYSTICKRIGHT_LEFT:
		case IPT_JOYSTICKRIGHT_RIGHT:
		case IPT_BUTTON1:
		case IPT_BUTTON2:
		case IPT_BUTTON3:
		case IPT_BUTTON4:
		case IPT_BUTTON5:
		case IPT_BUTTON6:
		case IPT_BUTTON7:
		case IPT_BUTTON8:
		case IPT_BUTTON9:
		case IPT_BUTTON10:
		case IPT_AD_STICK_X:
		case IPT_AD_STICK_Y:
		case IPT_AD_STICK_Z:
		case IPT_TRACKBALL_X:
		case IPT_TRACKBALL_Y:
		case IPT_LIGHTGUN_X:
		case IPT_LIGHTGUN_Y:
		case IPT_MOUSE_X:
		case IPT_MOUSE_Y:
		case IPT_START:
		case IPT_SELECT:
			result = INPUT_CLASS_CONTROLLER;
			break;

		case IPT_KEYPAD:
		case IPT_KEYBOARD:
			result = INPUT_CLASS_KEYBOARD;
			break;

		case IPT_CONFIG:
			result = INPUT_CLASS_CONFIG;
			break;

		case IPT_DIPSWITCH:
			result = INPUT_CLASS_DIPSWITCH;
			break;

		case 0:
			if (field->name && (field->name != (const char *) -1))
				result = INPUT_CLASS_MISC;
			else
				result = INPUT_CLASS_INTERNAL;
			break;

		default:
			result = INPUT_CLASS_INTERNAL;
			break;
	}
	return result;
}



int input_player_number(const input_field_config *port)
{
	return port->player;
}



/*-------------------------------------------------
    input_has_input_class - checks to see if a
    particular input class is present
-------------------------------------------------*/

int input_has_input_class(running_machine &machine, int inputclass)
{
	const input_port_config *port;
	const input_field_config *field;

	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
	{
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			if (input_classify_port(field) == inputclass)
				return TRUE;
		}
	}
	return FALSE;
}



/*-------------------------------------------------
    input_count_players - counts the number of
    active players
-------------------------------------------------*/

int input_count_players(running_machine &machine)
{
	const input_port_config *port;
	const input_field_config *field;
	int joystick_count;

	joystick_count = 0;
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
	{
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			if (input_classify_port(field) == INPUT_CLASS_CONTROLLER)
			{
				if (joystick_count <= field->player + 1)
					joystick_count = field->player + 1;
			}
		}
	}
	return joystick_count;
}



/***************************************************************************
    DEBUGGER SUPPORT
***************************************************************************/

/*-------------------------------------------------
    execute_input - debugger command to enter
    natural keyboard input
-------------------------------------------------*/

static void execute_input(running_machine &machine, int ref, int params, const char *param[])
{
	inputx_postn_coded_rate(machine, param[0], strlen(param[0]), attotime::zero);
}



/*-------------------------------------------------
    execute_dumpkbd - debugger command to natural
    keyboard codes
-------------------------------------------------*/

static void execute_dumpkbd(running_machine &machine, int ref, int params, const char *param[])
{
	inputx_code *codes = machine.input_port_data->codes;
	const char *filename;
	FILE *file = NULL;
	const inputx_code *code;
	char buffer[512];
	size_t pos;
	int i, j;
	size_t left_column_width = 24;

	/* was there a file specified? */
	filename = (params > 0) ? param[0] : NULL;
	if (filename != NULL)
	{
		/* if so, open it */
		file = fopen(filename, "w");
		if (file == NULL)
		{
			debug_console_printf(machine, "Cannot open \"%s\"\n", filename);
			return;
		}
	}

	if ((codes != NULL) && (codes[0].ch != 0))
	{
		/* loop through all codes */
		for (i = 0; codes[i].ch; i++)
		{
			code = &codes[i];
			pos = 0;

			/* describe the character code */
			pos += snprintf(&buffer[pos], ARRAY_LENGTH(buffer) - pos, "%08X (%s) ",
				code->ch,
				code_point_string(machine, code->ch));

			/* pad with spaces */
			while(pos < left_column_width)
				buffer[pos++] = ' ';
			buffer[pos] = '\0';

			/* identify the keys used */
			for (j = 0; j < ARRAY_LENGTH(code->field) && (code->field[j] != NULL); j++)
			{
				pos += snprintf(&buffer[pos], ARRAY_LENGTH(buffer) - pos, "%s'%s'",
					(j > 0) ? ", " : "",
					code->field[j]->name);
			}

			/* and output it as appropriate */
			if (file != NULL)
				fprintf(file, "%s\n", buffer);
			else
				debug_console_printf(machine, "%s\n", buffer);
		}
	}
	else
	{
		debug_console_printf(machine, "No natural keyboard support\n");
	}

	/* cleanup */
	if (file != NULL)
		fclose(file);

}



input_port_config *ioconfig_alloc_port(ioport_list &portlist, device_t &device, const char *tag)
{
	astring fulltag;
	device.subtag(fulltag, tag);
mame_printf_verbose("ioport '%s' created\n", fulltag.cstr());
	return &portlist.append(fulltag, *global_alloc(input_port_config(device, fulltag)));
}

input_port_config *ioconfig_modify_port(ioport_list &portlist, device_t &device, const char *tag)
{
	astring fulltag;
	device.subtag(fulltag, tag);
	input_port_config *port = portlist.find(fulltag.cstr());
	if (port == NULL)
		throw emu_fatalerror("Requested to modify nonexistent port '%s'", fulltag.cstr());
	port->bump_modcount();
	return port;
}

input_field_config *ioconfig_alloc_field(input_port_config &port, int type, input_port_value defval, input_port_value mask, const char *name)
{
	if (&port == NULL)
		throw emu_fatalerror("INPUT_TOKEN_FIELD encountered with no active port (mask=%X defval=%X)\n", mask, defval); \
	if (type != IPT_UNKNOWN && type != IPT_UNUSED)
		port.active |= mask;
	if (type == IPT_DIPSWITCH || type == IPT_CONFIG)
		defval = port_default_value(port.tag(), mask, defval, port.owner());
	return &port.fieldlist().append(*global_alloc(input_field_config(port, type, defval, mask, input_port_string_from_token(name))));
}

input_field_config *ioconfig_alloc_onoff(input_port_config &port, const char *name, input_port_value defval, input_port_value mask, const char *diplocation, astring &errorbuf)
{
	input_field_config *curfield = ioconfig_alloc_field(port, IPT_DIPSWITCH, defval, mask, name);
	if (name == DEF_STR(Service_Mode))
	{
		curfield->flags |= FIELD_FLAG_TOGGLE;
		curfield->seq[SEQ_TYPE_STANDARD].set(KEYCODE_F2);
	}
	if (diplocation != NULL)
		diplocation_list_alloc(*curfield, diplocation, errorbuf);
	ioconfig_alloc_setting(*curfield, defval & mask, DEF_STR(Off));
	ioconfig_alloc_setting(*curfield, ~defval & mask, DEF_STR(On));
	return curfield;
}

input_setting_config *ioconfig_alloc_setting(input_field_config &field, input_port_value value, const char *name)
{
	return &field.settinglist().append(*global_alloc(input_setting_config(field, value, input_port_string_from_token(name))));
}

void ioconfig_field_add_char(input_field_config &field, unicode_char ch, astring &errorbuf)
{
	for (int index = 0; index < ARRAY_LENGTH(field.chars); index++)
		if (field.chars[index] == 0)
		{
			field.chars[index] = ch;
			break;
		}
}

void ioconfig_add_code(input_field_config &field, int which, input_code code)
{
	field.seq[which] |= code;
}



input_type_entry::input_type_entry(UINT32 _type, ioport_group _group, int _player, const char *_token, const char *_name, input_seq standard)
	: type(_type),
	  group(_group),
	  player(_player),
	  token(_token),
	  name(_name),
	  m_next(NULL)
{
	defseq[SEQ_TYPE_STANDARD] = seq[SEQ_TYPE_STANDARD] = standard;
}

input_type_entry::input_type_entry(UINT32 _type, ioport_group _group, int _player, const char *_token, const char *_name, input_seq standard, input_seq decrement, input_seq increment)
	: type(_type),
	  group(_group),
	  player(_player),
	  token(_token),
	  name(_name),
	  m_next(NULL)
{
	defseq[SEQ_TYPE_STANDARD] = seq[SEQ_TYPE_STANDARD] = standard;
	defseq[SEQ_TYPE_INCREMENT] = seq[SEQ_TYPE_INCREMENT] = increment;
	defseq[SEQ_TYPE_DECREMENT] = seq[SEQ_TYPE_DECREMENT] = decrement;
}
