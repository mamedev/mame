// license:BSD-3-Clause
// copyright-holders:Dankan1890
/*********************************************************************

    mewui/custui.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/utils.h"
#include "mewui/selector.h"
#include "mewui/custui.h"

/***************************************************************************
    CUSTOM UI CLASS
***************************************************************************/
//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_custom_ui::ui_menu_custom_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_custom_ui::~ui_menu_custom_ui()
{
	mewui_globals::force_reset_main = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_custom_ui::handle()
{
	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL && menu_event->iptkey == IPT_UI_SELECT)
		switch ((FPTR)menu_event->itemref)
		{
			case FONT_MENU:
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_font_ui(machine(), container)));
				break;

			case COLORS_MENU:
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_colors_ui(machine(), container)));
				break;
		}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_custom_ui::populate()
{
	item_append("Fonts", NULL, 0, (void *)FONT_MENU);
	item_append("Colors", NULL, 0, (void *)COLORS_MENU);
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_custom_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	machine().ui().draw_text_full(container, "Custom UI Settings", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
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
	machine().ui().draw_text_full(container, "Custom UI Settings", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/***************************************************************************
    FONT UI CLASS
***************************************************************************/
//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_font_ui::ui_menu_font_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
#ifdef MEWUI_WINDOWS

	std::string name(machine.options().ui_font());
	list();

	m_bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	m_italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);
	m_class.actual = 0;

	for (size_t index = 0; index < m_class.ui.size(); index++)
	{
		if (m_class.ui[index] == name)
		{
			m_class.actual = index;
			break;
		}
	}
#endif

	m_info_size = machine.options().infos_size();
	m_font_size = machine.options().font_rows();

	for (emu_options::entry *f_entry = machine.options().first(); f_entry != NULL; f_entry = f_entry->next())
	{
		const char *name = f_entry->name();
		if (name && strlen(name) && !strcmp(OPTION_INFOS_SIZE, f_entry->name()))
		{
			m_info_max = atof(f_entry->maximum());
			m_info_min = atof(f_entry->minimum());
		}
		else if (name && strlen(name) && !strcmp(OPTION_FONT_ROWS, f_entry->name()))
		{
			m_font_max = atof(f_entry->maximum());
			m_font_min = atof(f_entry->minimum());
		}
	}

}

#ifdef MEWUI_WINDOWS
//-------------------------------------------------
//  fonts enumerator CALLBACK
//-------------------------------------------------

int CALLBACK EnumFontFamiliesExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
	c_uifonts *lpc = (c_uifonts*)lParam;
	std::string utf((char *)lpelfe->lfFaceName);
	if (utf[0] != '@')
		lpc->ui.push_back(utf);

	return 1;
}

//-------------------------------------------------
//  create fonts list
//-------------------------------------------------

void ui_menu_font_ui::list()
{
	// create LOGFONT structure
	LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0] = '\0';

	HDC hDC = GetDC( NULL );
	EnumFontFamiliesEx( hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, (LPARAM)&m_class, 0 );
	ReleaseDC( NULL, hDC );

	// sort
	std::stable_sort(m_class.ui.begin(), m_class.ui.end());

	// add default string to the top of array
	m_class.ui.insert(m_class.ui.begin(), std::string("default"));
}
#endif

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_font_ui::~ui_menu_font_ui()
{
	std::string error_string;

#ifdef MEWUI_WINDOWS
	std::string name(m_class.ui[m_class.actual]);
	if (m_class.ui[m_class.actual] != "default")
	{
		if (m_italic)
			name.insert(0, "[I]");
		if (m_bold)
			name.insert(0, "[B]");
	}
	machine().options().set_value(OPTION_UI_FONT, name.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
#endif

	machine().options().set_value(OPTION_INFOS_SIZE, m_info_size, OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_FONT_ROWS, m_font_size, OPTION_PRIORITY_CMDLINE, error_string);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_font_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	if (menu_event != NULL && menu_event->itemref != NULL)
		switch ((FPTR)menu_event->itemref)
		{
			case INFOS_SIZE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_info_size += 0.05f : m_info_size -= 0.05f;
					changed = true;
				}
				break;

			case FONT_SIZE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_font_size++ : m_font_size--;
					changed = true;
				}
				break;

#ifdef MEWUI_WINDOWS

			case MUI_FNT:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_class.actual++ : m_class.actual--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_class.ui, &m_class.actual)));
					changed = true;

				break;

			case MUI_BOLD:
			case MUI_ITALIC:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
				{
					((FPTR)menu_event->itemref == MUI_BOLD) ? m_bold = !m_bold : m_italic = !m_italic;
					changed = true;
				}
				break;
#endif
		}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_font_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags;
	std::string tmptxt;

#ifdef MEWUI_WINDOWS
	// add fonts option
	arrow_flags = get_arrow_flags(0, m_class.ui.size() - 1, m_class.actual);
	std::string name(m_class.ui[m_class.actual]);
	item_append("UI Font", name.c_str(), arrow_flags, (void *)MUI_FNT);

	if (name.compare("default") != 0)
	{
		item_append("Bold", m_bold ? "On" : "Off", m_bold ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)MUI_BOLD);
		item_append("Italic", m_italic ? "On" : "Off", m_italic ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)MUI_ITALIC);
	}
#endif

	arrow_flags = get_arrow_flags(m_font_min, m_font_max, m_font_size);
	strprintf(tmptxt, "%2d", m_font_size);
	item_append("Lines", tmptxt.c_str(), arrow_flags, (void *)FONT_SIZE);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	// add item
	tmptxt.clear();
	strprintf(tmptxt, "%3.2f", m_info_size);
	arrow_flags = get_arrow_flags(m_info_min, m_info_max, m_info_size);
	item_append("Infos text size", tmptxt.c_str(), arrow_flags, (void *)INFOS_SIZE);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_font_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	// top text
	std::string topbuf("UI Fonts Settings");

	machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
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
	machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	if ((FPTR)selectedref == INFOS_SIZE)
	{
		topbuf.assign("Sample text - Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

		machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL, m_info_size);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(origx2 - origx1, width);

		// compute our bounds
		x1 = 0.5f - 0.5f * maxwidth;
		x2 = x1 + maxwidth;
		y1 = origy2 + UI_BOX_TB_BORDER;
		y2 = origy2 + bottom;

		// draw a box
		machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

		// take off the borders
		x1 += UI_BOX_LR_BORDER;
		x2 -= UI_BOX_LR_BORDER;
		y1 += UI_BOX_TB_BORDER;

		// draw the text within it
		machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_LEFT, WRAP_NEVER,
		                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL, m_info_size);
	}
}


/***************************************************************************
    COLORS UI CLASS
***************************************************************************/
//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_colors_ui::ui_menu_colors_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	m_color_table[MUI_BACKGROUND_COLOR].color = UI_BACKGROUND_COLOR;
	m_color_table[MUI_BACKGROUND_COLOR].option = OPTION_UI_BACKGROUND_COLOR;
	m_color_table[MUI_BORDER_COLOR].color = UI_BORDER_COLOR;
	m_color_table[MUI_BORDER_COLOR].option = OPTION_UI_BORDER_COLOR;
	m_color_table[MUI_CLONE_COLOR].color = UI_CLONE_COLOR;
	m_color_table[MUI_CLONE_COLOR].option = OPTION_UI_CLONE_COLOR;
	m_color_table[MUI_DIPSW_COLOR].color = UI_DIPSW_COLOR;
	m_color_table[MUI_DIPSW_COLOR].option = OPTION_UI_DIPSW_COLOR;
	m_color_table[MUI_GFXVIEWER_BG_COLOR].color = UI_GFXVIEWER_BG_COLOR;
	m_color_table[MUI_GFXVIEWER_BG_COLOR].option = OPTION_UI_GFXVIEWER_BG_COLOR;
	m_color_table[MUI_MOUSEDOWN_BG_COLOR].color = UI_MOUSEDOWN_BG_COLOR;
	m_color_table[MUI_MOUSEDOWN_BG_COLOR].option = OPTION_UI_MOUSEDOWN_BG_COLOR;
	m_color_table[MUI_MOUSEDOWN_COLOR].color = UI_MOUSEDOWN_COLOR;
	m_color_table[MUI_MOUSEDOWN_COLOR].option = OPTION_UI_MOUSEDOWN_COLOR;
	m_color_table[MUI_MOUSEOVER_BG_COLOR].color = UI_MOUSEOVER_BG_COLOR;
	m_color_table[MUI_MOUSEOVER_BG_COLOR].option = OPTION_UI_MOUSEOVER_BG_COLOR;
	m_color_table[MUI_MOUSEOVER_COLOR].color = UI_MOUSEOVER_COLOR;
	m_color_table[MUI_MOUSEOVER_COLOR].option = OPTION_UI_MOUSEOVER_COLOR;
	m_color_table[MUI_SELECTED_BG_COLOR].color = UI_SELECTED_BG_COLOR;
	m_color_table[MUI_SELECTED_BG_COLOR].option = OPTION_UI_SELECTED_BG_COLOR;
	m_color_table[MUI_SELECTED_COLOR].color = UI_SELECTED_COLOR;
	m_color_table[MUI_SELECTED_COLOR].option = OPTION_UI_SELECTED_COLOR;
	m_color_table[MUI_SLIDER_COLOR].color = UI_SLIDER_COLOR;
	m_color_table[MUI_SLIDER_COLOR].option = OPTION_UI_SLIDER_COLOR;
	m_color_table[MUI_SUBITEM_COLOR].color = UI_SUBITEM_COLOR;
	m_color_table[MUI_SUBITEM_COLOR].option = OPTION_UI_SUBITEM_COLOR;
	m_color_table[MUI_TEXT_BG_COLOR].color = UI_TEXT_BG_COLOR;
	m_color_table[MUI_TEXT_BG_COLOR].option = OPTION_UI_TEXT_BG_COLOR;
	m_color_table[MUI_TEXT_COLOR].color = UI_TEXT_COLOR;
	m_color_table[MUI_TEXT_COLOR].option = OPTION_UI_TEXT_COLOR;
	m_color_table[MUI_UNAVAILABLE_COLOR].color = UI_UNAVAILABLE_COLOR;
	m_color_table[MUI_UNAVAILABLE_COLOR].option = OPTION_UI_UNAVAILABLE_COLOR;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_colors_ui::~ui_menu_colors_ui()
{
	std::string error_string;
	char dec_color[65];

	for (int index = 1; index < MUI_RESTORE; index++)
	{
		sprintf(dec_color, "%x", (UINT32)m_color_table[index].color);
		machine().options().set_value(m_color_table[index].option, dec_color, OPTION_PRIORITY_CMDLINE, error_string);
	}
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_colors_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL && menu_event->iptkey == IPT_UI_SELECT)
	{
		if ((FPTR)menu_event->itemref != MUI_RESTORE)
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_rgb_ui(machine(), container, &m_color_table[(FPTR)menu_event->itemref].color, item[selected].text)));
		else
		{
			changed = true;
			restore_colors();
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_colors_ui::populate()
{
	item_append("Normal text", NULL, 0, (void *)MUI_TEXT_COLOR);
	item_append("Selected m_color", NULL, 0, (void *)MUI_SELECTED_COLOR);
	item_append("Normal text background", NULL, 0, (void *)MUI_TEXT_BG_COLOR);
	item_append("Selected background m_color", NULL, 0, (void *)MUI_SELECTED_BG_COLOR);
	item_append("Subitem m_color", NULL, 0, (void *)MUI_SUBITEM_COLOR);
	item_append("Clone", NULL, 0, (void *)MUI_CLONE_COLOR);
	item_append("Border", NULL, 0, (void *)MUI_BORDER_COLOR);
	item_append("Background", NULL, 0, (void *)MUI_BACKGROUND_COLOR);
	item_append("Dipswitch", NULL, 0, (void *)MUI_DIPSW_COLOR);
	item_append("Unavailable m_color", NULL, 0, (void *)MUI_UNAVAILABLE_COLOR);
	item_append("Slider m_color", NULL, 0, (void *)MUI_SLIDER_COLOR);
	item_append("Gfx viewer background", NULL, 0, (void *)MUI_GFXVIEWER_BG_COLOR);
	item_append("Mouse over m_color", NULL, 0, (void *)MUI_MOUSEOVER_COLOR);
	item_append("Mouse over background m_color", NULL, 0, (void *)MUI_MOUSEOVER_BG_COLOR);
	item_append("Mouse down m_color", NULL, 0, (void *)MUI_MOUSEDOWN_COLOR);
	item_append("Mouse down background m_color", NULL, 0, (void *)MUI_MOUSEDOWN_BG_COLOR);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Restore originals colors", NULL, 0, (void *)MUI_RESTORE);

	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_colors_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	float line_height = machine().ui().get_line_height();

	// top text
	std::string topbuf("UI Colors Settings");

	machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
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
	machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text;
	machine().input().seq_name(ui_select_text, machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	topbuf.assign("Double click or press ").append(ui_select_text.c_str()).append(" to change the m_color value");

	machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// compute maxwidth
	topbuf.assign("Menu Preview");

	machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	maxwidth = width + 2.0f * UI_BOX_LR_BORDER;

	std::string sampletxt[5];

	sampletxt[0].assign("Normal");
	sampletxt[1].assign("Subitem");
	sampletxt[2].assign("Selected");
	sampletxt[3].assign("Mouse Over");
	sampletxt[4].assign("Clone");

	for (int x = 0; x < 5; x++)
	{
		machine().ui().draw_text_full(container, sampletxt[x].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	// compute our bounds for header
	x1 = origx2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = x1 + maxwidth;
	y1 = origy1;
	y2 = y1 + bottom - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// compute our bounds for menu preview
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + 2.0f * UI_BOX_TB_BORDER;
	y2 = y1 + 5.0f * line_height + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, m_color_table[MUI_BACKGROUND_COLOR].color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw normal text
	machine().ui().draw_text_full(container, sampletxt[0].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_TEXT_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, NULL, NULL);
	y1 += line_height;

	// draw subitem text
	machine().ui().draw_text_full(container, sampletxt[1].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_SUBITEM_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, NULL, NULL);
	y1 += line_height;

	// draw selected text
	highlight(container, x1, y1, x2, y1 + line_height, m_color_table[MUI_SELECTED_BG_COLOR].color);
	machine().ui().draw_text_full(container, sampletxt[2].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_SELECTED_COLOR].color, m_color_table[MUI_SELECTED_BG_COLOR].color, NULL, NULL);
	y1 += line_height;

	// draw mouse over text
	highlight(container, x1, y1, x2, y1 + line_height, m_color_table[MUI_MOUSEOVER_BG_COLOR].color);
	machine().ui().draw_text_full(container, sampletxt[3].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_MOUSEOVER_COLOR].color, m_color_table[MUI_MOUSEOVER_BG_COLOR].color, NULL, NULL);
	y1 += line_height;

	// draw clone text
	machine().ui().draw_text_full(container, sampletxt[4].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_CLONE_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, NULL, NULL);

}

//-------------------------------------------------
//  restore original colors
//-------------------------------------------------

void ui_menu_colors_ui::restore_colors()
{
	emu_options options;
	for (int index = 1; index < MUI_RESTORE; index++)
		m_color_table[index].color = rgb_t((UINT32)strtoul(options.value(m_color_table[index].option), NULL, 16));
}

/***************************************************************************
    RGB UI CLASS
***************************************************************************/
//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_rgb_ui::ui_menu_rgb_ui(running_machine &machine, render_container *container, rgb_t *_color, std::string _title) : ui_menu(machine, container)
{
	m_color = _color;
	m_key_active = false;
	m_lock_ref = 0;
	m_title.assign(_title);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_rgb_ui::~ui_menu_rgb_ui()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_rgb_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event;

	if (!m_key_active)
		menu_event = process(UI_MENU_PROCESS_LR_REPEAT);
	else
		menu_event = process(UI_MENU_PROCESS_ONLYCHAR);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		switch ((FPTR)menu_event->itemref)
		{
			case RGB_ALPHA:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->a() > 1)
				{
					m_color->set_a(m_color->a() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->a() < 255)
				{
					m_color->set_a(m_color->a() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_RED:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->r() > 1)
				{
					m_color->set_r(m_color->r() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->r() < 255)
				{
					m_color->set_r(m_color->r() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_GREEN:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->g() > 1)
				{
					m_color->set_g(m_color->g() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->g() < 255)
				{
					m_color->set_g(m_color->g() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_BLUE:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->b() > 1)
				{
					m_color->set_b(m_color->b() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->b() < 255)
				{
					m_color->set_b(m_color->b() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case PALETTE_CHOOSE:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_palette_sel(machine(), container, *m_color)));
				break;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_rgb_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags = MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	std::string text;
	std::string s_text = std::string(m_search).append("_");

	if (m_lock_ref != RGB_ALPHA)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->a());
		strprintf(text, "%3d", m_color->a());
		item_append("Alpha", text.c_str(), arrow_flags, (void *)RGB_ALPHA);
	}
	else
		item_append("Alpha", s_text.c_str(), 0, (void *)RGB_ALPHA);

	if (m_lock_ref != RGB_RED)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->r());
		strprintf(text, "%3d", m_color->r());
		item_append("Red", text.c_str(), arrow_flags, (void *)RGB_RED);
	}
	else
		item_append("Red", s_text.c_str(), 0, (void *)RGB_RED);

	if (m_lock_ref != RGB_GREEN)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->g());
		strprintf(text, "%3d", m_color->g());
		item_append("Green", text.c_str(), arrow_flags, (void *)RGB_GREEN);
	}
	else
		item_append("Green", s_text.c_str(), 0, (void *)RGB_GREEN);

	if (m_lock_ref != RGB_BLUE)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->b());
		strprintf(text, "%3d", m_color->b());
		item_append("Blue", text.c_str(), arrow_flags, (void *)RGB_BLUE);
	}
	else
		item_append("Blue", s_text.c_str(), 0, (void *)RGB_BLUE);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Choose from palette", NULL, 0, (void *)PALETTE_CHOOSE);
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_rgb_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;

	// top text
	std::string topbuf = std::string(m_title).append(" - ARGB Settings");
	machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
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
	machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	std::string sampletxt("Color preview =");
	maxwidth = origx2 - origx1;
	machine().ui().draw_text_full(container, sampletxt.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 = x1 + width;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the normal text
	machine().ui().draw_text_full(container, sampletxt.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);

	float t_x2 = x1 - UI_BOX_LR_BORDER + maxwidth;
	x1 = x2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = t_x2;
	y1 -= UI_BOX_TB_BORDER;

	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, *m_color);

}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void ui_menu_rgb_ui::inkey_special(const ui_menu_event *menu_event)
{
	if (menu_event->iptkey == IPT_UI_SELECT)
	{
		m_key_active = !m_key_active;
		m_lock_ref = (FPTR)menu_event->itemref;

		if (!m_key_active)
		{
			int val = atoi(m_search);
			val = m_color->clamp(val);

			switch ((FPTR)menu_event->itemref)
			{
				case RGB_ALPHA:
					m_color->set_a(val);
					break;

				case RGB_RED:
					m_color->set_r(val);
					break;

				case RGB_GREEN:
					m_color->set_g(val);
					break;

				case RGB_BLUE:
					m_color->set_b(val);
					break;
			}

			m_search[0] = 0;
			m_lock_ref = 0;
			return;
		}
	}

	if (!m_key_active)
	{
		m_search[0] = 0;
		return;
	}

	int buflen = strlen(m_search);

	// if it's a backspace and we can handle it, do so
	if (((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0))
		*(char *)utf8_previous_char(&m_search[buflen]) = 0;

	else if (buflen >= 3)
		return;

	// if it's any other key and we're not maxed out, update
	else if ((menu_event->unichar >= '0' && menu_event->unichar <= '9'))
		buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, menu_event->unichar);

	m_search[buflen] = 0;
}

/***************************************************************************
    PALETTE UI CLASS
***************************************************************************/
static const palcolor m_palette[] = {
	{ "White",  "FFFFFFFF" },
	{ "Silver", "FFC0C0C0" },
	{ "Gray",   "FF808080" },
	{ "Black",  "FF000000" },
	{ "Red",    "FFFF0000" },
	{ "Orange", "FFFFA500" },
	{ "Yellow", "FFFFFF00" },
	{ "Green",  "FF00FF00" },
	{ "Blue",   "FF0000FF" },
	{ "Violet", "FF8F00FF" }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_palette_sel::ui_menu_palette_sel(running_machine &machine, render_container *container, rgb_t &_color) : ui_menu(machine, container),
	m_original(_color)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_palette_sel::~ui_menu_palette_sel()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_palette_sel::handle()
{
	// process the menu
	const ui_menu_event *menu_event = process(MENU_FLAG_MEWUI_PALETTE);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			m_original = rgb_t((UINT32)strtoul(item[selected].subtext, NULL, 16));
			ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
			ui_menu::stack_pop(machine());
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_palette_sel::populate()
{
	for (int x = 0; x < ARRAY_LENGTH(m_palette); ++x)
		item_append(m_palette[x].name, m_palette[x].argb, MENU_FLAG_MEWUI_PALETTE, (void *)(FPTR)(x + 1));

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_palette_sel::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
}
