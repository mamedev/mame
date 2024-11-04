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

#include <chrono>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>


namespace ui {

class menu_analog : public menu
{
public:
	menu_analog(mame_ui_manager &mui, render_container &container);
	virtual ~menu_analog() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;
	virtual void menu_activated() override;

private:
	enum class pointer_action
	{
		NONE,
		SCROLL_UP,
		SCROLL_DOWN,
		SCROLL_DRAG,
		CHECK_TOGGLE_MENU
	};

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

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void find_fields();
	bool scroll_if_expired(std::chrono::steady_clock::time_point now);
	bool update_scroll_drag(ui_event const &uievt);

	static std::string item_text(int type, int value);

	item_data_vector m_item_data;
	field_data_vector m_field_data;
	std::string m_prompt;
	unsigned m_bottom_fields;
	int m_visible_fields;
	int m_top_field;
	bool m_hide_menu;

	float m_box_left;
	float m_box_top;
	float m_box_right;
	float m_box_bottom;

	pointer_action m_pointer_action;
	std::chrono::steady_clock::time_point m_scroll_repeat;
	std::pair<float, float> m_base_pointer;
	std::pair<float, float> m_last_pointer;
	int m_scroll_base;
	bool m_arrow_clicked_first;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_ANALOGIPT_H
