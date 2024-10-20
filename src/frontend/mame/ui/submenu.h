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
		const char *description = nullptr;
		const char *name = nullptr;
		core_options::entry::shared_ptr entry;
		core_options *options = nullptr;
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
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	std::vector<option> m_options;
	game_driver const *const m_driver;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SUBMENU_H
