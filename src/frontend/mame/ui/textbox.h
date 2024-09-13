// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/textbox.h

    Menu that displays a non-interactive text box

***************************************************************************/
#ifndef MAME_FRONTEND_UI_TEXTBOX_H
#define MAME_FRONTEND_UI_TEXTBOX_H

#include "ui/menu.h"
#include "ui/text.h"

#include <chrono>
#include <optional>
#include <string>
#include <tuple>
#include <utility>


namespace ui {

class menu_textbox : public menu
{
public:
	virtual ~menu_textbox() override;

protected:
	menu_textbox(mame_ui_manager &mui, render_container &container);

	void reset_layout();

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) = 0;

	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;
	virtual bool custom_mouse_scroll(int lines) override;

	virtual bool handle(event const *ev) override;

private:
	enum class pointer_action
	{
		NONE,
		SCROLL_UP,
		SCROLL_DOWN,
		SCROLL_DRAG,
		CHECK_EXIT
	};

	virtual void draw(uint32_t flags) override;

	bool scroll_if_expired(std::chrono::steady_clock::time_point now);
	bool pointer_in_line(float y, int line) const;

	std::optional<text_layout> m_layout;
	std::pair<float, float> m_line_bounds;
	float m_visible_top;
	float m_layout_width;
	float m_desired_width;
	int m_desired_lines;
	int m_window_lines;
	int m_top_line;

	pointer_action m_pointer_action;
	std::chrono::steady_clock::time_point m_scroll_repeat;
	std::pair<float, float> m_base_pointer;
	std::pair<float, float> m_last_pointer;
	int m_scroll_base;
};


class menu_fixed_textbox : public menu_textbox
{
public:
	menu_fixed_textbox(
			mame_ui_manager &mui,
			render_container &container,
			std::string &&headig,
			std::string &&content);
	virtual ~menu_fixed_textbox() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate() override;

	std::string const m_heading;
	std::string const m_content;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_TEXTBOX_H
