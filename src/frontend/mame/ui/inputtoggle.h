// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputtoggle.h

    Toggle inputs menu.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INPUTTOGGLE_H
#define MAME_FRONTEND_UI_INPUTTOGGLE_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <vector>


namespace ui {

class menu_input_toggles : public menu
{
public:
	menu_input_toggles(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_toggles();

protected:
	virtual void menu_activated() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::vector<std::reference_wrapper<ioport_field> > m_fields;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INPUTTOGGLE_H
