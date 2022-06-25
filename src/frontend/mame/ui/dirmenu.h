// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/dirmenu.h

    Internal UI user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_DIRMENU_H
#define MAME_FRONTEND_UI_DIRMENU_H

#pragma once

#include "ui/menu.h"

#include <string>
#include <vector>

namespace ui {

//-------------------------------------------------
//  class directory menu
//-------------------------------------------------

class menu_directory : public menu
{
public:
	menu_directory(mame_ui_manager &mui, render_container &container);
	virtual ~menu_directory() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_DIRMENU_H
