// license:BSD-3-Clause
// copyright-holders:Dankan1890
/*********************************************************************

    mewui/dsplmenu.c

    MEWUI video options menu.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/dsplmenu.h"
#include "mewui/selector.h"
#include "mewui/utils.h"

// SDL
#ifdef MEWUI_WINDOWS
#include "../osd/windows/winmain.h"
const char *ui_menu_display_options::video_modes[] = { "auto", "d3d", "gdi", "ddraw", "opengl" };
const char *ui_menu_display_options::video_modes_label[] = { "Auto", "Direct3D", "GDI", "DirectDraw", "OpenGL" };
#else
#include "../osd/modules/lib/osdobj_common.h"
const char *ui_menu_display_options::video_modes[] = { "auto", "opengl", "soft", "accel" };
const char *ui_menu_display_options::video_modes_label[] = { "Auto", "OpenGL", "Software", "SDL2 Accelerated" };
#endif

dspl_option ui_menu_display_options::m_options[] = {
	{ 0, NULL, NULL },
	{ 0, "Video Mode",               OSDOPTION_VIDEO },
#ifdef MEWUI_WINDOWS
	{ 0, "Hardware Stretch",         WINOPTION_HWSTRETCH },
	{ 0, "Triple Buffering",         WINOPTION_TRIPLEBUFFER },
	{ 0, "HLSL",                     WINOPTION_HLSL_ENABLE },
#endif
	{ 0, "GLSL",                     OSDOPTION_GL_GLSL },
	{ 0, "Bilinear Filtering",       OSDOPTION_FILTER },
	{ 0, "Bitmap Prescaling",        OSDOPTION_PRESCALE },
	{ 0, "Multi-Threaded Rendering", OSDOPTION_MULTITHREADING },
	{ 0, "Window Mode",              OSDOPTION_WINDOW },
	{ 0, "Enforce Aspect Ratio",     OSDOPTION_KEEPASPECT },
	{ 0, "Start Out Maximized",      OSDOPTION_MAXIMIZE },
	{ 0, "Synchronized Refresh",     OSDOPTION_SYNCREFRESH },
	{ 0, "Wait Vertical Sync",       OSDOPTION_WAITVSYNC }
};

/**************************************************
    MENU DISPLAY OPTIONS
**************************************************/
//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_display_options::ui_menu_display_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
#ifdef MEWUI_WINDOWS
	windows_options &options = downcast<windows_options &>(machine.options());
#else
	osd_options &options = downcast<osd_options &>(machine.options());
#endif

	for (int d = 2; d < ARRAY_LENGTH(m_options); ++d)
		m_options[d].status = options.int_value(m_options[d].option);

	m_options[1].status = 0;
	for (int cur = 0; cur < ARRAY_LENGTH(video_modes); ++cur)
		if (!core_stricmp(options.video(), video_modes[cur]))
		{
			m_options[1].status = cur;
			break;
		}
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_display_options::~ui_menu_display_options()
{
	std::string error_string;
	for (int d = 2; d < ARRAY_LENGTH(m_options); ++d)
		machine().options().set_value(m_options[d].option, m_options[d].status, OPTION_PRIORITY_CMDLINE, error_string);

	machine().options().set_value(m_options[1].option, video_modes[m_options[1].status], OPTION_PRIORITY_CMDLINE, error_string);
	mewui_globals::force_reset_main = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_display_options::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		int value = (FPTR)menu_event->itemref;
		if (!strcmp(m_options[value].option, OSDOPTION_VIDEO) || !strcmp(m_options[value].option, OSDOPTION_PRESCALE))
		{
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				changed = true;
				(menu_event->iptkey == IPT_UI_LEFT) ? m_options[value].status-- : m_options[value].status++;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT && !strcmp(m_options[value].option, OSDOPTION_VIDEO))
			{
				int total = ARRAY_LENGTH(video_modes_label);
				std::vector<std::string> s_sel(total);
				for (int index = 0; index < total; index++)
					s_sel[index].assign(video_modes_label[index]);

				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &m_options[value].status)));
			}
		}
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
		{
			changed = true;
			m_options[value].status = !m_options[value].status;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_display_options::populate()
{
	// add video mode option
	std::string v_text(video_modes_label[m_options[1].status]);
	UINT32 arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(video_modes) - 1, m_options[1].status);
	item_append(m_options[1].description, v_text.c_str(), arrow_flags, (void *)1);

	// add options items
	for (int opt = 2; opt < ARRAY_LENGTH(m_options); ++opt)
		if (strcmp(m_options[opt].option, OSDOPTION_PRESCALE))
			item_append(m_options[opt].description, m_options[opt].status ? "On" : "Off",
			            m_options[opt].status ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)opt);
		else
		{
			strprintf(v_text, "%d", m_options[opt].status);
			arrow_flags = get_arrow_flags(1, 3, m_options[opt].status);
			item_append(m_options[opt].description, v_text.c_str(), arrow_flags, (void *)(FPTR)opt);
		}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_display_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	machine().ui().draw_text_full(container, "Display Options", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

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
	machine().ui().draw_text_full(container, "Display Options", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}
