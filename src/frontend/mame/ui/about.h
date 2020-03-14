// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    ui/barcode.h

    "About" modal

***************************************************************************/

#ifndef MAME_FRONTEND_UI_ABOUT_H
#define MAME_FRONTEND_UI_ABOUT_H

#pragma once

#include "ui/menu.h"
#include <vector>

namespace ui {
class menu_about : public menu
{
public:
	menu_about(mame_ui_manager &mui, render_container &container);
	virtual ~menu_about() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	bool m_pause_checked;
	bool m_was_paused;
	std::vector<std::string> m_lines;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_ABOUT_H
