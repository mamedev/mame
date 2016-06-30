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
#include "ui/menuitem.h"

#include <limits>

namespace ui {
std::vector<submenu::option> submenu::misc_options = {
	{ submenu::option_type::HEAD, __("Miscellaneous Options") },
	{ submenu::option_type::UI,   __("Re-select last machine played"),           OPTION_REMEMBER_LAST },
	{ submenu::option_type::UI,   __("Enlarge images in the right panel"),       OPTION_ENLARGE_SNAPS },
	{ submenu::option_type::UI,   __("DATs info"),                               OPTION_DATS_ENABLED },
	{ submenu::option_type::EMU,  __("Cheats"),                                  OPTION_CHEAT },
	{ submenu::option_type::EMU,  __("Show mouse pointer"),                      OPTION_UI_MOUSE },
	{ submenu::option_type::EMU,  __("Confirm quit from machines"),              OPTION_CONFIRM_QUIT },
	{ submenu::option_type::EMU,  __("Skip information screen at startup"),      OPTION_SKIP_GAMEINFO },
	{ submenu::option_type::UI,   __("Force 4:3 aspect for snapshot display"),   OPTION_FORCED4X3 },
	{ submenu::option_type::UI,   __("Use image as background"),                 OPTION_USE_BACKGROUND },
	{ submenu::option_type::UI,   __("Skip bios selection menu"),                OPTION_SKIP_BIOS_MENU },
	{ submenu::option_type::UI,   __("Skip software parts selection menu"),      OPTION_SKIP_PARTS_MENU },
	{ submenu::option_type::UI,   __("Info auto audit"),                         OPTION_INFO_AUTO_AUDIT },
};

std::vector<submenu::option> submenu::advanced_options = {
	{ submenu::option_type::HEAD, __("Advanced Options") },
	{ submenu::option_type::HEAD, __("Performance Options") },
	{ submenu::option_type::EMU,  __("Auto frame skip"),                         OPTION_AUTOFRAMESKIP },
	{ submenu::option_type::EMU,  __("Frame skip"),                              OPTION_FRAMESKIP },
	{ submenu::option_type::EMU,  __("Throttle"),                                OPTION_THROTTLE },
	{ submenu::option_type::EMU,  __("Sleep"),                                   OPTION_SLEEP },
	{ submenu::option_type::EMU,  __("Speed"),                                   OPTION_SPEED },
	{ submenu::option_type::EMU,  __("Refresh speed"),                           OPTION_REFRESHSPEED },

	{ submenu::option_type::HEAD, __("Rotation Options") },
	{ submenu::option_type::EMU,  __("Rotate"),                                  OPTION_ROTATE },
	{ submenu::option_type::EMU,  __("Rotate right"),                            OPTION_ROR },
	{ submenu::option_type::EMU,  __("Rotate left"),                             OPTION_ROL },
	{ submenu::option_type::EMU,  __("Auto rotate right"),                       OPTION_AUTOROR },
	{ submenu::option_type::EMU,  __("Auto rotate left"),                        OPTION_AUTOROL },
	{ submenu::option_type::EMU,  __("Flip X"),                                  OPTION_FLIPX },
	{ submenu::option_type::EMU,  __("Flip Y"),                                  OPTION_FLIPY },

	{ submenu::option_type::HEAD, __("Artwork Options") },
	{ submenu::option_type::EMU,  __("Artwork Crop"),                            OPTION_ARTWORK_CROP },
	{ submenu::option_type::EMU,  __("Use Backdrops"),                           OPTION_USE_BACKDROPS },
	{ submenu::option_type::EMU,  __("Use Overlays"),                            OPTION_USE_OVERLAYS },
	{ submenu::option_type::EMU,  __("Use Bezels"),                              OPTION_USE_BEZELS },
	{ submenu::option_type::EMU,  __("Use Control Panels"),                      OPTION_USE_CPANELS },
	{ submenu::option_type::EMU,  __("Use Marquees"),                            OPTION_USE_MARQUEES },

	{ submenu::option_type::HEAD, __("State/Playback Options") },
	{ submenu::option_type::EMU,  __("Automatic save/restore"),                  OPTION_AUTOSAVE },
	{ submenu::option_type::EMU,  __("Bilinear snapshot"),                       OPTION_SNAPBILINEAR },
	{ submenu::option_type::EMU,  __("Burn-in"),                                 OPTION_BURNIN },

	{ submenu::option_type::HEAD, __("Input Options") },
	{ submenu::option_type::EMU,  __("Coin lockout"),                            OPTION_COIN_LOCKOUT },
	{ submenu::option_type::EMU,  __("Mouse"),                                   OPTION_MOUSE },
	{ submenu::option_type::EMU,  __("Joystick"),                                OPTION_JOYSTICK },
	{ submenu::option_type::EMU,  __("Lightgun"),                                OPTION_LIGHTGUN },
	{ submenu::option_type::EMU,  __("Multi-keyboard"),                          OPTION_MULTIKEYBOARD },
	{ submenu::option_type::EMU,  __("Multi-mouse"),                             OPTION_MULTIMOUSE },
	{ submenu::option_type::EMU,  __("Steadykey"),                               OPTION_STEADYKEY },
	{ submenu::option_type::EMU,  __("UI active"),                               OPTION_UI_ACTIVE },
	{ submenu::option_type::EMU,  __("Offscreen reload"),                        OPTION_OFFSCREEN_RELOAD },
	{ submenu::option_type::EMU,  __("Joystick deadzone"),                       OPTION_JOYSTICK_DEADZONE },
	{ submenu::option_type::EMU,  __("Joystick saturation"),                     OPTION_JOYSTICK_SATURATION },
	{ submenu::option_type::EMU,  __("Natural keyboard"),                        OPTION_NATURAL_KEYBOARD },
	{ submenu::option_type::EMU,  __("Simultaneous contradictory"),              OPTION_JOYSTICK_CONTRADICTORY },
	{ submenu::option_type::EMU,  __("Coin impulse"),                            OPTION_COIN_IMPULSE },
};

std::vector<submenu::option> submenu::control_options = {
	{ submenu::option_type::HEAD, __("Device Mapping") },
	{ submenu::option_type::EMU,  __("Lightgun Device Assignment"),              OPTION_LIGHTGUN_DEVICE },
	{ submenu::option_type::EMU,  __("Trackball Device Assignment"),             OPTION_TRACKBALL_DEVICE },
	{ submenu::option_type::EMU,  __("Pedal Device Assignment"),                 OPTION_PEDAL_DEVICE },
	{ submenu::option_type::EMU,  __("Adstick Device Assignment"),               OPTION_ADSTICK_DEVICE },
	{ submenu::option_type::EMU,  __("Paddle Device Assignment"),                OPTION_PADDLE_DEVICE },
	{ submenu::option_type::EMU,  __("Dial Device Assignment"),                  OPTION_DIAL_DEVICE },
	{ submenu::option_type::EMU,  __("Positional Device Assignment"),            OPTION_POSITIONAL_DEVICE },
	{ submenu::option_type::EMU,  __("Mouse Device Assignment"),                 OPTION_MOUSE_DEVICE }
};

std::vector<submenu::option> submenu::video_options = {
	{ submenu::option_type::HEAD, __("Video Options") },
	{ submenu::option_type::OSD,  __("Video Mode"),                              OSDOPTION_VIDEO },
	{ submenu::option_type::OSD,  __("Number Of Screens"),                       OSDOPTION_NUMSCREENS },
#if defined(UI_WINDOWS) && !defined(UI_SDL)
	{ submenu::option_type::OSD,  __("Triple Buffering"),                        WINOPTION_TRIPLEBUFFER },
	{ submenu::option_type::OSD,  __("HLSL"),                                    WINOPTION_HLSL_ENABLE },
#endif
	{ submenu::option_type::OSD,  __("GLSL"),                                    OSDOPTION_GL_GLSL },
	{ submenu::option_type::OSD,  __("Bilinear Filtering"),                      OSDOPTION_FILTER },
	{ submenu::option_type::OSD,  __("Bitmap Prescaling"),                       OSDOPTION_PRESCALE },
	{ submenu::option_type::OSD,  __("Window Mode"),                             OSDOPTION_WINDOW },
	{ submenu::option_type::EMU,  __("Enforce Aspect Ratio"),                    OPTION_KEEPASPECT },
	{ submenu::option_type::OSD,  __("Start Out Maximized"),                     OSDOPTION_MAXIMIZE },
	{ submenu::option_type::OSD,  __("Synchronized Refresh"),                    OSDOPTION_SYNCREFRESH },
	{ submenu::option_type::OSD,  __("Wait Vertical Sync"),                      OSDOPTION_WAITVSYNC }
};

//std::vector<submenu::option> submenu::export_options = {
//  { ui_submenu::option_type::COMMAND, __("Export XML format (like -listxml)"),               "exportxml" },
//  { ui_submenu::option_type::COMMAND, __("Export TXT format (like -listfull)"),              "exporttxt" },
//};


//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

submenu::submenu(mame_ui_manager &mui, render_container *container, std::vector<option> &suboptions, const game_driver *drv, emu_options *options)
	: menu(mui, container)
	, m_options(suboptions)
	, m_driver(drv)
{
	core_options *opts = nullptr;
	if (m_driver == nullptr)
		opts = dynamic_cast<core_options*>(&mui.machine().options());
	else
		opts = dynamic_cast<core_options*>(options);

	for (auto & sm_option : m_options)
	{
		switch (sm_option.type)
		{
			case option_type::EMU:
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
			case option_type::OSD:
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
			case option_type::UI:
				sm_option.entry = mui.options().get_entry(sm_option.name);
				sm_option.options = dynamic_cast<core_options*>(&mui.options());
				break;
			default:
				break;
		}
	}
}

submenu::~submenu()
{
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void submenu::handle()
{
	bool changed = false;
	std::string error_string, tmptxt;
	float f_cur, f_step;

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);

	if (menu_event != nullptr && menu_event->itemref != nullptr &&
			(menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT))
	{
		option &sm_option = *reinterpret_cast<option *>(menu_event->itemref);

		switch (sm_option.type)
		{
		case option_type::EMU:
		case option_type::UI:
		case option_type::OSD:
			switch (sm_option.entry->type())
			{
			case OPTION_BOOLEAN:
				changed = true;
				sm_option.options->set_value(sm_option.name, !strcmp(sm_option.entry->value(),"1") ? "0" : "1", OPTION_PRIORITY_CMDLINE, error_string);
				sm_option.entry->mark_changed();
				break;
			case OPTION_INTEGER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					int i_cur = atoi(sm_option.entry->value());
					(menu_event->iptkey == IPT_UI_LEFT) ? i_cur-- : i_cur++;
					sm_option.options->set_value(sm_option.name, i_cur, OPTION_PRIORITY_CMDLINE, error_string);
					sm_option.entry->mark_changed();
				}
				break;
			case OPTION_FLOAT:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					f_cur = atof(sm_option.entry->value());
					if (sm_option.entry->has_range())
					{
						f_step = atof(sm_option.entry->minimum());
						if (f_step <= 0.0f) {
							int pmin = getprecisionchr(sm_option.entry->minimum());
							int pmax = getprecisionchr(sm_option.entry->maximum());
							tmptxt = '1' + std::string((pmin > pmax) ? pmin : pmax, '0');
							f_step = 1 / atof(tmptxt.c_str());
						}
					}
					else
					{
						int precision = getprecisionchr(sm_option.entry->default_value());
						tmptxt = '1' + std::string(precision, '0');
						f_step = 1 / atof(tmptxt.c_str());
					}
					if (menu_event->iptkey == IPT_UI_LEFT)
						f_cur -= f_step;
					else
						f_cur += f_step;
					tmptxt = string_format("%g", f_cur);
					sm_option.options->set_value(sm_option.name, tmptxt.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					sm_option.entry->mark_changed();
				}
				break;
			case OPTION_STRING:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					std::string v_cur(sm_option.entry->value());
					int cur_value = std::distance(sm_option.value.begin(), std::find(sm_option.value.begin(), sm_option.value.end(), v_cur));
					if (menu_event->iptkey == IPT_UI_LEFT)
						v_cur = sm_option.value[--cur_value];
					else
						v_cur = sm_option.value[++cur_value];
					sm_option.options->set_value(sm_option.name, v_cur.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					sm_option.entry->mark_changed();
				}
				break;
			}
			break;
		default:
			osd_printf_error("Unhandled option: %s", _(sm_option.description));
			break;
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void submenu::populate()
{
	UINT32 arrow_flags;

	// add options
	for (auto sm_option = m_options.begin(); sm_option < m_options.end(); ++sm_option)
	{
		// skip first heading (is menu title)
		if (sm_option == m_options.begin() && sm_option->type == option_type::HEAD) continue;

		switch (sm_option->type)
		{
		case option_type::HEAD:
			item_append(_(sm_option->description), nullptr, FLAG_DISABLE | FLAG_UI_HEADING, nullptr);
			break;
		case option_type::SEP:
			item_append(menu_item_type::SEPARATOR);
			break;
		case option_type::CMD:
			item_append(_(sm_option->description), nullptr, 0, static_cast<void*>(&(*sm_option)));
			break;
		case option_type::EMU:
		case option_type::UI:
		case option_type::OSD:
			switch (sm_option->entry->type())
			{
			case OPTION_BOOLEAN:
				arrow_flags = sm_option->options->bool_value(sm_option->name) ? FLAG_RIGHT_ARROW : FLAG_LEFT_ARROW;
				item_append(_(sm_option->description),
					(arrow_flags == FLAG_RIGHT_ARROW) ? "On" : "Off",
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
				}
				break;
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
				}
				break;
			case OPTION_STRING:
				{
					std::string v_cur(sm_option->entry->value());
					int cur_value = std::distance(sm_option->value.begin(), std::find(sm_option->value.begin(), sm_option->value.end(), v_cur));
					arrow_flags = get_arrow_flags(0, sm_option->value.size() - 1, cur_value);
					item_append(_(sm_option->description),
						sm_option->options->value(sm_option->name),
						arrow_flags, static_cast<void*>(&(*sm_option)));
				}
				break;
			default:
				arrow_flags = FLAG_RIGHT_ARROW;
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

	item_append(menu_item_type::SEPARATOR);
	custombottom = customtop = ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void submenu::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	ui().draw_text_full(container, _(m_options[0].description), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, _(m_options[0].description), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	if (selectedref != nullptr)
	{
		option &selected_sm_option = *reinterpret_cast<option *>(selectedref);
		if (selected_sm_option.entry != nullptr)
		{
			ui().draw_text_full(container, selected_sm_option.entry->description(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
					mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);

			width += 2 * UI_BOX_LR_BORDER;
			maxwidth = MAX(origx2 - origx1, width);

			// compute our bounds
			x1 = 0.5f - 0.5f * maxwidth;
			x2 = x1 + maxwidth;
			y1 = origy2 + UI_BOX_TB_BORDER;
			y2 = origy2 + bottom;

			// draw a box
			ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

			// take off the borders
			x1 += UI_BOX_LR_BORDER;
			x2 -= UI_BOX_LR_BORDER;
			y1 += UI_BOX_TB_BORDER;

			// draw the text within it
			ui().draw_text_full(container, selected_sm_option.entry->description(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		}
	}
}

} // namespace ui
