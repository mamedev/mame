// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/videoopt.h

    Internal menus for video options

***************************************************************************/
#ifndef MAME_FRONTEND_UI_VIDEOOPT_H
#define MAME_FRONTEND_UI_VIDEOOPT_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_video_targets : public menu
{
public:
	menu_video_targets(mame_ui_manager &mui, render_container &container);
	virtual ~menu_video_targets() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};


class menu_video_options : public menu
{
public:
	menu_video_options(mame_ui_manager &mui, render_container &container, render_target &target, bool snapshot);
	menu_video_options(mame_ui_manager &mui, render_container &container, std::string &&title, render_target &target, bool snapshot);
	virtual ~menu_video_options() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	render_target &m_target;
	std::string const m_title;
	bool const m_show_title;
	bool const m_snapshot;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_VIDEOOPT_H
