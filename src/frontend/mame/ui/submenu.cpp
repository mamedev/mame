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
#include <iterator>

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_submenu::ui_submenu(running_machine &machine, render_container *container, std::vector<ui_submenu::option> &suboptions, const game_driver *drv, emu_options *options)
	: ui_menu(machine, container)
	, m_options(suboptions)
	, m_driver(drv)
{
	core_options *opts = nullptr;
	if (m_driver == nullptr)
		opts = dynamic_cast<core_options*>(&machine.options());
	else
		opts = dynamic_cast<core_options*>(options);

	for (auto & sm_option : m_options)
	{
		switch (sm_option.type)
		{
			case ui_submenu::EMU:
				sm_option.entry = opts->get_entry(sm_option.name);
				sm_option.options = opts;
				if (sm_option.entry->type() == OPTION_STRING)
				{
					sm_option.value.clear();
					std::string namestr(sm_option.entry->description());
					int lparen = namestr.find_first_of('(', 0);
					int vslash = namestr.find_first_of('|', lparen + 1);
					int rparen = namestr.find_first_of(')', vslash + 1);
					if (lparen != -1 && vslash != -1 && rparen != -1)
					{
						int semi;
						namestr.erase(rparen);
						namestr.erase(0, lparen + 1);
						while ((semi = namestr.find_first_of('|')) != -1)
						{
							sm_option.value.emplace_back(namestr.substr(0, semi));
							namestr.erase(0, semi + 1);
						}
						sm_option.value.emplace_back(namestr);
					}
				}
				break;
			case ui_submenu::OSD:
				sm_option.entry = opts->get_entry(sm_option.name);
				sm_option.options = opts;
				if (sm_option.entry->type() == OPTION_STRING)
				{
					sm_option.value.clear();
					std::string descr(sm_option.entry->description()), delim(", ");
					descr.erase(0, descr.find(":") + 2);
					size_t p1, p2 = 0;
					while ((p1 = descr.find_first_not_of(delim, p2)) != std::string::npos)
					{
						p2 = descr.find_first_of(delim, p1 + 1);
						if (p2 != std::string::npos)
						{
							std::string txt(descr.substr(p1, p2 - p1));
							if (txt != "or")
								sm_option.value.push_back(txt);
						}
						else
						{
							sm_option.value.push_back(descr.substr(p1));
							break;
						}
					}
				}
				break;
			case ui_submenu::UI:
				sm_option.entry = mame_machine_manager::instance()->ui().options().get_entry(sm_option.name);
				sm_option.options = dynamic_cast<core_options*>(&mame_machine_manager::instance()->ui().options());
				break;
			default:
				continue;
				break;
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
					case OPTION_STRING:
						if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
						{
							changed = true;
							std::string v_cur(sm_option->entry->value());
							int cur_value = std::distance(sm_option->value.begin(), std::find(sm_option->value.begin(), sm_option->value.end(), v_cur));
							if (m_event->iptkey == IPT_UI_LEFT)
								v_cur = sm_option->value[--cur_value];
							else
								v_cur = sm_option->value[++cur_value];
							sm_option->options->set_value(sm_option->name, v_cur.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
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
				item_append(ui_menu_item_type::SEPARATOR);
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
					case OPTION_STRING:
					{
						std::string v_cur(sm_option->entry->value());
						int cur_value = std::distance(sm_option->value.begin(), std::find(sm_option->value.begin(), sm_option->value.end(), v_cur));
						arrow_flags = get_arrow_flags(0, sm_option->value.size() - 1, cur_value);
						item_append(_(sm_option->description),
							sm_option->options->value(sm_option->name),
							arrow_flags, static_cast<void*>(&(*sm_option)));
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

	item_append(ui_menu_item_type::SEPARATOR);
	custombottom = customtop = mame_machine_manager::instance()->ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_submenu::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	mame_ui_manager &mui = mame_machine_manager::instance()->ui();

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
