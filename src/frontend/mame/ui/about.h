// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/about.h

    About box

***************************************************************************/
#ifndef MAME_FRONTEND_UI_ABOUT_H
#define MAME_FRONTEND_UI_ABOUT_H

#pragma once

#include "ui/text.h"
#include "ui/textbox.h"

#include <optional>
#include <string>
#include <vector>


namespace ui {

class menu_about : public menu_textbox
{
public:
	menu_about(mame_ui_manager &mui, render_container &container);
	virtual ~menu_about() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate() override;

	std::vector<std::string> const m_header;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_ABOUT_H
