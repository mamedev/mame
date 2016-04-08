// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/ctrlmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/ctrlmenu.h"

const char *ui_menu_controller_mapping::m_device_status[] = { "none", "keyboard", "mouse", "lightgun", "joystick" };

ui_menu_controller_mapping::ctrl_option ui_menu_controller_mapping::m_options[] = {
	{ 0, nullptr, nullptr },
	{ 0, __("Lightgun Device Assignment"),   OPTION_LIGHTGUN_DEVICE },
	{ 0, __("Trackball Device Assignment"),  OPTION_TRACKBALL_DEVICE },
	{ 0, __("Pedal Device Assignment"),      OPTION_PEDAL_DEVICE },
	{ 0, __("Adstick Device Assignment"),    OPTION_ADSTICK_DEVICE },
	{ 0, __("Paddle Device Assignment"),     OPTION_PADDLE_DEVICE },
	{ 0, __("Dial Device Assignment"),       OPTION_DIAL_DEVICE },
	{ 0, __("Positional Device Assignment"), OPTION_POSITIONAL_DEVICE },
	{ 0, __("Mouse Device Assignment"),      OPTION_MOUSE_DEVICE }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_controller_mapping::ui_menu_controller_mapping(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	for (int d = 1; d < ARRAY_LENGTH(m_options); ++d)
		m_options[d].status = check_status(machine.options().value(m_options[d].option), m_options[d].option);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_controller_mapping::~ui_menu_controller_mapping()
{
	std::string error_string;
	for (int d = 1; d < ARRAY_LENGTH(m_options); ++d)
	{
		if (strcmp(machine().options().value(m_options[d].option), m_device_status[m_options[d].status])!=0)
		{
			machine().options().set_value(m_options[d].option, m_device_status[m_options[d].status], OPTION_PRIORITY_CMDLINE, error_string);
			machine().options().mark_changed(m_options[d].option);
		}
	}
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_controller_mapping::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);
	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
		{
			changed = true;
			FPTR value = (FPTR)m_event->itemref;
			(m_event->iptkey == IPT_UI_RIGHT) ? m_options[value].status++ : m_options[value].status--;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_controller_mapping::populate()
{
	// add options
	for (int d = 1; d < ARRAY_LENGTH(m_options); ++d)
	{
		UINT32 arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(m_device_status) - 1, m_options[d].status);
		item_append(_(m_options[d].description), m_device_status[m_options[d].status], arrow_flags, (void *)(FPTR)d);
	}
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop =  machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_controller_mapping::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	ui_manager &mui = machine().ui();

	mui.draw_text_full(container, _("Device Mapping"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, _("Device Mapping"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

}

//-------------------------------------------------
//  return current value
//-------------------------------------------------

int ui_menu_controller_mapping::check_status(const char *status, const char *option)
{
	for (int d = 0; *m_device_status[d]; d++)
		if (!strcmp(m_device_status[d], status))
			return d;

	emu_options def_opt;
	const char *def_val = def_opt.value(option);

	for (int d = 0; *m_device_status[d]; d++)
		if (!strcmp(m_device_status[d], def_val))
			return d;

	return 1;
}
