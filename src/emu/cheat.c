/*********************************************************************

    cheat.c

    MAME cheat system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

    Cheat XML format:

    <mamecheat version="1">
        <cheat desc="blah">
           <parameter min="minval(0)" max="maxval(numitems)" step="stepval(1)">
              <item value="itemval(previtemval|minval+stepval)">text</item>
              ...
           </parameter>
           <script state="on|off|run|change(run)">
              <action condition="condexpr(1)">expression</action>
              ...
              <output condition="condexpr(1)" format="format(required)" line="line(0)" align="left|center|right(left)">
                 <argument count="count(1)">expression</argument>
              </output>
              ...
           </script>
           ...
           <comment>
              ... text ...
           </comment>
        </cheat>
        ...
    </mamecheat>

**********************************************************************

    Expressions are standard debugger expressions. Note that & and
    < must be escaped per XML rules. Within attributes you must use
    &amp; and &lt;. For tags, you can also use <![CDATA[ ... ]]>.

    Each cheat has its own context-specific variables:

        temp0-temp9 -- 10 temporary variables for any use
        param       -- the current value of the cheat parameter
        frame       -- the current frame index
        argindex    -- for arguments with multiple iterations, this is the index

    By default, each cheat has 10 temporary variables that are
    persistent while executing its scripts. Additional temporary
    variables may be requested via the 'tempvariables' attribute
    on the cheat.

**********************************************************************

    Cheats are generally broken down into categories based on
    which actions are defined and whether or not there is a
    parameter present:

    ---- Actions -----
    On   Off  Run  Chg  Param?  Type
    ===  ===  ===  ===  ======  =================================
     N    N    N    ?    None   Text-only (displays text in menu)
     Y    N    N    ?    None   Oneshot (select to activate)
     Y    Y    N    ?    None   On/Off (select to toggle)
     ?    ?    Y    ?    None   On/Off (select to toggle)

     ?    N    N    Y    Any    Oneshot parameter (select to alter)
     ?    Y    ?    ?    Value  Value parameter (off or a live value)
     ?    ?    Y    ?    Value  Value parameter (off or a live value)
     ?    Y    ?    ?    List   Item list parameter (off or a live value)
     ?    ?    Y    ?    List   Item list parameter (off or a live value)

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "xmlfile.h"
#include "ui.h"
#include "uimenu.h"
#include "cheat.h"
#include "debug/debugcpu.h"
#include "debug/express.h"

#include <ctype.h>




/***************************************************************************
    CONSTANTS
***************************************************************************/

/* turn this on to enable removing duplicate cheats; not sure if we should */
#define REMOVE_DUPLICATE_CHEATS	0

#define CHEAT_VERSION			1

#define DEFAULT_TEMP_VARIABLES	10
#define MAX_ARGUMENTS			32

enum _script_state
{
	SCRIPT_STATE_OFF = 0,
	SCRIPT_STATE_ON,
	SCRIPT_STATE_RUN,
	SCRIPT_STATE_CHANGE,
	SCRIPT_STATE_COUNT
};
typedef enum _script_state script_state;
DECLARE_ENUM_OPERATORS(script_state)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a single item (string + value) for a cheat parameter */
typedef struct _parameter_item parameter_item;
struct _parameter_item
{
	parameter_item *	next;							/* next item in list */
	astring				text;							/* name of the item */
	UINT64				value;							/* value of the item */
	int					valformat;						/* format of value */
	astring				curtext;						/* name of the current item */
};


/* a parameter for a cheat, which can be set in the UI */
typedef struct _cheat_parameter cheat_parameter;
struct _cheat_parameter
{
	UINT64				minval;							/* minimum value */
	int					minformat;						/* format of minimum value */
	UINT64				maxval;							/* maximum value */
	int					maxformat;						/* format of maximum value */
	UINT64				stepval;						/* step value */
	int					stepformat;						/* format of step value */
	UINT64				value;							/* live value of the parameter */
	char				valuestring[32];				/* small space for a value string */
	parameter_item *	itemlist;						/* list of items */
};


/* a single argument for an output display */
typedef struct _output_argument output_argument;
struct _output_argument
{
	output_argument *	next;							/* link to next argument */
	parsed_expression *	expression;						/* expression for argument */
	UINT64				count;							/* number of repetitions */
};


/* a single entry within a script, either an expression to execute or a string to output */
typedef struct _script_entry script_entry;
struct _script_entry
{
	script_entry *		next;							/* link to next entry */
	parsed_expression *	condition;						/* condition under which this is executed */
	parsed_expression *	expression;						/* expression to execute */
	astring				format;							/* string format to print */
	output_argument *	arglist;						/* list of arguments */
	INT8				line;							/* which line to print on */
	UINT8				justify;						/* justification when printing */
};


/* a script entry, specifying which state to execute under */
typedef struct _cheat_script cheat_script;
struct _cheat_script
{
	script_entry *		entrylist;						/* list of actions to perform */
	script_state		state;							/* which state this script is for */
};


/* a single cheat */
typedef struct _cheat_entry cheat_entry;
struct _cheat_entry
{
	cheat_entry *		next;							/* next cheat entry */
	astring				description;					/* string description/menu title */
	astring				comment;						/* comment data */
	cheat_parameter *	parameter;						/* parameter */
	cheat_script *		script[SCRIPT_STATE_COUNT];		/* up to 1 script for each state */
	symbol_table *		symbols;						/* symbol table for this cheat */
	script_state		state;							/* current cheat state */
	UINT32				numtemp;						/* number of temporary variables */
	UINT64				argindex;						/* argument index variable */
	UINT64 *			tempvar;						/* value of the temporary variables */
};


/* private machine-global data */
struct _cheat_private
{
	cheat_entry *		cheatlist;						/* cheat list */
	UINT64				framecount;						/* frame count */
	astring				output[UI_TARGET_FONT_ROWS*2];	/* array of output strings */
	UINT8				justify[UI_TARGET_FONT_ROWS*2];	/* justification for each string */
	UINT8				numlines;						/* number of lines available for output */
	INT8				lastline;						/* last line used for output */
	UINT8				disabled;						/* true if the cheat engine is disabled */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void cheat_exit(running_machine &machine);
static void cheat_frame(running_machine &machine);
static void cheat_execute_script(cheat_private *cheatinfo, cheat_entry *cheat, script_state state);

static cheat_entry *cheat_list_load(running_machine *machine, const char *filename);
static int cheat_list_save(const char *filename, const cheat_entry *cheatlist);
static void cheat_list_free(running_machine *machine, cheat_entry *cheat);
static cheat_entry *cheat_entry_load(running_machine *machine, const char *filename, xml_data_node *cheatnode);
static void cheat_entry_save(mame_file *cheatfile, const cheat_entry *cheat);
static void cheat_entry_free(running_machine *machine, cheat_entry *cheat);
static cheat_parameter *cheat_parameter_load(running_machine *machine, const char *filename, xml_data_node *paramnode);
static void cheat_parameter_save(mame_file *cheatfile, const cheat_parameter *param);
static void cheat_parameter_free(running_machine *machine, cheat_parameter *param);
static cheat_script *cheat_script_load(running_machine *machine, const char *filename, xml_data_node *scriptnode, cheat_entry *cheat);
static void cheat_script_save(mame_file *cheatfile, const cheat_script *script);
static void cheat_script_free(running_machine *machine, cheat_script *script);
static script_entry *script_entry_load(running_machine *machine, const char *filename, xml_data_node *entrynode, cheat_entry *cheat, int isaction);
static void script_entry_save(mame_file *cheatfile, const script_entry *entry);
static void script_entry_free(running_machine *machine, script_entry *entry);

static astring &quote_astring_expression(astring &string, int isattribute);
static int validate_format(const char *filename, int line, const script_entry *entry);
static UINT64 cheat_variable_get(void *globalref, void *ref);
static void cheat_variable_set(void *globalref, void *ref, UINT64 value);
static UINT64 execute_frombcd(void *globalref, void *ref, UINT32 params, const UINT64 *param);
static UINT64 execute_tobcd(void *globalref, void *ref, UINT32 params, const UINT64 *param);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    is_text_only_cheat - return TRUE if this
    cheat entry is text-only (no actions)
-------------------------------------------------*/

INLINE int is_text_only_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter == NULL &&
			cheat->script[SCRIPT_STATE_RUN] == NULL &&
			cheat->script[SCRIPT_STATE_OFF] == NULL &&
			cheat->script[SCRIPT_STATE_ON] == NULL);
}


/*-------------------------------------------------
    is_oneshot_cheat - return TRUE if this cheat
    entry is a one-shot cheat (no "run" or "off"
    action, but a valid "on" action)
-------------------------------------------------*/

INLINE int is_oneshot_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter == NULL &&
			cheat->script[SCRIPT_STATE_RUN] == NULL &&
			cheat->script[SCRIPT_STATE_OFF] == NULL &&
			cheat->script[SCRIPT_STATE_ON] != NULL);
}


/*-------------------------------------------------
    is_onoff_cheat - return TRUE if this cheat
    entry is a simple on/off toggle (either the
    "run" or "off" actions are present)
-------------------------------------------------*/

INLINE int is_onoff_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter == NULL &&
			(cheat->script[SCRIPT_STATE_RUN] != NULL ||
			 (cheat->script[SCRIPT_STATE_OFF] != NULL &&
			  cheat->script[SCRIPT_STATE_ON] != NULL)));
}


/*-------------------------------------------------
    is_value_parameter_cheat - return TRUE if this
    cheat entry has a parameter represented by an
    integer value
-------------------------------------------------*/

INLINE int is_value_parameter_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter != NULL && cheat->parameter->itemlist == NULL);
}


/*-------------------------------------------------
    is_itemlist_parameter_cheat - return TRUE if
    this cheat entry has a parameter represented
    by an item list
-------------------------------------------------*/

INLINE int is_itemlist_parameter_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter != NULL && cheat->parameter->itemlist != NULL);
}


/*-------------------------------------------------
    is_oneshot_parameter_cheat - return TRUE if
    this cheat entry is a one-shot cheat with a
    parameter (no "run" or "off" actions, but a
    valid "change" action)
-------------------------------------------------*/

INLINE int is_oneshot_parameter_cheat(const cheat_entry *cheat)
{
	return (cheat->parameter != NULL &&
			cheat->script[SCRIPT_STATE_RUN] == NULL &&
			cheat->script[SCRIPT_STATE_OFF] == NULL &&
			cheat->script[SCRIPT_STATE_CHANGE] != NULL);
}


/*-------------------------------------------------
    format_int - format an integer according to
    the format
-------------------------------------------------*/

INLINE const char *format_int(astring &string, UINT64 value, int format)
{
	switch (format)
	{
		default:
		case XML_INT_FORMAT_DECIMAL:
			string.printf("%d", (UINT32)value);
			break;

		case XML_INT_FORMAT_DECIMAL_POUND:
			string.printf("#%d", (UINT32)value);
			break;

		case XML_INT_FORMAT_HEX_DOLLAR:
			string.printf("$%X", (UINT32)value);
			break;

		case XML_INT_FORMAT_HEX_C:
			string.printf("0x%X", (UINT32)value);
			break;
	}
	return string;
}



/***************************************************************************
    SYSTEM INTERACTION
***************************************************************************/

/*-------------------------------------------------
    cheat_init - initialize the cheat engine,
    loading the cheat file
-------------------------------------------------*/

void cheat_init(running_machine *machine)
{
	cheat_private *cheatinfo;

	/* request a callback */
	machine->add_notifier(MACHINE_NOTIFY_FRAME, cheat_frame);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, cheat_exit);

	/* allocate memory */
	cheatinfo = auto_alloc_clear(machine, cheat_private);
	machine->cheat_data = cheatinfo;

	/* load the cheats */
	cheat_reload(machine);

	/* we rely on the debugger expression callbacks; if the debugger isn't
       enabled, we must jumpstart them manually */
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) == 0)
		debug_cpu_init(machine);
}


/*-------------------------------------------------
    cheat_reload - re-initialize the cheat engine,
    and reload the cheat file(s)
-------------------------------------------------*/

void cheat_reload(running_machine *machine)
{
	cheat_private *cheatinfo = machine->cheat_data;

	/* free everything */
	cheat_exit(*machine);

	/* reset our memory */
	auto_free(machine, cheatinfo);
	cheatinfo = machine->cheat_data = auto_alloc_clear(machine, cheat_private);

	/* load the cheat file, MESS will load a crc32.xml ( eg. 01234567.xml )
       and MAME will load gamename.xml */
	device_image_interface *image = NULL;
	for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
	{
		if (image->exists())
		{
			char mess_cheat_filename[9];
			UINT32	crc = image->crc();
			sprintf(mess_cheat_filename, "%08X", crc);
			if (crc!=0) {
				cheatinfo->cheatlist = cheat_list_load(machine, mess_cheat_filename);
				break;
			}
		}
	}
	if (cheatinfo->cheatlist == NULL)
	{
		cheatinfo->cheatlist = cheat_list_load(machine, machine->basename());
	}

	/* temporary: save the file back out as output.xml for comparison */
	if (cheatinfo->cheatlist != NULL)
		cheat_list_save("output", cheatinfo->cheatlist);
}


/*-------------------------------------------------
    cheat_exit - clean up on the way out
-------------------------------------------------*/

static void cheat_exit(running_machine &machine)
{
	cheat_private *cheatinfo = machine.cheat_data;

	/* free the list of cheats */
	if (cheatinfo->cheatlist != NULL)
		cheat_list_free(&machine, cheatinfo->cheatlist);
}


/*-------------------------------------------------
    cheat_get_global_enable - return the global
    enabled state of the cheat engine
-------------------------------------------------*/

int cheat_get_global_enable(running_machine *machine)
{
	cheat_private *cheatinfo = machine->cheat_data;

	if (cheatinfo != NULL)
	{
		return !cheatinfo->disabled;
	}

	return 0;
}


/*-------------------------------------------------
    cheat_set_global_enable - globally enable or
    disable the cheat engine
-------------------------------------------------*/

void cheat_set_global_enable(running_machine *machine, int enable)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *cheat;

	if (cheatinfo != NULL)
	{
		/* if we're enabled currently and we don't want to be, turn things off */
		if (!cheatinfo->disabled && !enable)
		{
			/* iterate over running cheats and execute any OFF Scripts */
			for (cheat = cheatinfo->cheatlist; cheat != NULL; cheat = cheat->next)
				if (cheat->state == SCRIPT_STATE_RUN)
					cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_OFF);
			popmessage("Cheats Disabled");
			cheatinfo->disabled = TRUE;
		}

		/* if we're disabled currently and we want to be enabled, turn things on */
		else if (cheatinfo->disabled && enable)
		{
			/* iterate over running cheats and execute any ON Scripts */
			cheatinfo->disabled = FALSE;
			for (cheat = cheatinfo->cheatlist; cheat != NULL; cheat = cheat->next)
				if (cheat->state == SCRIPT_STATE_RUN)
					cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
			popmessage("Cheats Enabled");
		}
	}
}



/***************************************************************************
    CHEAT UI
***************************************************************************/

/*-------------------------------------------------
    cheat_render_text - called by the UI system
    to render text
-------------------------------------------------*/

void cheat_render_text(running_machine *machine, render_container *container)
{
	cheat_private *cheatinfo = machine->cheat_data;
	if (cheatinfo != NULL)
	{
		int linenum;

		/* render any text and free it along the way */
		for (linenum = 0; linenum < ARRAY_LENGTH(cheatinfo->output); linenum++)
			if (cheatinfo->output[linenum].len() != 0)
			{
				/* output the text */
				ui_draw_text_full(container, cheatinfo->output[linenum],
						0.0f, (float)linenum * ui_get_line_height(), 1.0f,
						cheatinfo->justify[linenum], WRAP_NEVER, DRAW_OPAQUE,
						ARGB_WHITE, ARGB_BLACK, NULL, NULL);
			}
	}
}


/*-------------------------------------------------
    cheat_get_next_menu_entry - return the
    text needed to display this cheat in a menu
    item
-------------------------------------------------*/

void *cheat_get_next_menu_entry(running_machine *machine, void *previous, const char **description, const char **state, UINT32 *flags)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *preventry = (cheat_entry *)previous;
	cheat_entry *cheat;

	/* NULL previous means get the first */
	cheat = (preventry == NULL) ? cheatinfo->cheatlist : preventry->next;
	if (cheat == NULL)
		return NULL;

	/* description is standard */
	if (description != NULL)
		*description = cheat->description;

	/* some cheat entries are just text for display */
	if (is_text_only_cheat(cheat))
	{
		if (description != NULL)
		{
			while (isspace((UINT8)**description))
				*description += 1;
			if (**description == 0)
				*description = MENU_SEPARATOR_ITEM;
		}
		if (state != NULL)
			*state = NULL;
		if (flags != NULL)
			*flags = MENU_FLAG_DISABLE;
	}

	/* if we have no parameter and no run or off script, it's a oneshot cheat */
	else if (is_oneshot_cheat(cheat))
	{
		if (state != NULL)
			*state = "Set";
		if (flags != NULL)
			*flags = 0;
	}

	/* if we have no parameter, it's just on/off */
	else if (is_onoff_cheat(cheat))
	{
		if (state != NULL)
			*state = (cheat->state == SCRIPT_STATE_RUN) ? "On" : "Off";
		if (flags != NULL)
			*flags = cheat->state ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW;
	}

	/* if we have a value parameter, compute it */
	else if (is_value_parameter_cheat(cheat))
	{
		if (cheat->state == SCRIPT_STATE_OFF)
		{
			if (state != NULL)
				*state = is_oneshot_parameter_cheat(cheat) ? "Set" : "Off";
			if (flags != NULL)
				*flags = MENU_FLAG_RIGHT_ARROW;
		}
		else
		{
			if (state != NULL)
			{
				sprintf(cheat->parameter->valuestring, "%d", (UINT32)cheat->parameter->value);
				*state = cheat->parameter->valuestring;
			}
			if (flags != NULL)
			{
				*flags = MENU_FLAG_LEFT_ARROW;
				if (cheat->parameter->value < cheat->parameter->maxval)
					*flags |= MENU_FLAG_RIGHT_ARROW;
			}
		}
	}

	/* if we have an item list, pick the index */
	else if (is_itemlist_parameter_cheat(cheat))
	{
		if (cheat->state == SCRIPT_STATE_OFF)
		{
			if (state != NULL)
				*state = is_oneshot_parameter_cheat(cheat) ? "Set" : "Off";
			if (flags != NULL)
				*flags = MENU_FLAG_RIGHT_ARROW;
		}
		else
		{
			parameter_item *item = NULL/*, *prev = NULL*/;

			for (item = cheat->parameter->itemlist; item != NULL; /*prev = item, */item = item->next)
				if (item->value == cheat->parameter->value)
					break;
			if (state != NULL)
				*state = (item != NULL) ? item->text.cstr() : "??Invalid??";
			if (flags != NULL)
			{
				*flags = MENU_FLAG_LEFT_ARROW;
				if (item == NULL || item->next != NULL)
					*flags |= MENU_FLAG_RIGHT_ARROW;
				cheat->parameter->itemlist->curtext = item->text; /* Take a copy of the most current parameter for the popmessage (if used) */
			}
		}
	}

	/* return a pointer to this item */
	return cheat;
}


/*-------------------------------------------------
    cheat_activate - activate a oneshot cheat
-------------------------------------------------*/

int cheat_activate(running_machine *machine, void *entry)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *cheat = (cheat_entry *)entry;
	int changed = FALSE;

	/* if cheats have been toggled off no point in even trying to do anything */
	if (cheatinfo->disabled)
		return changed;

	/* if we have no parameter and no run or off script, but we do have an on script, it's a oneshot cheat */
	if (is_oneshot_cheat(cheat))
	{
		cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
		changed = TRUE;
		popmessage("Activated %s", cheat->description.cstr());
	}

	/* if we have no run or off script, but we do have parameter and change scripts and it's not in the off state, it's a oneshot list or selectable value cheat */
	else if (is_oneshot_parameter_cheat(cheat) && cheat->state != SCRIPT_STATE_OFF)
	{
		cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
		changed = TRUE;
		if (cheat->parameter->itemlist != NULL)
			popmessage("Activated\n %s = %s", cheat->description.cstr(), cheat->parameter->itemlist->curtext.cstr() );
		else
			popmessage("Activated\n %s = %d (0x%X)", cheat->description.cstr(), (UINT32)cheat->parameter->value, (UINT32)cheat->parameter->value );
	}

	return changed;
}


/*-------------------------------------------------
    cheat_select_default_state - select the
    default state for a cheat, or activate a
    oneshot cheat
-------------------------------------------------*/

int cheat_select_default_state(running_machine *machine, void *entry)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *cheat = (cheat_entry *)entry;
	int changed = FALSE;

	/* if we have no parameter and no run or off script, it's either text or a oneshot cheat */
	if (is_oneshot_cheat(cheat))
		;

	/* if we have no parameter, it's just on/off; default to off */
	else
	{
		if (cheat->state != SCRIPT_STATE_OFF)
		{
			cheat->state = SCRIPT_STATE_OFF;
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_OFF);
			changed = TRUE;
		}
	}
	return changed;
}


/*-------------------------------------------------
    cheat_select_previous_state - select the
    previous state for a cheat
-------------------------------------------------*/

int cheat_select_previous_state(running_machine *machine, void *entry)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *cheat = (cheat_entry *)entry;
	int changed = FALSE;

	/* if we have no parameter and no run or off script, it's either text or a oneshot cheat */
	if (is_oneshot_cheat(cheat))
		;

	/* if we have no parameter, it's just on/off */
	else if (is_onoff_cheat(cheat))
	{
		if (cheat->state != SCRIPT_STATE_OFF)
		{
			cheat->state = SCRIPT_STATE_OFF;
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_OFF);
			changed = TRUE;
		}
	}

	/* if we have a value parameter, compute it */
	else if (is_value_parameter_cheat(cheat))
	{
		if (cheat->parameter->value > cheat->parameter->minval)
		{
			if (cheat->parameter->value < cheat->parameter->minval + cheat->parameter->stepval)
				cheat->parameter->value = cheat->parameter->minval;
			else
				cheat->parameter->value -= cheat->parameter->stepval;
			if (!is_oneshot_parameter_cheat(cheat))
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
			changed = TRUE;
		}
		else if (cheat->state != SCRIPT_STATE_OFF)
		{
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_OFF);
			cheat->state = SCRIPT_STATE_OFF;
			changed = TRUE;
		}
	}

	/* if we have an item list, pick the index */
	else
	{
		parameter_item *item, *prev = NULL;

		for (item = cheat->parameter->itemlist; item != NULL; prev = item, item = item->next)
			if (item->value == cheat->parameter->value)
				break;
		if (prev != NULL)
		{
			if (cheat->state == SCRIPT_STATE_OFF)
			{
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
				cheat->state = SCRIPT_STATE_RUN;
			}
			cheat->parameter->value = prev->value;
			if (!is_oneshot_parameter_cheat(cheat))
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
			changed = TRUE;
		}
		else if (cheat->state != SCRIPT_STATE_OFF)
		{
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_OFF);
			cheat->state = SCRIPT_STATE_OFF;
			changed = TRUE;
		}
	}
	return changed;
}


/*-------------------------------------------------
    cheat_select_next_state - select the
    next state for a cheat
-------------------------------------------------*/

int cheat_select_next_state(running_machine *machine, void *entry)
{
	cheat_private *cheatinfo = machine->cheat_data;
	cheat_entry *cheat = (cheat_entry *)entry;
	int changed = FALSE;

	/* if we have no parameter and no run or off script, it's a oneshot cheat */
	if (is_oneshot_cheat(cheat))
		;

	/* if we have no parameter, it's just on/off */
	else if (is_onoff_cheat(cheat))
	{
		if (cheat->state != SCRIPT_STATE_RUN)
		{
			cheat->state = SCRIPT_STATE_RUN;
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
			changed = TRUE;
		}
	}

	/* if we have a value parameter, compute it */
	else if (is_value_parameter_cheat(cheat))
	{
		if (cheat->state == SCRIPT_STATE_OFF)
		{
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
			cheat->state = SCRIPT_STATE_RUN;
			cheat->parameter->value = cheat->parameter->minval;
			if (!is_oneshot_parameter_cheat(cheat))
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
			changed = TRUE;
		}
		else if (cheat->parameter->value < cheat->parameter->maxval)
		{
			if (cheat->parameter->value > cheat->parameter->maxval - cheat->parameter->stepval)
				cheat->parameter->value = cheat->parameter->maxval;
			else
				cheat->parameter->value += cheat->parameter->stepval;
			if (!is_oneshot_parameter_cheat(cheat))
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
			changed = TRUE;
		}
	}

	/* if we have an item list, pick the index */
	else
	{
		parameter_item *item;

		if (cheat->state == SCRIPT_STATE_OFF)
		{
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_ON);
			cheat->state = SCRIPT_STATE_RUN;
			cheat->parameter->value = cheat->parameter->itemlist->value;
			if (!is_oneshot_parameter_cheat(cheat))
				cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
			changed = TRUE;
		}
		else
		{
			for (item = cheat->parameter->itemlist; item != NULL; item = item->next)
				if (item->value == cheat->parameter->value)
					break;
			if (item->next != NULL)
			{
				cheat->parameter->value = item->next->value;
				if (!is_oneshot_parameter_cheat(cheat))
					cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_CHANGE);
				changed = TRUE;
			}
		}
	}
	return changed;
}


/*-------------------------------------------------
    cheat_get_comment - called by the UI system
    to help render displayable comments
-------------------------------------------------*/

astring &cheat_get_comment(void *entry)
{
	cheat_entry *cheat = (cheat_entry *)entry;
	return cheat->comment;
}



/***************************************************************************
    CHEAT EXECUTION
***************************************************************************/

/*-------------------------------------------------
    cheat_frame - per-frame callback
-------------------------------------------------*/

static void cheat_frame(running_machine &machine)
{
	cheat_private *cheatinfo = machine.cheat_data;
	cheat_entry *cheat;
	int linenum;

	/* set up for accumulating output */
	cheatinfo->lastline = 0;
	cheatinfo->numlines = floor(1.0f / ui_get_line_height());
	cheatinfo->numlines = MIN(cheatinfo->numlines, ARRAY_LENGTH(cheatinfo->output));
	for (linenum = 0; linenum < ARRAY_LENGTH(cheatinfo->output); linenum++)
		cheatinfo->output[linenum].reset();

	/* iterate over running cheats and execute them */
	for (cheat = cheatinfo->cheatlist; cheat != NULL; cheat = cheat->next)
		if (cheat->state == SCRIPT_STATE_RUN)
			cheat_execute_script(cheatinfo, cheat, SCRIPT_STATE_RUN);

	/* increment the frame counter */
	cheatinfo->framecount++;
}


/*-------------------------------------------------
    cheat_execute_script - execute the
    appropriate script
-------------------------------------------------*/

static void cheat_execute_script(cheat_private *cheatinfo, cheat_entry *cheat, script_state state)
{
	script_entry *entry;

	/* if cheat engine has been temporarily disabled or no script, bail */
	if (cheatinfo->disabled || cheat->script[state] == NULL)
		return;

	/* iterate over entries */
	for (entry = cheat->script[state]->entrylist; entry != NULL; entry = entry->next)
	{
		EXPRERR error;
		UINT64 result;

		/* evaluate the condition */
		if (entry->condition != NULL)
		{
			error = expression_execute(entry->condition, &result);
			if (error != EXPRERR_NONE)
				mame_printf_warning("Error executing conditional expression \"%s\": %s\n", expression_original_string(entry->condition), exprerr_to_string(error));

			/* if the condition is false, or we got an error, don't execute */
			if (error != EXPRERR_NONE || result == 0)
				continue;
		}

		/* if there is an action, execute it */
		if (entry->expression != NULL)
		{
			error = expression_execute(entry->expression, &result);
			if (error != EXPRERR_NONE)
				mame_printf_warning("Error executing expression \"%s\": %s\n", expression_original_string(entry->expression), exprerr_to_string(error));
		}

		/* if there is a string to display, compute it */
		if (entry->format.len() != 0)
		{
			UINT64 params[MAX_ARGUMENTS];
			output_argument *arg;
			int curarg = 0;
			int row;

			/* iterate over arguments and evaluate them */
			for (arg = entry->arglist; arg != NULL; arg = arg->next)
				for (cheat->argindex = 0; cheat->argindex < arg->count; cheat->argindex++)
				{
					error = expression_execute(arg->expression, &params[curarg++]);
					if (error != EXPRERR_NONE)
						mame_printf_warning("Error executing argument expression \"%s\": %s\n", expression_original_string(arg->expression), exprerr_to_string(error));
				}

			/* determine which row we belong to */
			row = entry->line;
			if (row == 0)
				row = (cheatinfo->lastline >= 0) ? cheatinfo->lastline + 1 : cheatinfo->lastline - 1;
			cheatinfo->lastline = row;
			row = (row < 0) ? cheatinfo->numlines + row : row - 1;
			row = MAX(row, 0);
			row = MIN(row, cheatinfo->numlines - 1);

			/* either re-use or allocate a string */
			astring &string = cheatinfo->output[row];
			cheatinfo->justify[row] = entry->justify;

			/* generate the astring */
			string.printf(entry->format,
				(UINT32)params[0],  (UINT32)params[1],  (UINT32)params[2],  (UINT32)params[3],
				(UINT32)params[4],  (UINT32)params[5],  (UINT32)params[6],  (UINT32)params[7],
				(UINT32)params[8],  (UINT32)params[9],  (UINT32)params[10], (UINT32)params[11],
				(UINT32)params[12], (UINT32)params[13], (UINT32)params[14], (UINT32)params[15],
				(UINT32)params[16], (UINT32)params[17], (UINT32)params[18], (UINT32)params[19],
				(UINT32)params[20], (UINT32)params[21], (UINT32)params[22], (UINT32)params[23],
				(UINT32)params[24], (UINT32)params[25], (UINT32)params[26], (UINT32)params[27],
				(UINT32)params[28], (UINT32)params[29], (UINT32)params[30], (UINT32)params[31]);
		}
	}
}



/***************************************************************************
    CHEAT FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    cheat_list_load - load a cheat file into
    memory and create the cheat entry list
-------------------------------------------------*/

static cheat_entry *cheat_list_load(running_machine *machine, const char *filename)
{
	xml_data_node *rootnode = NULL;
	cheat_entry *cheatlist = NULL;
	cheat_entry **cheattailptr = &cheatlist;
	mame_file *cheatfile = NULL;
	file_error filerr;

	/* open the file with the proper name */
	astring fname(filename, ".xml");
	filerr = mame_fopen(SEARCHPATH_CHEAT, fname, OPEN_FLAG_READ, &cheatfile);

	/* loop over all instrances of the files found in our search paths */
	while (filerr == FILERR_NONE)
	{
		xml_data_node *mamecheatnode, *cheatnode;
		xml_parse_options options;
		xml_parse_error error;
		cheat_entry *scannode;
		int version;

		mame_printf_verbose("Loading cheats file from %s\n", mame_file_full_name(cheatfile).cstr());

		/* read the XML file into internal data structures */
		memset(&options, 0, sizeof(options));
		options.error = &error;
		rootnode = xml_file_read(mame_core_file(cheatfile), &options);

		/* if unable to parse the file, just bail */
		if (rootnode == NULL)
		{
			mame_printf_error("%s.xml(%d): error parsing XML (%s)\n", filename, error.error_line, error.error_message);
			goto error;
		}

		/* find the layout node */
		mamecheatnode = xml_get_sibling(rootnode->child, "mamecheat");
		if (mamecheatnode == NULL)
		{
			mame_printf_error("%s.xml: missing mamecheatnode node", filename);
			goto error;
		}

		/* validate the config data version */
		version = xml_get_attribute_int(mamecheatnode, "version", 0);
		if (version != CHEAT_VERSION)
		{
			mame_printf_error("%s.xml(%d): Invalid cheat XML file: unsupported version", filename, mamecheatnode->line);
			goto error;
		}

		/* parse all the elements */
		for (cheatnode = xml_get_sibling(mamecheatnode->child, "cheat"); cheatnode != NULL; cheatnode = xml_get_sibling(cheatnode->next, "cheat"))
		{
			/* load this entry */
			cheat_entry *curcheat = cheat_entry_load(machine, filename, cheatnode);
			if (curcheat == NULL)
				goto error;

			/* make sure we're not a duplicate */
			scannode = NULL;
			if (REMOVE_DUPLICATE_CHEATS)
				for (scannode = cheatlist; scannode != NULL; scannode = scannode->next)
					if (scannode->description.cmp(curcheat->description) == 0)
					{
						mame_printf_verbose("Ignoring duplicate cheat '%s' from file %s\n", curcheat->description.cstr(), mame_file_full_name(cheatfile).cstr());
						break;
					}

			/* add to the end of the list */
			if (scannode == NULL)
			{
				*cheattailptr = curcheat;
				cheattailptr = &curcheat->next;
			}
		}

		/* free the file and loop for the next one */
		xml_file_free(rootnode);

		/* open the next file in sequence */
		filerr = mame_fclose_and_open_next(&cheatfile, fname, OPEN_FLAG_READ);
	}

	/* return the cheat list */
	return cheatlist;

error:
	cheat_list_free(machine, cheatlist);
	xml_file_free(rootnode);
	if (cheatfile != NULL)
		mame_fclose(cheatfile);
	return NULL;
}


/*-------------------------------------------------
    cheat_list_save - save a cheat file from
    memory to the given filename
-------------------------------------------------*/

static int cheat_list_save(const char *filename, const cheat_entry *cheatlist)
{
	mame_file *cheatfile;
	file_error filerr;

	/* open the file with the proper name */
	astring fname(filename, ".xml");
	filerr = mame_fopen(SEARCHPATH_CHEAT, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &cheatfile);

	/* if that failed, return nothing */
	if (filerr != FILERR_NONE)
		return FALSE;

	/* output the outer layers */
	mame_fprintf(cheatfile, "<?xml version=\"1.0\"?>\n");
	mame_fprintf(cheatfile, "<!-- This file is autogenerated; comments and unknown tags will be stripped -->\n");
	mame_fprintf(cheatfile, "<mamecheat version=\"%d\">\n", CHEAT_VERSION);

	/* iterate over cheats in the list and save them */
	for ( ; cheatlist != NULL; cheatlist = cheatlist->next)
		cheat_entry_save(cheatfile, cheatlist);

	/* close out the file */
	mame_fprintf(cheatfile, "</mamecheat>\n");
	mame_fclose(cheatfile);
	return TRUE;
}


/*-------------------------------------------------
    cheat_list_free - free a list of cheats
-------------------------------------------------*/

static void cheat_list_free(running_machine *machine, cheat_entry *cheat)
{
	while (cheat != NULL)
	{
		cheat_entry *entry = cheat;
		cheat = entry->next;
		cheat_entry_free(machine, entry);
	}
}


/*-------------------------------------------------
    cheat_entry_load - load a single cheat
    entry and create the underlying data
    structures
-------------------------------------------------*/

static cheat_entry *cheat_entry_load(running_machine *machine, const char *filename, xml_data_node *cheatnode)
{
	cheat_private *cheatinfo = machine->cheat_data;
	xml_data_node *paramnode, *scriptnode, *commentnode;
	const char *description;
	int tempcount, curtemp;
	cheat_entry *cheat;

	/* pull the variable count out ahead of things */
	tempcount = xml_get_attribute_int(cheatnode, "tempvariables", DEFAULT_TEMP_VARIABLES);
	if (tempcount < 1)
	{
		mame_printf_error("%s.xml(%d): invalid tempvariables attribute (%d)\n", filename, cheatnode->line, tempcount);
		return NULL;
	}

	/* allocate memory for the cheat */
	cheat = auto_alloc_clear(machine, cheat_entry);
	cheat->tempvar = auto_alloc_array_clear(machine, UINT64, tempcount);
	cheat->numtemp = tempcount;

	/* get the description */
	description = xml_get_attribute_string(cheatnode, "desc", NULL);
	if (description == NULL || description[0] == 0)
	{
		mame_printf_error("%s.xml(%d): empty or missing desc attribute on cheat\n", filename, cheatnode->line);
		return NULL;
	}
	cheat->description.cpy(description);

	/* create the symbol table */
	cheat->symbols = symtable_alloc(NULL, machine);
	symtable_add_register(cheat->symbols, "frame", &cheatinfo->framecount, cheat_variable_get, NULL);
	symtable_add_register(cheat->symbols, "argindex", &cheat->argindex, cheat_variable_get, NULL);
	for (curtemp = 0; curtemp < tempcount; curtemp++)
	{
		char tempname[20];
		sprintf(tempname, "temp%d", curtemp);
		symtable_add_register(cheat->symbols, tempname, &cheat->tempvar[curtemp], cheat_variable_get, cheat_variable_set);
	}
	symtable_add_function(cheat->symbols, "frombcd", NULL, 1, 1, execute_frombcd);
	symtable_add_function(cheat->symbols, "tobcd", NULL, 1, 1, execute_tobcd);

	/* read the first comment node */
	commentnode = xml_get_sibling(cheatnode->child, "comment");
	if (commentnode != NULL)
	{
		if (commentnode->value != NULL && commentnode->value[0] != 0)
			cheat->comment.cpy(commentnode->value);

		/* only one comment is kept */
		commentnode = xml_get_sibling(commentnode->next, "comment");
		if (commentnode != NULL)
			mame_printf_warning("%s.xml(%d): only one comment node is retained; ignoring additional nodes\n", filename, commentnode->line);
	}

	/* read the first parameter node */
	paramnode = xml_get_sibling(cheatnode->child, "parameter");
	if (paramnode != NULL)
	{
		/* load this parameter */
		cheat_parameter *curparam = cheat_parameter_load(machine, filename, paramnode);
		if (curparam == NULL)
			goto error;

		/* set this as the parameter and add the symbol */
		cheat->parameter = curparam;
		symtable_add_register(cheat->symbols, "param", &curparam->value, cheat_variable_get, NULL);

		/* only one parameter allowed */
		paramnode = xml_get_sibling(paramnode->next, "parameter");
		if (paramnode != NULL)
			mame_printf_warning("%s.xml(%d): only one parameter node allowed; ignoring additional nodes\n", filename, paramnode->line);
	}

	/* read the script nodes */
	for (scriptnode = xml_get_sibling(cheatnode->child, "script"); scriptnode != NULL; scriptnode = xml_get_sibling(scriptnode->next, "script"))
	{
		/* load this entry */
		cheat_script *curscript = cheat_script_load(machine, filename, scriptnode, cheat);
		if (curscript == NULL)
			goto error;

		/* if we have a script already for this slot, it is an error */
		if (cheat->script[curscript->state] != NULL)
			mame_printf_warning("%s.xml(%d): only one script per state allowed; ignoring additional scripts\n", filename, scriptnode->line);

		/* otherwise, fill in the slot */
		else
			cheat->script[curscript->state] = curscript;
	}

	/* set the initial state */
	cheat->state = SCRIPT_STATE_OFF;
	return cheat;

error:
	cheat_entry_free(machine, cheat);
	return NULL;
}


/*-------------------------------------------------
    cheat_entry_save - save a single cheat
    entry
-------------------------------------------------*/

static void cheat_entry_save(mame_file *cheatfile, const cheat_entry *cheat)
{
	script_state state;
	int scriptcount;

	/* count the scripts */
	scriptcount = 0;
	for (state = SCRIPT_STATE_OFF; state < SCRIPT_STATE_COUNT; state++)
		if (cheat->script[state] != NULL)
			scriptcount++;

	/* output the cheat tag */
	mame_fprintf(cheatfile, "\t<cheat desc=\"%s\"", cheat->description.cstr());
	if (cheat->numtemp != DEFAULT_TEMP_VARIABLES)
		mame_fprintf(cheatfile, " tempvariables=\"%d\"", cheat->numtemp);
	if (cheat->comment.len() == 0 && cheat->parameter == NULL && scriptcount == 0)
		mame_fprintf(cheatfile, " />\n");
	else
	{
		mame_fprintf(cheatfile, ">\n");

		/* save the comment */
		if (cheat->comment.len() != 0)
			mame_fprintf(cheatfile, "\t\t<comment><![CDATA[\n%s\n\t\t]]></comment>\n", cheat->comment.cstr());

		/* output the parameter, if present */
		if (cheat->parameter != NULL)
			cheat_parameter_save(cheatfile, cheat->parameter);

		/* output the script nodes */
		for (state = SCRIPT_STATE_OFF; state < SCRIPT_STATE_COUNT; state++)
			if (cheat->script[state] != NULL)
				cheat_script_save(cheatfile, cheat->script[state]);

		/* close the cheat tag */
		mame_fprintf(cheatfile, "\t</cheat>\n");
	}
}


/*-------------------------------------------------
    cheat_entry_free - free a single cheat entry
-------------------------------------------------*/

static void cheat_entry_free(running_machine *machine, cheat_entry *cheat)
{
	script_state state;

	if (cheat->parameter != NULL)
		cheat_parameter_free(machine, cheat->parameter);

	for (state = SCRIPT_STATE_OFF; state < SCRIPT_STATE_COUNT; state++)
		if (cheat->script[state] != NULL)
			cheat_script_free(machine, cheat->script[state]);

	if (cheat->symbols != NULL)
		symtable_free(cheat->symbols);

	auto_free(machine, cheat->tempvar);
	auto_free(machine, cheat);
}


/*-------------------------------------------------
    cheat_parameter_load - load a single cheat
    parameter and create the underlying data
    structures
-------------------------------------------------*/

static cheat_parameter *cheat_parameter_load(running_machine *machine, const char *filename, xml_data_node *paramnode)
{
	parameter_item **itemtailptr;
	xml_data_node *itemnode;
	cheat_parameter *param;

	/* allocate memory for it */
	param = auto_alloc_clear(machine, cheat_parameter);

	/* read the core attributes */
	param->minval = xml_get_attribute_int(paramnode, "min", 0);
	param->minformat = xml_get_attribute_int_format(paramnode, "min");
	param->maxval = xml_get_attribute_int(paramnode, "max", 0);
	param->maxformat = xml_get_attribute_int_format(paramnode, "max");
	param->stepval = xml_get_attribute_int(paramnode, "step", 1);
	param->stepformat = xml_get_attribute_int_format(paramnode, "step");

	/* iterate over items */
	itemtailptr = &param->itemlist;
	for (itemnode = xml_get_sibling(paramnode->child, "item"); itemnode != NULL; itemnode = xml_get_sibling(itemnode->next, "item"))
	{
		parameter_item *curitem;

		/* allocate memory for it */
		curitem = auto_alloc_clear(machine, parameter_item);

		/* check for NULL text */
		if (itemnode->value == NULL || itemnode->value[0] == 0)
		{
			mame_printf_error("%s.xml(%d): item is missing text\n", filename, itemnode->line);
			goto error;
		}
		curitem->text.cpy(itemnode->value);

		/* read the attributes */
		if (xml_get_attribute(itemnode, "value") == NULL)
		{
			mame_printf_error("%s.xml(%d): item is value\n", filename, itemnode->line);
			goto error;
		}
		curitem->value = xml_get_attribute_int(itemnode, "value", 0);
		curitem->valformat = xml_get_attribute_int_format(itemnode, "value");

		/* ensure the maximum expands to suit */
		param->maxval = MAX(param->maxval, curitem->value);

		/* add to the end of the list */
		*itemtailptr = curitem;
		itemtailptr = &curitem->next;
	}

	/* start at the minimum */
	param->value = param->minval;

	return param;

error:
	cheat_parameter_free(machine, param);
	return NULL;
}


/*-------------------------------------------------
    cheat_parameter_save - save a single cheat
    parameter
-------------------------------------------------*/

static void cheat_parameter_save(mame_file *cheatfile, const cheat_parameter *param)
{
	/* output the parameter tag */
	mame_fprintf(cheatfile, "\t\t<parameter");

	/* if no items, just output min/max/step */
	astring string;
	if (param->itemlist == NULL)
	{
		if (param->minval != 0)
			mame_fprintf(cheatfile, " min=\"%s\"", format_int(string, param->minval, param->minformat));
		if (param->maxval != 0)
			mame_fprintf(cheatfile, " max=\"%s\"", format_int(string, param->maxval, param->maxformat));
		if (param->stepval != 1)
			mame_fprintf(cheatfile, " step=\"%s\"", format_int(string, param->stepval, param->stepformat));
		mame_fprintf(cheatfile, "/>\n");
	}

	/* iterate over items */
	else
	{
		const parameter_item *curitem;

		for (curitem = param->itemlist; curitem != NULL; curitem = curitem->next)
			mame_fprintf(cheatfile, "\t\t\t<item value=\"%s\">%s</item>\n", format_int(string, curitem->value, curitem->valformat), curitem->text.cstr());
		mame_fprintf(cheatfile, "\t\t</parameter>\n");
	}
}


/*-------------------------------------------------
    cheat_parameter_free - free a single cheat
    parameter
-------------------------------------------------*/

static void cheat_parameter_free(running_machine *machine, cheat_parameter *param)
{
	while (param->itemlist != NULL)
	{
		parameter_item *item = param->itemlist;
		param->itemlist = item->next;

		auto_free(machine, item);
	}

	auto_free(machine, param);
}


/*-------------------------------------------------
    cheat_script_load - load a single cheat
    script and create the underlying data
    structures
-------------------------------------------------*/

static cheat_script *cheat_script_load(running_machine *machine, const char *filename, xml_data_node *scriptnode, cheat_entry *cheat)
{
	script_entry **entrytailptr;
	xml_data_node *entrynode;
	cheat_script *script;
	const char *state;

	/* allocate memory for it */
	script = auto_alloc_clear(machine, cheat_script);

	/* read the core attributes */
	script->state = SCRIPT_STATE_RUN;
	state = xml_get_attribute_string(scriptnode, "state", "run");
	if (strcmp(state, "on") == 0)
		script->state = SCRIPT_STATE_ON;
	else if (strcmp(state, "off") == 0)
		script->state = SCRIPT_STATE_OFF;
	else if (strcmp(state, "change") == 0)
		script->state = SCRIPT_STATE_CHANGE;
	else if (strcmp(state, "run") != 0)
	{
		mame_printf_error("%s.xml(%d): invalid script state '%s'\n", filename, scriptnode->line, state);
		goto error;
	}

	/* iterate over nodes within the script */
	entrytailptr = &script->entrylist;
	for (entrynode = scriptnode->child; entrynode != NULL; entrynode = entrynode->next)
	{
		script_entry *curentry = NULL;

		/* handle action nodes */
		if (strcmp(entrynode->name, "action") == 0)
			curentry = script_entry_load(machine, filename, entrynode, cheat, TRUE);

		/* handle output nodes */
		else if (strcmp(entrynode->name, "output") == 0)
			curentry = script_entry_load(machine, filename, entrynode, cheat, FALSE);

		/* anything else is ignored */
		else
		{
			mame_printf_warning("%s.xml(%d): unknown script item '%s' will be lost if saved\n", filename, entrynode->line, entrynode->name);
			continue;
		}

		if (curentry == NULL)
			goto error;

		/* add to the end of the list */
		*entrytailptr = curentry;
		entrytailptr = &curentry->next;
	}
	return script;

error:
	cheat_script_free(machine, script);
	return NULL;
}


/*-------------------------------------------------
    cheat_script_save - save a single cheat
    script
-------------------------------------------------*/

static void cheat_script_save(mame_file *cheatfile, const cheat_script *script)
{
	const script_entry *entry;

	/* output the script tag */
	mame_fprintf(cheatfile, "\t\t<script");
	switch (script->state)
	{
		case SCRIPT_STATE_OFF:		mame_fprintf(cheatfile, " state=\"off\"");		break;
		case SCRIPT_STATE_ON:		mame_fprintf(cheatfile, " state=\"on\"");		break;
		default:
		case SCRIPT_STATE_RUN:		mame_fprintf(cheatfile, " state=\"run\"");		break;
		case SCRIPT_STATE_CHANGE:	mame_fprintf(cheatfile, " state=\"change\"");	break;
	}
	mame_fprintf(cheatfile, ">\n");

	/* output entries */
	for (entry = script->entrylist; entry != NULL; entry = entry->next)
		script_entry_save(cheatfile, entry);

	/* close the tag */
	mame_fprintf(cheatfile, "\t\t</script>\n");
}


/*-------------------------------------------------
    cheat_script_free - free a single cheat
    script
-------------------------------------------------*/

static void cheat_script_free(running_machine *machine, cheat_script *script)
{
	while (script->entrylist != NULL)
	{
		script_entry *entry = script->entrylist;
		script->entrylist = entry->next;
		script_entry_free(machine, entry);
	}

	auto_free(machine, script);
}


/*-------------------------------------------------
    script_entry_load - load a single action
    or output create the underlying data
    structures
-------------------------------------------------*/

static script_entry *script_entry_load(running_machine *machine, const char *filename, xml_data_node *entrynode, cheat_entry *cheat, int isaction)
{
	const char *expression;
	script_entry *entry;
	EXPRERR experr;

	/* allocate memory for it */
	entry = auto_alloc_clear(machine, script_entry);

	/* read the condition if present */
	expression = xml_get_attribute_string(entrynode, "condition", NULL);
	if (expression != NULL)
	{
		experr = expression_parse(expression, cheat->symbols, &debug_expression_callbacks, machine, &entry->condition);
		if (experr != EXPRERR_NONE)
		{
			mame_printf_error("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, entrynode->line, expression, exprerr_to_string(experr));
			goto error;
		}
	}

	/* if this is an action, parse the expression */
	if (isaction)
	{
		expression = entrynode->value;
		if (expression == NULL || expression[0] == 0)
		{
			mame_printf_error("%s.xml(%d): missing expression in action tag\n", filename, entrynode->line);
			goto error;
		}
		experr = expression_parse(expression, cheat->symbols, &debug_expression_callbacks, machine, &entry->expression);
		if (experr != EXPRERR_NONE)
		{
			mame_printf_error("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, entrynode->line, expression, exprerr_to_string(experr));
			goto error;
		}
	}

	/* otherwise, parse the attributes and arguments */
	else
	{
		output_argument **argtailptr;
		const char *align, *format;
		xml_data_node *argnode;
		int totalargs = 0;

		/* extract format */
		format = xml_get_attribute_string(entrynode, "format", NULL);
		if (format == NULL || format[0] == 0)
		{
			mame_printf_error("%s.xml(%d): missing format in output tag\n", filename, entrynode->line);
			goto error;
		}
		entry->format.cpy(format);

		/* extract other attributes */
		entry->line = xml_get_attribute_int(entrynode, "line", 0);
		entry->justify = JUSTIFY_LEFT;
		align = xml_get_attribute_string(entrynode, "align", "left");
		if (strcmp(align, "center") == 0)
			entry->justify = JUSTIFY_CENTER;
		else if (strcmp(align, "right") == 0)
			entry->justify = JUSTIFY_RIGHT;
		else if (strcmp(align, "left") != 0)
		{
			mame_printf_error("%s.xml(%d): invalid alignment '%s' specified\n", filename, entrynode->line, align);
			goto error;
		}

		/* then parse arguments */
		argtailptr = &entry->arglist;
		for (argnode = xml_get_sibling(entrynode->child, "argument"); argnode != NULL; argnode = xml_get_sibling(argnode->next, "argument"))
		{
			output_argument *curarg;

			/* allocate memory for it */
			curarg = auto_alloc_clear(machine, output_argument);

			/* first extract attributes */
			curarg->count = xml_get_attribute_int(argnode, "count", 1);
			totalargs += curarg->count;

			/* max out on arguments */
			if (totalargs > MAX_ARGUMENTS)
			{
				mame_printf_error("%s.xml(%d): too many arguments (found %d, max is %d)\n", filename, argnode->line, totalargs, MAX_ARGUMENTS);
				goto error;
			}

			/* read the expression */
			expression = argnode->value;
			if (expression == NULL || expression[0] == 0)
			{
				mame_printf_error("%s.xml(%d): missing expression in argument tag\n", filename, argnode->line);
				goto error;
			}
			experr = expression_parse(expression, cheat->symbols, &debug_expression_callbacks, machine, &curarg->expression);
			if (experr != EXPRERR_NONE)
			{
				mame_printf_error("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, argnode->line, expression, exprerr_to_string(experr));
				goto error;
			}

			/* add to the end of the list */
			*argtailptr = curarg;
			argtailptr = &curarg->next;
		}

		/* validate the format against the arguments */
		if (!validate_format(filename, entrynode->line, entry))
			goto error;
	}
	return entry;

error:
	script_entry_free(machine, entry);
	return NULL;
}


/*-------------------------------------------------
    script_entry_save - save a single action
    or output
-------------------------------------------------*/

static void script_entry_save(mame_file *cheatfile, const script_entry *entry)
{
	astring tempstring;

	/* output an action */
	if (entry->format == NULL)
	{
		mame_fprintf(cheatfile, "\t\t\t<action");
		if (entry->condition != NULL)
		{
			quote_astring_expression(tempstring.cpy(expression_original_string(entry->condition)), TRUE);
			mame_fprintf(cheatfile, " condition=\"%s\"", tempstring.cstr());
		}
		quote_astring_expression(tempstring.cpy(expression_original_string(entry->expression)), FALSE);
		mame_fprintf(cheatfile, ">%s</action>\n", tempstring.cstr());
	}

	/* output an output */
	else
	{
		mame_fprintf(cheatfile, "\t\t\t<output format=\"%s\"", entry->format.cstr());
		if (entry->condition != NULL)
		{
			quote_astring_expression(tempstring.cpy(expression_original_string(entry->condition)), TRUE);
			mame_fprintf(cheatfile, " condition=\"%s\"", tempstring.cstr());
		}
		if (entry->line != 0)
			mame_fprintf(cheatfile, " line=\"%d\"", entry->line);
		if (entry->justify == JUSTIFY_CENTER)
			mame_fprintf(cheatfile, " align=\"center\"");
		else if (entry->justify == JUSTIFY_RIGHT)
			mame_fprintf(cheatfile, " align=\"right\"");
		if (entry->arglist == NULL)
			mame_fprintf(cheatfile, " />\n");

		/* output arguments */
		else
		{
			const output_argument *curarg;

			mame_fprintf(cheatfile, ">\n");
			for (curarg = entry->arglist; curarg != NULL; curarg = curarg->next)
			{
				mame_fprintf(cheatfile, "\t\t\t\t<argument");
				if (curarg->count != 1)
					mame_fprintf(cheatfile, " count=\"%d\"", (int)curarg->count);
				quote_astring_expression(tempstring.cpy(expression_original_string(curarg->expression)), FALSE);
				mame_fprintf(cheatfile, ">%s</argument>\n", tempstring.cstr());
			}
			mame_fprintf(cheatfile, "\t\t\t</output>\n");
		}
	}
}


/*-------------------------------------------------
    script_entry_free - free a single script
    entry
-------------------------------------------------*/

static void script_entry_free(running_machine *machine, script_entry *entry)
{
	if (entry->condition != NULL)
		expression_free(entry->condition);
	if (entry->expression != NULL)
		expression_free(entry->expression);

	while (entry->arglist != NULL)
	{
		output_argument *curarg = entry->arglist;
		entry->arglist = curarg->next;

		if (curarg->expression != NULL)
			expression_free(curarg->expression);
		auto_free(machine, curarg);
	}

	auto_free(machine, entry);
}



/***************************************************************************
    MISC HELPERS
***************************************************************************/

/*-------------------------------------------------
    quote_astring_expression - quote an expression
    string so that it is valid to embed in an XML
    document
-------------------------------------------------*/

static astring &quote_astring_expression(astring &string, int isattribute)
{
	string.replace(0, " && ", " and ");
	string.replace(0, " &&", " and ");
	string.replace(0, "&& ", " and ");
	string.replace(0, "&&", " and ");

	string.replace(0, " & ", " band ");
	string.replace(0, " &", " band ");
	string.replace(0, "& ", " band ");
	string.replace(0, "&", " band ");

	string.replace(0, " <= ", " le ");
	string.replace(0, " <=", " le ");
	string.replace(0, "<= ", " le ");
	string.replace(0, "<=", " le ");

	string.replace(0, " < ", " lt ");
	string.replace(0, " <", " lt ");
	string.replace(0, "< ", " lt ");
	string.replace(0, "<", " lt ");

	string.replace(0, " << ", " lshift ");
	string.replace(0, " <<", " lshift ");
	string.replace(0, "<< ", " lshift ");
	string.replace(0, "<<", " lshift ");

	return string;
}


/*-------------------------------------------------
    validate_format - check that a format string
    has the correct number and type of arguments
-------------------------------------------------*/

static int validate_format(const char *filename, int line, const script_entry *entry)
{
	const char *p = entry->format;
	const output_argument *curarg;
	int argsprovided;
	int argscounted;

	/* first count arguments */
	argsprovided = 0;
	for (curarg = entry->arglist; curarg != NULL; curarg = curarg->next)
		argsprovided += curarg->count;

	/* now scan the string for valid argument usage */
	p = strchr(p, '%');
	argscounted = 0;
	while (p != NULL)
	{
		/* skip past any valid attributes */
		p++;
		while (strchr("lh0123456789.-+ #", *p) != NULL)
			p++;

		/* look for a valid type */
		if (strchr("cdiouxX", *p) == NULL)
		{
			mame_printf_error("%s.xml(%d): invalid format specification \"%s\"\n", filename, line, entry->format.cstr());
			return FALSE;
		}
		argscounted++;

		/* look for the next one */
		p = strchr(p, '%');
	}

	/* did we match? */
	if (argscounted < argsprovided)
	{
		mame_printf_error("%s.xml(%d): too many arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, entry->format.cstr());
		return FALSE;
	}
	if (argscounted > argsprovided)
	{
		mame_printf_error("%s.xml(%d): not enough arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, entry->format.cstr());
		return FALSE;
	}
	return TRUE;
}


/*-------------------------------------------------
    cheat_variable_get - return the value of a
    cheat variable
-------------------------------------------------*/

static UINT64 cheat_variable_get(void *globalref, void *ref)
{
	return *(UINT64 *)ref;
}


/*-------------------------------------------------
    cheat_variable_set - set the value of a
    cheat variable
-------------------------------------------------*/

static void cheat_variable_set(void *globalref, void *ref, UINT64 value)
{
	*(UINT64 *)ref = value;
}


/*-------------------------------------------------
    execute_frombcd - convert a value from BCD
-------------------------------------------------*/

static UINT64 execute_frombcd(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	UINT64 value = param[0];
	UINT64 multiplier = 1;
	UINT64 result = 0;

	while (value != 0)
	{
		result += (value & 0x0f) * multiplier;
		value >>= 4;
		multiplier *= 10;
	}
	return result;
}


/*-------------------------------------------------
    execute_tobcd - convert a value to BCD
-------------------------------------------------*/

static UINT64 execute_tobcd(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	UINT64 value = param[0];
	UINT64 result = 0;
	UINT8 shift = 0;

	while (value != 0)
	{
		result += (value % 10) << shift;
		value /= 10;
		shift += 4;
	}
	return result;
}


