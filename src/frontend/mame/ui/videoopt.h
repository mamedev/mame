// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/videoopt.h

    Internal menus for video options

***************************************************************************/

#pragma once

#ifndef __UI_VIDEOOPT_H__
#define __UI_VIDEOOPT_H__


class ui_menu_video_targets : public ui_menu {
public:
	ui_menu_video_targets(running_machine &machine, render_container *container);
	virtual ~ui_menu_video_targets();
	virtual void populate() override;
	virtual void handle() override;
};

class ui_menu_video_options : public ui_menu {
public:
	ui_menu_video_options(running_machine &machine, render_container *container, render_target *target);
	virtual ~ui_menu_video_options();
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


#endif  /* __UI_VIDEOOPT_H__ */
