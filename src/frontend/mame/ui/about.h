// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/about.h

    About box

***************************************************************************/
#ifndef MAME_FRONTEND_UI_ABOUT_H
#define MAME_FRONTEND_UI_ABOUT_H

#pragma once

#include "ui/menu.h"

#include <optional>
#include <string>
#include <vector>


namespace ui {

class menu_about : public menu
{
public:
	menu_about(mame_ui_manager &mui, render_container &container);
	virtual ~menu_about() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void draw(uint32_t flags) override;
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	std::vector<std::string> const m_header;
	std::optional<text_layout> m_layout;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_ABOUT_H
