// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  consolewininfo.cpp - Win32 debug window handling
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

#include <string>

using namespace std::literals;


//**************************************************************************
//  ANONYMOUS NAMESPACE
//**************************************************************************

namespace
{

//-------------------------------------------------
//  copy_extension_list
//-------------------------------------------------

void copy_extension_list(std::ostringstream &dest, const char *extensions)
{
	// our extension lists are comma delimited; Win32 expects to see lists
	// delimited by semicolons
	char const *s = extensions;
	while (*s)
	{
		// append a semicolon if not at the beginning
		if (s != extensions)
			dest << ';';

		// append ".*"
		dest << "*.";

		// append the file extension
		while (*s && (*s != ','))
			dest << *s++;

		// if we found a comma, advance
		while (*s == ',')
			s++;
	}
}


//-------------------------------------------------
//  add_filter_entry
//-------------------------------------------------

void add_filter_entry(std::ostringstream &dest, const char *description, const char *extensions)
{
	// add the description
	dest << description;
	dest << " (";

	// add the extensions to the description
	copy_extension_list(dest, extensions);

	// add the trailing rparen and '\0' character
	dest << ")\0"s;

	// now add the extension list itself
	copy_extension_list(dest, extensions);

	// append a '\0'
	dest << '\0';
}


//-------------------------------------------------
//  build_generic_filter
//-------------------------------------------------

std::string build_generic_filter(device_image_interface *img, bool is_save)
{
	std::ostringstream filter;
	std::string file_extension;

	if (img)
		file_extension = img->file_extensions();

	if (!is_save)
		file_extension.append(",zip,7z");

	add_filter_entry(filter, "Common image types", file_extension.c_str());

	filter << "All files (*.*)\0*.*\0"s;

	if (!is_save)
		filter << "Compressed Images (*.zip;*.7z)\0*.zip;*.7z\0"s;

	return filter.str();
}


//-------------------------------------------------
//  win_get_file_name
//-------------------------------------------------

enum class win_get_file_name_type
{
	OPEN,
	SAVE
};

std::string win_get_file_name(win_get_file_name_type type, const std::string &filter, const std::string &dir)
{
	// convert to TCHAR
	osd::text::tstring t_filter = osd::text::to_tstring(filter);
	osd::text::tstring t_dir = osd::text::to_tstring(dir);

	// this is where the selected filename is stored
	TCHAR selected_filename[MAX_PATH];
	selected_filename[0] = '\0';

	// record the current working directory
	DWORD working_directory_length = GetCurrentDirectory(0, nullptr);
	TCHAR *working_directory = (TCHAR *) alloca(working_directory_length * sizeof(TCHAR));
	GetCurrentDirectory(working_directory_length, working_directory);

	// determine the flags
	DWORD flags;
	switch (type)
	{
	case win_get_file_name_type::OPEN:
		flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		break;
	case win_get_file_name_type::SAVE:
		flags = OFN_PATHMUSTEXIST;
		break;
	default:
		throw false;
	}

	// create the gnarly OPENFILENAME struct
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = selected_filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = t_filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = t_dir.c_str();
	ofn.Flags = flags;

	// get the filename
	BOOL result;
	switch (type)
	{
	case win_get_file_name_type::OPEN:
		result = GetOpenFileName(&ofn);
		break;
	case win_get_file_name_type::SAVE:
		result = GetSaveFileName(&ofn);
		break;
	default:
		throw false;
	}

	// restore the current working directory
	SetCurrentDirectory(working_directory);

	return result
		? osd::text::from_tstring(selected_filename)
		: "";
}

}

//**************************************************************************
//  CONSOLE WIN INFO
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

consolewin_info::consolewin_info(debugger_windows_interface &debugger)
	: disasmbasewin_info(debugger, true, "Debug", nullptr)
	, m_devices_menu(nullptr)
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
			AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)m_devices_menu, TEXT("Images"));
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


//-------------------------------------------------
//  dtor
//-------------------------------------------------

consolewin_info::~consolewin_info()
{
}


//-------------------------------------------------
//  set_cpu
//-------------------------------------------------

void consolewin_info::set_cpu(device_t &device)
{
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


//-------------------------------------------------
//  recompute_children
//-------------------------------------------------

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


//-------------------------------------------------
//  update_menu
//-------------------------------------------------

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
			//	AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_ITEM, TEXT("Mount Item..."));

			AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_OPEN, TEXT("Mount File..."));

			if (img.is_creatable())
				AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_CREATE, TEXT("Create..."));
			AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CLOSE, TEXT("Unmount"));

			if (img.device().type() == CASSETTE)
			{
				cassette_state const state = (cassette_state)(img.exists() ? (downcast<cassette_image_device *>(&img.device())->get_state() & CASSETTE_MASK_UISTATE) : CASSETTE_STOPPED);
				AppendMenu(devicesubmenu, MF_SEPARATOR, 0, nullptr);
				AppendMenu(devicesubmenu, flags_for_exists | ((state == CASSETTE_STOPPED) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_STOPPAUSE, TEXT("Pause/Stop"));
				AppendMenu(devicesubmenu, flags_for_exists | ((state == CASSETTE_PLAY) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_PLAY, TEXT("Play"));
				AppendMenu(devicesubmenu, flags_for_writing | ((state == CASSETTE_RECORD) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_RECORD, TEXT("Record"));
				AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_REWIND, TEXT("Rewind"));
				AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_FASTFORWARD, TEXT("Fast Forward"));
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


//-------------------------------------------------
//  handle_command
//-------------------------------------------------

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
					std::string filter = build_generic_filter(nullptr, false);
					{
						std::string opt_name = img->instance_name();
						std::string as = slmap.find(opt_name)->second;

						/* Make sure a folder was specified, and that it exists */
						if ((!osd::directory::open(as.c_str())) || (as.find(':') == std::string::npos))
						{
							/* Default to emu directory */
							osd_get_full_path(as, ".");
						}

						std::string buf = win_get_file_name(win_get_file_name_type::OPEN, filter, as);
						if (!buf.empty())
						{
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
							img->load_software(buf.c_str());
						}
					}
				}
				return true;
			case DEVOPTION_OPEN :
				{
					std::string filter = build_generic_filter(img, false);
					{
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

						std::string result = win_get_file_name(win_get_file_name_type::OPEN, filter, as);
						if (!result.empty())
							img->load(result);
					}
				}
				return true;
			case DEVOPTION_CREATE:
				{
					std::string filter = build_generic_filter(img, true);
					{
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

						std::string result = win_get_file_name(win_get_file_name_type::SAVE, filter, as);
						if (!result.empty())
							img->create(result, img->device_get_indexed_creatable_format(0), nullptr);
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
					cassette->seek(-60.0, SEEK_CUR);
					return true;
				case DEVOPTION_CASSETTE_FASTFORWARD:
					cassette->seek(+60.0, SEEK_CUR);
					return true;
				}
			}
		}
	}
	return disasmbasewin_info::handle_command(wparam, lparam);
}


//-------------------------------------------------
//  process_string
//-------------------------------------------------

void consolewin_info::process_string(char const *string)
{
	if (string[0] == 0) // an empty string is a single step
		machine().debugger().cpu().get_visible_cpu()->debug()->single_step();
	else                // otherwise, just process the command
		machine().debugger().console().execute_command(string, true);

	// clear the edit text box
	set_editwnd_text("");
}


//-------------------------------------------------
//  get_softlist_info
//-------------------------------------------------

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
