// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/inputmap.h

    Internal menus for input mappings.

***************************************************************************/

#pragma once

#ifndef __UI_INPUTMAP_H__
#define __UI_INPUTMAP_H__

//#include "drivenum.h"

class ui_menu_input_groups : public ui_menu {
public:
	ui_menu_input_groups(running_machine &machine, render_container *container);
	virtual ~ui_menu_input_groups();
	virtual void populate() override;
	virtual void handle() override;
};

class ui_menu_input : public ui_menu {
public:
	ui_menu_input(running_machine &machine, render_container *container);
	virtual ~ui_menu_input();
	virtual void handle() override;

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
		input_item_data *   next;               /* pointer to next item in the list */
		const void *        ref;                /* reference to type description for global inputs or field for game inputs */
		input_seq_type      seqtype;            /* sequence type */
		input_seq           seq;                /* copy of the live sequence */
		const input_seq *   defseq;             /* pointer to the default sequence */
		const char *        name;               /* pointer to the base name of the item */
		const char *        owner_name;         /* pointer to the name of the owner of the item */
		UINT32              sortorder;          /* sorting information */
		UINT8               type;               /* type of port */
	};

	void populate_and_sort(struct input_item_data *itemlist);
	virtual void update_input(struct input_item_data *seqchangeditem) = 0;
	void toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq);

protected:
	const void *        pollingref;
	input_seq_type      pollingseq;
	input_item_data *   pollingitem;

private:
	UINT16              last_sortorder;
	bool                record_next;
	input_seq           starting_seq;

	static int compare_items(const void *i1, const void *i2);
};

class ui_menu_input_general : public ui_menu_input {
public:
	ui_menu_input_general(running_machine &machine, render_container *container, int group);
	virtual ~ui_menu_input_general();
	virtual void populate() override;

protected:
	int group;
	virtual void update_input(struct input_item_data *seqchangeditem) override;
};

class ui_menu_input_specific : public ui_menu_input {
public:
	ui_menu_input_specific(running_machine &machine, render_container *container);
	virtual ~ui_menu_input_specific();
	virtual void populate() override;

protected:
	virtual void update_input(struct input_item_data *seqchangeditem) override;
};

class ui_menu_settings : public ui_menu {
public:
	ui_menu_settings(running_machine &machine, render_container *container, UINT32 type);
	virtual ~ui_menu_settings();
	virtual void populate() override;
	virtual void handle() override;

protected:
	/* DIP switch descriptor */
	struct dip_descriptor {
		dip_descriptor *    next;
		const char *        name;
		UINT32              mask;
		UINT32              state;
	};

	dip_descriptor *    diplist;
	int dipcount;
	int type;
};

class ui_menu_settings_dip_switches : public ui_menu_settings {
public:
	ui_menu_settings_dip_switches(running_machine &machine, render_container *container);
	virtual ~ui_menu_settings_dip_switches();

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
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
	virtual void populate() override;
	virtual void handle() override;

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
		int                 type;
		int                 min, max;
		int                 cur;
		int                 defvalue;
	};
};

#endif  /* __UI_INPUTMAP_H__ */
