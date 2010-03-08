/*********************************************************************

    debugvw.c

    Debugger view engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "debugvw.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcpu.h"
#include "debugcon.h"
#include "express.h"
#include "textbuf.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DEFAULT_DASM_LINES	(1000)
#define DEFAULT_DASM_WIDTH	(50)
#define DASM_MAX_BYTES		(16)
#define MEM_MAX_LINE_WIDTH	(1024)

enum _view_notification
{
	VIEW_NOTIFY_NONE,
	VIEW_NOTIFY_VISIBLE_CHANGED,
	VIEW_NOTIFY_CURSOR_CHANGED
};
typedef enum _view_notification view_notification;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* internal callback function pointers */
typedef int (*view_alloc_func)(debug_view *view);
typedef void (*view_free_func)(debug_view *view);
typedef void (*view_update_func)(debug_view *view);
typedef void (*view_notify_func)(debug_view *view, view_notification type);
typedef void (*view_char_func)(debug_view *view, int chval);


/* debug_view_callbacks contains internal callbacks specific to a given view */
typedef struct _debug_view_callbacks debug_view_callbacks;
struct _debug_view_callbacks
{
	view_alloc_func		alloc;					/* allocate memory */
	view_free_func		free;					/* free memory */
	view_update_func	update;					/* update contents */
	view_notify_func	notify;					/* notify of something changed */
	view_char_func		handlechar;				/* handle a typed character */
};


/* debug_view_section contains information about a section of a view */
typedef struct _debug_view_section debug_view_section;
struct _debug_view_section
{
	INT32				pos;					/* starting position */
	INT32				width;					/* width of this section */
};


/* debug view expression contains information about an embedded expression */
typedef struct _debug_view_expression debug_view_expression;
struct _debug_view_expression
{
	UINT64				result;					/* last result from the expression */
	parsed_expression *	parsed;					/* parsed expression data */
	astring				string;					/* copy of the expression string */
	UINT8				dirty;					/* true if the expression needs to be re-evaluated */
};


/* debug_view describes a single text-based view */
class debug_view
{
public:
	/* core view data */
	debug_view *		next;					/* link to the next view */
	running_machine *	machine;				/* machine associated with this view */
	UINT8				type;					/* type of view */
	void *				extra_data;				/* extra view-specific data */
	debug_view_callbacks cb;					/* callback for this view */

	/* OSD data */
	debug_view_osd_update_func osdupdate;		/* callback for the update */
	void *				osdprivate;				/* OSD-managed private data */

	/* visibility info */
	debug_view_xy		visible;				/* visible size (in rows and columns) */
	debug_view_xy		total;					/* total size (in rows and columns) */
	debug_view_xy		topleft;				/* top-left visible position (in rows and columns) */
	debug_view_xy		cursor;					/* cursor position */
	UINT8				supports_cursor;		/* does this view support a cursor? */
	UINT8				cursor_visible;			/* is the cursor visible? */

	/* update info */
	UINT8				recompute;				/* does this view require a recomputation? */
	UINT8				update_level;			/* update level; updates when this hits 0 */
	UINT8				update_pending;			/* true if there is a pending update */
	UINT8				osd_update_pending;		/* true if there is a pending update */
	debug_view_char *	viewdata;				/* current array of view data */
	int					viewdata_size;			/* number of elements of the viewdata array */
};


/* debug_view_registers contains data specific to a register view */
typedef struct _debug_view_register debug_view_register;
struct _debug_view_register
{
	UINT64				lastval;				/* last value */
	UINT64				currval;				/* current value */
	UINT32				regnum;					/* index */
	UINT8				tagstart;				/* starting tag char */
	UINT8				taglen;					/* number of tag chars */
	UINT8				valstart;				/* starting value char */
	UINT8				vallen;					/* number of value chars */
};


typedef struct _debug_view_registers debug_view_registers;
struct _debug_view_registers
{
	running_device *device;				/* CPU device whose registers we are showing */
	int					divider;				/* dividing column */
	UINT64				last_update;			/* execution counter at last update */
	debug_view_register reg[MAX_REGS];			/* register data */
};


/* debug_view_disasm contains data specific to a disassembly view */
typedef struct _debug_view_disasm debug_view_disasm;
struct _debug_view_disasm
{
	const address_space *space;					/* address space whose data we are disassembling */
	disasm_right_column	right_column;			/* right column contents */
	UINT32				backwards_steps;		/* number of backwards steps */
	UINT32				dasm_width;				/* width of the disassembly area */
	UINT8 *				last_direct_raw;		/* last direct raw value */
	UINT8 *				last_direct_decrypted;	/* last direct decrypted value */
	UINT32				last_change_count;		/* last comment change count */
	offs_t				last_pcbyte;			/* last PC byte value */
	int					divider1, divider2;		/* left and right divider columns */
	int					divider3;				/* comment divider column */
	debug_view_expression expression;			/* expression-related information */
	debug_view_xy		allocated;				/* allocated rows/columns */
	offs_t *			byteaddress;			/* addresses of the instructions */
	char *				dasm;					/* disassembled instructions */
};


/* debug_view_memory contains data specific to a memory view */
typedef struct _debug_view_memory debug_view_memory;
struct _debug_view_memory
{
	const memory_subview_item *subviewlist;		/* linked list of memory subviews */
	const memory_subview_item *desc;			/* description of our current subview */
	debug_view_expression expression;			/* expression describing the start address */
	UINT32				chunks_per_row;			/* number of chunks displayed per line */
	UINT8				bytes_per_chunk;		/* bytes per chunk */
	UINT8				reverse_view;			/* reverse-endian view? */
	UINT8				ascii_view;				/* display ASCII characters? */
	UINT8				no_translation;			/* don't run addresses through the cpu translation hook */
	debug_view_section	section[3];				/* (derived) 3 sections to manage */
	offs_t				maxaddr;				/* (derived) maximum address to display */
	UINT32				bytes_per_row;			/* (derived) number of bytes displayed per line */
	UINT32				byte_offset;			/* (derived) offset of starting visible byte */
	char				addrformat[10];			/* (derived) format string to use to print addresses */
};


/* debug_view_textbuf contains data specific to a textbuffer view */
typedef struct _debug_view_textbuf debug_view_textbuf;
struct _debug_view_textbuf
{
	text_buffer *		textbuf;				/* pointer to the text buffer */
	UINT8				at_bottom;				/* are we tracking new stuff being added? */
	UINT32				topseq;					/* sequence number of the top line */
};


/* memory_view_pos contains positioning data for memory views */
typedef struct _memory_view_pos memory_view_pos;
struct _memory_view_pos
{
	UINT8				spacing;				/* spacing between each entry */
	UINT8				shift[24];				/* shift for each character */
};


/* debugvw_priate contains internal global data for this module */
/* In mame.h: typedef struct _debugvw_private debugvw_priate; */
struct _debugvw_private
{
	debug_view *		viewlist;				/* list of views */
	const registers_subview_item *registers_subviews;/* linked list of registers subviews */
	const disasm_subview_item *disasm_subviews;	/* linked list of disassembly subviews */
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static const memory_view_pos memory_pos_table[9] =
{
	/* 0 bytes per chunk:                         */ {  0, { 0 } },
	/* 1 byte  per chunk: 00 11 22 33 44 55 66 77 */ {  3, { 0x04, 0x00, 0x80 } },
	/* 2 bytes per chunk:  0011  2233  4455  6677 */ {  6, { 0x8c, 0x0c, 0x08, 0x04, 0x00, 0x80 } },
	/* 3 bytes per chunk:                         */ {  0, { 0 } },
	/* 4 bytes per chunk:   00112233    44556677  */ { 12, { 0x9c, 0x9c, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80 } },
	/* 5 bytes per chunk:                         */ {  0, { 0 } },
	/* 6 bytes per chunk:                         */ {  0, { 0 } },
	/* 7 bytes per chunk:                         */ {  0, { 0 } },
	/* 8 bytes per chunk:     0011223344556677    */ { 24, { 0xbc, 0xbc, 0xbc, 0xbc, 0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24, 0x20, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80, 0x80, 0x80 } }
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_view_exit(running_machine *machine);

static int textbuf_view_alloc(debug_view *view, text_buffer *textbuf);
static void textbuf_view_free(debug_view *view);
static void textbuf_view_notify(debug_view *view, view_notification type);
static void textbuf_view_update(debug_view *view);

static int console_view_alloc(debug_view *view);

static int log_view_alloc(debug_view *view);

static const registers_subview_item *registers_view_enumerate_subviews(running_machine *machine);
static int registers_view_alloc(debug_view *view);
static void registers_view_free(debug_view *view);
static void registers_view_update(debug_view *view);

static const disasm_subview_item *disasm_view_enumerate_subviews(running_machine *machine);
static int disasm_view_alloc(debug_view *view);
static void disasm_view_free(debug_view *view);
static void disasm_view_notify(debug_view *view, view_notification type);
static void disasm_view_update(debug_view *view);
static void disasm_view_char(debug_view *view, int chval);

static const memory_subview_item *memory_view_enumerate_subviews(running_machine *machine);
static int memory_view_alloc(debug_view *view);
static void memory_view_free(debug_view *view);
static void memory_view_notify(debug_view *view, view_notification type);
static void memory_view_update(debug_view *view);
static void memory_view_char(debug_view *view, int chval);
static void memory_view_recompute(debug_view *view);
static int memory_view_needs_recompute(debug_view *view);
static void memory_view_get_cursor_pos(debug_view *view, offs_t *address, UINT8 *shift);
static void memory_view_set_cursor_pos(debug_view *view, offs_t address, UINT8 shift);
static int memory_view_read(debug_view_memory *memdata, UINT8 size, offs_t offs, UINT64 *data);
static void memory_view_write(debug_view_memory *memdata, UINT8 size, offs_t offs, UINT64 data);

static const debug_view_callbacks callback_table[] =
{
	{	NULL,					NULL,					NULL,					NULL,					NULL },
	{	console_view_alloc,		textbuf_view_free,		textbuf_view_update,	textbuf_view_notify,	NULL },
	{	registers_view_alloc,	registers_view_free,	registers_view_update,	NULL,					NULL },
	{	disasm_view_alloc,		disasm_view_free,		disasm_view_update,		disasm_view_notify,		disasm_view_char },
	{	memory_view_alloc,		memory_view_free,		memory_view_update,		memory_view_notify,		memory_view_char },
	{	log_view_alloc,			textbuf_view_free,		textbuf_view_update,	textbuf_view_notify,	NULL }
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    in_section - return TRUE if the given X
    coordinate is within a section
-------------------------------------------------*/

INLINE int in_section(int x, const debug_view_section *section)
{
	return (x >= section->pos && x < section->pos + section->width);
}


/*-------------------------------------------------
    adjust_visible_x_for_cursor - adjust a view's
    visible X position to ensure the cursor is
    visible
-------------------------------------------------*/

INLINE void adjust_visible_x_for_cursor(debug_view *view)
{
	if (view->cursor.x < view->topleft.x)
		view->topleft.x = view->cursor.x;
	else if (view->cursor.x >= view->topleft.x + view->visible.x - 1)
		view->topleft.x = view->cursor.x - view->visible.x + 2;
}


/*-------------------------------------------------
    adjust_visible_y_for_cursor - adjust a view's
    visible Y position to ensure the cursor is
    visible
-------------------------------------------------*/

INLINE void adjust_visible_y_for_cursor(debug_view *view)
{
	if (view->cursor.y < view->topleft.y)
		view->topleft.y = view->cursor.y;
	else if (view->cursor.y >= view->topleft.y + view->visible.y - 1)
		view->topleft.y = view->cursor.y - view->visible.y + 2;
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    debug_view_init - initializes the view system
-------------------------------------------------*/

void debug_view_init(running_machine *machine)
{
	debugvw_private *global;

	/* allocate memory for our globals */
	global = machine->debugvw_data = auto_alloc_clear(machine, debugvw_private);

	/* register for some manual cleanup */
	add_exit_callback(machine, debug_view_exit);

	/* build a list of disassembly and memory subviews */
	global->registers_subviews = registers_view_enumerate_subviews(machine);
	global->disasm_subviews = disasm_view_enumerate_subviews(machine);
}


/*-------------------------------------------------
    debug_view_exit - exits the view system
-------------------------------------------------*/

static void debug_view_exit(running_machine *machine)
{
	debugvw_private *global = machine->debugvw_data;

	/* kill all the views */
	while (global->viewlist != NULL)
		debug_view_free(global->viewlist);
}



/***************************************************************************
    VIEW CREATION/DELETION
***************************************************************************/

/*-------------------------------------------------
    debug_view_alloc - allocate a new debug
    view
-------------------------------------------------*/

debug_view *debug_view_alloc(running_machine *machine, int type, debug_view_osd_update_func osdupdate, void *osdprivate)
{
	debugvw_private *global = machine->debugvw_data;
	debug_view *view;

	assert(type >= 0 && type < ARRAY_LENGTH(callback_table));

	/* allocate memory for the view */
	view = auto_alloc_clear(machine, debug_view);

	/* set the view type information */
	view->machine = machine;
	view->type = type;
	view->cb = callback_table[type];
	view->osdupdate = osdupdate;
	view->osdprivate = osdprivate;

	/* set up some reasonable defaults */
	view->visible.x = view->total.x = 10;
	view->visible.y = view->total.y = 10;

	/* allocate memory for the buffer */
	view->viewdata_size = view->visible.y * view->visible.x;
	view->viewdata = auto_alloc_array(machine, debug_view_char, view->viewdata_size);

	/* allocate extra memory */
	if (view->cb.alloc != NULL && !(*view->cb.alloc)(view))
	{
		auto_free(machine, view->viewdata);
		auto_free(machine, view);
		return NULL;
	}

	/* link it in */
	view->next = global->viewlist;
	global->viewlist = view;

	/* require a recomputation on the first update */
	view->recompute = TRUE;
	view->update_pending = TRUE;

	return view;
}


/*-------------------------------------------------
    debug_view_free - free a debug view
-------------------------------------------------*/

void debug_view_free(debug_view *view)
{
	debugvw_private *global = view->machine->debugvw_data;
	debug_view **viewptr;

	/* find the view */
	for (viewptr = &global->viewlist; *viewptr != NULL; viewptr = &(*viewptr)->next)
		if (*viewptr == view)
		{
			/* unlink */
			*viewptr = view->next;

			/* free memory */
			if (view->cb.free != NULL)
				(*view->cb.free)(view);
			if (view->viewdata != NULL)
				auto_free(view->machine, view->viewdata);
			auto_free(view->machine, view);
			break;
		}
}



/***************************************************************************
    UPDATE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    debug_view_begin_update - bracket a sequence
    of changes so that only one update occurs
-------------------------------------------------*/

void debug_view_begin_update(debug_view *view)
{
	/* bump the level */
	view->update_level++;
}


/*-------------------------------------------------
    debug_view_end_update - bracket a sequence
    of changes so that only one update occurs
-------------------------------------------------*/

void debug_view_end_update(debug_view *view)
{
	/* if we hit zero, call the update function */
	if (view->update_level == 1)
	{
		while (view->update_pending)
		{
			int size;

			/* no longer pending, but flag for the OSD */
			view->update_pending = FALSE;
			view->osd_update_pending = TRUE;

			/* resize the viewdata if needed */
			size = view->visible.x * view->visible.y;
			if (size > view->viewdata_size)
			{
				view->viewdata_size = size;
				global_free(view->viewdata);
				view->viewdata = auto_alloc_array(view->machine, debug_view_char, view->viewdata_size);
			}

			/* update the view */
			if (view->cb.update != NULL)
				(*view->cb.update)(view);
		}
	}

	/* decrement the level */
	view->update_level--;
}


/*-------------------------------------------------
    debug_view_flush_updates - force all updates
    to notify the OSD
-------------------------------------------------*/

void debug_view_flush_updates(running_machine *machine)
{
	debugvw_private *global = machine->debugvw_data;
	debug_view *view;

	/* skip if we're not ready yet */
	if (global == NULL)
		return;

	/* loop over each view and force an update */
	for (view = global->viewlist; view != NULL; view = view->next)
		if (view->osd_update_pending)
		{
			/* update the owner */
			if (view->osdupdate != NULL)
				(*view->osdupdate)(view, view->osdprivate);
			view->osd_update_pending = FALSE;
		}
}


/*-------------------------------------------------
    debug_view_update_all - force all views to
    refresh
-------------------------------------------------*/

void debug_view_update_all(running_machine *machine)
{
	debugvw_private *global = machine->debugvw_data;
	debug_view *view;

	/* skip if we're not ready yet */
	if (global == NULL)
		return;

	/* loop over each view and force an update */
	for (view = global->viewlist; view != NULL; view = view->next)
	{
		debug_view_begin_update(view);
		view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    debug_view_update_type - force all views of
    a given type to refresh
-------------------------------------------------*/

void debug_view_update_type(running_machine *machine, int type)
{
	debugvw_private *global = machine->debugvw_data;
	debug_view *view;

	/* skip if we're not ready yet */
	if (global == NULL)
		return;

	/* loop over each view and force an update */
	for (view = global->viewlist; view != NULL; view = view->next)
		if (view->type == type)
		{
			debug_view_begin_update(view);
			view->recompute = view->update_pending = TRUE;
			debug_view_end_update(view);
		}
}



/***************************************************************************
    STANDARD VIEW PROPERTIES
***************************************************************************/

/*-------------------------------------------------
    debug_view_get_chars - return a pointer to
    a 2-dimentional array of characters that
    represent the visible area of the view
-------------------------------------------------*/

const debug_view_char *debug_view_get_chars(debug_view *view)
{
	return view->viewdata;
}


/*-------------------------------------------------
    debug_view_type_character - type a character
    into a view
-------------------------------------------------*/

void debug_view_type_character(debug_view *view, int character)
{
	/* if the view has a character handler, forward it on */
	if (view->cb.handlechar != NULL)
		(*view->cb.handlechar)(view, character);
}



/***************************************************************************
    STANDARD VIEW SIZING
***************************************************************************/

/*-------------------------------------------------
    debug_view_get_total_size - return the total
    view size in rows and columns
-------------------------------------------------*/

debug_view_xy debug_view_get_total_size(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->total;
}


/*-------------------------------------------------
    debug_view_get_visible_size - return the
    visible size in rows and columns
-------------------------------------------------*/

debug_view_xy debug_view_get_visible_size(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->visible;
}


/*-------------------------------------------------
    debug_view_get_visible_position - return the
    top left position of the visible area in rows
    and columns
-------------------------------------------------*/

debug_view_xy debug_view_get_visible_position(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->topleft;
}


/*-------------------------------------------------
    debug_view_set_visible_size - set the visible
    size in rows and columns
-------------------------------------------------*/

void debug_view_set_visible_size(debug_view *view, debug_view_xy size)
{
	if (size.x != view->visible.x || size.y != view->visible.y)
	{
		debug_view_begin_update(view);
		view->visible = size;
		view->update_pending = TRUE;
		if (view->cb.notify != NULL)
			(*view->cb.notify)(view, VIEW_NOTIFY_VISIBLE_CHANGED);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    debug_view_set_visible_position - set the
    top left position of the visible area in rows
    and columns
-------------------------------------------------*/

void debug_view_set_visible_position(debug_view *view, debug_view_xy pos)
{
	if (pos.x != view->topleft.x || pos.y != view->topleft.y)
	{
		debug_view_begin_update(view);
		view->topleft = pos;
		view->update_pending = TRUE;
		if (view->cb.notify != NULL)
			(*view->cb.notify)(view, VIEW_NOTIFY_VISIBLE_CHANGED);
		debug_view_end_update(view);
	}
}



/***************************************************************************
    STANDARD VIEW CURSOR MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    debug_view_get_cursor_position - return the
    current cursor position as a row and column
-------------------------------------------------*/

debug_view_xy debug_view_get_cursor_position(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->cursor;
}


/*-------------------------------------------------
    debug_view_get_cursor_supported - return TRUE
    if a cursor is supported for this view type
-------------------------------------------------*/

int debug_view_get_cursor_supported(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->supports_cursor;
}


/*-------------------------------------------------
    debug_view_get_cursor_visible - return TRUE
    if a cursor is currently visible
-------------------------------------------------*/

int debug_view_get_cursor_visible(debug_view *view)
{
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return view->cursor_visible;
}


/*-------------------------------------------------
    debug_view_set_cursor_position - set the
    current cursor position as a row and column
-------------------------------------------------*/

void debug_view_set_cursor_position(debug_view *view, debug_view_xy pos)
{
	if (pos.x != view->cursor.x || pos.y != view->cursor.y)
	{
		debug_view_begin_update(view);
		view->cursor = pos;
		view->update_pending = TRUE;
		if (view->cb.notify != NULL)
			(*view->cb.notify)(view, VIEW_NOTIFY_CURSOR_CHANGED);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    debug_view_set_cursor_visible - set the
    visible state of the cursor
-------------------------------------------------*/

void debug_view_set_cursor_visible(debug_view *view, int visible)
{
	if (visible != view->cursor_visible)
	{
		debug_view_begin_update(view);
		view->cursor_visible = visible;
		view->update_pending = TRUE;
		if (view->cb.notify != NULL)
			(*view->cb.notify)(view, VIEW_NOTIFY_CURSOR_CHANGED);
		debug_view_end_update(view);
	}
}



/***************************************************************************
    GENERIC EXPRESSION HANDLING
***************************************************************************/

/*-------------------------------------------------
    debug_view_expression_alloc - allocate data
    for an expression
-------------------------------------------------*/

static void debug_view_expression_alloc(debug_view_expression *expression)
{
}


/*-------------------------------------------------
    debug_view_expression_free - free data
    allocated for an expression
-------------------------------------------------*/

static void debug_view_expression_free(debug_view_expression *expression)
{
	if (expression->parsed != NULL)
		expression_free(expression->parsed);
}


/*-------------------------------------------------
    debug_view_expression_set - set a new
    expression string
-------------------------------------------------*/

static void debug_view_expression_set(debug_view_expression *expression, const char *string)
{
	expression->string.cpy(string);
	expression->dirty = TRUE;
}


/*-------------------------------------------------
    debug_view_expression_changed_value - update an
    expression and return TRUE if its value has
    changed
-------------------------------------------------*/

static int debug_view_expression_changed_value(debug_view *view, debug_view_expression *expression, running_device *cpu)
{
	int changed = expression->dirty;
	EXPRERR exprerr;

	/* if dirty, re-evaluate */
	if (expression->dirty)
	{
		symbol_table *symtable = (cpu != NULL) ? debug_cpu_get_symtable(cpu) : debug_cpu_get_global_symtable(view->machine);
		parsed_expression *expr;

		/* parse the new expression */
		exprerr = expression_parse(expression->string, symtable, &debug_expression_callbacks, view->machine, &expr);

		/* if it worked, update the expression */
		if (exprerr == EXPRERR_NONE)
		{
			if (expression->parsed != NULL)
				expression_free(expression->parsed);
			expression->parsed = expr;
		}
	}

	/* if we have a parsed expression, evalute it */
	if (expression->parsed != NULL)
	{
		UINT64 oldresult = expression->result;

		/* recompute the value of the expression */
		exprerr = expression_execute(expression->parsed, &expression->result);
		changed |= (expression->result != oldresult);
	}

	/* expression no longer dirty by definition */
	expression->dirty = FALSE;
	return changed;
}



/***************************************************************************
    TEXT BUFFER-BASED VIEWS
***************************************************************************/

/*-------------------------------------------------
    console_view_alloc - allocate memory for the
    console view
-------------------------------------------------*/

static int console_view_alloc(debug_view *view)
{
	return textbuf_view_alloc(view, debug_console_get_textbuf());
}


/*-------------------------------------------------
    log_view_alloc - allocate memory for the log
    view
-------------------------------------------------*/

static int log_view_alloc(debug_view *view)
{
	return textbuf_view_alloc(view, debug_errorlog_get_textbuf());
}


/*-------------------------------------------------
    textbuf_view_alloc - allocate memory for a
    text buffer-based view
-------------------------------------------------*/

static int textbuf_view_alloc(debug_view *view, text_buffer *textbuf)
{
	debug_view_textbuf *textdata;

	/* allocate memory */
	textdata = auto_alloc_clear(view->machine, debug_view_textbuf);

	/* by default we track live */
	textdata->textbuf = textbuf;
	textdata->at_bottom = TRUE;

	/* stash the extra data pointer */
	view->extra_data = textdata;
	return TRUE;
}


/*-------------------------------------------------
    textbuf_view_free - free memory for a text
    buffer-based view
-------------------------------------------------*/

static void textbuf_view_free(debug_view *view)
{
	debug_view_textbuf *textdata = (debug_view_textbuf *)view->extra_data;

	/* free any memory we callocated */
	auto_free(view->machine, textdata);
	view->extra_data = NULL;
}


/*-------------------------------------------------
    textbuf_view_update - update a text buffer-
    based view
-------------------------------------------------*/

static void textbuf_view_update(debug_view *view)
{
	debug_view_textbuf *textdata = (debug_view_textbuf *)view->extra_data;
	debug_view_char *dest = view->viewdata;
	UINT32 curseq = 0, row;

	/* update the console info */
	view->total.x = text_buffer_max_width(textdata->textbuf);
	view->total.y = text_buffer_num_lines(textdata->textbuf);
	if (view->total.x < 80)
		view->total.x = 80;

	/* determine the starting sequence number */
	if (!textdata->at_bottom)
	{
		curseq = textdata->topseq;
		if (!text_buffer_get_seqnum_line(textdata->textbuf, curseq))
			textdata->at_bottom = TRUE;
	}
	if (textdata->at_bottom)
	{
		curseq = text_buffer_line_index_to_seqnum(textdata->textbuf, view->total.y - 1);
		if (view->total.y < view->visible.y)
			curseq -= view->total.y - 1;
		else
			curseq -= view->visible.y - 1;
	}
	view->topleft.y = curseq - text_buffer_line_index_to_seqnum(textdata->textbuf, 0);

	/* loop over visible rows */
	for (row = 0; row < view->visible.y; row++)
	{
		const char *line = text_buffer_get_seqnum_line(textdata->textbuf, curseq++);
		UINT32 col = 0;

		/* if this visible row is valid, add it to the buffer */
		if (line != NULL)
		{
			size_t len = strlen(line);
			UINT32 effcol = view->topleft.x;

			/* copy data */
			while (col < view->visible.x && effcol < len)
			{
				dest->byte = line[effcol++];
				dest->attrib = DCA_NORMAL;
				dest++;
				col++;
			}
		}

		/* fill the rest with blanks */
		while (col < view->visible.x)
		{
			dest->byte = ' ';
			dest->attrib = DCA_NORMAL;
			dest++;
			col++;
		}
	}
}


/*-------------------------------------------------
    textbuf_view_notify - handle notification of
    updates to visible area
-------------------------------------------------*/

static void textbuf_view_notify(debug_view *view, view_notification type)
{
	debug_view_textbuf *textdata = (debug_view_textbuf *)view->extra_data;

	if (type == VIEW_NOTIFY_VISIBLE_CHANGED)
	{
		/* if the bottom line is visible, just track the bottom */
		textdata->at_bottom = (view->total.y >= view->topleft.y && view->total.y <= view->topleft.y + view->visible.y);

		/* otherwise, track the seqence number of the top line */
		if (!textdata->at_bottom)
			textdata->topseq = text_buffer_line_index_to_seqnum(textdata->textbuf, view->topleft.y);
	}
}



/***************************************************************************
    REGISTERS VIEW
***************************************************************************/

/*-------------------------------------------------
    registers_view_enumerate_subviews - enumerate
    all possible subviews for a registers view
-------------------------------------------------*/

static const registers_subview_item *registers_view_enumerate_subviews(running_machine *machine)
{
	registers_subview_item *head = NULL;
	registers_subview_item **tailptr = &head;
	int curindex = 0;

	/* iterate over CPUs with program address spaces */
	for (running_device *cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		registers_subview_item *subview = auto_alloc(machine, registers_subview_item);

		/* populate the subview */
		subview->next = NULL;
		subview->index = curindex++;
		subview->name.printf("CPU '%s' (%s)", cpu->tag(), cpu->name());
		subview->device = cpu;

		/* add to the list */
		*tailptr = subview;
		tailptr = &subview->next;
	}

	return head;
}


/*-------------------------------------------------
    registers_view_alloc - allocate memory for the
    registers view
-------------------------------------------------*/

static int registers_view_alloc(debug_view *view)
{
	/* fail if no available subviews */
	if (view->machine->debugvw_data->registers_subviews == NULL)
		return FALSE;

	/* allocate memory */
	debug_view_registers *regdata = auto_alloc_clear(view->machine, debug_view_registers);

	/* default to the first subview */
	regdata->device = view->machine->debugvw_data->registers_subviews->device;

	/* stash the extra data pointer */
	view->extra_data = regdata;
	return TRUE;
}


/*-------------------------------------------------
    registers_view_free - free memory for the
    registers view
-------------------------------------------------*/

static void registers_view_free(debug_view *view)
{
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;

	/* free any memory we callocated */
	if (regdata != NULL)
		auto_free(view->machine, regdata);
	view->extra_data = NULL;
}


/*-------------------------------------------------
    registers_view_add_register - adds a register
    to the registers view
-------------------------------------------------*/

static void registers_view_add_register(debug_view *view, int regnum, const char *str)
{
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;
	int tagstart, taglen, valstart, vallen;
	const char *colon;

	colon = strchr(str, ':');

	/* if no colon, mark everything as tag */
	if (colon == NULL)
	{
		tagstart = 0;
		taglen = (int)strlen(str);
		valstart = 0;
		vallen = 0;
	}

	/* otherwise, break the string at the colon */
	else
	{
		tagstart = 0;
		taglen = colon - str;
		valstart = (colon + 1) - str;
		vallen = (int)strlen(colon + 1);
	}

	/* now trim spaces */
	while (isspace((UINT8)str[tagstart]) && taglen > 0)
		tagstart++, taglen--;
	while (isspace((UINT8)str[tagstart + taglen - 1]) && taglen > 0)
		taglen--;
	while (isspace((UINT8)str[valstart]) && vallen > 0)
		valstart++, vallen--;
	while (isspace((UINT8)str[valstart + vallen - 1]) && vallen > 0)
		vallen--;
	if (str[valstart] == '!')
		valstart++, vallen--;

	/* note the register number and info */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = cpu_get_reg(regdata->device, regnum);
	regdata->reg[view->total.y].regnum   = regnum;
	regdata->reg[view->total.y].tagstart = tagstart;
	regdata->reg[view->total.y].taglen   = taglen;
	regdata->reg[view->total.y].valstart = valstart;
	regdata->reg[view->total.y].vallen   = vallen;
	view->total.y++;

	/* adjust the divider and total cols, if necessary */
	regdata->divider = MAX(regdata->divider, 1 + taglen + 1);
	view->total.x = MAX(view->total.x, 1 + taglen + 2 + vallen + 1);
}


/*-------------------------------------------------
    registers_view_recompute - recompute all info
    for the registers view
-------------------------------------------------*/

static void registers_view_recompute(debug_view *view)
{
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;
	int regnum, maxtaglen, maxvallen;
	const cpu_state_table *table;

	/* if no CPU, reset to the first one */
	if (regdata->device == NULL)
		regdata->device = view->machine->firstcpu;
	table = cpu_get_state_table(regdata->device);

	/* reset the view parameters */
	view->topleft.y = 0;
	view->topleft.x = 0;
	view->total.y = 0;
	view->total.x = 0;
	regdata->divider = 0;

	/* add a cycles entry: cycles:99999999 */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS + 1;
	regdata->reg[view->total.y].tagstart = 0;
	regdata->reg[view->total.y].taglen   = 6;
	regdata->reg[view->total.y].valstart = 7;
	regdata->reg[view->total.y].vallen   = 8;
	maxtaglen = regdata->reg[view->total.y].taglen;
	maxvallen = regdata->reg[view->total.y].vallen;
	view->total.y++;

	/* add a beam entry: beamx:123 */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS + 2;
	regdata->reg[view->total.y].tagstart = 0;
	regdata->reg[view->total.y].taglen   = 5;
	regdata->reg[view->total.y].valstart = 6;
	regdata->reg[view->total.y].vallen   = 3;
	maxtaglen = MAX(maxtaglen, regdata->reg[view->total.y].taglen);
	maxvallen = MAX(maxvallen, regdata->reg[view->total.y].vallen);
	view->total.y++;

	/* add a beam entry: beamy:456 */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS + 3;
	regdata->reg[view->total.y].tagstart = 0;
	regdata->reg[view->total.y].taglen   = 5;
	regdata->reg[view->total.y].valstart = 6;
	regdata->reg[view->total.y].vallen   = 3;
	maxtaglen = MAX(maxtaglen, regdata->reg[view->total.y].taglen);
	maxvallen = MAX(maxvallen, regdata->reg[view->total.y].vallen);
	view->total.y++;

	/* add a beam entry: frame:123456 */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS + 4;
	regdata->reg[view->total.y].tagstart = 0;
	regdata->reg[view->total.y].taglen   = 5;
	regdata->reg[view->total.y].valstart = 6;
	regdata->reg[view->total.y].vallen   = 6;
	maxtaglen = MAX(maxtaglen, regdata->reg[view->total.y].taglen);
	maxvallen = MAX(maxvallen, regdata->reg[view->total.y].vallen);
	view->total.y++;

	/* add a flags entry: flags:xxxxxxxx */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS + 5;
	regdata->reg[view->total.y].tagstart = 0;
	regdata->reg[view->total.y].taglen   = 5;
	regdata->reg[view->total.y].valstart = 6;
	regdata->reg[view->total.y].vallen   = (UINT32)strlen(cpu_get_flags_string(regdata->device));
	maxtaglen = MAX(maxtaglen, regdata->reg[view->total.y].taglen);
	maxvallen = MAX(maxvallen, regdata->reg[view->total.y].vallen);
	view->total.y++;

	/* add a divider entry */
	regdata->reg[view->total.y].lastval  =
	regdata->reg[view->total.y].currval  = 0;
	regdata->reg[view->total.y].regnum   = MAX_REGS;
	view->total.y++;

	/* set the current divider and total cols */
	regdata->divider = 1 + maxtaglen + 1;
	view->total.x = 1 + maxtaglen + 2 + maxvallen + 1;

	/* add all registers into it */
	for (regnum = 0; regnum < MAX_REGS; regnum++)
	{
		const char *str = NULL;
		int regid;

		/* identify the register id */
		if (table != NULL)
		{
			if (regnum >= table->entrycount)
				break;
			if ((table->entrylist[regnum].validmask & table->subtypemask) == 0)
				continue;
			regid = table->entrylist[regnum].index;
		}
		else
			regid = regnum;

		/* retrieve the string for this register */
		str = cpu_get_reg_string(regdata->device, regid);

		/* did we get a string? */
		if (str != NULL && str[0] != 0 && str[0] != '~')
			registers_view_add_register(view, regid, str);
	}

	/* no longer need to recompute */
	view->recompute = FALSE;
}


/*-------------------------------------------------
    registers_view_update - update the contents of
    the register view
-------------------------------------------------*/

static void registers_view_update(debug_view *view)
{
	running_device *screen = view->machine->primary_screen;
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;
	debug_view_char *dest = view->viewdata;
	UINT64 total_cycles;
	UINT32 row, i;

	/* if our assumptions changed, revisit them */
	if (view->recompute || regdata->device == NULL)
		registers_view_recompute(view);

	/* cannot update if no active CPU */
	total_cycles = cpu_get_total_cycles(regdata->device);

	/* loop over visible rows */
	for (row = 0; row < view->visible.y; row++)
	{
		UINT32 effrow = view->topleft.y + row;
		UINT32 col = 0;

		/* if this visible row is valid, add it to the buffer */
		if (effrow < view->total.y)
		{
			debug_view_register *reg = &regdata->reg[effrow];
			UINT32 effcol = view->topleft.x;
			char temp[256], dummy[100];
			UINT8 attrib = DCA_NORMAL;
			UINT32 len = 0;
			char *data = dummy;

			/* get the effective string */
			dummy[0] = 0;
			if (reg->regnum >= MAX_REGS)
			{
				reg->lastval = reg->currval;
				switch (reg->regnum)
				{
					case MAX_REGS:
						reg->tagstart = reg->valstart = reg->vallen = 0;
						reg->taglen = view->total.x;
						for (i = 0; i < view->total.x; i++)
							dummy[i] = '-';
						dummy[i] = 0;
						break;

					case MAX_REGS + 1:
						sprintf(dummy, "cycles:%-8d", *cpu_get_icount_ptr(regdata->device));
						reg->currval = *cpu_get_icount_ptr(regdata->device);
						break;

					case MAX_REGS + 2:
						if (screen != NULL)
							sprintf(dummy, "beamx:%3d", video_screen_get_hpos(screen));
						break;

					case MAX_REGS + 3:
						if (screen != NULL)
							sprintf(dummy, "beamy:%3d", video_screen_get_vpos(screen));
						break;

					case MAX_REGS + 4:
						if (screen != NULL)
							sprintf(dummy, "frame:%-6d", (UINT32)video_screen_get_frame_number(screen));
						break;

					case MAX_REGS + 5:
						sprintf(dummy, "flags:%s", cpu_get_flags_string(regdata->device));
						break;
				}
			}
			else
			{
				data = (char *)cpu_get_reg_string(regdata->device, reg->regnum);
				if (regdata->last_update != total_cycles)
					reg->lastval = reg->currval;
				reg->currval = cpu_get_reg(regdata->device, reg->regnum);
			}

			/* see if we changed */
			if (reg->lastval != reg->currval)
				attrib = DCA_CHANGED;

			/* build up a string */
			if (reg->taglen < regdata->divider - 1)
			{
				memset(&temp[len], ' ', regdata->divider - 1 - reg->taglen);
				len += regdata->divider - 1 - reg->taglen;
			}

			memcpy(&temp[len], &data[reg->tagstart], reg->taglen);
			len += reg->taglen;

			temp[len++] = ' ';
			temp[len++] = ' ';

			memcpy(&temp[len], &data[reg->valstart], reg->vallen);
			len += reg->vallen;

			temp[len++] = ' ';
			temp[len] = 0;

			/* copy data */
			while (col < view->visible.x && effcol < len)
			{
				dest->byte = temp[effcol++];
				dest->attrib = attrib | ((effcol <= regdata->divider) ? DCA_ANCILLARY : DCA_NORMAL);
				dest++;
				col++;
			}
		}

		/* fill the rest with blanks */
		while (col < view->visible.x)
		{
			dest->byte = ' ';
			dest->attrib = DCA_NORMAL;
			dest++;
			col++;
		}
	}

	/* remember the last update */
	regdata->last_update = total_cycles;
}


/*-------------------------------------------------
    registers_view_get_subview_list - return a
    linked list of subviews
-------------------------------------------------*/

const registers_subview_item *registers_view_get_subview_list(debug_view *view)
{
	assert(view->type == DVT_REGISTERS);
	return view->machine->debugvw_data->registers_subviews;
}


/*-------------------------------------------------
    registers_view_get_subview - return the current
    subview index
-------------------------------------------------*/

int registers_view_get_subview(debug_view *view)
{
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;
	const registers_subview_item *subview;
	int index = 0;

	assert(view->type == DVT_REGISTERS);
	debug_view_begin_update(view);
	debug_view_end_update(view);

	for (subview = view->machine->debugvw_data->registers_subviews; subview != NULL; subview = subview->next)
	{
		if (subview->device == regdata->device)
			return index;
		index++;
	}
	return 0;
}


/*-------------------------------------------------
    registers_view_set_subview - select a new
    subview by index
-------------------------------------------------*/

void registers_view_set_subview(debug_view *view, int index)
{
	const registers_subview_item *subview = registers_view_get_subview_by_index(view->machine->debugvw_data->registers_subviews, index);
	debug_view_registers *regdata = (debug_view_registers *)view->extra_data;

	assert(view->type == DVT_REGISTERS);
	assert(subview != NULL);
	if (subview == NULL)
		return;

	/* handle a change */
	if (subview->device != regdata->device)
	{
		debug_view_begin_update(view);
		regdata->device = subview->device;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}



/***************************************************************************
    DISASSEMBLY VIEW
***************************************************************************/

/*-------------------------------------------------
    disasm_view_enumerate_subviews - enumerate
    all possible subviews for a disassembly view
-------------------------------------------------*/

static const disasm_subview_item *disasm_view_enumerate_subviews(running_machine *machine)
{
	disasm_subview_item *head = NULL;
	disasm_subview_item **tailptr = &head;
	int curindex = 0;

	/* iterate over CPUs with program address spaces */
	for (running_device *cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		const address_space *space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM);
		if (space != NULL)
		{
			disasm_subview_item *subview = auto_alloc(machine, disasm_subview_item);

			/* populate the subview */
			subview->next = NULL;
			subview->index = curindex++;
			subview->name.printf("CPU '%s' (%s)", cpu->tag(), cpu->name());
			subview->space = space;

			/* add to the list */
			*tailptr = subview;
			tailptr = &subview->next;
		}
	}

	return head;
}


/*-------------------------------------------------
    disasm_view_alloc - allocate disasm for the
    disassembly view
-------------------------------------------------*/

static int disasm_view_alloc(debug_view *view)
{
	debug_view_disasm *dasmdata;
	int total_comments = 0;
	running_device *cpu;

	/* fail if no available subviews */
	if (view->machine->debugvw_data->disasm_subviews == NULL)
		return FALSE;

	/* allocate disasm */
	dasmdata = auto_alloc_clear(view->machine, debug_view_disasm);

	/* default to the first subview */
	dasmdata->space = view->machine->debugvw_data->disasm_subviews->space;

	/* allocate the expression data */
	debug_view_expression_alloc(&dasmdata->expression);

	/* count the number of comments */
	for (cpu = view->machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		total_comments += debug_comment_get_count(cpu);

	/* initialize */
	dasmdata->right_column = (total_comments > 0) ? DASM_RIGHTCOL_COMMENTS : DASM_RIGHTCOL_RAW;
	dasmdata->backwards_steps = 3;
	dasmdata->dasm_width = DEFAULT_DASM_WIDTH;

	/* stash the extra data pointer */
	view->total.y = DEFAULT_DASM_LINES;
	view->extra_data = dasmdata;

	/* we support cursors */
	view->supports_cursor = TRUE;
	return TRUE;
}


/*-------------------------------------------------
    disasm_view_free - free disasm for the
    disassembly view
-------------------------------------------------*/

static void disasm_view_free(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	/* free any disasm we callocated */
	if (dasmdata != NULL)
	{
		debug_view_expression_free(&dasmdata->expression);
		if (dasmdata->byteaddress != NULL)
			auto_free(view->machine, dasmdata->byteaddress);
		if (dasmdata->dasm != NULL)
			auto_free(view->machine, dasmdata->dasm);
		auto_free(view->machine, dasmdata);
	}
	view->extra_data = NULL;
}


/*-------------------------------------------------
    disasm_view_notify - handle notification of
    updates to cursor changes
-------------------------------------------------*/

static void disasm_view_notify(debug_view *view, view_notification type)
{
	if (type == VIEW_NOTIFY_CURSOR_CHANGED)
		adjust_visible_y_for_cursor(view);
}


/*-------------------------------------------------
    disasm_view_char - handle a character typed
    within the current view
-------------------------------------------------*/

static void disasm_view_char(debug_view *view, int chval)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	debug_view_xy origcursor = view->cursor;
	UINT8 end_buffer = 3;
	INT32 temp;

	switch (chval)
	{
		case DCH_UP:
			if (view->cursor.y > 0)
				view->cursor.y--;
			break;

		case DCH_DOWN:
			if (view->cursor.y < view->total.y - 1)
				view->cursor.y++;
			break;

		case DCH_PUP:
			temp = view->cursor.y - (view->visible.y - end_buffer);
			if (temp < 0)
				view->cursor.y = 0;
			else
				view->cursor.y = temp;
			break;

		case DCH_PDOWN:
			temp = view->cursor.y + (view->visible.y - end_buffer);
			if (temp > view->total.y - 1)
				view->cursor.y = view->total.y - 1;
			else
				view->cursor.y = temp;
			break;

		case DCH_HOME:				/* set the active column to the PC */
		{
			offs_t pc = memory_address_to_byte(dasmdata->space, cpu_get_pc(dasmdata->space->cpu)) & dasmdata->space->logbytemask;
			int curline;

			/* figure out which row the pc is on */
			for (curline = 0; curline < dasmdata->allocated.y; curline++)
				if (dasmdata->byteaddress[curline] == pc)
					view->cursor.y = curline;
			break;
		}

		case DCH_CTRLHOME:
			view->cursor.y = 0;
			break;

		case DCH_CTRLEND:
			view->cursor.y = view->total.y - 1;
			break;
	}

	/* send a cursor changed notification */
	if (view->cursor.y != origcursor.y)
	{
		debug_view_begin_update(view);
		disasm_view_notify(view, VIEW_NOTIFY_CURSOR_CHANGED);
		view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    disasm_view_find_pc_backwards - back up the
    specified numberof instructions from the given
    PC
-------------------------------------------------*/

static offs_t disasm_view_find_pc_backwards(const address_space *space, offs_t targetpc, int numinstrs)
{
	int minlen = memory_byte_to_address(space, cpu_get_min_opcode_bytes(space->cpu));
	int maxlen = memory_byte_to_address(space, cpu_get_max_opcode_bytes(space->cpu));
	offs_t targetpcbyte = memory_address_to_byte(space, targetpc) & space->logbytemask;
	offs_t lastgoodpc = targetpc;
	offs_t fillpcbyte, curpc;
	UINT8 opbuf[1024], argbuf[1024];
	char dasmbuffer[100];

	/* compute the increment */
	if (minlen == 0) minlen = 1;
	if (maxlen == 0) maxlen = 1;

	/* start off numinstrs back */
	curpc = targetpc - minlen * numinstrs;
	if (curpc > targetpc)
		curpc = 0;

	/* loop until we find what we are looking for */
	fillpcbyte = targetpcbyte;
	while (1)
	{
		offs_t curpcbyte = memory_address_to_byte(space, curpc) & space->logbytemask;
		offs_t scanpc;
		int instcount = 0;
		int instlen;

		/* fill the buffer up to the target */
		while (curpcbyte < fillpcbyte)
		{
			fillpcbyte--;
			opbuf[1000 + fillpcbyte - targetpcbyte] = debug_read_opcode(space, fillpcbyte, 1, FALSE);
			argbuf[1000 + fillpcbyte - targetpcbyte] = debug_read_opcode(space, fillpcbyte, 1, TRUE);
		}

		/* loop until we get past the target instruction */
		for (scanpc = curpc; scanpc < targetpc; scanpc += instlen)
		{
			offs_t scanpcbyte = memory_address_to_byte(space, scanpc) & space->logbytemask;
			offs_t physpcbyte = scanpcbyte;

			/* get the disassembly, but only if mapped */
			instlen = 1;
			if (debug_cpu_translate(space, TRANSLATE_FETCH, &physpcbyte))
				instlen = debug_cpu_disassemble(space->cpu, dasmbuffer, scanpc, &opbuf[1000 + scanpcbyte - targetpcbyte], &argbuf[1000 + scanpcbyte - targetpcbyte]) & DASMFLAG_LENGTHMASK;

			/* count this one */
			instcount++;
		}

		/* if we ended up right on targetpc, this is a good candidate */
		if (scanpc == targetpc && instcount <= numinstrs)
			lastgoodpc = curpc;

		/* we're also done if we go back too far */
		if (targetpc - curpc >= numinstrs * maxlen)
			break;

		/* and if we hit 0, we're done */
		if (curpc == 0)
			break;

		/* back up one more and try again */
		curpc -= minlen;
		if (curpc > targetpc)
			curpc = 0;
	}

	return lastgoodpc;
}


/*-------------------------------------------------
    disasm_view_generate_bytes - generate the
    opcode byte values
-------------------------------------------------*/

static void disasm_view_generate_bytes(const address_space *space, offs_t pcbyte, int numbytes, int minbytes, char *string, int maxchars, int encrypted)
{
	int byte, offset = 0;

	/* output the first value */
	if (maxchars >= 2 * minbytes)
		offset = sprintf(string, "%s", core_i64_hex_format(debug_read_opcode(space, pcbyte, minbytes, FALSE), minbytes * 2));

	/* output subsequent values */
	for (byte = minbytes; byte < numbytes && offset + 1 + 2 * minbytes < maxchars; byte += minbytes)
		offset += sprintf(&string[offset], " %s", core_i64_hex_format(debug_read_opcode(space, pcbyte + byte, minbytes, encrypted), minbytes * 2));

	/* if we ran out of room, indicate more */
	string[maxchars - 1] = 0;
	if (byte < numbytes && maxchars > 3)
		string[maxchars - 2] = string[maxchars - 3] = string[maxchars - 4] = '.';
}


/*-------------------------------------------------
    disasm_view_recompute - recompute selected info
    for the disassembly view
-------------------------------------------------*/

static int disasm_view_recompute(debug_view *view, offs_t pc, int startline, int lines)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	const address_space *space = dasmdata->space;
	int minbytes, maxbytes, maxbytes_clamped;
	int changed = FALSE;
	int line;

	/* determine how many characters we need for an address and set the divider */
	dasmdata->divider1 = 1 + space->logaddrchars + 1;

	/* assume a fixed number of characters for the disassembly */
	dasmdata->divider2 = dasmdata->divider1 + 1 + dasmdata->dasm_width + 1;

	/* determine how many bytes we might need to display */
	minbytes = cpu_get_min_opcode_bytes(space->cpu);
	maxbytes = cpu_get_max_opcode_bytes(space->cpu);

	/* ensure that the PC is aligned to the minimum opcode size */
	pc &= ~memory_byte_to_address_end(space, minbytes - 1);

	/* set the width of the third column according to display mode */
	if (dasmdata->right_column == DASM_RIGHTCOL_RAW || dasmdata->right_column == DASM_RIGHTCOL_ENCRYPTED)
	{
		maxbytes_clamped = MIN(maxbytes, DASM_MAX_BYTES);
		view->total.x = dasmdata->divider2 + 1 + 2 * maxbytes_clamped + (maxbytes_clamped / minbytes - 1) + 1;
	}
	else if (dasmdata->right_column == DASM_RIGHTCOL_COMMENTS)
		view->total.x = dasmdata->divider2 + 1 + 50;		/* DEBUG_COMMENT_MAX_LINE_LENGTH */
	else
		view->total.x = dasmdata->divider2 + 1;

	/* reallocate memory if we don't have enough */
	if (dasmdata->allocated.x < view->total.x || dasmdata->allocated.y < view->total.y)
	{
		/* update our values */
		dasmdata->allocated.x = view->total.x;
		dasmdata->allocated.y = view->total.y;

		/* allocate address array */
		auto_free(view->machine, dasmdata->byteaddress);
		dasmdata->byteaddress = auto_alloc_array(view->machine, offs_t, dasmdata->allocated.y);

		/* allocate disassembly buffer */
		auto_free(view->machine, dasmdata->dasm);
		dasmdata->dasm = auto_alloc_array(view->machine, char, dasmdata->allocated.x * dasmdata->allocated.y);
	}

	/* iterate over lines */
	for (line = 0; line < lines; line++)
	{
		int instr = startline + line;
		char *destbuf = &dasmdata->dasm[instr * dasmdata->allocated.x];
		char buffer[100], oldbuf[100];
		offs_t pcbyte, physpcbyte;
		int numbytes = 0;

		/* convert PC to a byte offset */
		pcbyte = memory_address_to_byte(space, pc) & space->logbytemask;

		/* save a copy of the previous line as a backup if we're only doing one line */
		if (lines == 1)
			strncpy(oldbuf, destbuf, MIN(sizeof(oldbuf), dasmdata->allocated.x));

		/* convert back and set the address of this instruction */
		dasmdata->byteaddress[instr] = pcbyte;
		sprintf(&destbuf[0], " %s  ", core_i64_hex_format(memory_byte_to_address(space, pcbyte), space->logaddrchars));

		/* make sure we can translate the address, and then disassemble the result */
		physpcbyte = pcbyte;
		if (debug_cpu_translate(space, TRANSLATE_FETCH_DEBUG, &physpcbyte))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, FALSE);
				argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, TRUE);
			}

			/* disassemble the result */
			pc += numbytes = debug_cpu_disassemble(space->cpu, buffer, pc & space->logaddrmask, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}
		else
			strcpy(buffer, "<unmapped>");

		/* append the disassembly to the buffer */
		sprintf(&destbuf[dasmdata->divider1 + 1], "%-*s  ", dasmdata->dasm_width, buffer);

		/* output the right column */
		if (dasmdata->right_column == DASM_RIGHTCOL_RAW || dasmdata->right_column == DASM_RIGHTCOL_ENCRYPTED)
		{
			/* get the bytes */
			numbytes = memory_address_to_byte(space, numbytes) & space->logbytemask;
			disasm_view_generate_bytes(space, pcbyte, numbytes, minbytes, &destbuf[dasmdata->divider2], dasmdata->allocated.x - dasmdata->divider2, dasmdata->right_column == DASM_RIGHTCOL_ENCRYPTED);
		}
		else if (dasmdata->right_column == DASM_RIGHTCOL_COMMENTS)
		{
			offs_t comment_address = memory_byte_to_address(space, dasmdata->byteaddress[instr]);
			const char *text;

			/* get and add the comment, if present */
			text = debug_comment_get_text(space->cpu, comment_address, debug_comment_get_opcode_crc32(space->cpu, comment_address));
			if (text != NULL)
				sprintf(&destbuf[dasmdata->divider2], "// %.*s", dasmdata->allocated.x - dasmdata->divider2 - 1, text);
		}

		/* see if the line changed at all */
		if (lines == 1 && strncmp(oldbuf, destbuf, MIN(sizeof(oldbuf), dasmdata->allocated.x)) != 0)
			changed = TRUE;
	}

	/* update opcode base information */
	dasmdata->last_direct_decrypted = space->direct.decrypted;
	dasmdata->last_direct_raw = space->direct.raw;
	dasmdata->last_change_count = debug_comment_all_change_count(space->machine);

	/* now longer need to recompute */
	view->recompute = FALSE;
	return changed;
}


/*-------------------------------------------------
    disasm_view_update - update the contents of
    the disassembly view
-------------------------------------------------*/

static void disasm_view_update(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	const address_space *space = dasmdata->space;
	debug_view_char *dest = view->viewdata;
	int recomputed_this_time = FALSE;
	offs_t pc, pcbyte;
	EXPRERR exprerr;
	UINT32 row;

	/* no space, do nothing */
	if (space == NULL)
		return;
	pc = cpu_get_pc(space->cpu);
	pcbyte = memory_address_to_byte(space, pc) & space->logbytemask;

	/* if our expression is dirty, fix it */
	if (dasmdata->expression.dirty)
	{
		parsed_expression *expr;

		/* parse the new expression */
		exprerr = expression_parse(dasmdata->expression.string, debug_cpu_get_symtable(space->cpu), &debug_expression_callbacks, space->machine, &expr);

		/* if it worked, update the expression */
		if (exprerr == EXPRERR_NONE)
		{
			if (dasmdata->expression.parsed != NULL)
				expression_free(dasmdata->expression.parsed);
			dasmdata->expression.parsed = expr;
		}

		/* always recompute if the expression is dirty */
		view->recompute = TRUE;
	}

	/* if we're tracking a value, make sure it is visible */
	if (dasmdata->expression.parsed != NULL)
	{
		UINT64 result;

		/* recompute the value of the expression */
		exprerr = expression_execute(dasmdata->expression.parsed, &result);
		if (exprerr == EXPRERR_NONE && result != dasmdata->expression.result)
		{
			offs_t resultbyte = memory_address_to_byte(space, result) & space->logbytemask;

			/* update the result */
			dasmdata->expression.result = result;

			/* see if the new result is an address we already have */
			for (row = 0; row < dasmdata->allocated.y; row++)
				if (dasmdata->byteaddress[row] == resultbyte)
					break;

			/* if we didn't find it, or if it's really close to the bottom, recompute */
			if (row == dasmdata->allocated.y || row >= view->total.y - view->visible.y)
				view->recompute = TRUE;

			/* otherwise, if it's not visible, adjust the view so it is */
			else if (row < view->topleft.y || row >= view->topleft.y + view->visible.y - 2)
				view->topleft.y = (row > 3) ? row - 3 : 0;
		}

		/* no longer dirty */
		dasmdata->expression.dirty = FALSE;
	}

	/* if the opcode base has changed, rework things */
	if (space->direct.decrypted != dasmdata->last_direct_decrypted || space->direct.raw != dasmdata->last_direct_raw)
		view->recompute = TRUE;

	/* if the comments have changed, redo it */
	if (dasmdata->last_change_count != debug_comment_all_change_count(space->machine))
		view->recompute = TRUE;

	/* if we need to recompute, do it */
recompute:
	if (view->recompute)
	{
		/* recompute the view */
		if (dasmdata->byteaddress != NULL && dasmdata->last_change_count != debug_comment_all_change_count(space->machine))
		{
			/* smoosh us against the left column, but not the top row */
			view->topleft.x = 0;

			/* recompute from where we last recomputed! */
			disasm_view_recompute(view, memory_byte_to_address(space, dasmdata->byteaddress[0]), 0, view->total.y);
		}
		else
		{
			/* determine the addresses of what we will display */
			offs_t backpc = disasm_view_find_pc_backwards(space, (UINT32)dasmdata->expression.result, dasmdata->backwards_steps);

			/* put ourselves back in the top left */
			view->topleft.y = 0;
			view->topleft.x = 0;

			disasm_view_recompute(view, backpc, 0, view->total.y);
		}
		recomputed_this_time = TRUE;
	}

	/* figure out the row where the PC is and recompute the disassembly */
	if (pcbyte != dasmdata->last_pcbyte)
	{
		/* find the row with the PC on it */
		for (row = 0; row < view->visible.y; row++)
		{
			UINT32 effrow = view->topleft.y + row;
			if (effrow >= dasmdata->allocated.y)
				break;
			if (pcbyte == dasmdata->byteaddress[effrow])
			{
				/* see if we changed */
				int changed = disasm_view_recompute(view, pc, effrow, 1);
				if (changed && !recomputed_this_time)
				{
					view->recompute = TRUE;
					goto recompute;
				}

				/* set the effective row and PC */
				view->cursor.y = effrow;
			}
		}
		dasmdata->last_pcbyte = pcbyte;
	}

	/* loop over visible rows */
	for (row = 0; row < view->visible.y; row++)
	{
		UINT32 effrow = view->topleft.y + row;
		UINT8 attrib = DCA_NORMAL;
		debug_cpu_breakpoint *bp;
		UINT32 col = 0;

		/* if this visible row is valid, add it to the buffer */
		if (effrow < dasmdata->allocated.y)
		{
			const char *data = &dasmdata->dasm[effrow * dasmdata->allocated.x];
			UINT32 effcol = view->topleft.x;
			UINT32 len = 0;

			/* if we're on the line with the PC, recompute and hilight it */
			if (pcbyte == dasmdata->byteaddress[effrow])
				attrib = DCA_CURRENT;

			/* if we're on a line with a breakpoint, tag it changed */
			else
			{
				const cpu_debug_data *cpuinfo = cpu_get_debug_data(space->cpu);
				for (bp = cpuinfo->bplist; bp != NULL; bp = bp->next)
					if (dasmdata->byteaddress[effrow] == (memory_address_to_byte(space, bp->address) & space->logbytemask))
						attrib = DCA_CHANGED;
			}

			/* if we're on the active column and everything is couth, highlight it */
			if (view->cursor_visible && effrow == view->cursor.y)
				attrib |= DCA_SELECTED;

			/* get the effective string */
			len = (UINT32)strlen(data);

			/* copy data */
			while (col < view->visible.x && effcol < len)
			{
				dest->byte = data[effcol++];
				dest->attrib = (effcol <= dasmdata->divider1 || effcol >= dasmdata->divider2) ? (attrib | DCA_ANCILLARY) : attrib;

				/* comments are just green for now - maybe they shouldn't even be this? */
				if (effcol >= dasmdata->divider2 && dasmdata->right_column == DASM_RIGHTCOL_COMMENTS)
					attrib |= DCA_COMMENT;

				dest++;
				col++;
			}
		}

		/* fill the rest with blanks */
		while (col < view->visible.x)
		{
			dest->byte = ' ';
			dest->attrib = (effrow < view->total.y) ? (attrib | DCA_ANCILLARY) : attrib;
			dest++;
			col++;
		}
	}
}


/*-------------------------------------------------
    disasm_view_get_subview_list - return a linked
    list of subviews
-------------------------------------------------*/

const disasm_subview_item *disasm_view_get_subview_list(debug_view *view)
{
	assert(view->type == DVT_DISASSEMBLY);
	return view->machine->debugvw_data->disasm_subviews;
}


/*-------------------------------------------------
    disasm_view_get_subview - return the current
    subview index
-------------------------------------------------*/

int disasm_view_get_subview(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	const disasm_subview_item *subview;
	int index = 0;

	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);

	for (subview = view->machine->debugvw_data->disasm_subviews; subview != NULL; subview = subview->next)
	{
		if (subview->space == dasmdata->space)
			return index;
		index++;
	}
	return 0;
}


/*-------------------------------------------------
    disasm_view_get_expression - return the
    expression string describing the home address
-------------------------------------------------*/

const char *disasm_view_get_expression(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return dasmdata->expression.string;
}


/*-------------------------------------------------
    disasm_view_get_right_column - return the
    contents of the right column
-------------------------------------------------*/

disasm_right_column disasm_view_get_right_column(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return dasmdata->right_column;
}


/*-------------------------------------------------
    disasm_view_get_backward_steps - return the
    number of instructions displayed before the
    home address
-------------------------------------------------*/

UINT32 disasm_view_get_backward_steps(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return dasmdata->backwards_steps;
}


/*-------------------------------------------------
    disasm_view_get_disasm_width - return the
    width in characters of the main disassembly
    section
-------------------------------------------------*/

UINT32 disasm_view_get_disasm_width(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return dasmdata->dasm_width;
}


/*-------------------------------------------------
    disasm_view_get_selected_address - return the
    PC of the currently selected address in the
    view
-------------------------------------------------*/

offs_t disasm_view_get_selected_address(debug_view *view)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	assert(view->type == DVT_DISASSEMBLY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memory_byte_to_address(dasmdata->space, dasmdata->byteaddress[view->cursor.y]);
}


/*-------------------------------------------------
    disasm_view_set_subview - select a new subview
    by index
-------------------------------------------------*/

void disasm_view_set_subview(debug_view *view, int index)
{
	const disasm_subview_item *subview = disasm_view_get_subview_by_index(view->machine->debugvw_data->disasm_subviews, index);
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	assert(view->type == DVT_DISASSEMBLY);
	assert(subview != NULL);
	if (subview == NULL)
		return;

	/* handle a change */
	if (subview->space != dasmdata->space)
	{
		debug_view_begin_update(view);
		dasmdata->space = subview->space;

		/* we need to recompute the expression in the context of the new space's CPU */
		dasmdata->expression.dirty = TRUE;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    disasm_view_set_expression - set the
    expression string describing the home address
-------------------------------------------------*/

void disasm_view_set_expression(debug_view *view, const char *expression)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	assert(view->type == DVT_DISASSEMBLY);
	assert(expression != NULL);

	debug_view_begin_update(view);
	debug_view_expression_set(&dasmdata->expression, expression);
	view->recompute = view->update_pending = TRUE;
	debug_view_end_update(view);
}


/*-------------------------------------------------
    disasm_view_set_right_column - set the
    contents of the right column
-------------------------------------------------*/

void disasm_view_set_right_column(debug_view *view, disasm_right_column contents)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	assert(view->type == DVT_DISASSEMBLY);
	assert(contents == DASM_RIGHTCOL_RAW || contents == DASM_RIGHTCOL_ENCRYPTED || contents == DASM_RIGHTCOL_COMMENTS);

	if (contents != dasmdata->right_column)
	{
		debug_view_begin_update(view);
		dasmdata->right_column = contents;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    disasm_view_set_backward_steps - set the
    number of instructions displayed before the
    home address
-------------------------------------------------*/

void disasm_view_set_backward_steps(debug_view *view, UINT32 steps)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	assert(view->type == DVT_DISASSEMBLY);

	if (steps != dasmdata->backwards_steps)
	{
		debug_view_begin_update(view);
		dasmdata->backwards_steps = steps;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    disasm_view_set_disasm_width - set the
    width in characters of the main disassembly
    section
-------------------------------------------------*/

void disasm_view_set_disasm_width(debug_view *view, UINT32 width)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;

	assert(view->type == DVT_DISASSEMBLY);

	if (width != dasmdata->dasm_width)
	{
		debug_view_begin_update(view);
		dasmdata->dasm_width = width;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    disasm_view_set_selected_address - set the
    PC of the currently selected address in the
    view
-------------------------------------------------*/

void disasm_view_set_selected_address(debug_view *view, offs_t address)
{
	debug_view_disasm *dasmdata = (debug_view_disasm *)view->extra_data;
	offs_t byteaddress = memory_address_to_byte(dasmdata->space, address) & dasmdata->space->logbytemask;
	int line;

	assert(view->type == DVT_DISASSEMBLY);

	for (line = 0; line < view->total.y; line++)
		if (dasmdata->byteaddress[line] == byteaddress)
		{
			view->cursor.y = line;
			debug_view_set_cursor_position(view, view->cursor);
			break;
		}
}



/***************************************************************************
    MEMORY VIEW
***************************************************************************/

/*-------------------------------------------------
    memory_view_enumerate_subviews - enumerate
    all possible subviews for a memory view
-------------------------------------------------*/

static const memory_subview_item *memory_view_enumerate_subviews(running_machine *machine)
{
	memory_subview_item *head = NULL;
	memory_subview_item **tailptr = &head;
	int curindex = 0;

	/* first add all the device's address spaces */
	for (running_device *device = machine->devicelist.first(); device != NULL; device = device->next)
		for (int spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			const address_space *space = device->space(spacenum);
			if (space != NULL)
			{
				memory_subview_item *subview = auto_alloc(machine, memory_subview_item);

				/* populate the subview */
				subview->next = NULL;
				subview->index = curindex++;
				if (device->type == CPU)
					subview->name.printf("CPU '%s' (%s) %s memory", device->tag(), device->name(), space->name);
				else
					subview->name.printf("%s '%s' space #%d memory", device->name(), device->tag(), spacenum);
				subview->space = space;
				subview->endianness = space->endianness;
				subview->prefsize = space->dbits / 8;

				/* add to the list */
				*tailptr = subview;
				tailptr = &subview->next;
			}
		}

	/* then add all the memory regions */
	for (const region_info *region = machine->regionlist.first(); region != NULL; region = region->next)
	{
		memory_subview_item *subview = auto_alloc(machine, memory_subview_item);

		/* populate the subview */
		subview->next = NULL;
		subview->index = curindex++;
		subview->name.printf("Region '%s'", region->name.cstr());
		subview->base = *region;
		subview->length = region->bytes();
		subview->offsetxor = NATIVE_ENDIAN_VALUE_LE_BE(region->width() - 1, 0);
		subview->endianness = region->endianness();
		subview->prefsize = MIN(region->width(), 8);

		/* add to the list */
		*tailptr = subview;
		tailptr = &subview->next;
	}

	/* finally add all global array symbols */
	for (int itemnum = 0; itemnum < 10000; itemnum++)
	{
		/* stop when we run out of items */
		UINT32 valsize, valcount;
		void *base;
		const char *name = state_save_get_indexed_item(machine, itemnum, &base, &valsize, &valcount);
		if (name == NULL)
			break;

		/* if this is a single-entry global, add it */
		if (valcount > 1 && strstr(name, "globals/"))
		{
			memory_subview_item *subview = auto_alloc(machine, memory_subview_item);

			/* populate the subview */
			subview->next = NULL;
			subview->index = curindex++;
			subview->name.cpy(strrchr(name, '/') + 1);
			subview->base = base;
			subview->length = valcount * valsize;
			subview->prefsize = MIN(valsize, 8);

			/* add to the list */
			*tailptr = subview;
			tailptr = &subview->next;
		}
	}

	return head;
}


/*-------------------------------------------------
    memory_view_alloc - allocate memory for the
    memory view
-------------------------------------------------*/

static int memory_view_alloc(debug_view *view)
{
	const memory_subview_item *subviews = memory_view_enumerate_subviews(view->machine);
	debug_view_memory *memdata;

	/* if no subviews, fail */
	if (subviews == NULL)
		return FALSE;

	/* allocate memory */
	memdata = auto_alloc_clear(view->machine, debug_view_memory);
	memdata->subviewlist = subviews;

	/* allocate the expression data */
	debug_view_expression_alloc(&memdata->expression);

	/* stash the extra data pointer */
	view->extra_data = memdata;

	/* we support cursors */
	view->supports_cursor = TRUE;

	/* default to the first subview */
	memdata->desc = memdata->subviewlist;

	/* start out with 16 bytes in a single column and ASCII displayed */
	memdata->bytes_per_chunk = memdata->desc->prefsize;
	memdata->chunks_per_row = 16 / memdata->desc->prefsize;
	memdata->bytes_per_row = memdata->bytes_per_chunk * memdata->chunks_per_row;
	memdata->ascii_view = TRUE;

	return TRUE;
}


/*-------------------------------------------------
    memory_view_free - free memory for the
    memory view
-------------------------------------------------*/

static void memory_view_free(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	/* free any memory we allocated */
	if (memdata != NULL)
	{
		while (memdata->subviewlist != NULL)
		{
			memory_subview_item *item = (memory_subview_item *)memdata->subviewlist;
			memdata->subviewlist = item->next;
			auto_free(view->machine, item);
		}
		debug_view_expression_free(&memdata->expression);
		auto_free(view->machine, memdata);
	}
	view->extra_data = NULL;
}


/*-------------------------------------------------
    memory_view_notify - handle notification of
    updates to cursor changes
-------------------------------------------------*/

static void memory_view_notify(debug_view *view, view_notification type)
{
	if (type == VIEW_NOTIFY_CURSOR_CHANGED)
	{
		offs_t address;
		UINT8 shift;

		/* normalize the cursor */
		memory_view_get_cursor_pos(view, &address, &shift);
		memory_view_set_cursor_pos(view, address, shift);
	}
}


/*-------------------------------------------------
    memory_view_update - update the contents of
    the memory view
-------------------------------------------------*/

static void memory_view_update(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const address_space *space = memdata->desc->space;
	const memory_view_pos *posdata;
	UINT32 row;

	/* if we need to recompute, do it now */
	if (memory_view_needs_recompute(view))
		memory_view_recompute(view);

	/* get positional data */
	posdata = &memory_pos_table[memdata->bytes_per_chunk];

	/* loop over visible rows */
	for (row = 0; row < view->visible.y; row++)
	{
		debug_view_char *destmin = view->viewdata + row * view->visible.x;
		debug_view_char *destmax = destmin + view->visible.x;
		debug_view_char *destrow = destmin - view->topleft.x;
		UINT32 effrow = view->topleft.y + row;
		debug_view_char *dest;
		int ch, chunknum;

		/* reset the line of data; section 1 is normal, others are ancillary, cursor is selected */
		dest = destmin;
		for (ch = 0; ch < view->visible.x; ch++, dest++)
		{
			UINT32 effcol = view->topleft.x + ch;
			dest->byte = ' ';
			dest->attrib = DCA_ANCILLARY;
			if (in_section(effcol, &memdata->section[1]))
			{
				dest->attrib = DCA_NORMAL;
				if (view->cursor_visible && effrow == view->cursor.y && effcol == view->cursor.x)
					dest->attrib |= DCA_SELECTED;
			}
		}

		/* if this visible row is valid, add it to the buffer */
		if (effrow < view->total.y)
		{
			offs_t addrbyte = memdata->byte_offset + effrow * memdata->bytes_per_row;
			offs_t address = (space != NULL) ? memory_byte_to_address(space, addrbyte) : addrbyte;
			char addrtext[20];

			/* generate the address */
			sprintf(addrtext, memdata->addrformat, address);
			dest = destrow + memdata->section[0].pos + 1;
			for (ch = 0; addrtext[ch] != 0 && ch < memdata->section[0].width - 1; ch++, dest++)
				if (dest >= destmin && dest < destmax)
					dest->byte = addrtext[ch];

			/* generate the data */
			for (chunknum = 0; chunknum < memdata->chunks_per_row; chunknum++)
			{
				int chunkindex = memdata->reverse_view ? (memdata->chunks_per_row - 1 - chunknum) : chunknum;
				UINT64 chunkdata;
				int ismapped;

				ismapped = memory_view_read(memdata, memdata->bytes_per_chunk, addrbyte + chunknum * memdata->bytes_per_chunk, &chunkdata);
				dest = destrow + memdata->section[1].pos + 1 + chunkindex * posdata->spacing;
				for (ch = 0; ch < posdata->spacing; ch++, dest++)
					if (dest >= destmin && dest < destmax)
					{
						UINT8 shift = posdata->shift[ch];
						if (shift < 64)
							dest->byte = ismapped ? "0123456789ABCDEF"[(chunkdata >> shift) & 0x0f] : '*';
					}
			}

			/* generate the ASCII data */
			if (memdata->section[2].width > 0)
			{
				dest = destrow + memdata->section[2].pos + 1;
				for (ch = 0; ch < memdata->bytes_per_row; ch++, dest++)
					if (dest >= destmin && dest < destmax)
					{
						int ismapped;
						UINT64 chval;

						ismapped = memory_view_read(memdata, 1, addrbyte + ch, &chval);
						dest->byte = (ismapped && isprint(chval)) ? chval : '.';
					}
			}
		}
	}
}


/*-------------------------------------------------
    memory_view_char - handle a character typed
    within the current view
-------------------------------------------------*/

static void memory_view_char(debug_view *view, int chval)
{
	static const char hexvals[] = "0123456789abcdef";
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	offs_t address;
	char *hexchar;
	int ismapped;
	UINT64 data;
	UINT32 delta;
	UINT8 shift;

	/* get the position */
	memory_view_get_cursor_pos(view, &address, &shift);

	/* handle the incoming key */
	switch (chval)
	{
		case DCH_UP:
			if (address >= memdata->byte_offset + memdata->bytes_per_row)
				address -= memdata->bytes_per_row;
			break;

		case DCH_DOWN:
			if (address <= memdata->maxaddr - memdata->bytes_per_row)
				address += memdata->bytes_per_row;
			break;

		case DCH_PUP:
			for (delta = (view->visible.y - 2) * memdata->bytes_per_row; delta > 0; delta -= memdata->bytes_per_row)
				if (address >= memdata->byte_offset + delta)
				{
					address -= delta;
					break;
				}
			break;

		case DCH_PDOWN:
			for (delta = (view->visible.y - 2) * memdata->bytes_per_row; delta > 0; delta -= memdata->bytes_per_row)
				if (address <= memdata->maxaddr - delta)
				{
					address += delta;
					break;
				}
			break;

		case DCH_HOME:
			address -= address % memdata->bytes_per_row;
			shift = (memdata->bytes_per_chunk * 8) - 4;
			break;

		case DCH_CTRLHOME:
			address = memdata->byte_offset;
			shift = (memdata->bytes_per_chunk * 8) - 4;
			break;

		case DCH_END:
			address += (memdata->bytes_per_row - (address % memdata->bytes_per_row) - 1);
			shift = 0;
			break;

		case DCH_CTRLEND:
			address = memdata->maxaddr;
			shift = 0;
			break;

		case DCH_CTRLLEFT:
			if (address >= memdata->byte_offset + memdata->bytes_per_chunk)
				address -= memdata->bytes_per_chunk;
			break;

		case DCH_CTRLRIGHT:
			if (address <= memdata->maxaddr - memdata->bytes_per_chunk)
				address += memdata->bytes_per_chunk;
			break;

		default:
			hexchar = (char *)strchr(hexvals, tolower(chval));
			if (hexchar == NULL)
				break;
			ismapped = memory_view_read(memdata, memdata->bytes_per_chunk, address, &data);
			if (!ismapped)
				break;
			data &= ~((UINT64)0x0f << shift);
			data |= (UINT64)(hexchar - hexvals) << shift;
			memory_view_write(memdata, memdata->bytes_per_chunk, address, data);
			/* fall through... */

		case DCH_RIGHT:
			if (shift == 0 && address != memdata->maxaddr)
			{
				shift = memdata->bytes_per_chunk * 8 - 4;
				address += memdata->bytes_per_chunk;
			}
			else
				shift -= 4;
			break;

		case DCH_LEFT:
			if (shift == memdata->bytes_per_chunk * 8 - 4 && address != memdata->byte_offset)
			{
				shift = 0;
				address -= memdata->bytes_per_chunk;
			}
			else
				shift += 4;
			break;
	}

	/* set a new position */
	debug_view_begin_update(view);
	memory_view_set_cursor_pos(view, address, shift);
	view->update_pending = TRUE;
	debug_view_end_update(view);
}


/*-------------------------------------------------
    memory_view_recompute - recompute the internal
    data and structure of the memory view
-------------------------------------------------*/

static void memory_view_recompute(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const address_space *space = memdata->desc->space;
	offs_t cursoraddr;
	UINT8 cursorshift;
	int addrchars;

	/* get the current cursor position */
	memory_view_get_cursor_pos(view, &cursoraddr, &cursorshift);

	/* determine the maximum address and address format string from the raw information */
	if (space != NULL)
	{
		memdata->maxaddr = memdata->no_translation ? space->bytemask : space->logbytemask;
		addrchars = memdata->no_translation ? space->addrchars : space->logaddrchars;
	}
	else
	{
		memdata->maxaddr = memdata->desc->length - 1;
		addrchars = sprintf(memdata->addrformat, "%X", memdata->maxaddr);
	}

	/* generate an 8-byte aligned format for the address */
	if (!memdata->reverse_view)
		sprintf(memdata->addrformat, "%*s%%0%dX", 8 - addrchars, "", addrchars);
	else
		sprintf(memdata->addrformat, "%%0%dX%*s", addrchars, 8 - addrchars, "");

	/* if we are viewing a space with a minimum chunk size, clamp the bytes per chunk */
	if (space != NULL && space->ashift < 0)
	{
		UINT32 min_bytes_per_chunk = 1 << -space->ashift;
		while (memdata->bytes_per_chunk < min_bytes_per_chunk)
		{
			memdata->bytes_per_chunk *= 2;
			memdata->chunks_per_row /= 2;
		}
		memdata->chunks_per_row = MAX(1, memdata->chunks_per_row);
	}

	/* recompute the byte offset based on the most recent expression result */
	memdata->bytes_per_row = memdata->bytes_per_chunk * memdata->chunks_per_row;
	memdata->byte_offset = memdata->expression.result % memdata->bytes_per_row;

	/* compute the section widths */
	memdata->section[0].width = 1 + 8 + 1;
	memdata->section[1].width = 1 + 3 * memdata->bytes_per_row + 1;
	memdata->section[2].width = memdata->ascii_view ? (1 + memdata->bytes_per_row + 1) : 0;

	/* compute the section positions */
	if (!memdata->reverse_view)
	{
		memdata->section[0].pos = 0;
		memdata->section[1].pos = memdata->section[0].pos + memdata->section[0].width;
		memdata->section[2].pos = memdata->section[1].pos + memdata->section[1].width;
		view->total.x = memdata->section[2].pos + memdata->section[2].width;
	}
	else
	{
		memdata->section[2].pos = 0;
		memdata->section[1].pos = memdata->section[2].pos + memdata->section[2].width;
		memdata->section[0].pos = memdata->section[1].pos + memdata->section[1].width;
		view->total.x = memdata->section[0].pos + memdata->section[0].width;
	}

	/* derive total sizes from that */
	view->total.y = ((UINT64)memdata->maxaddr - (UINT64)memdata->byte_offset + (UINT64)memdata->bytes_per_row - 1) / memdata->bytes_per_row;

	/* reset the current cursor position */
	memory_view_set_cursor_pos(view, cursoraddr, cursorshift);
}


/*-------------------------------------------------
    memory_view_needs_recompute - determine if
    anything has changed that requires a
    recomputation
-------------------------------------------------*/

static int memory_view_needs_recompute(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const address_space *space = memdata->desc->space;
	int recompute = view->recompute;

	/* handle expression changes */
	if (debug_view_expression_changed_value(view, &memdata->expression, (space != NULL && space->cpu != NULL && space->cpu->type == CPU) ? space->cpu : NULL))
	{
		recompute = TRUE;
		view->topleft.y = (memdata->expression.result - memdata->byte_offset) / memdata->bytes_per_row;
		view->topleft.y = MAX(view->topleft.y, 0);
		view->topleft.y = MIN(view->topleft.y, view->total.y - 1);
		memory_view_set_cursor_pos(view, memdata->expression.result, memdata->bytes_per_chunk * 8 - 4);
	}

	/* expression is clean at this point, and future recomputation is not necessary */
	view->recompute = FALSE;
	return recompute;
}


/*-------------------------------------------------
    memory_view_get_cursor_pos - return the cursor
    position as an address and a shift value
-------------------------------------------------*/

static void memory_view_get_cursor_pos(debug_view *view, offs_t *address, UINT8 *shift)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const memory_view_pos *posdata = &memory_pos_table[memdata->bytes_per_chunk];
	int xposition, chunknum, chunkoffs;

	/* start with the base address for this row */
	*address = memdata->byte_offset + view->cursor.y * memdata->bytes_per_chunk * memdata->chunks_per_row;

	/* determine the X position within the middle section, clamping as necessary */
	xposition = view->cursor.x - memdata->section[1].pos - 1;
	if (xposition < 0)
		xposition = 0;
	else if (xposition >= posdata->spacing * memdata->chunks_per_row)
		xposition = posdata->spacing * memdata->chunks_per_row - 1;

	/* compute chunk number and offset within that chunk */
	chunknum = xposition / posdata->spacing;
	chunkoffs = xposition % posdata->spacing;

	/* reverse the chunknum if we're reversed */
	if (memdata->reverse_view)
		chunknum = memdata->chunks_per_row - 1 - chunknum;

	/* compute the address and shift */
	*address += chunknum * memdata->bytes_per_chunk;
	*shift = posdata->shift[chunkoffs] & 0x7f;
}


/*-------------------------------------------------
    memory_view_set_cursor_pos - set the cursor
    position as a function of an address and a
    shift value
-------------------------------------------------*/

static void memory_view_set_cursor_pos(debug_view *view, offs_t address, UINT8 shift)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const memory_view_pos *posdata = &memory_pos_table[memdata->bytes_per_chunk];
	int chunknum;

	/* offset the address by the byte offset */
	if (address < memdata->byte_offset)
		address = memdata->byte_offset;
	address -= memdata->byte_offset;

	/* compute the Y coordinate and chunk index */
	view->cursor.y = address / memdata->bytes_per_row;
	chunknum = (address % memdata->bytes_per_row) / memdata->bytes_per_chunk;

	/* reverse the chunknum if we're reversed */
	if (memdata->reverse_view)
		chunknum = memdata->chunks_per_row - 1 - chunknum;

	/* scan within the chunk to find the shift */
	for (view->cursor.x = 0; view->cursor.x < posdata->spacing; view->cursor.x++)
		if (posdata->shift[view->cursor.x] == shift)
			break;

	/* add in the chunk offset and shift to the right of divider1 */
	view->cursor.x += memdata->section[1].pos + 1 + posdata->spacing * chunknum;

	/* clamp to the window bounds */
	view->cursor.x = MIN(view->cursor.x, view->total.x);
	view->cursor.y = MIN(view->cursor.y, view->total.y);

	/* scroll if out of range */
	adjust_visible_x_for_cursor(view);
	adjust_visible_y_for_cursor(view);
}


/*-------------------------------------------------
    memory_view_read - generic memory view data
    reader
-------------------------------------------------*/

static int memory_view_read(debug_view_memory *memdata, UINT8 size, offs_t offs, UINT64 *data)
{
	/* if no raw data, just use the standard debug routines */
	if (memdata->desc->space != NULL)
	{
		const address_space *space = memdata->desc->space;
		offs_t dummyaddr = offs;
		int ismapped;

		ismapped = memdata->no_translation ? TRUE : debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &dummyaddr);
		*data = ~(UINT64)0;
		if (ismapped)
		{
			switch (size)
			{
				case 1:	*data = debug_read_byte(space, offs, !memdata->no_translation); break;
				case 2:	*data = debug_read_word(space, offs, !memdata->no_translation); break;
				case 4:	*data = debug_read_dword(space, offs, !memdata->no_translation); break;
				case 8:	*data = debug_read_qword(space, offs, !memdata->no_translation); break;
			}
		}
		return ismapped;
	}

	/* if larger than a byte, reduce by half and recurse */
	if (size > 1)
	{
		UINT64 data0, data1;
		int ismapped;

		size /= 2;
		ismapped  = memory_view_read(memdata, size, offs + 0 * size, &data0);
		ismapped |= memory_view_read(memdata, size, offs + 1 * size, &data1);
		if (memdata->desc->endianness == ENDIANNESS_LITTLE)
			*data = data0 | (data1 << (size * 8));
		else
			*data = data1 | (data0 << (size * 8));
		return ismapped;
	}

	/* all 0xff if out of bounds */
	offs ^= memdata->desc->offsetxor;
	if (offs >= memdata->desc->length)
		return FALSE;
	*data = *((UINT8 *)memdata->desc->base + offs);
	return TRUE;
}


/*-------------------------------------------------
    memory_view_write - generic memory view data
    writer
-------------------------------------------------*/

static void memory_view_write(debug_view_memory *memdata, UINT8 size, offs_t offs, UINT64 data)
{
	/* if no raw data, just use the standard debug routines */
	if (memdata->desc->space != NULL)
	{
		const address_space *space = memdata->desc->space;

		switch (size)
		{
			case 1:	debug_write_byte(space, offs, data, !memdata->no_translation); break;
			case 2:	debug_write_word(space, offs, data, !memdata->no_translation); break;
			case 4:	debug_write_dword(space, offs, data, !memdata->no_translation); break;
			case 8:	debug_write_qword(space, offs, data, !memdata->no_translation); break;
		}
		return;
	}

	/* if larger than a byte, reduce by half and recurse */
	if (size > 1)
	{
		size /= 2;
		if (memdata->desc->endianness == ENDIANNESS_LITTLE)
		{
			memory_view_write(memdata, size, offs + 0 * size, data);
			memory_view_write(memdata, size, offs + 1 * size, data >> (8 * size));
		}
		else
		{
			memory_view_write(memdata, size, offs + 1 * size, data);
			memory_view_write(memdata, size, offs + 0 * size, data >> (8 * size));
		}
		return;
	}

	/* ignore if out of bounds */
	offs ^= memdata->desc->offsetxor;
	if (offs >= memdata->desc->length)
		return;
	*((UINT8 *)memdata->desc->base + offs) = data;

/* hack for FD1094 editing */
#ifdef FD1094_HACK
	if (memdata->desc->base == memory_region(view->machine, "user2"))
	{
		extern void fd1094_regenerate_key(running_machine *machine);
		fd1094_regenerate_key(view->machine);
	}
#endif
}


/*-------------------------------------------------
    memory_view_get_subview_list - return a linked
    list of subviews
-------------------------------------------------*/

const memory_subview_item *memory_view_get_subview_list(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	return memdata->subviewlist;
}


/*-------------------------------------------------
    memory_view_get_subview - return the current
    subview index
-------------------------------------------------*/

int memory_view_get_subview(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);

	return memdata->desc->index;
}


/*-------------------------------------------------
    memory_view_get_expression - return the
    expression string describing the home address
-------------------------------------------------*/

const char *memory_view_get_expression(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->expression.string;
}


/*-------------------------------------------------
    memory_view_get_bytes_per_chunk - return the
    currently displayed bytes per chunk
-------------------------------------------------*/

UINT8 memory_view_get_bytes_per_chunk(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->bytes_per_chunk;
}


/*-------------------------------------------------
    memory_view_get_chunks_per_row - return the
    number of chunks displayed across a row
-------------------------------------------------*/

UINT32 memory_view_get_chunks_per_row(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->chunks_per_row;
}


/*-------------------------------------------------
    memory_view_get_reverse - return TRUE if the
    memory view is displayed reverse
-------------------------------------------------*/

UINT8 memory_view_get_reverse(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->reverse_view;
}


/*-------------------------------------------------
    memory_view_get_ascii - return TRUE if the
    memory view is displaying an ASCII
    representation
-------------------------------------------------*/

UINT8 memory_view_get_ascii(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->ascii_view;
}


/*-------------------------------------------------
    memory_view_get_physical - return TRUE if the
    memory view is displaying physical addresses
    versus logical addresses
-------------------------------------------------*/

UINT8 memory_view_get_physical(debug_view *view)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	assert(view->type == DVT_MEMORY);
	debug_view_begin_update(view);
	debug_view_end_update(view);
	return memdata->no_translation;
}


/*-------------------------------------------------
    memory_view_set_subview - select a new subview
    by index
-------------------------------------------------*/

void memory_view_set_subview(debug_view *view, int index)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;
	const memory_subview_item *subview;

	assert(view->type == DVT_MEMORY);

	/* pick the requested view */
	subview = memory_view_get_subview_by_index(memdata->subviewlist, index);
	if (subview == NULL)
		return;

	/* handle a change */
	if (subview != memdata->desc)
	{
		debug_view_begin_update(view);
		memdata->desc = subview;
		memdata->chunks_per_row = memdata->bytes_per_chunk * memdata->chunks_per_row / memdata->desc->prefsize;
		memdata->bytes_per_chunk = memdata->desc->prefsize;

		/* we need to recompute the expression in the context of the new space */
		memdata->expression.dirty = TRUE;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    memory_view_set_expression - set the
    expression string describing the home address
-------------------------------------------------*/

void memory_view_set_expression(debug_view *view, const char *expression)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);
	assert(expression != NULL);

	debug_view_begin_update(view);
	debug_view_expression_set(&memdata->expression, expression);
	view->recompute = view->update_pending = TRUE;
	debug_view_end_update(view);
}


/*-------------------------------------------------
    memory_view_set_bytes_per_chunk - specify the
    number of bytes displayed per chunk
-------------------------------------------------*/

void memory_view_set_bytes_per_chunk(debug_view *view, UINT8 chunkbytes)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);
	assert(chunkbytes < ARRAY_LENGTH(memory_pos_table) && memory_pos_table[chunkbytes].spacing != 0);

	if (chunkbytes != memdata->bytes_per_chunk)
	{
		int endianness = memdata->desc->endianness;
		offs_t address;
		UINT8 shift;

		debug_view_begin_update(view);
		memory_view_get_cursor_pos(view, &address, &shift);
		address += (shift / 8) ^ ((endianness == ENDIANNESS_LITTLE) ? 0 : (memdata->bytes_per_chunk - 1));
		shift %= 8;

		memdata->bytes_per_chunk = chunkbytes;
		memdata->chunks_per_row = memdata->bytes_per_row / chunkbytes;
		view->recompute = view->update_pending = TRUE;

		shift += 8 * ((address % memdata->bytes_per_chunk) ^ ((endianness == ENDIANNESS_LITTLE) ? 0 : (memdata->bytes_per_chunk - 1)));
		address -= address % memdata->bytes_per_chunk;
		memory_view_set_cursor_pos(view, address, shift);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    memory_view_set_chunks_per_row - specify the
    number of chunks displayed across a row
-------------------------------------------------*/

void memory_view_set_chunks_per_row(debug_view *view, UINT32 rowchunks)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);

	if (rowchunks < 1)
		return;

	if (rowchunks != memdata->chunks_per_row)
	{
		offs_t address;
		UINT8 shift;

		debug_view_begin_update(view);
		memory_view_get_cursor_pos(view, &address, &shift);
		memdata->chunks_per_row = rowchunks;
		view->recompute = view->update_pending = TRUE;
		memory_view_set_cursor_pos(view, address, shift);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    memory_view_set_reverse - specify TRUE if the
    memory view is displayed reverse
-------------------------------------------------*/

void memory_view_set_reverse(debug_view *view, UINT8 reverse)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);

	if (reverse != memdata->reverse_view)
	{
		offs_t address;
		UINT8 shift;

		debug_view_begin_update(view);
		memory_view_get_cursor_pos(view, &address, &shift);
		memdata->reverse_view = reverse;
		view->recompute = view->update_pending = TRUE;
		memory_view_set_cursor_pos(view, address, shift);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    memory_view_set_ascii - specify TRUE if the
    memory view should display an ASCII
    representation
-------------------------------------------------*/

void memory_view_set_ascii(debug_view *view, UINT8 ascii)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);

	if (ascii != memdata->ascii_view)
	{
		offs_t address;
		UINT8 shift;

		debug_view_begin_update(view);
		memory_view_get_cursor_pos(view, &address, &shift);
		memdata->ascii_view = ascii;
		view->recompute = view->update_pending = TRUE;
		memory_view_set_cursor_pos(view, address, shift);
		debug_view_end_update(view);
	}
}


/*-------------------------------------------------
    memory_view_set_physical - specify TRUE if the
    memory view should display physical addresses
    versus logical addresses
-------------------------------------------------*/

void memory_view_set_physical(debug_view *view, UINT8 physical)
{
	debug_view_memory *memdata = (debug_view_memory *)view->extra_data;

	assert(view->type == DVT_MEMORY);

	if (physical != memdata->no_translation)
	{
		debug_view_begin_update(view);
		memdata->no_translation = physical;
		view->recompute = view->update_pending = TRUE;
		debug_view_end_update(view);
	}
}
