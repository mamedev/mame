/***************************************************************************

    uimenu.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UIMENU_H__
#define __UIMENU_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* flags for menu items */
#define MENU_FLAG_LEFT_ARROW		(1 << 0)
#define MENU_FLAG_RIGHT_ARROW		(1 << 1)
#define MENU_FLAG_INVERT			(1 << 2)
#define MENU_FLAG_MULTILINE			(1 << 3)
#define MENU_FLAG_REDTEXT			(1 << 4)
#define MENU_FLAG_DISABLE			(1 << 5)

/* special menu item for separators */
#define MENU_SEPARATOR_ITEM			"---"

/* flags to pass to ui_menu_process */
#define UI_MENU_PROCESS_NOKEYS		1
#define UI_MENU_PROCESS_LR_REPEAT	2
#define UI_MENU_PROCESS_CUSTOM_ONLY	4

/* options for ui_menu_reset */
enum _ui_menu_reset_options
{
	UI_MENU_RESET_SELECT_FIRST,
	UI_MENU_RESET_REMEMBER_POSITION,
	UI_MENU_RESET_REMEMBER_REF
};
typedef enum _ui_menu_reset_options ui_menu_reset_options;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward declarations */
typedef struct _ui_menu ui_menu;


/* menu-related callback functions */
typedef void (*ui_menu_handler_func)(running_machine *machine, ui_menu *menu, void *parameter, void *state);
typedef void (*ui_menu_custom_func)(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x, float y, float x2, float y2);
typedef void (*ui_menu_destroy_state_func)(ui_menu *menu, void *state);


/* menu-related events */
typedef struct _ui_menu_event ui_menu_event;
struct _ui_menu_event
{
	void *			itemref;			/* reference for the selected item */
	int				iptkey;				/* one of the IPT_* values from inptport.h */
	unicode_char	unichar;			/* unicode character if iptkey == IPT_SPECIAL */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* initialization */
void ui_menu_init(running_machine *machine);



/* ----- core menu management ----- */

/* allocate a new menu */
ui_menu *ui_menu_alloc(running_machine *machine, ui_menu_handler_func handler, void *parameter);

/* free a menu */
void ui_menu_free(ui_menu *menu);

/* free all items in the menu, and all memory allocated from the memory pool */
void ui_menu_reset(ui_menu *menu, ui_menu_reset_options options);

/* returns TRUE if the menu has any non-default items in it */
int ui_menu_populated(ui_menu *menu);

/* append a new item to the end of the menu */
void ui_menu_item_append(ui_menu *menu, const char *text, const char *subtext, UINT32 flags, void *ref);

/* process a menu, drawing it and returning any interesting events */
const ui_menu_event *ui_menu_process(running_machine *machine, ui_menu *menu, UINT32 flags);

/* configure the menu for custom rendering */
void ui_menu_set_custom_render(ui_menu *menu, ui_menu_custom_func custom, float top, float bottom);

/* allocate permanent memory to represent the menu's state */
void *ui_menu_alloc_state(ui_menu *menu, size_t size, ui_menu_destroy_state_func destroy_state);

/* allocate temporary memory from the menu's memory pool */
void *ui_menu_pool_alloc(ui_menu *menu, size_t size);

/* make a temporary string copy in the menu's memory pool */
const char *ui_menu_pool_strdup(ui_menu *menu, const char *string);

/* retrieves the index of the currently selected menu item */
void *ui_menu_get_selection(ui_menu *menu);

/* changes the index of the currently selected menu item */
void ui_menu_set_selection(ui_menu *menu, void *selected_itemref);



/* ----- menu stack management ----- */

/* reset the menus, clearing everything */
void ui_menu_stack_reset(running_machine *machine);

/* push a new menu onto the stack */
void ui_menu_stack_push(ui_menu *menu);

/* pop a menu from the stack */
void ui_menu_stack_pop(running_machine *machine);



/* ----- UI system interaction ----- */

/* master handler */
UINT32 ui_menu_ui_handler(running_machine *machine, UINT32 state);

/* slider handler */
UINT32 ui_slider_ui_handler(running_machine *machine, UINT32 state);

/* force game select menu */
void ui_menu_force_game_select(running_machine *machine);
int ui_menu_is_force_game_select(void);


#endif	/* __UIMENU_H__ */
