/***************************************************************************

    input.c

    Handle input from the user.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    To do:
        * auto-selecting joystick configs
        * per-joystick configs?
        * test half-axis selections
        * add input test menu
        * get rid of osd_customize_inputport_list

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "profiler.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* joystick mapping codes */
#define JOYSTICK_MAP_NEUTRAL	0x00
#define JOYSTICK_MAP_LEFT		0x01
#define JOYSTICK_MAP_RIGHT		0x02
#define JOYSTICK_MAP_UP			0x04
#define JOYSTICK_MAP_DOWN		0x08
#define JOYSTICK_MAP_STICKY		0x0f

/* the largest number of tracked pressed switches for memory */
#define MAX_PRESSED_SWITCHES	64

/* invalid memory value for axis polling */
#define INVALID_AXIS_VALUE		0x7fffffff


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* single device item */
typedef struct _input_device_item input_device_item;
struct _input_device_item
{
	input_device_class		devclass;				/* device class of parent item */
	int						devindex;				/* device index of parent item */
	astring					name;					/* string name of item */
	astring					token;					/* tokenized name for non-standard items */
	void *					internal;				/* internal callback pointer */
	input_item_class		itemclass;				/* class of the item */
	input_item_id			itemid;					/* originally specified item id */
	item_get_state_func		getstate;				/* get state callback */
	INT32					current;				/* current raw value */
	INT32					memory;					/* "memory" value, to remember where we started during polling */
	INT32					oncelatch;				/* latched "once" value, cleared after each read  */

	/* keyboard information */
	INT32					steadykey;				/* the live steadykey state */
	INT32					oldkey;					/* old live state */
};


/* a 9x9 joystick map */
typedef struct _joystick_map joystick_map;
struct _joystick_map
{
	UINT8					map[9][9];				/* 9x9 grid */
};


/* a single input device */
struct _input_device
{
	running_machine *		machine;				/* machine we are attached to */
	astring					name;					/* string name of device */
	input_device_class		devclass;				/* class of this device */
	int						devindex;				/* device index of this device */
	input_device_item *		item[ITEM_ID_ABSOLUTE_MAXIMUM];	/* array of pointers to items */
	input_item_id			maxitem;				/* maximum item index */
	void *					internal;				/* internal callback pointer */

	/* joystick information */
	joystick_map			joymap;					/* joystick map for this device */
	UINT8					lastmap;				/* last joystick map value for this device */
};


/* a list of devices as an array */
typedef struct _input_device_list input_device_list;
struct _input_device_list
{
	input_device **			list;					/* the array */
	int						count;					/* elements in the array */
	UINT8					enabled;				/* is this class enabled? */
	UINT8					multi;					/* are multiple instances of this class allowed? */
};


/* code <-> string matching */
typedef struct _code_string_table code_string_table;
struct _code_string_table
{
	UINT32					code;
	const char *			string;
};


struct _input_private
{
	/* array of devices for each class */
	input_device_list	device_list[DEVICE_CLASS_MAXIMUM];
	input_code			code_pressed_memory[MAX_PRESSED_SWITCHES];

	/* device configuration */
	UINT8				steadykey_enabled;
	UINT8				lightgun_reload_button;
	const char *		joystick_map_default;
	INT32				joystick_deadzone;
	INT32				joystick_saturation;
};



/***************************************************************************
    TOKEN/STRING TABLES
***************************************************************************/

/* token strings for device classes */
static const code_string_table devclass_token_table[] =
{
	{ DEVICE_CLASS_KEYBOARD, "KEYCODE" },
	{ DEVICE_CLASS_MOUSE,    "MOUSECODE" },
	{ DEVICE_CLASS_LIGHTGUN, "GUNCODE" },
	{ DEVICE_CLASS_JOYSTICK, "JOYCODE" },
	{ ~0,                    "UNKCODE" }
};

/* friendly strings for device classes */
static const code_string_table devclass_string_table[] =
{
	{ DEVICE_CLASS_KEYBOARD, "Kbd" },
	{ DEVICE_CLASS_MOUSE,    "Mouse" },
	{ DEVICE_CLASS_LIGHTGUN, "Gun" },
	{ DEVICE_CLASS_JOYSTICK, "Joy" },
	{ ~0,                    "Unk" }
};

/* token strings for item modifiers */
static const code_string_table modifier_token_table[] =
{
	{ ITEM_MODIFIER_POS,     "POS" },
	{ ITEM_MODIFIER_NEG,     "NEG" },
	{ ITEM_MODIFIER_LEFT,    "LEFT" },
	{ ITEM_MODIFIER_RIGHT,   "RIGHT" },
	{ ITEM_MODIFIER_UP,      "UP" },
	{ ITEM_MODIFIER_DOWN,    "DOWN" },
	{ ~0,                    "" }
};

/* friendly strings for item modifiers */
static const code_string_table modifier_string_table[] =
{
	{ ITEM_MODIFIER_POS,     "+" },
	{ ITEM_MODIFIER_NEG,     "-" },
	{ ITEM_MODIFIER_LEFT,    "Left" },
	{ ITEM_MODIFIER_RIGHT,   "Right" },
	{ ITEM_MODIFIER_UP,      "Up" },
	{ ITEM_MODIFIER_DOWN,    "Down" },
	{ ~0,                    "" }
};

/* token strings for item classes */
static const code_string_table itemclass_token_table[] =
{
	{ ITEM_CLASS_SWITCH,     "SWITCH" },
	{ ITEM_CLASS_ABSOLUTE,   "ABSOLUTE" },
	{ ITEM_CLASS_RELATIVE,   "RELATIVE" },
	{ ~0,                    "" }
};

/* token strings for standard item ids */
static const code_string_table itemid_token_table[] =
{
	/* standard keyboard codes */
	{ ITEM_ID_A,             "A" },
	{ ITEM_ID_B,             "B" },
	{ ITEM_ID_C,             "C" },
	{ ITEM_ID_D,             "D" },
	{ ITEM_ID_E,             "E" },
	{ ITEM_ID_F,             "F" },
	{ ITEM_ID_G,             "G" },
	{ ITEM_ID_H,             "H" },
	{ ITEM_ID_I,             "I" },
	{ ITEM_ID_J,             "J" },
	{ ITEM_ID_K,             "K" },
	{ ITEM_ID_L,             "L" },
	{ ITEM_ID_M,             "M" },
	{ ITEM_ID_N,             "N" },
	{ ITEM_ID_O,             "O" },
	{ ITEM_ID_P,             "P" },
	{ ITEM_ID_Q,             "Q" },
	{ ITEM_ID_R,             "R" },
	{ ITEM_ID_S,             "S" },
	{ ITEM_ID_T,             "T" },
	{ ITEM_ID_U,             "U" },
	{ ITEM_ID_V,             "V" },
	{ ITEM_ID_W,             "W" },
	{ ITEM_ID_X,             "X" },
	{ ITEM_ID_Y,             "Y" },
	{ ITEM_ID_Z,             "Z" },
	{ ITEM_ID_0,             "0" },
	{ ITEM_ID_1,             "1" },
	{ ITEM_ID_2,             "2" },
	{ ITEM_ID_3,             "3" },
	{ ITEM_ID_4,             "4" },
	{ ITEM_ID_5,             "5" },
	{ ITEM_ID_6,             "6" },
	{ ITEM_ID_7,             "7" },
	{ ITEM_ID_8,             "8" },
	{ ITEM_ID_9,             "9" },
	{ ITEM_ID_F1,            "F1" },
	{ ITEM_ID_F2,            "F2" },
	{ ITEM_ID_F3,            "F3" },
	{ ITEM_ID_F4,            "F4" },
	{ ITEM_ID_F5,            "F5" },
	{ ITEM_ID_F6,            "F6" },
	{ ITEM_ID_F7,            "F7" },
	{ ITEM_ID_F8,            "F8" },
	{ ITEM_ID_F9,            "F9" },
	{ ITEM_ID_F10,           "F10" },
	{ ITEM_ID_F11,           "F11" },
	{ ITEM_ID_F12,           "F12" },
	{ ITEM_ID_F13,           "F13" },
	{ ITEM_ID_F14,           "F14" },
	{ ITEM_ID_F15,           "F15" },
	{ ITEM_ID_ESC,           "ESC" },
	{ ITEM_ID_TILDE,         "TILDE" },
	{ ITEM_ID_MINUS,         "MINUS" },
	{ ITEM_ID_EQUALS,        "EQUALS" },
	{ ITEM_ID_BACKSPACE,     "BACKSPACE" },
	{ ITEM_ID_TAB,           "TAB" },
	{ ITEM_ID_OPENBRACE,     "OPENBRACE" },
	{ ITEM_ID_CLOSEBRACE,    "CLOSEBRACE" },
	{ ITEM_ID_ENTER,         "ENTER" },
	{ ITEM_ID_COLON,         "COLON" },
	{ ITEM_ID_QUOTE,         "QUOTE" },
	{ ITEM_ID_BACKSLASH,     "BACKSLASH" },
	{ ITEM_ID_BACKSLASH2,    "BACKSLASH2" },
	{ ITEM_ID_COMMA,         "COMMA" },
	{ ITEM_ID_STOP,          "STOP" },
	{ ITEM_ID_SLASH,         "SLASH" },
	{ ITEM_ID_SPACE,         "SPACE" },
	{ ITEM_ID_INSERT,        "INSERT" },
	{ ITEM_ID_DEL,           "DEL" },
	{ ITEM_ID_HOME,          "HOME" },
	{ ITEM_ID_END,           "END" },
	{ ITEM_ID_PGUP,          "PGUP" },
	{ ITEM_ID_PGDN,          "PGDN" },
	{ ITEM_ID_LEFT,          "LEFT" },
	{ ITEM_ID_RIGHT,         "RIGHT" },
	{ ITEM_ID_UP,            "UP" },
	{ ITEM_ID_DOWN,          "DOWN" },
	{ ITEM_ID_0_PAD,         "0PAD" },
	{ ITEM_ID_1_PAD,         "1PAD" },
	{ ITEM_ID_2_PAD,         "2PAD" },
	{ ITEM_ID_3_PAD,         "3PAD" },
	{ ITEM_ID_4_PAD,         "4PAD" },
	{ ITEM_ID_5_PAD,         "5PAD" },
	{ ITEM_ID_6_PAD,         "6PAD" },
	{ ITEM_ID_7_PAD,         "7PAD" },
	{ ITEM_ID_8_PAD,         "8PAD" },
	{ ITEM_ID_9_PAD,         "9PAD" },
	{ ITEM_ID_SLASH_PAD,     "SLASHPAD" },
	{ ITEM_ID_ASTERISK,      "ASTERISK" },
	{ ITEM_ID_MINUS_PAD,     "MINUSPAD" },
	{ ITEM_ID_PLUS_PAD,      "PLUSPAD" },
	{ ITEM_ID_DEL_PAD,       "DELPAD" },
	{ ITEM_ID_ENTER_PAD,     "ENTERPAD" },
	{ ITEM_ID_PRTSCR,        "PRTSCR" },
	{ ITEM_ID_PAUSE,         "PAUSE" },
	{ ITEM_ID_LSHIFT,        "LSHIFT" },
	{ ITEM_ID_RSHIFT,        "RSHIFT" },
	{ ITEM_ID_LCONTROL,      "LCONTROL" },
	{ ITEM_ID_RCONTROL,      "RCONTROL" },
	{ ITEM_ID_LALT,          "LALT" },
	{ ITEM_ID_RALT,          "RALT" },
	{ ITEM_ID_SCRLOCK,       "SCRLOCK" },
	{ ITEM_ID_NUMLOCK,       "NUMLOCK" },
	{ ITEM_ID_CAPSLOCK,      "CAPSLOCK" },
	{ ITEM_ID_LWIN,          "LWIN" },
	{ ITEM_ID_RWIN,          "RWIN" },
	{ ITEM_ID_MENU,          "MENU" },
	{ ITEM_ID_CANCEL,        "CANCEL" },

	/* standard mouse/joystick/gun codes */
	{ ITEM_ID_XAXIS,         "XAXIS" },
	{ ITEM_ID_YAXIS,         "YAXIS" },
	{ ITEM_ID_ZAXIS,         "ZAXIS" },
	{ ITEM_ID_RXAXIS,        "RXAXIS" },
	{ ITEM_ID_RYAXIS,        "RYAXIS" },
	{ ITEM_ID_RZAXIS,        "RZAXIS" },
	{ ITEM_ID_SLIDER1,       "SLIDER1" },
	{ ITEM_ID_SLIDER2,       "SLIDER2" },
	{ ITEM_ID_BUTTON1,       "BUTTON1" },
	{ ITEM_ID_BUTTON2,       "BUTTON2" },
	{ ITEM_ID_BUTTON3,       "BUTTON3" },
	{ ITEM_ID_BUTTON4,       "BUTTON4" },
	{ ITEM_ID_BUTTON5,       "BUTTON5" },
	{ ITEM_ID_BUTTON6,       "BUTTON6" },
	{ ITEM_ID_BUTTON7,       "BUTTON7" },
	{ ITEM_ID_BUTTON8,       "BUTTON8" },
	{ ITEM_ID_BUTTON9,       "BUTTON9" },
	{ ITEM_ID_BUTTON10,      "BUTTON10" },
	{ ITEM_ID_BUTTON11,      "BUTTON11" },
	{ ITEM_ID_BUTTON12,      "BUTTON12" },
	{ ITEM_ID_BUTTON13,      "BUTTON13" },
	{ ITEM_ID_BUTTON14,      "BUTTON14" },
	{ ITEM_ID_BUTTON15,      "BUTTON15" },
	{ ITEM_ID_BUTTON16,      "BUTTON16" },
	{ ITEM_ID_START,         "START" },
	{ ITEM_ID_SELECT,        "SELECT" },

	/* Hats */
	{ ITEM_ID_HAT1UP,        "HAT1UP" },
	{ ITEM_ID_HAT1DOWN,      "HAT1DOWN" },
	{ ITEM_ID_HAT1LEFT,      "HAT1LEFT" },
	{ ITEM_ID_HAT1RIGHT,     "HAT1RIGHT" },
	{ ITEM_ID_HAT2UP,        "HAT2UP" },
	{ ITEM_ID_HAT2DOWN,      "HAT2DOWN" },
	{ ITEM_ID_HAT2LEFT,      "HAT2LEFT" },
	{ ITEM_ID_HAT2RIGHT,     "HAT2RIGHT" },
	{ ITEM_ID_HAT3UP,        "HAT3UP" },
	{ ITEM_ID_HAT3DOWN,      "HAT3DOWN" },
	{ ITEM_ID_HAT3LEFT,      "HAT3LEFT" },
	{ ITEM_ID_HAT3RIGHT,     "HAT3RIGHT" },
	{ ITEM_ID_HAT4UP,        "HAT4UP" },
	{ ITEM_ID_HAT4DOWN,      "HAT4DOWN" },
	{ ITEM_ID_HAT4LEFT,      "HAT4LEFT" },
	{ ITEM_ID_HAT4RIGHT,     "HAT4RIGHT" },

	/* Additional IDs */
	{ ITEM_ID_ADD_SWITCH1,   "ADDSW1" },
	{ ITEM_ID_ADD_SWITCH2,   "ADDSW2" },
	{ ITEM_ID_ADD_SWITCH3,   "ADDSW3" },
	{ ITEM_ID_ADD_SWITCH4,	 "ADDSW4" },
	{ ITEM_ID_ADD_SWITCH5,	 "ADDSW5" },
	{ ITEM_ID_ADD_SWITCH6,   "ADDSW6" },
	{ ITEM_ID_ADD_SWITCH7,   "ADDSW7" },
	{ ITEM_ID_ADD_SWITCH8,   "ADDSW8" },
	{ ITEM_ID_ADD_SWITCH9,   "ADDSW9" },
	{ ITEM_ID_ADD_SWITCH10,  "ADDSW10" },
	{ ITEM_ID_ADD_SWITCH11,  "ADDSW11" },
	{ ITEM_ID_ADD_SWITCH12,  "ADDSW12" },
	{ ITEM_ID_ADD_SWITCH13,  "ADDSW13" },
	{ ITEM_ID_ADD_SWITCH14,  "ADDSW14" },
	{ ITEM_ID_ADD_SWITCH15,  "ADDSW15" },
	{ ITEM_ID_ADD_SWITCH16,  "ADDSW16" },
	{ ITEM_ID_ADD_ABSOLUTE1, "ADDAXIS1" },
	{ ITEM_ID_ADD_ABSOLUTE2, "ADDAXIS2" },
	{ ITEM_ID_ADD_ABSOLUTE3, "ADDAXIS3" },
	{ ITEM_ID_ADD_ABSOLUTE4, "ADDAXIS4" },
	{ ITEM_ID_ADD_ABSOLUTE5, "ADDAXIS5" },
	{ ITEM_ID_ADD_ABSOLUTE6, "ADDAXIS6" },
	{ ITEM_ID_ADD_ABSOLUTE7, "ADDAXIS7" },
	{ ITEM_ID_ADD_ABSOLUTE8, "ADDAXIS8" },
	{ ITEM_ID_ADD_ABSOLUTE9, "ADDAXIS9" },
	{ ITEM_ID_ADD_ABSOLUTE10,"ADDAXIS10" },
	{ ITEM_ID_ADD_ABSOLUTE11,"ADDAXIS11" },
	{ ITEM_ID_ADD_ABSOLUTE12,"ADDAXIS12" },
	{ ITEM_ID_ADD_ABSOLUTE13,"ADDAXIS13" },
	{ ITEM_ID_ADD_ABSOLUTE14,"ADDAXIS14" },
	{ ITEM_ID_ADD_ABSOLUTE15,"ADDAXIS15" },
	{ ITEM_ID_ADD_ABSOLUTE16,"ADDAXIS16" },
	{ ITEM_ID_ADD_RELATIVE1, "ADDREL1" },
	{ ITEM_ID_ADD_RELATIVE2, "ADDREL2" },
	{ ITEM_ID_ADD_RELATIVE3, "ADDREL3" },
	{ ITEM_ID_ADD_RELATIVE4, "ADDREL4" },
	{ ITEM_ID_ADD_RELATIVE5, "ADDREL5" },
	{ ITEM_ID_ADD_RELATIVE6, "ADDREL6" },
	{ ITEM_ID_ADD_RELATIVE7, "ADDREL7" },
	{ ITEM_ID_ADD_RELATIVE8, "ADDREL8" },
	{ ITEM_ID_ADD_RELATIVE9, "ADDREL9" },
	{ ITEM_ID_ADD_RELATIVE10,"ADDREL10" },
	{ ITEM_ID_ADD_RELATIVE11,"ADDREL11" },
	{ ITEM_ID_ADD_RELATIVE12,"ADDREL12" },
	{ ITEM_ID_ADD_RELATIVE13,"ADDREL13" },
	{ ITEM_ID_ADD_RELATIVE14,"ADDREL14" },
	{ ITEM_ID_ADD_RELATIVE15,"ADDREL15" },
	{ ITEM_ID_ADD_RELATIVE16,"ADDREL16" },

	{ ~0,                    NULL }
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* stashed copy of the most recently initialized machine for the debugging hack */
static running_machine *stashed_machine;

/* standard joystick mappings */
const char			joystick_map_8way[] = "7778...4445";
const char			joystick_map_4way_sticky[] = "s8.4s8.44s8.4445";
const char			joystick_map_4way_diagonal[] = "4444s8888..444458888.444555888.ss5.222555666.222256666.2222s6666.2222s6666";



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void input_frame(running_machine &machine);
static input_device_item *input_code_item(running_machine *machine, input_code code);
static INT32 convert_absolute_value(running_machine *machine, input_code code, input_device_item *item);
static INT32 convert_relative_value(input_code code, input_device_item *item);
static INT32 convert_switch_value(running_machine *machine, input_code code, input_device_item *item);
static INT32 apply_deadzone_and_saturation(running_machine *machine, input_code code, INT32 result);
static int joystick_map_parse(const char *mapstring, joystick_map *map);
static void joystick_map_print(const char *header, const char *origstring, const joystick_map *map);
static void input_code_reset_axes(running_machine *machine);
static int input_code_check_axis(running_machine *machine, input_device_item *item, input_code code);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_code_device - given an input_code return
    a pointer to the associated device
-------------------------------------------------*/

INLINE input_device *input_code_device(running_machine *machine, input_code code)
{
	/* if the class is valid... */
	input_device_class devclass = INPUT_CODE_DEVCLASS(code);
	if (devclass > DEVICE_CLASS_INVALID && devclass < DEVICE_CLASS_MAXIMUM)
	{
		input_device_list *device_list = machine->input_data->device_list;

		/* ...and the index is valid for that class, return a pointer to the device */
		int devindex = INPUT_CODE_DEVINDEX(code);
		if (devindex < device_list[devclass].count)
			return device_list[devclass].list[devindex];
	}

	/* otherwise, return NULL */
	return NULL;
}


/*-------------------------------------------------
    device_item_to_code - convert a device/item
    pair to a standard code
-------------------------------------------------*/

INLINE input_code device_item_to_code(input_device *device, input_item_id itemid)
{
	int devindex = device->devindex;

	assert(devindex < device->machine->input_data->device_list[device->devclass].count);
	assert(itemid < ITEM_ID_ABSOLUTE_MAXIMUM);
	assert(device->item[itemid] != NULL);

	return INPUT_CODE(device->devclass, devindex, device->item[itemid]->itemclass, ITEM_MODIFIER_NONE, itemid);
}


/*-------------------------------------------------
    input_item_standard_class - return the class
    of a standard item
-------------------------------------------------*/

INLINE input_item_class input_item_standard_class(input_device_class devclass, input_item_id itemid)
{
	/* most everything standard is a switch, apart from the axes */
	if (itemid == ITEM_ID_OTHER_SWITCH || itemid < ITEM_ID_XAXIS || (itemid > ITEM_ID_SLIDER2 && itemid < ITEM_ID_ADD_ABSOLUTE1))
		return ITEM_CLASS_SWITCH;

	/* standard mouse axes are relative */
	else if (devclass == DEVICE_CLASS_MOUSE || itemid == ITEM_ID_OTHER_AXIS_RELATIVE || (itemid >= ITEM_ID_ADD_RELATIVE1 && itemid <= ITEM_ID_ADD_RELATIVE16))
		return ITEM_CLASS_RELATIVE;

	/* all other standard axes are absolute */
	else
		return ITEM_CLASS_ABSOLUTE;
}


/*-------------------------------------------------
    input_item_update_value - update the value
    of an input item
-------------------------------------------------*/

INLINE void input_item_update_value(running_machine *machine, input_device_item *item)
{
	input_device_list *device_list = machine->input_data->device_list;

	item->current = (*item->getstate)(device_list[item->devclass].list[item->devindex]->internal, item->internal);
}


/*-------------------------------------------------
    code_pressed_memory_reset - reset the array
    of memory for pressed switches
-------------------------------------------------*/

INLINE void code_pressed_memory_reset(running_machine *machine)
{
	input_code *code_pressed_memory = machine->input_data->code_pressed_memory;
	int memnum;

	for (memnum = 0; memnum < MAX_PRESSED_SWITCHES; memnum++)
		code_pressed_memory[memnum] = INPUT_CODE_INVALID;
}


/*-------------------------------------------------
    string_to_code - convert a string to a code
    via table lookup
-------------------------------------------------*/

INLINE UINT32 string_to_code(const code_string_table *table, const char *string)
{
	/* find a matching string */
	for ( ; table->code != ~0; table++)
		if (strcmp(string, table->string) == 0)
			return table->code;

	/* on failure, return ~0 */
	return ~0;
}


/*-------------------------------------------------
    code_to_string - convert a code to a string
    via table lookup
-------------------------------------------------*/

INLINE const char *code_to_string(const code_string_table *table, UINT32 code)
{
	/* find a matching code, or  */
	for ( ; table->code != ~0; table++)
		if (table->code == code)
			return table->string;

	/* return the default string */
	return table->string;
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    input_init - initialize the input lists
-------------------------------------------------*/

void input_init(running_machine *machine)
{
	joystick_map map;
	input_private *state;
	input_device_list *device_list;

	/* remember this machine */
	stashed_machine = machine;

	/* allocate private memory */
	machine->input_data = state = auto_alloc_clear(machine, input_private);
	device_list = state->device_list;

	/* reset code memory */
	code_pressed_memory_reset(machine);

	/* request a per-frame callback for bookkeeping */
	machine->add_notifier(MACHINE_NOTIFY_FRAME, input_frame);

	/* read input enable options */
	device_list[DEVICE_CLASS_KEYBOARD].enabled = TRUE;
	device_list[DEVICE_CLASS_MOUSE].enabled = options_get_bool(machine->options(), OPTION_MOUSE);
	device_list[DEVICE_CLASS_LIGHTGUN].enabled = options_get_bool(machine->options(), OPTION_LIGHTGUN);
	device_list[DEVICE_CLASS_JOYSTICK].enabled = options_get_bool(machine->options(), OPTION_JOYSTICK);

	/* read input device multi options */
	device_list[DEVICE_CLASS_KEYBOARD].multi = options_get_bool(machine->options(), OPTION_MULTIKEYBOARD);
	device_list[DEVICE_CLASS_MOUSE].multi = options_get_bool(machine->options(), OPTION_MULTIMOUSE);
	device_list[DEVICE_CLASS_LIGHTGUN].multi = TRUE;
	device_list[DEVICE_CLASS_JOYSTICK].multi = TRUE;

	/* read other input options */
	state->steadykey_enabled = options_get_bool(machine->options(), OPTION_STEADYKEY);
	state->lightgun_reload_button = options_get_bool(machine->options(), OPTION_OFFSCREEN_RELOAD);
	state->joystick_deadzone = (INT32)(options_get_float(machine->options(), OPTION_JOYSTICK_DEADZONE) * INPUT_ABSOLUTE_MAX);
	state->joystick_saturation = (INT32)(options_get_float(machine->options(), OPTION_JOYSTICK_SATURATION) * INPUT_ABSOLUTE_MAX);

	/* get the default joystick map */
	state->joystick_map_default = options_get_string(machine->options(), OPTION_JOYSTICK_MAP);
	if (state->joystick_map_default[0] == 0 || strcmp(state->joystick_map_default, "auto") == 0)
		state->joystick_map_default = joystick_map_8way;
	if (!joystick_map_parse(state->joystick_map_default, &map))
		mame_printf_error("Invalid joystick map: %s\n", state->joystick_map_default);
	else if (state->joystick_map_default != joystick_map_8way)
		joystick_map_print("Input: Default joystick map", state->joystick_map_default, &map);
}


/*-------------------------------------------------
    input_device_class_enable - enable or disable
    a device class
-------------------------------------------------*/

void input_device_class_enable(running_machine *machine, input_device_class devclass, UINT8 enable)
{
	input_device_list *device_list = machine->input_data->device_list;

	assert(devclass > DEVICE_CLASS_INVALID && devclass < DEVICE_CLASS_MAXIMUM);
	device_list[devclass].enabled = enable;
}


/*-------------------------------------------------
    input_enable_device_class - is a device class
    enabled?
-------------------------------------------------*/

UINT8 input_device_class_enabled(running_machine *machine, input_device_class devclass)
{
	input_device_list *device_list = machine->input_data->device_list;

	assert(devclass > DEVICE_CLASS_INVALID && devclass < DEVICE_CLASS_MAXIMUM);
	return device_list[devclass].enabled;
}


/*-------------------------------------------------
    input_device_set_joystick_map - set the
    joystick map for a device
-------------------------------------------------*/

int input_device_set_joystick_map(running_machine *machine, int devindex, const char *mapstring)
{
	input_device_list *device_list = machine->input_data->device_list;
	int startindex = devindex;
	int stopindex = devindex;
	joystick_map map;
	int joynum;

	/* parse the map */
	if (!joystick_map_parse(mapstring, &map))
		return FALSE;

	/* devindex -1 means set the same for all */
	if (devindex == -1)
	{
		startindex = 0;
		stopindex = device_list[DEVICE_CLASS_JOYSTICK].count - 1;
		joystick_map_print("Input: Changing default joystick map", mapstring, &map);
	}

	/* ignore if out of range */
	else if (devindex >= device_list[DEVICE_CLASS_JOYSTICK].count)
		return TRUE;

	/* iterate over joysticks and set the map */
	for (joynum = startindex; joynum <= stopindex; joynum++)
		device_list[DEVICE_CLASS_JOYSTICK].list[joynum]->joymap = map;
	return TRUE;
}


/*-------------------------------------------------
    input_frame - per-frame callback for various
    bookkeeping
-------------------------------------------------*/

static void input_frame(running_machine &machine)
{
	input_private *state = machine.input_data;

	/* if steadykey is enabled, do processing here */
	if (state->steadykey_enabled)
	{
		input_device_list *device_list = state->device_list;
		int devnum;

		/* iterate over keyboards */
		for (devnum = 0; devnum < device_list[DEVICE_CLASS_KEYBOARD].count; devnum++)
		{
			input_device *device = device_list[DEVICE_CLASS_KEYBOARD].list[devnum];
			input_item_id itemid;
			int changed = FALSE;

			/* update the state of all the keys and see if any changed state */
			for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
			{
				input_device_item *item = device->item[itemid];
				if (item != NULL && item->itemclass == ITEM_CLASS_SWITCH)
				{
					input_item_update_value(&machine, item);
					if ((item->current ^ item->oldkey) & 1)
					{
						changed = TRUE;

						/* if the keypress was missed, turn it on for one frame */
						if (((item->current | item->steadykey) & 1) == 0)
							item->steadykey = 1;
					}
				}
			}

			/* if the keyboard state is stable, copy it over */
			for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
			{
				input_device_item *item = device->item[itemid];
				if (item != NULL && item->itemclass == ITEM_CLASS_SWITCH)
				{
					if (!changed)
						item->steadykey = item->current;
					item->oldkey = item->current;
				}
			}
		}
	}
}



/***************************************************************************
    OSD CONFIGURATION AND ACCESS
***************************************************************************/

/*-------------------------------------------------
    input_device_add - add a new input device
-------------------------------------------------*/

input_device *input_device_add(running_machine *machine, input_device_class devclass, const char *name, void *internal)
{
	input_private *state = machine->input_data;
	input_device_list *devlist = &state->device_list[devclass];

	assert_always(machine->phase() == MACHINE_PHASE_INIT, "Can only call input_device_add at init time!");
	assert(name != NULL);
	assert(devclass != DEVICE_CLASS_INVALID && devclass < DEVICE_CLASS_MAXIMUM);

	/* allocate a new device */
	input_device *device = auto_alloc_clear(machine, input_device);
	input_device **newlist = auto_alloc_array(machine, input_device *, devlist->count + 1);
	for (int devnum = 0; devnum < devlist->count; devnum++)
		newlist[devnum] = devlist->list[devnum];
	auto_free(machine, devlist->list);
	devlist->list = newlist;
	devlist->list[devlist->count++] = device;

	/* fill in the data */
	device->machine = machine;
	device->name.cpy(name);
	device->devclass = devclass;
	device->devindex = devlist->count - 1;
	device->internal = internal;

	/* default to 8-way map for joysticks */
	if (devclass == DEVICE_CLASS_JOYSTICK)
	{
		joystick_map_parse(state->joystick_map_default, &device->joymap);
		device->lastmap = JOYSTICK_MAP_NEUTRAL;
	}

	mame_printf_verbose("Input: Adding %s #%d: %s\n", code_to_string(devclass_string_table, devclass), devlist->count, device->name.cstr());
	return device;
}


/*-------------------------------------------------
    input_device_item_add - add a new item to an
    input device
-------------------------------------------------*/

void input_device_item_add(input_device *device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate)
{
	input_device_item *item;
	input_item_id itemid_std = itemid;

	assert_always(device->machine->phase() == MACHINE_PHASE_INIT, "Can only call input_device_item_add at init time!");
	assert(name != NULL);
	assert(itemid > ITEM_ID_INVALID && itemid < ITEM_ID_MAXIMUM);
	assert(getstate != NULL);

	/* if we have a generic ID, pick a new internal one */
	if (itemid >= ITEM_ID_OTHER_SWITCH && itemid <= ITEM_ID_OTHER_AXIS_RELATIVE)
		for (itemid = (input_item_id)(ITEM_ID_MAXIMUM + 1); itemid <= ITEM_ID_ABSOLUTE_MAXIMUM; itemid++)
			if (device->item[itemid] == NULL)
				break;
	assert(itemid <= ITEM_ID_ABSOLUTE_MAXIMUM);

	/* make sure we don't have any overlap */
	assert(device->item[itemid] == NULL);

	/* allocate a new item and copy data into it */
	item = auto_alloc_clear(device->machine, input_device_item);
	device->item[itemid] = item;
	device->maxitem = MAX(device->maxitem, itemid);

	/* copy in the data passed in from the item list */
	item->devclass = device->devclass;
	item->devindex = device->devindex;
	item->name.cpy(name);
	item->internal = internal;
	item->itemclass = input_item_standard_class(device->devclass, itemid_std);
	item->itemid = itemid;
	item->getstate = getstate;

	/* if we're custom, create a tokenized name */
	if (itemid > ITEM_ID_MAXIMUM)
	{
		/* copy the item name, removing spaces/underscores and making all caps */
		item->token.cpy(name).toupper().delchr(' ').delchr('_');
	}

	/* otherwise, make sure we have a valid standard token */
	else
	{
		assert(code_to_string(itemid_token_table, itemid) != NULL);
	}
}



/***************************************************************************
    STATE QUERIES
***************************************************************************/

/*-------------------------------------------------
    input_code_value - return the value of a
    given input code
-------------------------------------------------*/

INT32 input_code_value(running_machine *machine, input_code code)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_class devclass = INPUT_CODE_DEVCLASS(code);
	int startindex = INPUT_CODE_DEVINDEX(code);
	int stopindex = startindex;
	INT32 result = 0;
	int curindex;

	profiler_mark_start(PROFILER_INPUT);

	/* return 0 for any disabled or invalid device classes */
	if (devclass <= DEVICE_CLASS_INVALID || devclass >= DEVICE_CLASS_MAXIMUM || !device_list[devclass].enabled)
		goto exit;
	if (startindex >= device_list[devclass].count)
		goto exit;

	/* if this is not a multi device, only return data for item 0 */
	if (!device_list[devclass].multi)
	{
		if (startindex != 0)
			goto exit;

		/* otherwise, iterate over all */
		startindex = 0;
		stopindex = device_list[devclass].count - 1;
	}

	/* iterate over all indices */
	for (curindex = startindex; curindex <= stopindex; curindex++)
	{
		/* lookup the item for the appropriate index */
		input_device_item *item = input_code_item(machine, INPUT_CODE_SET_DEVINDEX(code, curindex));
		if (item == NULL)
			continue;

		/* update the value of this item */
		input_item_update_value(machine, item);

		/* process items according to their native type */
		switch (item->itemclass)
		{
			case ITEM_CLASS_ABSOLUTE:
				result = convert_absolute_value(machine, code, item);
				break;

			case ITEM_CLASS_RELATIVE:
				result += convert_relative_value(code, item);
				break;

			case ITEM_CLASS_SWITCH:
				result |= convert_switch_value(machine, code, item);
				break;

			default:
				break;
		}
	}

exit:
	profiler_mark_end();
	return result;
}


/*-------------------------------------------------
    input_code_pressed - return non-zero if a
    given input code has been pressed
-------------------------------------------------*/

int input_code_pressed(running_machine *machine, input_code code)
{
	return (input_code_value(machine, code) != 0);
}


/*-------------------------------------------------
    input_code_pressed_once - return non-zero if a
    given input code has transitioned from off to
    on since the last call
-------------------------------------------------*/

int input_code_pressed_once(running_machine *machine, input_code code)
{
	input_code *code_pressed_memory = machine->input_data->code_pressed_memory;
	int curvalue = input_code_pressed(machine, code);
	int memnum, empty = -1;

	/* look for the code in the memory */
	for (memnum = 0; memnum < MAX_PRESSED_SWITCHES; memnum++)
	{
		/* were we previous pressed on the last time through here? */
		if (code_pressed_memory[memnum] == code)
		{
			/* if no longer pressed, clear entry */
			if (curvalue == 0)
				code_pressed_memory[memnum] = INPUT_CODE_INVALID;

			/* always return 0 */
			return 0;
		}

		/* remember the first empty entry */
		if (empty == -1 && code_pressed_memory[memnum] == INPUT_CODE_INVALID)
			empty = memnum;
	}

	/* if we get here, we were not previously pressed; if still not pressed, return 0 */
	if (curvalue == 0)
		return 0;

	/* otherwise, add ourself to the memory and return 1 */
	assert(empty != -1);
	if (empty != -1)
		code_pressed_memory[empty] = code;
	return 1;
}


/*-------------------------------------------------
    input_code_from_input_item_id - translates
    an input_item_id to an input_code
-------------------------------------------------*/

input_code input_code_from_input_item_id(running_machine *machine, input_item_id itemid)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_class devclass;

	/* iterate over device classes and devices */
	for (devclass = DEVICE_CLASS_FIRST_VALID; devclass < DEVICE_CLASS_MAXIMUM; devclass++)
	{
		input_device_list *devlist = &device_list[devclass];
		int devnum;

		/* iterate over devices within each class */
		for (devnum = 0; devnum < devlist->count; devnum++)
		{
			input_device *device = devlist->list[devnum];
			if (device->item[itemid] != NULL)
				return device_item_to_code(device, itemid);
		}
	}
	return 0;
}


/*-------------------------------------------------
    input_code_poll_switches - poll for any input
-------------------------------------------------*/

input_code input_code_poll_switches(running_machine *machine, int reset)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_class devclass;

	/* if resetting memory, do it now */
	if (reset)
	{
		code_pressed_memory_reset(machine);
		input_code_reset_axes(machine);
	}

	/* iterate over device classes and devices */
	for (devclass = DEVICE_CLASS_FIRST_VALID; devclass < DEVICE_CLASS_MAXIMUM; devclass++)
	{
		input_device_list *devlist = &device_list[devclass];
		int devnum;

		/* iterate over devices within each class */
		for (devnum = 0; devnum < devlist->count; devnum++)
		{
			input_device *device = devlist->list[devnum];
			input_item_id itemid;

			/* iterate over items within each device */
			for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
			{
				input_device_item *item = device->item[itemid];
				if (item != NULL)
				{
					input_code code = device_item_to_code(device, itemid);

					/* if the item is natively a switch, poll it */
					if (item->itemclass == ITEM_CLASS_SWITCH)
					{
						if (input_code_pressed_once(machine, code))
							return code;
						else
						    continue;
					}

					/* skip if there is not enough axis movement */
					if (!input_code_check_axis(machine, item, code))
						continue;

					/* otherwise, poll axes digitally */
					code = INPUT_CODE_SET_ITEMCLASS(code, ITEM_CLASS_SWITCH);

					/* if this is a joystick X axis, check with left/right modifiers */
					if (devclass == DEVICE_CLASS_JOYSTICK && INPUT_CODE_ITEMID(code) == ITEM_ID_XAXIS)
					{
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_LEFT);
						if (input_code_pressed_once(machine, code))
							return code;
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_RIGHT);
						if (input_code_pressed_once(machine, code))
							return code;
					}

					/* if this is a joystick Y axis, check with up/down modifiers */
					else if (devclass == DEVICE_CLASS_JOYSTICK && INPUT_CODE_ITEMID(code) == ITEM_ID_YAXIS)
					{
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_UP);
						if (input_code_pressed_once(machine, code))
							return code;
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_DOWN);
						if (input_code_pressed_once(machine, code))
							return code;
					}

					/* any other axis, check with pos/neg modifiers */
					else
					{
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_POS);
						if (input_code_pressed_once(machine, code))
							return code;
						code = INPUT_CODE_SET_MODIFIER(code, ITEM_MODIFIER_NEG);
						if (input_code_pressed_once(machine, code))
							return code;
					}
				}
			}
		}
	}

	/* if nothing, return an invalid code */
	return INPUT_CODE_INVALID;
}


/*-------------------------------------------------
    input_code_poll_keyboard_switches - poll for
    any keyboard-specific input
-------------------------------------------------*/

input_code input_code_poll_keyboard_switches(running_machine *machine, int reset)
{
	input_device_list *devlist = &machine->input_data->device_list[DEVICE_CLASS_KEYBOARD];
	int devnum;

	/* if resetting memory, do it now */
	if (reset)
		code_pressed_memory_reset(machine);

	/* iterate over devices within each class */
	for (devnum = 0; devnum < devlist->count; devnum++)
	{
		input_device *device = devlist->list[devnum];
		input_item_id itemid;

		/* iterate over items within each device */
		for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
		{
			input_device_item *item = device->item[itemid];
			if (item != NULL && item->itemclass == ITEM_CLASS_SWITCH)
			{
				input_code code = device_item_to_code(device, itemid);
				if (input_code_pressed_once(machine, code))
					return code;
			}
		}
	}

	/* if nothing, return an invalid code */
	return INPUT_CODE_INVALID;
}


/*-------------------------------------------------
    input_code_check_axis - see if axis has
    move far enough
-------------------------------------------------*/

static int input_code_check_axis(running_machine *machine, input_device_item *item, input_code code)
{
	INT32 curval, diff;

	/* poll the current value */
	curval = input_code_value(machine, code);

	/* if we've already reported this one, don't bother */
	if (item->memory == INVALID_AXIS_VALUE)
		return FALSE;

    /* ignore min/max for lightguns */
    /* so the selection will not be affected by a gun going out of range */
    if ((INPUT_CODE_DEVCLASS(code) == DEVICE_CLASS_LIGHTGUN)
        && (INPUT_CODE_ITEMID(code) == ITEM_ID_XAXIS || INPUT_CODE_ITEMID(code) == ITEM_ID_YAXIS)
        && (curval == INPUT_ABSOLUTE_MAX || curval == INPUT_ABSOLUTE_MIN))
        return FALSE;

	/* compute the diff against memory */
	diff = curval - item->memory;
	if (diff < 0)
		diff = -diff;

	/* for absolute axes, look for 25% of maximum */
	if (item->itemclass == ITEM_CLASS_ABSOLUTE && diff > (INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN) / 4)
	{
		item->memory = INVALID_AXIS_VALUE;
		return TRUE;
	}

	/* for relative axes, look for ~20 pixels movement */
	if (item->itemclass == ITEM_CLASS_RELATIVE && diff > 20 * INPUT_RELATIVE_PER_PIXEL)
	{
		item->memory = INVALID_AXIS_VALUE;
		return TRUE;
	}

    return FALSE;
}


/*-------------------------------------------------
    input_code_reset_axes - reset axes memory
-------------------------------------------------*/

static void input_code_reset_axes(running_machine *machine)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_class devclass;

	/* iterate over device classes and devices */
	for (devclass = DEVICE_CLASS_FIRST_VALID; devclass < DEVICE_CLASS_MAXIMUM; devclass++)
	{
		input_device_list *devlist = &device_list[devclass];
		int devnum;

		/* iterate over devices within each class */
		for (devnum = 0; devnum < devlist->count; devnum++)
		{
			input_device *device = devlist->list[devnum];
			input_item_id itemid;

			/* iterate over items within each device */
			for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
			{
				input_device_item *item = device->item[itemid];
				if (item != NULL)
				{
					input_code code = device_item_to_code(device, itemid);

					/* skip any switches */
					if (item->itemclass == ITEM_CLASS_SWITCH)
						continue;

					/* poll the current value and reset the memory */
					item->memory = input_code_value(machine, code);
				}
			}
		}
	}
}


/*-------------------------------------------------
    input_code_poll_axes - poll for any input
-------------------------------------------------*/

input_code input_code_poll_axes(running_machine *machine, int reset)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_class devclass;

	/* if resetting memory, do it now */
	if (reset)
		input_code_reset_axes(machine);

	/* iterate over device classes and devices */
	for (devclass = DEVICE_CLASS_FIRST_VALID; devclass < DEVICE_CLASS_MAXIMUM; devclass++)
	{
		input_device_list *devlist = &device_list[devclass];
		int devnum;

		/* iterate over devices within each class */
		for (devnum = 0; devnum < devlist->count; devnum++)
		{
			input_device *device = devlist->list[devnum];
			input_item_id itemid;

			/* iterate over items within each device */
			for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
			{
				input_device_item *item = device->item[itemid];
				if (item != NULL)
				{
					input_code code = device_item_to_code(device, itemid);

					/* skip any switches */
					if (item->itemclass == ITEM_CLASS_SWITCH)
						continue;

					/* check if there is enough axis movement */
					if (input_code_check_axis(machine, item, code))
						return code;
				}
			}
		}
	}

	/* if nothing, return an invalid code */
	return INPUT_CODE_INVALID;
}



/***************************************************************************
    STRINGS AND TOKENIZATION
***************************************************************************/

/*-------------------------------------------------
    input_code_name - convert an input code into
    a friendly name
-------------------------------------------------*/

astring &input_code_name(running_machine *machine, astring &string, input_code code)
{
	input_device_list *device_list = machine->input_data->device_list;
	input_device_item *item = input_code_item(machine, code);
	const char *devclass;
	const char *devcode;
	const char *modifier;
	char devindex[10];

	/* if nothing there, return an empty string */
	if (item == NULL)
		return string.reset();

	/* determine the devclass part */
	devclass = code_to_string(devclass_string_table, INPUT_CODE_DEVCLASS(code));

	/* determine the devindex part */
	sprintf(devindex, "%d", INPUT_CODE_DEVINDEX(code) + 1);

	/* if we're unifying all devices, don't display a number */
	if (!device_list[INPUT_CODE_DEVCLASS(code)].multi)
		devindex[0] = 0;

	/* keyboard 0 doesn't show a class or index if it is the only one */
	if (item->devclass == DEVICE_CLASS_KEYBOARD && device_list[DEVICE_CLASS_KEYBOARD].count == 1)
	{
		devclass = "";
		devindex[0] = 0;
	}

	/* devcode part comes from the item name */
	devcode = item->name;

	/* determine the modifier part */
	modifier = code_to_string(modifier_string_table, INPUT_CODE_MODIFIER(code));

	/* devcode is redundant with joystick switch left/right/up/down */
	if (item->devclass == DEVICE_CLASS_JOYSTICK && INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
		if (INPUT_CODE_MODIFIER(code) >= ITEM_MODIFIER_LEFT && INPUT_CODE_MODIFIER(code) <= ITEM_MODIFIER_DOWN)
			devcode = "";

	/* concatenate the strings */
	string.cpy(devclass);
	if (devindex[0] != 0)
		string.cat(" ").cat(devindex);
	if (devcode[0] != 0)
		string.cat(" ").cat(devcode);
	if (modifier[0] != 0)
		string.cat(" ").cat(modifier);

	/* delete any leading spaces */
	return string.trimspace();
}


/*-------------------------------------------------
    input_code_to_token - create a token for
    a given code
-------------------------------------------------*/

astring &input_code_to_token(running_machine *machine, astring &string, input_code code)
{
	input_device_item *item = input_code_item(machine, code);
	const char *devclass;
	const char *devcode;
	const char *itemclass;
	const char *modifier;
	char devindex[10];

	/* determine the devclass part */
	devclass = code_to_string(devclass_token_table, INPUT_CODE_DEVCLASS(code));

	/* determine the devindex part; keyboard 0 doesn't show an index */
	sprintf(devindex, "%d", INPUT_CODE_DEVINDEX(code) + 1);
	if (INPUT_CODE_DEVCLASS(code) == DEVICE_CLASS_KEYBOARD && INPUT_CODE_DEVINDEX(code) == 0)
		devindex[0] = 0;

	/* determine the itemid part; look up in the table if we don't have a token */
	if (item != NULL && item->token.len() != 0)
		devcode = item->token;
	else
	{
		devcode = code_to_string(itemid_token_table, INPUT_CODE_ITEMID(code));
		if (devcode == NULL)
			devcode = "UNKNOWN";
	}

	/* determine the modifier part */
	modifier = code_to_string(modifier_token_table, INPUT_CODE_MODIFIER(code));

	/* determine the itemclass part; if we match the native class, we don't include this */
	if (item != NULL && item->itemclass == INPUT_CODE_ITEMCLASS(code))
		itemclass = "";
	else
		itemclass = code_to_string(itemclass_token_table, INPUT_CODE_ITEMCLASS(code));

	/* concatenate the strings */
	string.cpy(devclass);
	if (devindex[0] != 0)
		string.cat("_").cat(devindex);
	if (devcode[0] != 0)
		string.cat("_").cat(devcode);
	if (modifier[0] != 0)
		string.cat("_").cat(modifier);
	if (itemclass[0] != 0)
		string.cat("_").cat(itemclass);
	return string;
}


/*-------------------------------------------------
    input_code_from_token - extract an input
    code from a token
-------------------------------------------------*/

input_code input_code_from_token(running_machine *machine, const char *_token)
{
	UINT32 devclass, itemid, devindex, modifier, standard;
	UINT32 itemclass = ITEM_CLASS_INVALID;
	input_code code = INPUT_CODE_INVALID;
	astring token[6];
	int numtokens, curtok;

	/* copy the token and break it into pieces */
	for (numtokens = 0; numtokens < ARRAY_LENGTH(token); )
	{
		/* make a token up to the next underscore */
		char *score = (char *)strchr(_token, '_');
		token[numtokens++].cpy(_token, (score == NULL) ? strlen(_token) : (score - _token));

		/* if we hit the end, we're done, else advance our pointer */
		if (score == NULL)
			break;
		_token = score + 1;
	}

	/* first token should be the devclass */
	curtok = 0;
	devclass = string_to_code(devclass_token_table, token[curtok++]);
	if (devclass == ~0)
		goto exit;

	/* second token might be index; look for number */
	devindex = 0;
	if (numtokens > 2 && sscanf(token[curtok], "%d", &devindex) == 1)
	{
		curtok++;
		devindex--;
	}
	if (curtok >= numtokens)
		goto exit;

	/* next token is the item ID */
	itemid = string_to_code(itemid_token_table, token[curtok]);
	standard = (itemid != ~0);

	/* if we're a standard code, default the itemclass based on it */
	if (standard)
		itemclass = input_item_standard_class((input_device_class)devclass, (input_item_id)itemid);

	/* otherwise, keep parsing */
	else
	{
		input_device_list *device_list = (machine != NULL) ? machine->input_data->device_list : NULL;
		input_device *device;

		/* if this is an invalid device, we have nothing to look up */
		if (device_list == NULL || devindex >= device_list[devclass].count)
			goto exit;
		device = device_list[devclass].list[devindex];

		/* if not a standard code, look it up in the device specific codes */
		for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem; itemid++)
		{
			input_device_item *item = device->item[itemid];
			if (item != NULL && token[curtok].cmp(item->token) == 0)
			{
				/* take the itemclass from the item */
				itemclass = item->itemclass;
				break;
			}
		}

		/* bail on fail */
		if (itemid > device->maxitem)
			goto exit;
	}
	curtok++;

	/* if we have another token, it is probably a modifier */
	modifier = ITEM_MODIFIER_NONE;
	if (curtok < numtokens)
	{
		modifier = string_to_code(modifier_token_table, token[curtok]);
		if (modifier != ~0)
			curtok++;
		else
			modifier = ITEM_MODIFIER_NONE;
	}

	/* if we have another token, it is the item class */
	if (curtok < numtokens)
	{
		UINT32 temp = string_to_code(itemclass_token_table, token[curtok]);
		if (temp != ~0)
		{
			curtok++;
			itemclass = temp;
		}
	}

	/* we should have consumed all tokens */
	if (curtok != numtokens)
		goto exit;

	/* assemble the final code */
	code = INPUT_CODE(devclass, devindex, itemclass, modifier, itemid);

exit:
	return code;
}



/***************************************************************************
    DEBUGGIN UTILITIES
***************************************************************************/

/*-------------------------------------------------
    debug_global_input_code_pressed - return TRUE
    if the given input code has been pressed
-------------------------------------------------*/

INT32 debug_global_input_code_pressed(input_code code)
{
	if (!mame_is_valid_machine(stashed_machine))
		return 0;
	return input_code_pressed(stashed_machine, code);
}

INT32 debug_global_input_code_pressed_once(input_code code)
{
	if (!mame_is_valid_machine(stashed_machine))
		return 0;
	return input_code_pressed_once(stashed_machine, code);
}

/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_code_item - get a pointer to the item
    from the code
-------------------------------------------------*/

static input_device_item *input_code_item(running_machine *machine, input_code code)
{
	input_device *device = input_code_device(machine, code);
	input_item_id itemid;

	/* if no device, we fail */
	if (device == NULL)
		return NULL;

	/* if the devcode is in range, return a pointer to the item */
	itemid = INPUT_CODE_ITEMID(code);
	if (itemid < ARRAY_LENGTH(device->item))
		return device->item[itemid];

	/* otherwise fail with NULL */
	return NULL;
}


/*-------------------------------------------------
    convert_absolute_value - convert an absolute
    value into the class specified by code
-------------------------------------------------*/

static INT32 convert_absolute_value(running_machine *machine, input_code code, input_device_item *item)
{
	input_private *state = machine->input_data;

	/* make sure values are valid */
	assert(item->current >= INPUT_ABSOLUTE_MIN && item->current <= INPUT_ABSOLUTE_MAX);

	/* relative value conversion not supported */
	if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_RELATIVE)
		return 0;

	/* if we want the absolute value, process it according to the modifier field */
	else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_ABSOLUTE)
	{
		input_item_modifier modifier = INPUT_CODE_MODIFIER(code);
		INT32 result = apply_deadzone_and_saturation(machine, code, item->current);

		/* if we're doing a lightgun reload hack, override the value */
		if (state->lightgun_reload_button && item->devclass == DEVICE_CLASS_LIGHTGUN)
		{
			input_device_item *button2_item = state->device_list[item->devclass].list[item->devindex]->item[ITEM_ID_BUTTON2];
			if (button2_item != NULL)
			{
				/* if it is pressed, return (min,max) */
				input_item_update_value(machine, button2_item);
				if (button2_item->current)
					result = (INPUT_CODE_ITEMID(code) == ITEM_ID_XAXIS) ? INPUT_ABSOLUTE_MIN : INPUT_ABSOLUTE_MAX;
			}
		}

		/* standard axis: apply deadzone and saturation */
		if (modifier == ITEM_MODIFIER_NONE)
			return result;

		/* positive/negative: scale to full axis */
		else if (modifier == ITEM_MODIFIER_POS)
			return MAX(result, 0) * 2 + INPUT_ABSOLUTE_MIN;
		else if (modifier == ITEM_MODIFIER_NEG)
			return MAX(-result, 0) * 2 + INPUT_ABSOLUTE_MIN;
	}

	/* if we want a switch value, process it according to the modifier field */
	else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
	{
		input_item_modifier modifier = INPUT_CODE_MODIFIER(code);

		/* left/right/up/down: if this is a joystick, fetch the paired X/Y axis values and convert */
		if (modifier >= ITEM_MODIFIER_LEFT && modifier <= ITEM_MODIFIER_DOWN && item->devclass == DEVICE_CLASS_JOYSTICK)
		{
			input_device *device = state->device_list[item->devclass].list[item->devindex];
			input_device_item *xaxis_item = device->item[ITEM_ID_XAXIS];
			input_device_item *yaxis_item = device->item[ITEM_ID_YAXIS];
			if (xaxis_item != NULL && yaxis_item != NULL)
			{
				INT32 xaxisval, yaxisval;
				UINT8 mapval;

				/* determine which item we didn't update, and update it */
				assert(item == xaxis_item || item == yaxis_item);
				input_item_update_value(machine, (item == xaxis_item) ? yaxis_item : xaxis_item);

				/* now map the X and Y axes to a 9x9 grid */
				xaxisval = ((xaxis_item->current - INPUT_ABSOLUTE_MIN) * 9) / (INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN + 1);
				yaxisval = ((yaxis_item->current - INPUT_ABSOLUTE_MIN) * 9) / (INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN + 1);
				mapval = device->joymap.map[yaxisval][xaxisval];

				/* handle stickiness */
				if (mapval == JOYSTICK_MAP_STICKY)
					mapval = device->lastmap;
				else
					device->lastmap = mapval;

				/* return based on whether the appropriate bit is set */
				return (mapval >> (modifier - ITEM_MODIFIER_LEFT)) & 1;
			}
		}

		/* positive/negative: TRUE if past the deadzone in either direction */
		if (modifier == ITEM_MODIFIER_POS || modifier == ITEM_MODIFIER_RIGHT || modifier == ITEM_MODIFIER_DOWN)
			return (apply_deadzone_and_saturation(machine, code, item->current) > 0);
		else if (modifier == ITEM_MODIFIER_NEG || modifier == ITEM_MODIFIER_LEFT || modifier == ITEM_MODIFIER_UP)
			return (apply_deadzone_and_saturation(machine, code, item->current) < 0);
	}

	return 0;
}


/*-------------------------------------------------
    convert_relative_value - convert a relative
    value into the class specified by code
-------------------------------------------------*/

static INT32 convert_relative_value(input_code code, input_device_item *item)
{
	/* if we want relative values, take it as-is */
	if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_RELATIVE)
		return item->current;

	/* absolute value conversion not supported */
	else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_ABSOLUTE)
		return 0;

	/* if we want a switch value, process it according to the modifier field */
	else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
	{
		input_item_modifier modifier = INPUT_CODE_MODIFIER(code);

		/* positive/negative: TRUE if non-zero in either direction */
		if (modifier == ITEM_MODIFIER_POS || modifier == ITEM_MODIFIER_RIGHT || modifier == ITEM_MODIFIER_DOWN)
			return (item->current > 0);
		else if (modifier == ITEM_MODIFIER_NEG || modifier == ITEM_MODIFIER_LEFT || modifier == ITEM_MODIFIER_UP)
			return (item->current < 0);
	}

	return 0;
}


/*-------------------------------------------------
    convert_switch_value - convert a switch
    value into the class specified by code
-------------------------------------------------*/

static INT32 convert_switch_value(running_machine *machine, input_code code, input_device_item *item)
{
	input_private *state = machine->input_data;

	/* only a switch is supported */
	if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
	{
		/* if we're doing a lightgun reload hack, button 1 and 2 operate differently */
		if (state->lightgun_reload_button && item->devclass == DEVICE_CLASS_LIGHTGUN)
		{
			/* button 1 is pressed if either button 1 or 2 are active */
			if (INPUT_CODE_ITEMID(code) == ITEM_ID_BUTTON1)
			{
				input_device_item *button2_item = state->device_list[item->devclass].list[item->devindex]->item[ITEM_ID_BUTTON2];
				if (button2_item != NULL)
				{
					input_item_update_value(machine, button2_item);
					return item->current | button2_item->current;
				}
			}

			/* button 2 is never officially pressed */
			if (INPUT_CODE_ITEMID(code) == ITEM_ID_BUTTON2)
				return 0;
		}

		/* steadykey for keyboards */
		if (state->steadykey_enabled && item->devclass == DEVICE_CLASS_KEYBOARD)
			return item->steadykey;

		/* everything else is just the current value as-is */
		return item->current;
	}

	return 0;
}


/*-------------------------------------------------
    apply_deadzone_and_saturation - apply global
    deadzone and saturation parameters to an
    absolute value
-------------------------------------------------*/

static INT32 apply_deadzone_and_saturation(running_machine *machine, input_code code, INT32 result)
{
	input_private *state = machine->input_data;
	int negative = FALSE;

	/* ignore if not a joystick */
	if (INPUT_CODE_DEVCLASS(code) != DEVICE_CLASS_JOYSTICK)
		return result;

	/* properties are symmetric */
	if (result < 0)
	{
		negative = TRUE;
		result = -result;
	}

	/* if in the deadzone, return 0 */
	if (result < state->joystick_deadzone)
		result = 0;

	/* if saturated, return the max */
	else if (result > state->joystick_saturation)
		result = INPUT_ABSOLUTE_MAX;

	/* otherwise, scale */
	else
		result = (INT64)(result - state->joystick_deadzone) * (INT64)INPUT_ABSOLUTE_MAX / (INT64)(state->joystick_saturation - state->joystick_deadzone);

	/* apply sign and return */
	return negative ? -result : result;
}


/*-------------------------------------------------
    joystick_map_parse - parse a string into
    a joystick map
-------------------------------------------------*/

static int joystick_map_parse(const char *mapstring, joystick_map *map)
{
	int rownum, colnum;

	/* iterate over rows */
	for (rownum = 0; rownum < 9; rownum++)
	{
		/* if we're done, copy from another row */
		if (*mapstring == 0 || *mapstring == '.')
		{
			int symmetric = (rownum >= 5 && *mapstring == 0);
			const UINT8 *srcrow = &map->map[symmetric ? (8 - rownum) : (rownum - 1)][0];

			/* if this is row 0, we don't have a source row -- invalid */
			if (rownum == 0)
				return FALSE;

			/* copy from the srcrow, applying up/down symmetry if in the bottom half */
			for (colnum = 0; colnum < 9; colnum++)
			{
				UINT8 val = srcrow[colnum];
				if (symmetric)
					val = (val & (JOYSTICK_MAP_LEFT | JOYSTICK_MAP_RIGHT)) | ((val & JOYSTICK_MAP_UP) << 1) | ((val & JOYSTICK_MAP_DOWN) >> 1);
				map->map[rownum][colnum] = val;
			}
		}

		/* otherwise, parse this column */
		else
		{
			for (colnum = 0; colnum < 9; colnum++)
			{
				/* if we're at the end of row, copy previous to the middle, then apply left/right symmetry */
				if (colnum > 0 && (*mapstring == 0 || *mapstring == '.'))
				{
					int symmetric = (colnum >= 5);
					UINT8 val = map->map[rownum][symmetric ? (8 - colnum) : (colnum - 1)];
					if (symmetric)
						val = (val & (JOYSTICK_MAP_UP | JOYSTICK_MAP_DOWN)) | ((val & JOYSTICK_MAP_LEFT) << 1) | ((val & JOYSTICK_MAP_RIGHT) >> 1);
					map->map[rownum][colnum] = val;
				}

				/* otherwise, convert the character to its value */
				else
				{
					static const UINT8 charmap[] =
					{
						JOYSTICK_MAP_UP | JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_UP,
						JOYSTICK_MAP_UP | JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_NEUTRAL,
						JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_DOWN | JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_DOWN,
						JOYSTICK_MAP_DOWN | JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_STICKY
					};
					static const char validchars[] = "789456123s";
					const char *ptr = strchr(validchars, *mapstring++);

					/* invalid characters exit immediately */
					if (ptr == NULL)
						return FALSE;
					map->map[rownum][colnum] = charmap[ptr - validchars];
				}
			}
		}

		/* if we ended with a period, advance to the next row */
		if (*mapstring == '.')
			mapstring++;
	}

	return TRUE;
}


/*-------------------------------------------------
    joystick_map_print - print a joystick map via
    the verbose output
-------------------------------------------------*/

static void joystick_map_print(const char *header, const char *origstring, const joystick_map *map)
{
	int rownum, colnum;

	mame_printf_verbose("%s = %s\n", header, origstring);
	for (rownum = 0; rownum < 9; rownum++)
	{
		mame_printf_verbose("  ");
		for (colnum = 0; colnum < 9; colnum++)
			switch (map->map[rownum][colnum])
			{
				case JOYSTICK_MAP_UP | JOYSTICK_MAP_LEFT:	mame_printf_verbose("7");	break;
				case JOYSTICK_MAP_UP:						mame_printf_verbose("8");	break;
				case JOYSTICK_MAP_UP | JOYSTICK_MAP_RIGHT:	mame_printf_verbose("9");	break;
				case JOYSTICK_MAP_LEFT:						mame_printf_verbose("4");	break;
				case JOYSTICK_MAP_NEUTRAL:					mame_printf_verbose("5");	break;
				case JOYSTICK_MAP_RIGHT:					mame_printf_verbose("6");	break;
				case JOYSTICK_MAP_DOWN | JOYSTICK_MAP_LEFT:	mame_printf_verbose("1");	break;
				case JOYSTICK_MAP_DOWN:						mame_printf_verbose("2");	break;
				case JOYSTICK_MAP_DOWN | JOYSTICK_MAP_RIGHT:mame_printf_verbose("3");	break;
				case JOYSTICK_MAP_STICKY:					mame_printf_verbose("s");	break;
				default:									mame_printf_verbose("?");	break;
			}

		mame_printf_verbose("\n");
	}
}
