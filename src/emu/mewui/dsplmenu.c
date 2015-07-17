/*********************************************************************

    mewui/dsplmenu.c

    Internal MEWUI user interface.

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

	// osd
	m_options[MT_ENABLED] = options.multithreading();
	m_options[WINDOW_ENABLED] = options.window();
	m_options[KAR_ENABLED] = options.keep_aspect();
	m_options[MAXIM_ENABLED] = options.maximize();
	m_options[SYNCREF_ENABLED] = options.sync_refresh();
	m_options[WAITSYNC_ENABLED] = options.wait_vsync();
	m_options[FILTER_ENABLED] = options.filter();
	m_options[PRESCALE_ENABLED] = (options.prescale() == 1);

#ifdef MEWUI_WINDOWS
	// windows
	m_options[HWSTRETCH_ENABLED] = options.hwstretch();
	m_options[HLSL_ENABLED] = options.d3d_hlsl_enable();
	m_options[TBUFFER_ENABLED] = options.triple_buffer();
#endif

	int total = ARRAY_LENGTH(video_modes);

	for (cur_video = 0; cur_video < total; cur_video++)
		if (!core_stricmp(options.video(), video_modes[cur_video]))
			break;

	if (cur_video == total)
		cur_video = 0;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_display_options::~ui_menu_display_options()
{
	std::string error_string;

#ifdef MEWUI_WINDOWS
	machine().options().set_value(WINOPTION_HWSTRETCH, m_options[HWSTRETCH_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(WINOPTION_HLSL_ENABLE, m_options[HLSL_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(WINOPTION_TRIPLEBUFFER, m_options[TBUFFER_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
#endif

	machine().options().set_value(OSDOPTION_VIDEO, video_modes[cur_video], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_FILTER, m_options[FILTER_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_PRESCALE, m_options[PRESCALE_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_MULTITHREADING, m_options[MT_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_WINDOW, m_options[WINDOW_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_KEEPASPECT, m_options[KAR_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_MAXIMIZE, m_options[MAXIM_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_SYNCREFRESH, m_options[SYNCREF_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OSDOPTION_WAITVSYNC, m_options[WAITSYNC_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
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
		switch ((FPTR)menu_event->itemref)
		{
			case VIDEO_MODE:
			{
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					(menu_event->iptkey == IPT_UI_LEFT) ? cur_video-- : cur_video++;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					int total = ARRAY_LENGTH(video_modes_label);
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; index++)
						s_sel[index].assign(video_modes_label[index]);

					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &cur_video)));
				}
				break;
			}

			default:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
				{
					changed = true;
					int value = (FPTR)menu_event->itemref;
					m_options[value] = !m_options[value];
				}
				break;
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
	std::string v_text(video_modes_label[cur_video]);
	UINT32 arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(video_modes) - 1, cur_video);
	item_append("Video Mode", v_text.c_str(), arrow_flags, (void *)VIDEO_MODE);

#ifdef MEWUI_WINDOWS
	// add hardware stretch option
	item_append("Hardware Stretch", m_options[HWSTRETCH_ENABLED] ? "On" : "Off",
	            m_options[HWSTRETCH_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)HWSTRETCH_ENABLED);

	// add Triple Buffer option
	item_append("Triple Buffering", m_options[TBUFFER_ENABLED] ? "On" : "Off",
	            m_options[TBUFFER_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)TBUFFER_ENABLED);

	// add HLSL option
	item_append("HLSL", m_options[HLSL_ENABLED] ? "On" : "Off",
	            m_options[HLSL_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)HLSL_ENABLED);
#endif

	// add bilinear option
	item_append("Bilinear Filtering", m_options[FILTER_ENABLED] ? "On" : "Off",
	            m_options[FILTER_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)FILTER_ENABLED);

	// add Bitmap Prescale option
	item_append("Bitmap Prescaling", m_options[PRESCALE_ENABLED] ? "On" : "Off",
	            m_options[PRESCALE_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)PRESCALE_ENABLED);

	// add multithreaded rendering option
	item_append("Multi-Threaded Rendering", m_options[MT_ENABLED] ? "On" : "Off",
	            m_options[MT_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)MT_ENABLED);

	// add Window Mode option
	item_append("Enable Window Mode", m_options[WINDOW_ENABLED] ? "On" : "Off",
	            m_options[WINDOW_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)WINDOW_ENABLED);

	// add Aspect Ratio option
	item_append("Enforce Aspect Ratio", m_options[KAR_ENABLED] ? "On" : "Off",
	            m_options[KAR_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)KAR_ENABLED);

	// add Maximized option
	item_append("Start Out Maximized", m_options[MAXIM_ENABLED] ? "On" : "Off",
	            m_options[MAXIM_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)MAXIM_ENABLED);

	// add Synchronized Refresh option
	item_append("Synchronized Refresh", m_options[SYNCREF_ENABLED] ? "On" : "Off",
	            m_options[SYNCREF_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)SYNCREF_ENABLED);

	// add Vertical Sync option
	item_append("Wait Vertical Sync", m_options[WAITSYNC_ENABLED] ? "On" : "Off",
	            m_options[WAITSYNC_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)WAITSYNC_ENABLED);

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
