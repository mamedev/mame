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


namespace ui {

class menu_textbox : public menu
{
public:
	virtual ~menu_textbox() override;

protected:
	menu_textbox(mame_ui_manager &mui, render_container &container);

	void reset_layout();
	void handle_key(int key);

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) = 0;

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

} // namespace ui

#endif // MAME_FRONTEND_UI_TEXTBOX_H
