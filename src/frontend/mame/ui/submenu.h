// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota,Jeffrey Clark
/***************************************************************************

    ui/submenu.h

    UI options menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SUBMENU_H
#define MAME_FRONTEND_UI_SUBMENU_H

#pragma once

#include "ui/menu.h"

#include "emuopts.h"

#include <string>
#include <vector>


namespace ui {

//-------------------------------------------------
//  class ui menu
//-------------------------------------------------
class submenu : public menu
{
public:
	enum class option_type
	{
		HEAD,
		SEP,
		MENU,
		CMD,
		EMU,
		UI,
		OSD,
	};

	struct option
	{
		option_type type;
		const char  *description;
		const char  *name;
		core_options::entry::shared_ptr entry;
		core_options *options;
		std::vector<std::string> value;
	};

	submenu(mame_ui_manager &mui, render_container &container, std::vector<option> const &suboptions, const game_driver *drv = nullptr, emu_options *options = nullptr);
	submenu(mame_ui_manager &mui, render_container &container, std::vector<option> &&suboptions, const game_driver *drv = nullptr, emu_options *options = nullptr);
	virtual ~submenu();

	static std::vector<option> misc_options();
	static std::vector<option> advanced_options();
	static std::vector<option> control_options();
	static std::vector<option> video_options();
	//static std::vector<option> export_options();

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::vector<option> m_options;
	game_driver const   *m_driver;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SUBMENU_H
