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

#include <optional>
#include <string>


namespace ui {

class menu_textbox : public menu
{
public:
	virtual ~menu_textbox() override;

protected:
	menu_textbox(mame_ui_manager &mui, render_container &container);

	void reset_layout();
	bool handle_key(int key);

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) = 0;

	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual bool custom_mouse_scroll(int lines) override;

private:
	virtual void draw(uint32_t flags) override;

	std::optional<text_layout> m_layout;
	float m_layout_width;
	float m_desired_width;
	int m_desired_lines;
	int m_window_lines;
	int m_top_line;
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
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	std::string const m_heading;
	std::string const m_content;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_TEXTBOX_H
