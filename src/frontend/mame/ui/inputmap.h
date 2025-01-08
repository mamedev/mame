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
#include "iptseqpoll.h"

#include <string>
#include <vector>


namespace ui {

class menu_input_groups : public menu
{
public:
	menu_input_groups(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_groups() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;
};


class menu_input : public menu
{
public:
	virtual ~menu_input() override;

protected:
	enum {
		INPUT_TYPE_DIGITAL = 0,
		INPUT_TYPE_ANALOG = 1,
		INPUT_TYPE_ANALOG_DEC = INPUT_TYPE_ANALOG + SEQ_TYPE_DECREMENT,
		INPUT_TYPE_ANALOG_INC = INPUT_TYPE_ANALOG + SEQ_TYPE_INCREMENT,
		INPUT_TYPE_TOTAL = INPUT_TYPE_ANALOG + SEQ_TYPE_TOTAL
	};

	// internal input menu item data
	struct input_item_data
	{
		const void *        ref = nullptr;              // reference to type description for global inputs or field for game inputs
		input_seq_type      seqtype = SEQ_TYPE_INVALID; // sequence type
		input_seq           seq;                        // copy of the live sequence
		const input_seq *   defseq = nullptr;           // pointer to the default sequence
		std::string         name;                       // base name of the item
		const device_t *    owner = nullptr;            // pointer to the owner of the item
		ioport_group        group = IPG_INVALID;        // group type
		uint8_t             type = 0U;                  // type of port
		bool                is_optional = false;        // true if this input is considered optional
	};
	using data_vector = std::vector<input_item_data>;

	menu_input(mame_ui_manager &mui, render_container &container);

	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

	void populate_sorted();
	void toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq);

	data_vector data;
	input_item_data *pollingitem;

private:
	std::unique_ptr<input_sequence_poller> seq_poll;
	std::string assignprompt, appendprompt;
	std::string clearprompt, defaultprompt;
	std::string errormsg;
	input_item_data *erroritem;
	input_item_data *lastitem;
	bool record_next;
	osd_ticks_t modified_ticks;
	input_seq starting_seq;

	virtual bool handle(event const *ev) override;
	virtual void update_input(input_item_data &seqchangeditem) = 0;
};


class menu_input_general : public menu_input
{
public:
	menu_input_general(mame_ui_manager &mui, render_container &container, int group, std::string &&heading);
	virtual ~menu_input_general() override;

protected:
	virtual void menu_activated() override;

private:
	virtual void populate() override;
	virtual void update_input(input_item_data &seqchangeditem) override;

	const int group;
};


class menu_input_specific : public menu_input
{
public:
	menu_input_specific(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_specific() override;

protected:
	virtual void menu_activated() override;

private:
	virtual void populate() override;
	virtual void update_input(input_item_data &seqchangeditem) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INPUTMAP_H
