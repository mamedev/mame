// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/***************************************************************************

    ui/analogipt.h

    Analog inputs menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_ANALOGIPT_H
#define MAME_FRONTEND_UI_ANALOGIPT_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <vector>


namespace ui {

class menu_analog : public menu
{
public:
	menu_analog(mame_ui_manager &mui, render_container &container);
	virtual ~menu_analog() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		ANALOG_ITEM_KEYSPEED = 0,
		ANALOG_ITEM_CENTERSPEED,
		ANALOG_ITEM_REVERSE,
		ANALOG_ITEM_SENSITIVITY,
		ANALOG_ITEM_COUNT
	};

	struct item_data
	{
		item_data(ioport_field &f, int t) noexcept;

		std::reference_wrapper<ioport_field> field;
		int type;
		int defvalue;
		int min, max;
		int cur;
	};

	struct field_data
	{
		field_data(ioport_field &f) noexcept;

		std::reference_wrapper<ioport_field> field;
		float range;
		float neutral;
		float origin;
		u8 shift;
		bool show_neutral;
	};

	using item_data_vector = std::vector<item_data>;
	using field_data_vector = std::vector<field_data>;

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	void find_fields();

	item_data_vector m_item_data;
	field_data_vector m_field_data;
	std::string m_prompt;
	unsigned m_visible_fields;
	int m_top_field;
	bool m_hide_menu;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_ANALOGIPT_H
