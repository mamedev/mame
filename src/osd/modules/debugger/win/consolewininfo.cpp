// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  consolewininfo.c - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "consolewininfo.h"

#include "debugviewinfo.h"
#include "uimetrics.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "imagedev/cassette.h"

#include "strconv.h"
#include "winutf8.h"


consolewin_info::consolewin_info(debugger_windows_interface &debugger) :
	disasmbasewin_info(debugger, true, "Debug", nullptr),
	m_current_cpu(nullptr),
	m_devices_menu(nullptr)
{
	if ((window() == nullptr) || (m_views[0] == nullptr))
		goto cleanup;

	// create the views
	m_views[1].reset(global_alloc(debugview_info(debugger, *this, window(), DVT_STATE)));
	if (!m_views[1]->is_valid())
		goto cleanup;
	m_views[2].reset(global_alloc(debugview_info(debugger, *this, window(), DVT_CONSOLE)));
	if (!m_views[2]->is_valid())
		goto cleanup;

	{
		// Add image menu only if image devices exist
		image_interface_iterator iter(machine().root_device());
		if (iter.first() != nullptr)
		{
			m_devices_menu = CreatePopupMenu();
			for (device_image_interface &img : iter)
			{
				if (!img.user_loadable())
					continue;
				osd::text::tstring tc_buf = osd::text::to_tstring(string_format("%s : %s", img.device().name(), img.exists() ? img.filename() : "[no image]"));
				AppendMenu(m_devices_menu, MF_ENABLED, 0, tc_buf.c_str());
			}
			AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)m_devices_menu, TEXT("Media"));
		}

		// get the work bounds
		RECT work_bounds, bounds;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &work_bounds, 0);

		// adjust the min/max sizes for the window style
		bounds.top = bounds.left = 0;
		bounds.right = bounds.bottom = EDGE_WIDTH + m_views[1]->maxwidth() + (2 * EDGE_WIDTH) + 100 + EDGE_WIDTH;
		AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);
		set_minwidth(bounds.right - bounds.left);

		bounds.top = bounds.left = 0;
		bounds.right = bounds.bottom = EDGE_WIDTH + m_views[1]->maxwidth() + (2 * EDGE_WIDTH) + std::max(m_views[0]->maxwidth(), m_views[2]->maxwidth()) + EDGE_WIDTH;
		AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);
		set_maxwidth(bounds.right - bounds.left);

		// position the window at the bottom-right
		int const bestwidth = (std::min<uint32_t>)(maxwidth(), work_bounds.right - work_bounds.left);
		int const bestheight = (std::min<uint32_t>)(500, work_bounds.bottom - work_bounds.top);
		SetWindowPos(window(), HWND_TOP,
					work_bounds.right - bestwidth, work_bounds.bottom - bestheight,
					bestwidth, bestheight,
					SWP_SHOWWINDOW);
	}

	// recompute the children
	set_cpu(*machine().debugger().cpu().get_visible_cpu());

	// mark the edit box as the default focus and set it
	editwin_info::set_default_focus();
	return;

cleanup:
	m_views[2].reset();
	m_views[1].reset();
	m_views[0].reset();
}


consolewin_info::~consolewin_info()
{
}

void consolewin_info::set_cpu(device_t &device)
{
	// exit if this cpu is already selected
	if (&device != m_current_cpu)
	{
		m_current_cpu = &device;

		// first set all the views to the new cpu number
		m_views[0]->set_source_for_device(device);
		m_views[1]->set_source_for_device(device);

		// then update the caption
		std::string title = string_format("Debug: %s - %s '%s'", device.machine().system().name, device.name(), device.tag());
		std::string curtitle = win_get_window_text_utf8(window());
		if (title != curtitle)
			win_set_window_text_utf8(window(), title.c_str());

		// and recompute the children
		recompute_children();
	}
}


void consolewin_info::recompute_children()
{
	// get the parent's dimensions
	RECT parent;
	GetClientRect(window(), &parent);

	// registers always get their desired width, and span the entire height
	RECT regrect;
	regrect.top = parent.top + EDGE_WIDTH;
	regrect.bottom = parent.bottom - EDGE_WIDTH;
	regrect.left = parent.left + EDGE_WIDTH;
	regrect.right = regrect.left + m_views[1]->maxwidth();

	// edit box goes at the bottom of the remaining area
	RECT editrect;
	editrect.bottom = parent.bottom - EDGE_WIDTH;
	editrect.top = editrect.bottom - metrics().debug_font_height() - 4;
	editrect.left = regrect.right + (EDGE_WIDTH * 2);
	editrect.right = parent.right - EDGE_WIDTH;

	// console and disassembly split the difference
	RECT disrect;
	disrect.top = parent.top + EDGE_WIDTH;
	disrect.bottom = ((editrect.top - parent.top) / 2) - EDGE_WIDTH;
	disrect.left = regrect.right + (EDGE_WIDTH * 2);
	disrect.right = parent.right - EDGE_WIDTH;

	RECT conrect;
	conrect.top = disrect.bottom + (EDGE_WIDTH * 2);
	conrect.bottom = editrect.top - EDGE_WIDTH;
	conrect.left = regrect.right + (EDGE_WIDTH * 2);
	conrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	m_views[0]->set_bounds(disrect);
	m_views[1]->set_bounds(regrect);
	m_views[2]->set_bounds(conrect);
	set_editwnd_bounds(editrect);
}


void consolewin_info::update_menu()
{
	disasmbasewin_info::update_menu();

	if (m_devices_menu != nullptr)
	{
		// create the image menu
		uint32_t cnt = 0;
		for (device_image_interface &img : image_interface_iterator(machine().root_device()))
		{
			if (!img.user_loadable())
				continue;

			HMENU const devicesubmenu = CreatePopupMenu();

			UINT_PTR const new_item = ID_DEVICE_OPTIONS + (cnt * DEVOPTION_MAX);

			UINT flags_for_exists = MF_ENABLED | MF_STRING;
			if (!img.exists())
				flags_for_exists |= MF_GRAYED;

			UINT flags_for_writing = flags_for_exists;
			if (img.is_readonly())
				flags_for_writing |= MF_GRAYED;

			// not working properly, removed for now until investigation can be done
			//if (get_softlist_info(&img))
			//  AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_ITEM, TEXT("Mount Item..."));

			AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_OPEN, TEXT("Mount File..."));

			if (img.is_creatable())
				AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_CREATE, TEXT("Create..."));

			if (img.exists())
			{
				AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CLOSE, TEXT("Unmount"));

				if (img.device().type() == CASSETTE)
				{
					cassette_state const state = downcast<cassette_image_device *>(&img.device())->get_state() & CASSETTE_MASK_UISTATE;
					AppendMenu(devicesubmenu, MF_SEPARATOR, 0, nullptr);
					AppendMenu(devicesubmenu, flags_for_exists | ((state == CASSETTE_STOPPED) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_STOPPAUSE, TEXT("Pause/Stop"));
					AppendMenu(devicesubmenu, flags_for_exists | ((state == CASSETTE_PLAY) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_PLAY, TEXT("Play"));
					AppendMenu(devicesubmenu, flags_for_writing | ((state == CASSETTE_RECORD) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_RECORD, TEXT("Record"));
					AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_REWIND, TEXT("Rewind"));
					AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_FASTFORWARD, TEXT("Fast Forward"));
					AppendMenu(devicesubmenu, MF_SEPARATOR, 0, nullptr);
					// Motor state can be overriden by the driver
					cassette_state const motor_state = downcast<cassette_image_device *>(&img.device())->get_state() & CASSETTE_MASK_MOTOR;
					AppendMenu(devicesubmenu, flags_for_exists | ((motor_state == CASSETTE_MOTOR_ENABLED) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_MOTOR, TEXT("Motor"));
					cassette_state const speaker_state = downcast<cassette_image_device *>(&img.device())->get_state() & CASSETTE_MASK_SPEAKER;
					AppendMenu(devicesubmenu, flags_for_exists | ((speaker_state == CASSETTE_SPEAKER_ENABLED) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_SOUND, TEXT("Audio while Loading"));
				}
			}

			std::string filename;
			if (img.basename())
			{
				filename.assign(img.basename());

				// if the image has been loaded through softlist, also show the loaded part
				if (img.loaded_through_softlist())
				{
					const software_part *tmp = img.part_entry();
					if (!tmp->name().empty())
					{
						filename.append(" (");
						filename.append(tmp->name());
						// also check if this part has a specific part_id (e.g. "Map Disc", "Bonus Disc", etc.), and in case display it
						if (img.get_feature("part_id") != nullptr)
						{
							filename.append(": ");
							filename.append(img.get_feature("part_id"));
						}
						filename.append(")");
					}
				}
			}
			else
				filename.assign("---");

			// Get instance names like the File Manager
			osd::text::tstring tc_buf = osd::text::to_tstring(string_format("%s (%s): %s", img.instance_name(), img.brief_instance_name(), filename.c_str()));
			std::transform(tc_buf.begin(), tc_buf.begin()+1, tc_buf.begin(), ::toupper); // turn first char to uppercase
			ModifyMenu(m_devices_menu, cnt, MF_BYPOSITION | MF_POPUP, (UINT_PTR)devicesubmenu, tc_buf.c_str());

			cnt++;
		}
	}
}


bool consolewin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	if ((HIWORD(wparam) == 0) && (LOWORD(wparam) >= ID_DEVICE_OPTIONS))
	{
		uint32_t const devid = (LOWORD(wparam) - ID_DEVICE_OPTIONS) / DEVOPTION_MAX;
		image_interface_iterator iter(machine().root_device());
		device_image_interface *const img = iter.byindex(devid);
		if (img != nullptr)
		{
			switch ((LOWORD(wparam) - ID_DEVICE_OPTIONS) % DEVOPTION_MAX)
			{
			case DEVOPTION_ITEM :
				{
					std::string filter;
					build_generic_filter(nullptr, false, filter);
					{
						osd::text::tstring t_filter = osd::text::to_tstring(filter);

						// convert a pipe-char delimited string into a NUL delimited string
						for (int i = 0; t_filter[i] != '\0'; i++)
						{
							if (t_filter[i] == '|')
								t_filter[i] = '\0';
						}

						std::string opt_name = img->instance_name();
						std::string as = slmap.find(opt_name)->second;

						/* Make sure a folder was specified, and that it exists */
						if ((!osd::directory::open(as.c_str())) || (as.find(':') == std::string::npos))
						{
							/* Default to emu directory */
							osd_get_full_path(as, ".");
						}
						osd::text::tstring t_dir = osd::text::to_tstring(as);

						// display the dialog
						TCHAR selectedFilename[MAX_PATH];
						selectedFilename[0] = '\0';
						OPENFILENAME ofn;
						memset(&ofn, 0, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = nullptr;
						ofn.lpstrFile = selectedFilename;
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrFilter = t_filter.c_str();
						ofn.nFilterIndex = 1;
						ofn.lpstrFileTitle = nullptr;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = t_dir.c_str();
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if (GetOpenFileName(&ofn))
						{
							std::string buf = std::string(osd::text::from_tstring(selectedFilename));
							// Get the Item name out of the full path
							size_t t1 = buf.find(".zip"); // get rid of zip name and anything after
							if (t1 != std::string::npos)
								buf.erase(t1);
							t1 = buf.find(".7z"); // get rid of 7zip name and anything after
							if (t1 != std::string::npos)
								buf.erase(t1);
							t1 = buf.find_last_of("\\");   // put the swlist name in
							buf[t1] = ':';
							t1 = buf.find_last_of("\\"); // get rid of path; we only want the item name
							buf.erase(0, t1+1);

							// load software
							img->load_software( buf.c_str());
						}
					}
				}
				return true;
			case DEVOPTION_OPEN :
				{
					std::string filter;
					build_generic_filter(img, false, filter);
					{
						osd::text::tstring t_filter = osd::text::to_tstring(filter);

						// convert a pipe-char delimited string into a NUL delimited string
						for (int i = 0; t_filter[i] != '\0'; i++)
						{
							if (t_filter[i] == '|')
								t_filter[i] = '\0';
						}

						char buf[400];
						std::string as;
						strcpy(buf, machine().options().emu_options::sw_path());
						// This pulls out the first path from a multipath field
						const char* t1 = strtok(buf, ";");
						if (t1)
							as = t1; // the first path of many
						else
							as = buf; // the only path

						/* Make sure a folder was specified, and that it exists */
						if ((!osd::directory::open(as.c_str())) || (as.find(':') == std::string::npos))
						{
							/* Default to emu directory */
							osd_get_full_path(as, ".");
						}
						osd::text::tstring t_dir = osd::text::to_tstring(as);

						TCHAR selectedFilename[MAX_PATH];
						selectedFilename[0] = '\0';
						OPENFILENAME ofn;
						memset(&ofn, 0, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = nullptr;
						ofn.lpstrFile = selectedFilename;
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrFilter = t_filter.c_str();
						ofn.nFilterIndex = 1;
						ofn.lpstrFileTitle = nullptr;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = t_dir.c_str();
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if (GetOpenFileName(&ofn))
						{
							auto utf8_buf = osd::text::from_tstring(selectedFilename);
							img->load(utf8_buf.c_str());
						}
					}
				}
				return true;
			case DEVOPTION_CREATE:
				{
					std::string filter;
					build_generic_filter(img, true, filter);
					{
						osd::text::tstring t_filter = osd::text::to_tstring(filter);
						// convert a pipe-char delimited string into a NUL delimited string
						for (int i = 0; t_filter[i] != '\0'; i++)
						{
							if (t_filter[i] == '|')
								t_filter[i] = '\0';
						}

						char buf[400];
						std::string as;
						strcpy(buf, machine().options().emu_options::sw_path());
						// This pulls out the first path from a multipath field
						const char* t1 = strtok(buf, ";");
						if (t1)
							as = t1; // the first path of many
						else
							as = buf; // the only path

						/* Make sure a folder was specified, and that it exists */
						if ((!osd::directory::open(as.c_str())) || (as.find(':') == std::string::npos))
						{
							/* Default to emu directory */
							osd_get_full_path(as, ".");
						}
						osd::text::tstring t_dir = osd::text::to_tstring(as);

						TCHAR selectedFilename[MAX_PATH];
						selectedFilename[0] = '\0';
						OPENFILENAME ofn;
						memset(&ofn, 0, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = nullptr;
						ofn.lpstrFile = selectedFilename;
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrFilter = t_filter.c_str();
						ofn.nFilterIndex = 1;
						ofn.lpstrFileTitle = nullptr;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = t_dir.c_str();
						ofn.Flags = OFN_PATHMUSTEXIST;

						if (GetSaveFileName(&ofn))
						{
							auto utf8_buf = osd::text::from_tstring(selectedFilename);
							img->create(utf8_buf.c_str(), img->device_get_indexed_creatable_format(0), nullptr);
						}
					}
				}
				return true;
			case DEVOPTION_CLOSE:
				img->unload();
				return true;
			}
			if (img->device().type() == CASSETTE)
			{
				cassette_image_device *const cassette = downcast<cassette_image_device *>(&img->device());
				bool s;
				switch ((LOWORD(wparam) - ID_DEVICE_OPTIONS) % DEVOPTION_MAX)
				{
				case DEVOPTION_CASSETTE_STOPPAUSE:
					cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
					return true;
				case DEVOPTION_CASSETTE_PLAY:
					cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
					return true;
				case DEVOPTION_CASSETTE_RECORD:
					cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
					return true;
				case DEVOPTION_CASSETTE_REWIND:
					cassette->seek(0.0, SEEK_SET);  // to start
					return true;
				case DEVOPTION_CASSETTE_FASTFORWARD:
					cassette->seek(+300.0, SEEK_CUR); // 5 minutes forward or end, whichever comes first
					break;
				case DEVOPTION_CASSETTE_MOTOR:
					s =((cassette->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED);
					cassette->change_state(s ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
					break;
				case DEVOPTION_CASSETTE_SOUND:
					s =((cassette->get_state() & CASSETTE_MASK_SPEAKER) == CASSETTE_SPEAKER_MUTED);
					cassette->change_state(s ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);
					break;
				}
			}
		}
	}
	return disasmbasewin_info::handle_command(wparam, lparam);
}


void consolewin_info::process_string(std::string const &string)
{
	if (string.empty()) // an empty string is a single step
		machine().debugger().cpu().get_visible_cpu()->debug()->single_step();
	else                // otherwise, just process the command
		machine().debugger().console().execute_command(string, true);

	// clear the edit text box
	set_editwnd_text("");
}


void consolewin_info::build_generic_filter(device_image_interface *img, bool is_save, std::string &filter)
{
	std::string file_extension;

	if (img)
		file_extension = img->file_extensions();

	if (!is_save)
		file_extension.append(",zip,7z");

	add_filter_entry(filter, "Common image types", file_extension.c_str());

	filter.append("All files (*.*)|*.*|");

	if (!is_save)
		filter.append("Compressed Images (*.zip;*.7z)|*.zip;*.7z|");
}


void consolewin_info::add_filter_entry(std::string &dest, const char *description, const char *extensions)
{
	// add the description
	dest.append(description);
	dest.append(" (");

	// add the extensions to the description
	copy_extension_list(dest, extensions);

	// add the trailing rparen and '|' character
	dest.append(")|");

	// now add the extension list itself
	copy_extension_list(dest, extensions);

	// append a '|'
	dest.append("|");
}


void consolewin_info::copy_extension_list(std::string &dest, const char *extensions)
{
	// our extension lists are comma delimited; Win32 expects to see lists
	// delimited by semicolons
	char const *s = extensions;
	while (*s)
	{
		// append a semicolon if not at the beginning
		if (s != extensions)
			dest.push_back(';');

		// append ".*"
		dest.append("*.");

		// append the file extension
		while (*s && (*s != ','))
			dest.push_back(*s++);

		// if we found a comma, advance
		while(*s == ',')
			s++;
	}
}

//============================================================
//  get_softlist_info
//============================================================
bool consolewin_info::get_softlist_info(device_image_interface *img)
{
	bool has_software = false;
	bool passes_tests = false;
	std::string sl_dir, opt_name = img->instance_name();

	// Get the path to suitable software
	for (software_list_device &swlist : software_list_device_iterator(machine().root_device()))
	{
		for (const software_info &swinfo : swlist.get_info())
		{
			const software_part &part = swinfo.parts().front();
			if (swlist.is_compatible(part) == SOFTWARE_IS_COMPATIBLE)
			{
				for (device_image_interface &image : image_interface_iterator(machine().root_device()))
				{
					if (!image.user_loadable())
						continue;
					if (!has_software && (opt_name == image.instance_name()))
					{
						const char *interface = image.image_interface();
						if (interface && part.matches_interface(interface))
						{
							sl_dir = "\\" + swlist.list_name();
							has_software = true;
						}
					}
				}
			}
		}
	}

	if (has_software)
	{
		/* Get the media_path */
		char rompath[400];
		strcpy(rompath, machine().options().emu_options::media_path());
		// Now, scan through the media_path looking for the required folder
		char* sl_root = strtok(rompath, ";");
		while (sl_root && !passes_tests)
		{
			std::string test_path = sl_root + sl_dir;
			if (osd::directory::open(test_path.c_str()))
			{
				passes_tests = true;
				slmap[opt_name] = test_path;
			}
			sl_root = strtok(NULL, ";");
		}
	}

	return passes_tests;
}
