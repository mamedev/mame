/***************************************************************************

    uimain.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UIMAIN_H__
#define __UIMAIN_H__

#include "crsshair.h"
#include "drivenum.h"

class ui_menu_main : public ui_menu {
public:
	ui_menu_main(running_machine &machine, render_container *container);
	virtual ~ui_menu_main();
	virtual void populate();
	virtual void handle();

private:
	enum {
		INPUT_GROUPS,
		INPUT_SPECIFIC,
		SETTINGS_DIP_SWITCHES,
		SETTINGS_DRIVER_CONFIG,
		ANALOG,
		BOOKKEEPING,
		GAME_INFO,
		IMAGE_MENU_IMAGE_INFO,
		IMAGE_MENU_FILE_MANAGER,
		MESS_MENU_TAPE_CONTROL,
		MESS_MENU_BITBANGER_CONTROL,
		SLOT_DEVICES,
		NETWORK_DEVICES,
		KEYBOARD_MODE,
		SLIDERS,
		VIDEO_TARGETS,
		VIDEO_OPTIONS,
		CROSSHAIR,
		CHEAT,
		MEMORY_CARD,
		SELECT_GAME,
		BIOS_SELECTION,
	};
};

class ui_menu_keyboard_mode : public ui_menu {
public:
	ui_menu_keyboard_mode(running_machine &machine, render_container *container);
	virtual ~ui_menu_keyboard_mode();
	virtual void populate();
	virtual void handle();
};

class ui_menu_slot_devices : public ui_menu {
public:
	ui_menu_slot_devices(running_machine &machine, render_container *container);
	virtual ~ui_menu_slot_devices();
	virtual void populate();
	virtual void handle();

private:
	int slot_get_current_index(device_slot_interface *slot);
	int slot_get_length(device_slot_interface *slot);
	const char *slot_get_next(device_slot_interface *slot);
	const char *slot_get_prev(device_slot_interface *slot);
	const char *get_slot_device(device_slot_interface *slot);
	void set_slot_device(device_slot_interface *slot, const char *val);
};

class ui_menu_network_devices : public ui_menu {
public:
	ui_menu_network_devices(running_machine &machine, render_container *container);
	virtual ~ui_menu_network_devices();
	virtual void populate();
	virtual void handle();
};

class ui_menu_input_groups : public ui_menu {
public:
	ui_menu_input_groups(running_machine &machine, render_container *container);
	virtual ~ui_menu_input_groups();
	virtual void populate();
	virtual void handle();
};

class ui_menu_input : public ui_menu {
public:
	ui_menu_input(running_machine &machine, render_container *container);
	virtual ~ui_menu_input();
	virtual void handle();

protected:
	enum {
		INPUT_TYPE_DIGITAL = 0,
		INPUT_TYPE_ANALOG = 1,
		INPUT_TYPE_ANALOG_DEC = INPUT_TYPE_ANALOG + SEQ_TYPE_DECREMENT,
		INPUT_TYPE_ANALOG_INC = INPUT_TYPE_ANALOG + SEQ_TYPE_INCREMENT,
		INPUT_TYPE_TOTAL = INPUT_TYPE_ANALOG + SEQ_TYPE_TOTAL
	};

	/* internal input menu item data */
	struct input_item_data {
		input_item_data *	next;				/* pointer to next item in the list */
		const void *		ref;				/* reference to type description for global inputs or field for game inputs */
		input_seq_type		seqtype;			/* sequence type */
		input_seq			seq;				/* copy of the live sequence */
		const input_seq *	defseq;				/* pointer to the default sequence */
		const char *		name;				/* pointer to the base name of the item */
		UINT16				sortorder;			/* sorting information */
		UINT8				type;				/* type of port */
	};

	void populate_and_sort(struct input_item_data *itemlist);
	virtual void update_input(struct input_item_data *seqchangeditem) = 0;
	void toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq);

protected:
	const void *		pollingref;
	input_seq_type		pollingseq;
	input_item_data *	pollingitem;

private:
	UINT16				last_sortorder;
	bool				record_next;
	input_seq			starting_seq;

	static int compare_items(const void *i1, const void *i2);
};

class ui_menu_input_general : public ui_menu_input {
public:
	ui_menu_input_general(running_machine &machine, render_container *container, int group);
	virtual ~ui_menu_input_general();
	virtual void populate();

protected:
	int group;
	virtual void update_input(struct input_item_data *seqchangeditem);
};

class ui_menu_input_specific : public ui_menu_input {
public:
	ui_menu_input_specific(running_machine &machine, render_container *container);
	virtual ~ui_menu_input_specific();
	virtual void populate();

protected:
	virtual void update_input(struct input_item_data *seqchangeditem);
};

class ui_menu_settings : public ui_menu {
public:
	ui_menu_settings(running_machine &machine, render_container *container, UINT32 type);
	virtual ~ui_menu_settings();
	virtual void populate();
	virtual void handle();

protected:
	/* DIP switch descriptor */
	struct dip_descriptor {
		dip_descriptor *	next;
		const char *		name;
		UINT32				mask;
		UINT32				state;
	};

	dip_descriptor *	diplist;
	int dipcount;
	int type;
};

class ui_menu_settings_dip_switches : public ui_menu_settings {
public:
	ui_menu_settings_dip_switches(running_machine &machine, render_container *container);
	virtual ~ui_menu_settings_dip_switches();

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);
private:
	void custom_render_one(float x1, float y1, float x2, float y2, const dip_descriptor *dip, UINT32 selectedmask);
};

class ui_menu_settings_driver_config : public ui_menu_settings {
public:
	ui_menu_settings_driver_config(running_machine &machine, render_container *container);
	virtual ~ui_menu_settings_driver_config();
};

class ui_menu_analog : public ui_menu {
public:
	ui_menu_analog(running_machine &machine, render_container *container);
	virtual ~ui_menu_analog();
	virtual void populate();
	virtual void handle();

private:
	enum {
		ANALOG_ITEM_KEYSPEED = 0,
		ANALOG_ITEM_CENTERSPEED,
		ANALOG_ITEM_REVERSE,
		ANALOG_ITEM_SENSITIVITY,
		ANALOG_ITEM_COUNT
	};

	/* internal analog menu item data */
	struct analog_item_data {
		ioport_field *field;
		int					type;
		int					min, max;
		int					cur;
		int 				defvalue;
	};
};

class ui_menu_bookkeeping : public ui_menu {
public:
	ui_menu_bookkeeping(running_machine &machine, render_container *container);
	virtual ~ui_menu_bookkeeping();
	virtual void populate();
	virtual void handle();

private:
	attotime prevtime;
};

class ui_menu_game_info : public ui_menu {
public:
	ui_menu_game_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_info();
	virtual void populate();
	virtual void handle();
};

class ui_menu_cheat : public ui_menu {
public:
	ui_menu_cheat(running_machine &machine, render_container *container);
	virtual ~ui_menu_cheat();
	virtual void populate();
	virtual void handle();
};

class ui_menu_memory_card : public ui_menu {
public:
	ui_menu_memory_card(running_machine &machine, render_container *container);
	virtual ~ui_menu_memory_card();
	virtual void populate();
	virtual void handle();

private:
	enum {
		MEMCARD_ITEM_SELECT = 1,
		MEMCARD_ITEM_LOAD,
		MEMCARD_ITEM_EJECT,
		MEMCARD_ITEM_CREATE
	};

	int cardnum;
};

class ui_menu_sliders : public ui_menu {
public:
	ui_menu_sliders(running_machine &machine, render_container *container, bool menuless_mode);
	virtual ~ui_menu_sliders();
	virtual void populate();
	virtual void handle();

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

private:
	bool menuless_mode, hidden;
};

class ui_menu_video_targets : public ui_menu {
public:
	ui_menu_video_targets(running_machine &machine, render_container *container);
	virtual ~ui_menu_video_targets();
	virtual void populate();
	virtual void handle();
};

class ui_menu_video_options : public ui_menu {
public:
	ui_menu_video_options(running_machine &machine, render_container *container, render_target *target);
	virtual ~ui_menu_video_options();
	virtual void populate();
	virtual void handle();

private:
	enum {
		VIDEO_ITEM_ROTATE = 0x80000000,
		VIDEO_ITEM_BACKDROPS,
		VIDEO_ITEM_OVERLAYS,
		VIDEO_ITEM_BEZELS,
		VIDEO_ITEM_CPANELS,
		VIDEO_ITEM_MARQUEES,
		VIDEO_ITEM_ZOOM,
		VIDEO_ITEM_VIEW
	};

	render_target *target;
};

class ui_menu_crosshair : public ui_menu {
public:
	ui_menu_crosshair(running_machine &machine, render_container *container);
	virtual ~ui_menu_crosshair();
	virtual void populate();
	virtual void handle();

private:
	enum {
		CROSSHAIR_ITEM_VIS = 0,
		CROSSHAIR_ITEM_PIC,
		CROSSHAIR_ITEM_AUTO_TIME
	};

	/* internal crosshair menu item data */
	struct crosshair_item_data {
		UINT8				type;
		UINT8				player;
		UINT8				min, max;
		UINT8				cur;
		UINT8				defvalue;
		char				last_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
		char				next_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
	};
};

class ui_menu_quit_game : public ui_menu {
public:
	ui_menu_quit_game(running_machine &machine, render_container *container);
	virtual ~ui_menu_quit_game();
	virtual void populate();
	virtual void handle();
};

class ui_menu_select_game : public ui_menu {
public:
	ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename);
	virtual ~ui_menu_select_game();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum { VISIBLE_GAMES_IN_LIST = 15 };
	UINT8				error;
	UINT8				rerandomize;
	char				search[40];
	int					matchlist[VISIBLE_GAMES_IN_LIST];
	const game_driver	**driverlist;

	driver_enumerator *drivlist;

	void build_driver_list();
};

class ui_menu_bios_selection : public ui_menu {
public:
	ui_menu_bios_selection(running_machine &machine, render_container *container);
	virtual ~ui_menu_bios_selection();
	virtual void populate();
	virtual void handle();

private:
};
/* force game select menu */
void ui_menu_force_game_select(running_machine &machine, render_container *container);

#endif	/* __UIMAIN_H__ */
