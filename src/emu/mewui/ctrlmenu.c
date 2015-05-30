/*********************************************************************

	mewui/ctrlmenu.c

	Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/ctrlmenu.h"

const char *ui_menu_controller_mapping::device_status[] = { "none", "keyboard", "mouse", "lightgun", "joystick" };

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_controller_mapping::ui_menu_controller_mapping(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	m_options[ADSTICK_DEVICE_ASSIGN] = check_status(machine.options().adstick_device());
	m_options[PADDLE_DEVICE_ASSIGN] = check_status(machine.options().paddle_device());
	m_options[LIGHTGUN_DEVICE_ASSIGN] = check_status(machine.options().lightgun_device());
	m_options[TRACKBALL_DEVICE_ASSIGN] = check_status(machine.options().trackball_device());
	m_options[DIAL_DEVICE_ASSIGN] = check_status(machine.options().dial_device());
	m_options[POSITIONAL_DEVICE_ASSIGN] = check_status(machine.options().positional_device());
	m_options[MOUSE_DEVICE_ASSIGN] = check_status(machine.options().mouse_device());
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_controller_mapping::~ui_menu_controller_mapping()
{
	std::string error_string;
	machine().options().set_value(OPTION_LIGHTGUN_DEVICE, device_status[m_options[LIGHTGUN_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_ADSTICK_DEVICE, device_status[m_options[ADSTICK_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_PADDLE_DEVICE, device_status[m_options[PADDLE_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_DIAL_DEVICE, device_status[m_options[DIAL_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_POSITIONAL_DEVICE, device_status[m_options[POSITIONAL_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_TRACKBALL_DEVICE, device_status[m_options[TRACKBALL_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_MOUSE_DEVICE, device_status[m_options[MOUSE_DEVICE_ASSIGN]], OPTION_PRIORITY_CMDLINE, error_string);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_controller_mapping::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			changed = true;
			int value = (FPTR)menu_event->itemref;
			(menu_event->iptkey == IPT_UI_RIGHT) ? m_options[value]++ : m_options[value]--;
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
	// add lightgun device assignment option
	UINT32 arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[LIGHTGUN_DEVICE_ASSIGN]);
	item_append("Lightgun Device Assignment", device_status[m_options[LIGHTGUN_DEVICE_ASSIGN]], arrow_flags, (void *)LIGHTGUN_DEVICE_ASSIGN);

	// add paddle device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[PADDLE_DEVICE_ASSIGN]);
	item_append("Paddle Device Assignment", device_status[m_options[PADDLE_DEVICE_ASSIGN]], arrow_flags, (void *)PADDLE_DEVICE_ASSIGN);

	// add adstick device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[ADSTICK_DEVICE_ASSIGN]);
	item_append("Adstick Device Assignment", device_status[m_options[ADSTICK_DEVICE_ASSIGN]], arrow_flags, (void *)ADSTICK_DEVICE_ASSIGN);

	// add trackball device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[TRACKBALL_DEVICE_ASSIGN]);
	item_append("Trackball Device Assignment", device_status[m_options[TRACKBALL_DEVICE_ASSIGN]], arrow_flags, (void *)TRACKBALL_DEVICE_ASSIGN);

	// add positional device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[POSITIONAL_DEVICE_ASSIGN]);
	item_append("Positional Device Assignment", device_status[m_options[POSITIONAL_DEVICE_ASSIGN]], arrow_flags, (void *)POSITIONAL_DEVICE_ASSIGN);

	// add dial device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[DIAL_DEVICE_ASSIGN]);
	item_append("Dial Device Assignment", device_status[m_options[DIAL_DEVICE_ASSIGN]], arrow_flags, (void *)DIAL_DEVICE_ASSIGN);

	// add mouse device assignment option
	arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(device_status) - 1, m_options[MOUSE_DEVICE_ASSIGN]);
	item_append("Mouse Device Assignment", device_status[m_options[MOUSE_DEVICE_ASSIGN]], arrow_flags, (void *)MOUSE_DEVICE_ASSIGN);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop =  machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_controller_mapping::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;

	machine().ui().draw_text_full(container, "Device Mapping", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, "Device Mapping", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

}

//-------------------------------------------------
//  return current value
//-------------------------------------------------

int ui_menu_controller_mapping::check_status(const char *status)
{
	for (int d = 0; *device_status[d]; d++)
		if (!strcmp(device_status[d], status))
			return d;

	return 0;

}
