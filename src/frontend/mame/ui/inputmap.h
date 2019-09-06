// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/inputmap.h

    Internal menus for input mappings.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INPUTMAP_H
#define MAME_FRONTEND_UI_INPUTMAP_H

#pragma once

#include "ui/menu.h"


namespace ui {
class menu_input_groups : public menu
{
public:
	menu_input_groups(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_groups() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
};

class menu_input : public menu
{
public:
	menu_input(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input() override;

protected:
	enum {
		INPUT_TYPE_DIGITAL = 0,
		INPUT_TYPE_ANALOG = 1,
		INPUT_TYPE_ANALOG_DEC = INPUT_TYPE_ANALOG + SEQ_TYPE_DECREMENT,
		INPUT_TYPE_ANALOG_INC = INPUT_TYPE_ANALOG + SEQ_TYPE_INCREMENT,
		INPUT_TYPE_TOTAL = INPUT_TYPE_ANALOG + SEQ_TYPE_TOTAL
	};

	/* internal input menu item data */
	struct input_item_data
	{
		input_item_data *   next;               /* pointer to next item in the list */
		const void *        ref;                /* reference to type description for global inputs or field for game inputs */
		input_seq_type      seqtype;            /* sequence type */
		input_seq           seq;                /* copy of the live sequence */
		const input_seq *   defseq;             /* pointer to the default sequence */
		const char *        name;               /* pointer to the base name of the item */
		const char *        owner_name;         /* pointer to the name of the owner of the item */
		ioport_group        group;              /* group type */
		uint8_t               type;               /* type of port */
		bool                is_optional;        /* true if this input is considered optional */
	};

	void populate_sorted(std::vector<input_item_data *> &&itemarray);
	void toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq);

	const void *        pollingref;
	input_seq_type      pollingseq;
	input_item_data *   pollingitem;

private:
	input_item_data *   lastitem;
	bool                record_next;
	input_seq           starting_seq;

	virtual void handle() override;
	virtual void update_input(struct input_item_data *seqchangeditem) = 0;
};

class menu_input_general : public menu_input
{
public:
	menu_input_general(mame_ui_manager &mui, render_container &container, int group);
	virtual ~menu_input_general() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void update_input(struct input_item_data *seqchangeditem) override;

	int group;
};

class menu_input_specific : public menu_input
{
public:
	menu_input_specific(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_specific() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void update_input(struct input_item_data *seqchangeditem) override;
};

class menu_settings : public menu
{
public:
	menu_settings(mame_ui_manager &mui, render_container &container, uint32_t type);
	virtual ~menu_settings() override;

protected:
	/* DIP switch descriptor */
	struct dip_descriptor
	{
		dip_descriptor *    next;
		const char *        name;
		device_t *          owner;
		uint32_t            mask;
		uint32_t            state;
	};

	dip_descriptor *    diplist;
	int dipcount;
	int type;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
};

class menu_settings_dip_switches : public menu_settings
{
public:
	menu_settings_dip_switches(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_dip_switches() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	void custom_render_one(float x1, float y1, float x2, float y2, const dip_descriptor *dip, uint32_t selectedmask);
};

class menu_settings_driver_config : public menu_settings
{
public:
	menu_settings_driver_config(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_driver_config();
};

class menu_analog : public menu
{
public:
	menu_analog(mame_ui_manager &mui, render_container &container);
	virtual ~menu_analog() override;

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

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_INPUTMAP_H */
