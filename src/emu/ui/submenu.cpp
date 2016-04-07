// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota,Jeffrey Clark
/***************************************************************************

    ui/submenu.cpp

    UI options menu

***************************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/submenu.h"
#include "ui/utils.h"
#include <limits>

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_submenu::ui_submenu(running_machine &machine, render_container *container, std::vector<ui_submenu::option> &suboptions)
	: ui_menu(machine, container),
	m_options(suboptions)
{
	for (auto & sm_option : m_options)
	{
		if (sm_option.type < ui_submenu::EMU)
			continue;

		// fixme use switch
		sm_option.entry = machine.options().get_entry(sm_option.name);
		if (sm_option.entry == nullptr)
		{
			sm_option.entry = machine.ui().options().get_entry(sm_option.name);
			sm_option.options = dynamic_cast<core_options*>(&machine.ui().options());
		}
		else
		{
			sm_option.options = dynamic_cast<core_options*>(&machine.options());
		}
	}
}

ui_submenu::~ui_submenu()
{
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void ui_submenu::handle()
{
	bool changed = false;
	std::string error_string, tmptxt;
	float f_cur, f_step;

	// process the menu
	const ui_menu_event *m_event = process(UI_MENU_PROCESS_LR_REPEAT);

	if (m_event != nullptr && m_event->itemref != nullptr &&
			(m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT || m_event->iptkey == IPT_UI_SELECT))
	{
		ui_submenu::option *sm_option = (ui_submenu::option *)m_event->itemref;

		switch (sm_option->type)
		{
			case ui_submenu::EMU:
			case ui_submenu::UI:
			case ui_submenu::OSD:
				switch (sm_option->entry->type())
				{
					case OPTION_BOOLEAN:
						changed = true;
						sm_option->options->set_value(sm_option->name, !strcmp(sm_option->entry->value(),"1") ? "0" : "1", OPTION_PRIORITY_CMDLINE, error_string);
						sm_option->entry->mark_changed();
						break;
					case OPTION_INTEGER:
						if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
						{
							changed = true;
							int i_cur = atoi(sm_option->entry->value());
							(m_event->iptkey == IPT_UI_LEFT) ? i_cur-- : i_cur++;
							sm_option->options->set_value(sm_option->name, i_cur, OPTION_PRIORITY_CMDLINE, error_string);
							sm_option->entry->mark_changed();
						}
						break;
					case OPTION_FLOAT:
						if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
						{
							changed = true;
							f_cur = atof(sm_option->entry->value());
							if (sm_option->entry->has_range())
							{
								f_step = atof(sm_option->entry->minimum());
								if (f_step <= 0.0f) {
									int pmin = getprecisionchr(sm_option->entry->minimum());
									int pmax = getprecisionchr(sm_option->entry->maximum());
									tmptxt = '1' + std::string((pmin > pmax) ? pmin : pmax, '0');
									f_step = 1 / atof(tmptxt.c_str());
								}
							}
							else
							{
								int precision = getprecisionchr(sm_option->entry->default_value());
								tmptxt = '1' + std::string(precision, '0');
								f_step = 1 / atof(tmptxt.c_str());
							}
							if (m_event->iptkey == IPT_UI_LEFT)
								f_cur -= f_step;
							else
								f_cur += f_step;
							tmptxt = string_format("%g", f_cur);
							sm_option->options->set_value(sm_option->name, tmptxt.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
							sm_option->entry->mark_changed();
						}
						break;
				}
				break;
			default:
				osd_printf_error("Unhandled option: %s", _(sm_option->description));
				break;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_submenu::populate()
{
	UINT32 arrow_flags;

	// add options
	for (auto sm_option = m_options.begin(); sm_option < m_options.end(); sm_option++)
	{
		// skip first heading (is menu title)
		if (sm_option == m_options.begin() && sm_option->type == ui_submenu::HEAD) continue;

		switch (sm_option->type)
		{
			case ui_submenu::HEAD:
				item_append(_(sm_option->description), nullptr, MENU_FLAG_DISABLE | MENU_FLAG_UI_HEADING, nullptr);
				break;
			case ui_submenu::SEP:
				item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
				break;
			case ui_submenu::CMD:
				item_append(_(sm_option->description), nullptr, 0, static_cast<void*>(&(*sm_option)));
				break;
			case ui_submenu::EMU:
			case ui_submenu::UI:
			case ui_submenu::OSD:
				switch (sm_option->entry->type())
				{
					case OPTION_BOOLEAN:
						arrow_flags = sm_option->options->bool_value(sm_option->name) ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW;
						item_append(_(sm_option->description),
							(arrow_flags == MENU_FLAG_RIGHT_ARROW) ? "On" : "Off",
							arrow_flags,
							static_cast<void*>(&(*sm_option)));
						break;
					case OPTION_INTEGER:
					{
						int i_min, i_max;
						int i_cur = atoi(sm_option->entry->value());
						if (sm_option->entry->has_range())
						{
							i_min = atoi(sm_option->entry->minimum());
							i_max = atoi(sm_option->entry->maximum());
						}
						else
						{
							i_min = std::numeric_limits<int>::min();
							i_max = std::numeric_limits<int>::max();
						}
						arrow_flags = get_arrow_flags(i_min, i_max, i_cur);
						item_append(_(sm_option->description),
							sm_option->entry->value(),
							arrow_flags,
							static_cast<void*>(&(*sm_option)));
						break;
					}
					case OPTION_FLOAT:
					{
						float f_min, f_max;
						float f_cur = atof(sm_option->entry->value());
						if (sm_option->entry->has_range())
						{
							f_min = atof(sm_option->entry->minimum());
							f_max = atof(sm_option->entry->maximum());
						}
						else
						{
							f_min = 0.0f;
							f_max = std::numeric_limits<float>::max();
						}
						arrow_flags = get_arrow_flags(f_min, f_max, f_cur);
						std::string tmptxt = string_format("%g", f_cur);
						item_append(_(sm_option->description),
							tmptxt.c_str(),
							arrow_flags,
							static_cast<void*>(&(*sm_option)));
						break;
					}
					default:
						arrow_flags = MENU_FLAG_RIGHT_ARROW;
						item_append(_(sm_option->description),
							sm_option->options->value(sm_option->name),
							arrow_flags, static_cast<void*>(&(*sm_option)));
						break;
				}
				break;
			default:
				osd_printf_error("Unknown option type: %s", _(sm_option->description));
				break;
		}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	custombottom = customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_submenu::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	mui.draw_text_full(container, _(m_options[0].description), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

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
	mui.draw_text_full(container, _(m_options[0].description), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	if (selectedref != nullptr)
	{
		ui_submenu::option *selected_sm_option = (ui_submenu::option *)selectedref;

		if (selected_sm_option->entry != nullptr)
		{
			mui.draw_text_full(container, selected_sm_option->entry->description(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
					DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);

			width += 2 * UI_BOX_LR_BORDER;
			maxwidth = MAX(origx2 - origx1, width);

			// compute our bounds
			x1 = 0.5f - 0.5f * maxwidth;
			x2 = x1 + maxwidth;
			y1 = origy2 + UI_BOX_TB_BORDER;
			y2 = origy2 + bottom;

			// draw a box
			mui.draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

			// take off the borders
			x1 += UI_BOX_LR_BORDER;
			x2 -= UI_BOX_LR_BORDER;
			y1 += UI_BOX_TB_BORDER;

			// draw the text within it
			mui.draw_text_full(container, selected_sm_option->entry->description(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
					DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		}
	}
}
