// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota,Jeffrey Clark
/***************************************************************************

    ui/submenu.cpp

    UI options menu

***************************************************************************/

#include "emu.h"
#include "ui/submenu.h"

#include "ui/menuitem.h"
#include "ui/ui.h"
#include "ui/utils.h"

#if defined(UI_WINDOWS) && !defined(UI_SDL)
#include "../osd/windows/winmain.h"
#else
#include "../osd/modules/lib/osdobj_common.h"
#endif
#include "../osd/modules/input/input_module.h"

#include <limits>


namespace ui {

std::vector<submenu::option> submenu::misc_options()
{
	return std::vector<option>{
			{ option_type::HEAD, N_("Miscellaneous Options") },
			{ option_type::UI,   N_("Skip imperfect emulation warnings"),       OPTION_SKIP_WARNINGS },
			{ option_type::UI,   N_("Re-select last system launched"),          OPTION_REMEMBER_LAST },
			{ option_type::UI,   N_("Enlarge images in the right panel"),       OPTION_ENLARGE_SNAPS },
			{ option_type::EMU,  N_("Cheats"),                                  OPTION_CHEAT },
			{ option_type::EMU,  N_("Show mouse pointer"),                      OPTION_UI_MOUSE },
			{ option_type::EMU,  N_("Confirm quit from emulation"),             OPTION_CONFIRM_QUIT },
			{ option_type::EMU,  N_("Skip system information screen"),          OPTION_SKIP_GAMEINFO },
			{ option_type::UI,   N_("Force 4:3 aspect for snapshot display"),   OPTION_FORCED4X3 },
			{ option_type::UI,   N_("Use image as background"),                 OPTION_USE_BACKGROUND },
			{ option_type::UI,   N_("Skip BIOS selection menu"),                OPTION_SKIP_BIOS_MENU },
			{ option_type::UI,   N_("Skip software part selection menu"),       OPTION_SKIP_PARTS_MENU },
			{ option_type::UI,   N_("Info auto audit"),                         OPTION_INFO_AUTO_AUDIT },
			{ option_type::UI,   N_("Hide romless machine from available list"),OPTION_HIDE_ROMLESS },
			{ option_type::EMU,  N_("Pause game on menu"),                      OPTION_PAUSE_GAME_ON_MENU }};
}

std::vector<submenu::option> submenu::advanced_options()
{
	return std::vector<option>{
			{ option_type::HEAD, N_("Advanced Options") },
			{ option_type::HEAD, N_("Performance Options") },
			{ option_type::EMU,  N_("Auto frame skip"),                         OPTION_AUTOFRAMESKIP },
			{ option_type::EMU,  N_("Frame skip"),                              OPTION_FRAMESKIP },
			{ option_type::EMU,  N_("Throttle"),                                OPTION_THROTTLE },
			{ option_type::UI,   N_("Mute when unthrottled"),                   OPTION_UNTHROTTLE_MUTE },
			{ option_type::EMU,  N_("Sleep"),                                   OPTION_SLEEP },
			{ option_type::EMU,  N_("Speed"),                                   OPTION_SPEED },
			{ option_type::EMU,  N_("Adjust speed to match refresh rate"),      OPTION_REFRESHSPEED },
			{ option_type::EMU,  N_("Low latency"),                             OPTION_LOWLATENCY },

			{ option_type::HEAD, N_("Rotation Options") },
			{ option_type::EMU,  N_("Rotate"),                                  OPTION_ROTATE },
			{ option_type::EMU,  N_("Rotate right"),                            OPTION_ROR },
			{ option_type::EMU,  N_("Rotate left"),                             OPTION_ROL },
			{ option_type::EMU,  N_("Auto rotate right"),                       OPTION_AUTOROR },
			{ option_type::EMU,  N_("Auto rotate left"),                        OPTION_AUTOROL },
			{ option_type::EMU,  N_("Flip X"),                                  OPTION_FLIPX },
			{ option_type::EMU,  N_("Flip Y"),                                  OPTION_FLIPY },

			{ option_type::HEAD, N_("Artwork Options") },
			{ option_type::EMU,  N_("Zoom to screen area"),                     OPTION_ARTWORK_CROP },

			{ option_type::HEAD, N_("State/Playback Options") },
			{ option_type::EMU,  N_("Automatic save/restore"),                  OPTION_AUTOSAVE },
			{ option_type::EMU,  N_("Allow rewind"),                            OPTION_REWIND },
			{ option_type::EMU,  N_("Rewind capacity"),                         OPTION_REWIND_CAPACITY },
			{ option_type::EMU,  N_("Bilinear filtering for snapshots"),        OPTION_SNAPBILINEAR },
			{ option_type::EMU,  N_("Burn-in"),                                 OPTION_BURNIN },

			{ option_type::HEAD, N_("Input Options") },
			{ option_type::EMU,  N_("Coin lockout"),                            OPTION_COIN_LOCKOUT },
			{ option_type::EMU,  N_("Mouse"),                                   OPTION_MOUSE },
			{ option_type::EMU,  N_("Joystick"),                                OPTION_JOYSTICK },
			{ option_type::EMU,  N_("Lightgun"),                                OPTION_LIGHTGUN },
			{ option_type::EMU,  N_("Multi-keyboard"),                          OPTION_MULTIKEYBOARD },
			{ option_type::EMU,  N_("Multi-mouse"),                             OPTION_MULTIMOUSE },
			{ option_type::EMU,  N_("Steadykey"),                               OPTION_STEADYKEY },
			{ option_type::EMU,  N_("UI active"),                               OPTION_UI_ACTIVE },
			{ option_type::EMU,  N_("Off-screen reload"),                       OPTION_OFFSCREEN_RELOAD },
			{ option_type::EMU,  N_("Joystick deadzone"),                       OPTION_JOYSTICK_DEADZONE },
			{ option_type::EMU,  N_("Joystick saturation"),                     OPTION_JOYSTICK_SATURATION },
			{ option_type::EMU,  N_("Joystick threshold"),                      OPTION_JOYSTICK_THRESHOLD },
			{ option_type::EMU,  N_("Natural keyboard"),                        OPTION_NATURAL_KEYBOARD },
			{ option_type::EMU,  N_("Allow contradictory joystick inputs"),     OPTION_JOYSTICK_CONTRADICTORY },
			{ option_type::EMU,  N_("Coin impulse"),                            OPTION_COIN_IMPULSE } };
}

std::vector<submenu::option> submenu::control_options()
{
	return std::vector<option>{
			{ option_type::HEAD, N_("Input Device Options") },
			{ option_type::EMU,  N_("Lightgun Device Assignment"),              OPTION_LIGHTGUN_DEVICE },
			{ option_type::EMU,  N_("Trackball Device Assignment"),             OPTION_TRACKBALL_DEVICE },
			{ option_type::EMU,  N_("Pedal Device Assignment"),                 OPTION_PEDAL_DEVICE },
			{ option_type::EMU,  N_("AD Stick Device Assignment"),              OPTION_ADSTICK_DEVICE },
			{ option_type::EMU,  N_("Paddle Device Assignment"),                OPTION_PADDLE_DEVICE },
			{ option_type::EMU,  N_("Dial Device Assignment"),                  OPTION_DIAL_DEVICE },
			{ option_type::EMU,  N_("Positional Device Assignment"),            OPTION_POSITIONAL_DEVICE },
			{ option_type::EMU,  N_("Mouse Device Assignment"),                 OPTION_MOUSE_DEVICE },
			{ option_type::SEP },
			{ option_type::OSD,  N_("Keyboard Input Provider"),                 OSD_KEYBOARDINPUT_PROVIDER },
			{ option_type::OSD,  N_("Mouse Input Provider"),                    OSD_MOUSEINPUT_PROVIDER },
			{ option_type::OSD,  N_("Lightgun Input Provider"),                 OSD_LIGHTGUNINPUT_PROVIDER },
			{ option_type::OSD,  N_("Joystick Input Provider"),                 OSD_JOYSTICKINPUT_PROVIDER } };
}

std::vector<submenu::option> submenu::video_options()
{
	return std::vector<option>{
			{ option_type::HEAD, N_("Video Options") },
			{ option_type::OSD,  N_("Video Mode"),                              OSDOPTION_VIDEO },
			{ option_type::OSD,  N_("Number Of Screens"),                       OSDOPTION_NUMSCREENS },
#if defined(UI_WINDOWS) && !defined(UI_SDL)
			{ option_type::OSD,  N_("Triple Buffering"),                        WINOPTION_TRIPLEBUFFER },
			{ option_type::OSD,  N_("HLSL"),                                    WINOPTION_HLSL_ENABLE },
#endif
			{ option_type::OSD,  N_("GLSL"),                                    OSDOPTION_GL_GLSL },
			{ option_type::OSD,  N_("Bilinear Filtering"),                      OSDOPTION_FILTER },
			{ option_type::OSD,  N_("Bitmap Prescaling"),                       OSDOPTION_PRESCALE },
			{ option_type::OSD,  N_("Window Mode"),                             OSDOPTION_WINDOW },
			{ option_type::EMU,  N_("Enforce Aspect Ratio"),                    OPTION_KEEPASPECT },
			{ option_type::OSD,  N_("Start Out Maximized"),                     OSDOPTION_MAXIMIZE },
			{ option_type::OSD,  N_("Synchronized Refresh"),                    OSDOPTION_SYNCREFRESH },
			{ option_type::OSD,  N_("Wait Vertical Sync"),                      OSDOPTION_WAITVSYNC } };
}

//std::vector<submenu::option> submenu::export_options()
//{
//  return std::vector<option>{
//          { option_type::COMMAND, N_("Export XML format (like -listxml)"),               "exportxml" },
//          { option_type::COMMAND, N_("Export TXT format (like -listfull)"),              "exporttxt" } };
//}


//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

submenu::submenu(mame_ui_manager &mui, render_container &container, std::vector<option> const &suboptions, const game_driver *drv, emu_options *options)
	: submenu(mui, container, std::vector<option>(suboptions), drv, options)
{
}

submenu::submenu(mame_ui_manager &mui, render_container &container, std::vector<option> &&suboptions, const game_driver *drv, emu_options *options)
	: menu(mui, container)
	, m_options(std::move(suboptions))
	, m_driver(drv)
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_(m_options[0].description));

	core_options *opts = nullptr;
	if (m_driver == nullptr)
		opts = dynamic_cast<core_options *>(&mui.machine().options());
	else
		opts = dynamic_cast<core_options *>(options);

	for (option &sm_option : m_options)
	{
		switch (sm_option.type)
		{
		case option_type::EMU:
			sm_option.entry = opts->get_entry(sm_option.name);
			sm_option.options = opts;
			if ((sm_option.entry->type() == core_options::option_type::STRING) || (sm_option.entry->type() == core_options::option_type::PATH) || (sm_option.entry->type() == core_options::option_type::MULTIPATH))
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
			if ((sm_option.entry->type() == core_options::option_type::STRING) || (sm_option.entry->type() == core_options::option_type::PATH) || (sm_option.entry->type() == core_options::option_type::MULTIPATH))
			{
				sm_option.value.clear();
				std::string descr(machine().options().get_entry(sm_option.name)->description()), delim(", ");
				descr.erase(0, descr.find(":") + 2);

				std::string default_value(sm_option.entry->default_value());
				std::string auto_value(OSDOPTVAL_AUTO);
				if (default_value == auto_value)
					descr = auto_value + delim + descr;

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
//  handle the options menu
//-------------------------------------------------

bool submenu::handle(event const *ev)
{
	bool changed = false;
	std::string error_string, tmptxt;
	float f_cur, f_step;

	// process the menu
	if (ev && ev->itemref && (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT || ev->iptkey == IPT_UI_SELECT))
	{
		option &sm_option = *reinterpret_cast<option *>(ev->itemref);

		switch (sm_option.type)
		{
		case option_type::EMU:
		case option_type::UI:
		case option_type::OSD:
			switch (sm_option.entry->type())
			{
			case core_options::option_type::BOOLEAN:
				changed = true;
				sm_option.options->set_value(sm_option.name, !strcmp(sm_option.entry->value(),"1") ? "0" : "1", OPTION_PRIORITY_CMDLINE);
				break;
			case core_options::option_type::INTEGER:
				if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					int i_cur = atoi(sm_option.entry->value());
					(ev->iptkey == IPT_UI_LEFT) ? i_cur-- : i_cur++;
					sm_option.options->set_value(sm_option.name, i_cur, OPTION_PRIORITY_CMDLINE);
				}
				break;
			case core_options::option_type::FLOAT:
				if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					f_cur = atof(sm_option.entry->value());
					if (sm_option.entry->has_range())
					{
						const char *minimum = sm_option.entry->minimum();
						const char *maximum = sm_option.entry->maximum();
						f_step = atof(minimum);
						if (f_step <= 0.0F) {
							int pmin = getprecisionchr(minimum);
							int pmax = getprecisionchr(maximum);
							tmptxt = '1' + std::string((pmin > pmax) ? pmin : pmax, '0');
							f_step = 1 / atof(tmptxt.c_str());
						}
					}
					else
					{
						int precision = getprecisionchr(sm_option.entry->default_value().c_str());
						tmptxt = '1' + std::string(precision, '0');
						f_step = 1 / atof(tmptxt.c_str());
					}
					if (ev->iptkey == IPT_UI_LEFT)
						f_cur -= f_step;
					else
						f_cur += f_step;
					tmptxt = string_format("%g", f_cur);
					sm_option.options->set_value(sm_option.name, tmptxt.c_str(), OPTION_PRIORITY_CMDLINE);
				}
				break;
			case core_options::option_type::STRING:
			case core_options::option_type::PATH:
			case core_options::option_type::MULTIPATH:
				if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					std::string v_cur(sm_option.entry->value());
					int cur_value = std::distance(sm_option.value.begin(), std::find(sm_option.value.begin(), sm_option.value.end(), v_cur));
					if (ev->iptkey == IPT_UI_LEFT)
						v_cur = sm_option.value[--cur_value];
					else
						v_cur = sm_option.value[++cur_value];
					sm_option.options->set_value(sm_option.name, v_cur.c_str(), OPTION_PRIORITY_CMDLINE);
				}
				break;
			default:
				break;
			}
			break;
		default:
			osd_printf_error("Unhandled option: %s\n", _(sm_option.description));
			break;
		}
	}

	if (changed) // FIXME: most changes should only require updating the item's subtext
		reset(reset_options::REMEMBER_REF);
	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void submenu::populate()
{
	// add options
	for (auto sm_option = m_options.begin(); sm_option < m_options.end(); ++sm_option)
	{
		uint32_t arrow_flags;

		// skip first heading (is menu title)
		if (sm_option == m_options.begin() && sm_option->type == option_type::HEAD) continue;

		switch (sm_option->type)
		{
		case option_type::HEAD:
			item_append(_(sm_option->description), FLAG_DISABLE | FLAG_UI_HEADING, nullptr);
			break;
		case option_type::SEP:
			item_append(menu_item_type::SEPARATOR);
			break;
		case option_type::CMD:
			item_append(_(sm_option->description), 0, reinterpret_cast<void *>(&*sm_option));
			break;
		case option_type::EMU:
		case option_type::UI:
		case option_type::OSD:
			switch (sm_option->entry->type())
			{
			case core_options::option_type::BOOLEAN:
				item_append_on_off(
						_(sm_option->description),
						sm_option->options->bool_value(sm_option->name),
						0,
						static_cast<void*>(&(*sm_option)));
				break;
			case core_options::option_type::INTEGER:
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
					item_append(
							_(sm_option->description),
							sm_option->entry->value(),
							arrow_flags,
							reinterpret_cast<void *>(&*sm_option));
				}
				break;
			case core_options::option_type::FLOAT:
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
						f_min = 0.0F;
						f_max = std::numeric_limits<float>::max();
					}
					arrow_flags = get_arrow_flags(f_min, f_max, f_cur);
					std::string tmptxt = string_format("%g", f_cur);
					item_append(
							_(sm_option->description),
							tmptxt,
							arrow_flags,
							reinterpret_cast<void *>(&*sm_option));
				}
				break;
			case core_options::option_type::STRING:
				{
					std::string v_cur(sm_option->entry->value());
					int const cur_value = std::distance(sm_option->value.begin(), std::find(sm_option->value.begin(), sm_option->value.end(), v_cur));
					arrow_flags = get_arrow_flags(0, int(unsigned(sm_option->value.size() - 1)), cur_value);
					item_append(
							_(sm_option->description),
							sm_option->options->value(sm_option->name),
							arrow_flags,
							reinterpret_cast<void *>(&*sm_option));
				}
				break;
			default:
				arrow_flags = FLAG_RIGHT_ARROW;
				item_append(
						_(sm_option->description),
						sm_option->options->value(sm_option->name),
						arrow_flags,
						reinterpret_cast<void *>(&*sm_option));
				break;
			}
			break;
		default:
			osd_printf_error("Unknown option type: %s\n", _(sm_option->description));
			break;
		}
	}

	item_append(menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void submenu::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(0.0F, line_height() + (3.0F * tb_border()));
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void submenu::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	if (selectedref)
	{
		option &selected_sm_option(*reinterpret_cast<option *>(selectedref));
		if (selected_sm_option.entry)
		{
			char const *const bottomtext[] = { selected_sm_option.entry->description() };
			draw_text_box(
					std::begin(bottomtext), std::end(bottomtext),
					origx1, origx2, origy2 + tb_border(), origy2 + bottom,
					text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
					ui().colors().text_color(), ui().colors().background_color());
		}
	}
}

} // namespace ui
