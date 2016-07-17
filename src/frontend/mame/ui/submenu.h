// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota,Jeffrey Clark
/***************************************************************************

    ui/submenu.h

    UI options menu.

***************************************************************************/
#pragma once

#ifndef MAME_FRONTEND_UI_SUBMENU_H
#define MAME_FRONTEND_UI_SUBMENU_H

#include "emuopts.h"
#include "ui/menu.h"

#if defined(UI_WINDOWS) && !defined(UI_SDL)
#include "../osd/windows/winmain.h"
#else
#include "../osd/modules/lib/osdobj_common.h"
#endif

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
		core_options::entry *entry;
		core_options *options;
		std::vector<std::string> value;
	};

	submenu(mame_ui_manager &mui, render_container &container, std::vector<option> const &suboptions, const game_driver *drv = nullptr, emu_options *options = nullptr);
	submenu(mame_ui_manager &mui, render_container &container, std::vector<option> &&suboptions, const game_driver *drv = nullptr, emu_options *options = nullptr);
	virtual ~submenu();

	static std::vector<option> const misc_options;
	static std::vector<option> const advanced_options;
	static std::vector<option> const control_options;
	static std::vector<option> const video_options;
	//static std::vector<option> const export_options;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate() override;
	virtual void handle() override;

	std::vector<option> m_options;
	game_driver const   *m_driver;
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_SUBMENU_H */
