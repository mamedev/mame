/***************************************************************************

    menu.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MENU_H__
#define __UI_MENU_H__

#include "render.h"
#include "ui/stackable.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* flags for menu items */
#define MENU_FLAG_LEFT_ARROW        (1 << 0)
#define MENU_FLAG_RIGHT_ARROW       (1 << 1)
#define MENU_FLAG_INVERT            (1 << 2)
#define MENU_FLAG_MULTILINE         (1 << 3)
#define MENU_FLAG_REDTEXT           (1 << 4)
#define MENU_FLAG_DISABLE           (1 << 5)

/* special menu item for separators */
#define MENU_SEPARATOR_ITEM         "---"

/* flags to pass to ui_menu_process */
#define UI_MENU_PROCESS_NOKEYS      1
#define UI_MENU_PROCESS_LR_REPEAT   2
#define UI_MENU_PROCESS_CUSTOM_ONLY 4

/* options for ui_menu_reset */
enum ui_menu_reset_options
{
	UI_MENU_RESET_SELECT_FIRST,
	UI_MENU_RESET_REMEMBER_POSITION,
	UI_MENU_RESET_REMEMBER_REF
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* menu-related events */
struct ui_menu_event
{
	void *          itemref;            /* reference for the selected item */
	int             iptkey;             /* one of the IPT_* values from inptport.h */
	unicode_char    unichar;            /* unicode character if iptkey == IPT_SPECIAL */
};

struct ui_menu_pool
{
	ui_menu_pool *      next;           /* chain to next one */
	UINT8 *             top;            /* top of the pool */
	UINT8 *             end;            /* end of the pool */
};


class ui_menu_item
{
public:
	const char *        text;
	const char *        subtext;
	UINT32              flags;
	void *              ref;

	inline bool is_selectable() const;
};

class ui_menu : public ui_stackable
{
public:
	ui_menu(running_machine &machine, render_container *container);
	virtual ~ui_menu();

	ui_menu_event       menu_event;         /* the UI menu_event that occurred */
	int                 resetpos;           /* reset position */
	void *              resetref;           /* reset reference */
	int                 selected;           /* which item is selected */
	int                 hover;              /* which item is being hovered over */
	int                 visitems;           /* number of visible items */
	int                 numitems;           /* number of items in the menu */
	int                 allocitems;         /* allocated size of array */
	ui_menu_item *      item;               /* pointer to array of items */
	float               customtop;          /* amount of extra height to add at the top */
	float               custombottom;       /* amount of extra height to add at the bottom */
	ui_menu_pool *      pool;               /* list of memory pools */

	/* free all items in the menu, and all memory allocated from the memory pool */
	virtual void reset();
	void reset(ui_menu_reset_options options);

	virtual void do_handle();

	/* returns true if the menu has any non-default items in it */
	bool populated();

	/* append a new item to the end of the menu */
	void item_append(const char *text, const char *subtext, UINT32 flags, void *ref);

	/* process a menu, drawing it and returning any interesting events */
	const ui_menu_event *process(UINT32 flags);

	/* configure the menu for custom rendering */
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	/* allocate temporary memory from the menu's memory pool */
	void *m_pool_alloc(size_t size);

	/* make a temporary string copy in the menu's memory pool */
	const char *pool_strdup(const char *string);

	/* retrieves the index of the currently selected menu item */
	void *get_selection();

	/* changes the index of the currently selected menu item */
	void set_selection(void *selected_itemref);

	/* Global initialization */
	static void init(running_machine &machine);
	static void exit(running_machine &machine);

	/* master handler */
	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

	/* Used by sliders */
	void validate_selection(int scandir);

	/* To be reimplemented in the menu subclass */
	virtual void populate() = 0;

	/* To be reimplemented in the menu subclass */
	virtual void handle() = 0;

private:
	static bitmap_rgb32 *hilight_bitmap;
	static render_texture *hilight_texture, *arrow_texture;

	int					top_line;

	void draw(bool customonly);
	void draw_text_box_menu();
	void handle_events();
	void handle_keys(UINT32 flags);
	void clear_free_list();

	inline bool exclusive_input_pressed(int key, int repeat);
	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);
};

#endif  /* __UI_MENU_H__ */
