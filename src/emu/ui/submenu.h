// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota,Jeffrey Clark
/***************************************************************************

    ui/submenu.h

    UI options menu.

***************************************************************************/
#pragma once

#ifndef __UI_SUBMENU_H__
#define __UI_SUBMENU_H__

#include "ui/menu.h"

//-------------------------------------------------
//  class ui menu
//-------------------------------------------------
class ui_submenu : public ui_menu
{
public:
	enum option_type {
		HEAD,
		SEP,
		MENU,
		CMD,
		EMU,
		UI,
		OSD,
	};

	struct option {
		option_type type;
		const char  *description;
		const char  *name;
		core_options::entry *entry;
		core_options (*options);
	};

	ui_submenu(running_machine &machine, render_container *container, std::vector<ui_submenu::option> &suboptions);
	virtual ~ui_submenu();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	std::vector<option> &m_options;
};

static std::vector<ui_submenu::option> misc_submenu_options = {
	{ ui_submenu::HEAD, _("Miscellaneous Options") },
	{ ui_submenu::UI,   _("Re-select last machine played"),           OPTION_REMEMBER_LAST },
	{ ui_submenu::UI,   _("Enlarge images in the right panel"),       OPTION_ENLARGE_SNAPS },
	{ ui_submenu::UI,   _("DATs info"),                               OPTION_DATS_ENABLED },
	{ ui_submenu::EMU,  _("Cheats"),                                  OPTION_CHEAT },
	{ ui_submenu::EMU,  _("Show mouse pointer"),                      OPTION_UI_MOUSE },
	{ ui_submenu::EMU,  _("Confirm quit from machines"),              OPTION_CONFIRM_QUIT },
	{ ui_submenu::EMU,  _("Skip information screen at startup"),      OPTION_SKIP_GAMEINFO },
	{ ui_submenu::UI,   _("Force 4:3 aspect for snapshot display"),   OPTION_FORCED4X3 },
	{ ui_submenu::UI,   _("Use image as background"),                 OPTION_USE_BACKGROUND },
	{ ui_submenu::UI,   _("Skip bios selection menu"),                OPTION_SKIP_BIOS_MENU },
	{ ui_submenu::UI,   _("Skip software parts selection menu"),      OPTION_SKIP_PARTS_MENU },
	{ ui_submenu::UI,   _("Info auto audit"),                         OPTION_INFO_AUTO_AUDIT },
};

static std::vector<ui_submenu::option> advanced_submenu_options = {
	{ ui_submenu::HEAD, _("Advanced Options") },
	{ ui_submenu::HEAD, _("Performance Options") },
	{ ui_submenu::EMU,  _("Auto frame skip"),                         OPTION_AUTOFRAMESKIP },
	{ ui_submenu::EMU,  _("Frame skip"),                              OPTION_FRAMESKIP },
	{ ui_submenu::EMU,  _("Throttle"),                                OPTION_THROTTLE },
	{ ui_submenu::EMU,  _("Sleep"),                                   OPTION_SLEEP },
	{ ui_submenu::EMU,  _("Speed"),                                   OPTION_SPEED },
	{ ui_submenu::EMU,  _("Refresh speed"),                           OPTION_REFRESHSPEED },
//};

//static std::vector<ui_submenu::option> rotate_submenu_options = {
	{ ui_submenu::HEAD, _("Rotation Options") },
	{ ui_submenu::EMU,  _("Rotate"),                                  OPTION_ROTATE },
	{ ui_submenu::EMU,  _("Rotate right"),                            OPTION_ROR },
	{ ui_submenu::EMU,  _("Rotate left"),                             OPTION_ROL },
	{ ui_submenu::EMU,  _("Auto rotate right"),                       OPTION_AUTOROR },
	{ ui_submenu::EMU,  _("Auto rotate left"),                        OPTION_AUTOROL },
	{ ui_submenu::EMU,  _("Flip X"),                                  OPTION_FLIPX },
	{ ui_submenu::EMU,  _("Flip Y"),                                  OPTION_FLIPY },
//};

//static std::vector<ui_submenu::option> artwork_submenu_options = {
	{ ui_submenu::HEAD, _("Artwork Options") },
	{ ui_submenu::EMU,  _("Artwork Crop"),                            OPTION_ARTWORK_CROP },
	{ ui_submenu::EMU,  _("Use Backdrops"),                           OPTION_USE_BACKDROPS },
	{ ui_submenu::EMU,  _("Use Overlays"),                            OPTION_USE_OVERLAYS },
	{ ui_submenu::EMU,  _("Use Bezels"),                              OPTION_USE_BEZELS },
	{ ui_submenu::EMU,  _("Use Control Panels"),                      OPTION_USE_CPANELS },
	{ ui_submenu::EMU,  _("Use Marquees"),                            OPTION_USE_MARQUEES },
//};

//static std::vector<ui_submenu::option> state_submenu_options = {
	{ ui_submenu::HEAD, _("State/Playback Options") },
	{ ui_submenu::EMU,  _("Automatic save/restore"),                  OPTION_AUTOSAVE },
	{ ui_submenu::EMU,  _("Bilinear snapshot"),                       OPTION_SNAPBILINEAR },
	{ ui_submenu::EMU,  _("Burn-in"),                                 OPTION_BURNIN },
//};

//static std::vector<ui_submenu::option> input_submenu_options = {
	{ ui_submenu::HEAD, _("Input Options") },
	{ ui_submenu::EMU,  _("Coin lockout"),                            OPTION_COIN_LOCKOUT },
	{ ui_submenu::EMU,  _("Mouse"),                                   OPTION_MOUSE },
	{ ui_submenu::EMU,  _("Joystick"),                                OPTION_JOYSTICK },
	{ ui_submenu::EMU,  _("Lightgun"),                                OPTION_LIGHTGUN },
	{ ui_submenu::EMU,  _("Multi-keyboard"),                          OPTION_MULTIKEYBOARD },
	{ ui_submenu::EMU,  _("Multi-mouse"),                             OPTION_MULTIMOUSE },
	{ ui_submenu::EMU,  _("Steadykey"),                               OPTION_STEADYKEY },
	{ ui_submenu::EMU,  _("UI active"),                               OPTION_UI_ACTIVE },
	{ ui_submenu::EMU,  _("Offscreen reload"),                        OPTION_OFFSCREEN_RELOAD },
	{ ui_submenu::EMU,  _("Joystick deadzone"),                       OPTION_JOYSTICK_DEADZONE },
	{ ui_submenu::EMU,  _("Joystick saturation"),                     OPTION_JOYSTICK_SATURATION },
	{ ui_submenu::EMU,  _("Natural keyboard"),                        OPTION_NATURAL_KEYBOARD },
	{ ui_submenu::EMU,  _("Simultaneous contradictory"),              OPTION_JOYSTICK_CONTRADICTORY },
	{ ui_submenu::EMU,  _("Coin impulse"),                            OPTION_COIN_IMPULSE },
};

//static std::vector<ui_submenu::option> export_submenu_options = {
//	{ ui_submenu::COMMAND, _("Export XML format (like -listxml)"),               "exportxml" },
//	{ ui_submenu::COMMAND, _("Export TXT format (like -listfull)"),              "exporttxt" },
//};

#endif /* __UI_SUBMENU_H__ */
