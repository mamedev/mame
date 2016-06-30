// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/videoopt.h

    Internal menus for video options

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_VIDEOOPT_H
#define MAME_FRONTEND_UI_VIDEOOPT_H

#include "ui/menu.h"

namespace ui {
class menu_video_targets : public menu
{
public:
	menu_video_targets(mame_ui_manager &mui, render_container *container);
	virtual ~menu_video_targets() override;
	virtual void populate() override;
	virtual void handle() override;
};

class menu_video_options : public menu
{
public:
	menu_video_options(mame_ui_manager &mui, render_container *container, render_target *target);
	virtual ~menu_video_options() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	enum {
		VIDEO_ITEM_ROTATE = 0x80000000,
		VIDEO_ITEM_BACKDROPS,
		VIDEO_ITEM_OVERLAYS,
		VIDEO_ITEM_BEZELS,
		VIDEO_ITEM_CPANELS,
		VIDEO_ITEM_MARQUEES,
		VIDEO_ITEM_ZOOM,
		VIDEO_ITEM_VIEW
	};

	render_target *target;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_VIDEOOPT_H */
