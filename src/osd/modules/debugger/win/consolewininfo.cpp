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

// devices
#include "imagedev/cassette.h"

// emu
#include "debug/debugcon.h"
#include "debugger.h"
#include "image.h"
#include "softlist_dev.h"
#include "debug/debugcpu.h"

// util
#include "util/xmlfile.h"

// osd/windows
#include "winutf8.h"

// osd
#include "strconv.h"

// C++
#include <vector>

// Windows
#include <commctrl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shtypes.h>
#include <wrl/client.h>


namespace osd::debugger::win {

namespace {

class comdlg_filter_helper
{
public:
	comdlg_filter_helper(comdlg_filter_helper const &) = delete;
	comdlg_filter_helper &operator=(comdlg_filter_helper const &) = delete;

	comdlg_filter_helper(device_image_interface &device, bool include_archives)
	{
		m_count = 0U;

		std::wstring const extensions = osd::text::to_wstring(device.file_extensions());
		std::wstring_view extview = extensions;
		m_description = L"Media Image Files (";
		for (auto comma = extview.find(','); !extview.empty(); comma = extview.find(','))
		{
			bool const found = std::wstring_view::npos != comma;
			std::wstring_view const ext = found ? extview.substr(0, comma) : extview;
			extview.remove_prefix(found ? (comma + 1) : extview.length());
			if (m_extensions.empty())
			{
				m_default = ext;
				m_description.append(L"*.");
				m_extensions.append(L"*.");
			}
			else
			{
				m_description.append(L"; *.");
				m_extensions.append(L";*.");
			}
			m_description.append(ext);
			m_extensions.append(ext);
		}
		m_description.append(1, L')');
		m_specs[m_count].pszName = m_description.c_str();
		m_specs[m_count].pszSpec = m_extensions.c_str();
		++m_count;

		if (include_archives)
		{
			m_specs[m_count].pszName = L"Archive Files (*.zip; *.7z)";
			m_specs[m_count].pszSpec = L"*.zip;*.7z";
			++m_count;
		}

		m_specs[m_count].pszName = L"All Files (*.*)";
		m_specs[m_count].pszSpec = L"*.*";
		++m_count;
	}

	UINT file_types() const noexcept
	{
		return m_count;
	}

	COMDLG_FILTERSPEC const *filter_spec() const noexcept
	{
		return m_specs;
	}

	LPCWSTR default_extension() const noexcept
	{
		return m_default.c_str();
	}

private:
	COMDLG_FILTERSPEC m_specs[3];
	std::wstring m_description;
	std::wstring m_extensions;
	std::wstring m_default;
	UINT m_count;
};


template <typename T>
void choose_image(device_image_interface &device, HWND owner, REFCLSID class_id, bool allow_archives, T &&handler)
{
	HRESULT hr;

	// create file dialog
	Microsoft::WRL::ComPtr<IFileDialog> dialog;
	hr = CoCreateInstance(class_id, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));

	// set file types
	if (SUCCEEDED(hr))
	{
		DWORD flags;
		hr = dialog->GetOptions(&flags);
		if (SUCCEEDED(hr))
			hr = dialog->SetOptions(flags | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);
		comdlg_filter_helper filters(device, allow_archives);
		if (SUCCEEDED(hr))
			hr = dialog->SetFileTypes(filters.file_types(), filters.filter_spec());
		if (SUCCEEDED(hr))
			hr = dialog->SetFileTypeIndex(1);
		if (SUCCEEDED(hr))
			hr = dialog->SetDefaultExtension(filters.default_extension());
	}

	// set starting folder
	if (SUCCEEDED(hr))
	{
		std::string dir = device.working_directory();
		if (dir.empty())
		{
			dir = device.device().machine().image().setup_working_directory();
			device.set_working_directory(dir);
		}
		std::string full;
		if (!dir.empty() && !osd_get_full_path(full, dir))
		{
			// FIXME: strip off archive names - opening a file inside an archive decompresses it to a temporary location
			std::wstring wfull = osd::text::to_wstring(full);
			Microsoft::WRL::ComPtr<IShellItem> item;
			if (SUCCEEDED(SHCreateItemFromParsingName(wfull.c_str(), nullptr, IID_PPV_ARGS(&item))))
			{
				//dialog->SetFolder(item); disabled until
			}
		}
	}

	// show the dialog
	if (SUCCEEDED(hr))
	{
		hr = dialog->Show(owner);
		if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
			return;
	}
	if (SUCCEEDED(hr))
	{
		Microsoft::WRL::ComPtr<IShellItem> result;
		hr = dialog->GetResult(&result);
		if (SUCCEEDED(hr))
		{
			PWSTR selection = nullptr;
			hr = result->GetDisplayName(SIGDN_FILESYSPATH, &selection);
			if (SUCCEEDED(hr))
			{
				std::string const utf_selection = osd::text::from_wstring(selection);
				CoTaskMemFree(selection);
				handler(utf_selection);
			}
		}
	}

	if (!SUCCEEDED(hr))
	{
		int pressed;
		TaskDialog(
				owner,
				nullptr, // instance
				nullptr, // title
				L"Error showing file dialog",
				nullptr, // content
				TDCBF_OK_BUTTON,
				TD_ERROR_ICON,
				&pressed);
	}
}

} // anonymous namespace



consolewin_info::consolewin_info(debugger_windows_interface &debugger) :
	sourcewin_info(debugger, true, "Debug", nullptr),
	m_current_cpu(nullptr),
	m_devices_menu(nullptr)
{
	if (!window() || !m_views[VIEW_IDX_SOURCE] || !m_views[VIEW_IDX_DISASM])
		goto cleanup;

	// create the views
	m_views[VIEW_IDX_STATE].reset(new debugview_info(debugger, *this, window(), DVT_STATE));
	if (!m_views[VIEW_IDX_STATE]->is_valid())
		goto cleanup;
	m_views[VIEW_IDX_CONSOLE].reset(new debugview_info(debugger, *this, window(), DVT_CONSOLE));
	if (!m_views[VIEW_IDX_CONSOLE]->is_valid())
		goto cleanup;

	{
		// add image menu only if image devices exist
		image_interface_enumerator iter(machine().root_device());
		if (iter.first() != nullptr)
		{
			m_devices_menu = CreatePopupMenu();
			for (device_image_interface &img : iter)
			{
				if (img.user_loadable())
				{
					osd::text::tstring tc_buf = osd::text::to_tstring(string_format("%s : %s", img.device().name(), img.exists() ? img.filename() : "[no image]"));
					AppendMenu(m_devices_menu, MF_ENABLED, 0, tc_buf.c_str());
				}
			}
			AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)m_devices_menu, TEXT("Media"));
		}

		// add the settings menu
		HMENU const settingsmenu = CreatePopupMenu();
		AppendMenu(settingsmenu, MF_ENABLED, ID_SAVE_WINDOWS, TEXT("Save Window Arrangement"));
		AppendMenu(settingsmenu, MF_ENABLED, ID_GROUP_WINDOWS, TEXT("Group Debugger Windows (requires restart)"));
		AppendMenu(settingsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
		AppendMenu(settingsmenu, MF_ENABLED, ID_LIGHT_BACKGROUND, TEXT("Light Background"));
		AppendMenu(settingsmenu, MF_ENABLED, ID_DARK_BACKGROUND, TEXT("Dark Background"));
		AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)settingsmenu, TEXT("Settings"));

		// get the work bounds
		RECT work_bounds, bounds;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &work_bounds, 0);

		// adjust the min/max sizes for the window style
		bounds.top = bounds.left = 0;
		bounds.right = bounds.bottom = EDGE_WIDTH + m_views[VIEW_IDX_STATE]->maxwidth() + (2 * EDGE_WIDTH) + 100 + EDGE_WIDTH;
		AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);
		set_minwidth(bounds.right - bounds.left);

		bounds.top = bounds.left = 0;
		bounds.right = bounds.bottom = EDGE_WIDTH + m_views[VIEW_IDX_STATE]->maxwidth() + (2 * EDGE_WIDTH) + std::max(m_views[VIEW_IDX_DISASM]->maxwidth(), m_views[VIEW_IDX_CONSOLE]->maxwidth()) + EDGE_WIDTH;
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
	set_cpu(*machine().debugger().console().get_visible_cpu());

	// mark the edit box as the default focus and set it
	editwin_info::set_default_focus();

	hide_src_window();
	return;

cleanup:
	m_views[VIEW_IDX_CONSOLE].reset();
	m_views[VIEW_IDX_STATE].reset();
	m_views[VIEW_IDX_DISASM].reset();
	m_views[VIEW_IDX_SOURCE].reset();
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
		m_views[VIEW_IDX_DISASM]->set_source_for_device(device);
		m_views[VIEW_IDX_STATE]->set_source_for_device(device);

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
	regrect.right = regrect.left + m_views[VIEW_IDX_STATE]->maxwidth();

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
	set_srcwnd_bounds(disrect);
	m_views[VIEW_IDX_DISASM]->set_bounds(disrect);
	m_views[VIEW_IDX_STATE]->set_bounds(regrect);
	m_views[VIEW_IDX_CONSOLE]->set_bounds(conrect);
	set_editwnd_bounds(editrect);
}


void consolewin_info::update_menu()
{
	sourcewin_info::update_menu();

	if (m_devices_menu)
	{
		// create the image menu
		uint32_t cnt = 0;
		for (device_image_interface &img : image_interface_enumerator(machine().root_device()))
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

			// FIXME: needs a real software item picker to be useful
			//if (get_softlist_info(img))
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

	HMENU const menu = GetMenu(window());
	CheckMenuItem(menu, ID_SAVE_WINDOWS, MF_BYCOMMAND | (debugger().get_save_window_arrangement() ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_GROUP_WINDOWS, MF_BYCOMMAND | (debugger().get_group_windows_setting() ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_LIGHT_BACKGROUND, MF_BYCOMMAND | ((ui_metrics::THEME_LIGHT_BACKGROUND == metrics().get_color_theme()) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_DARK_BACKGROUND, MF_BYCOMMAND | ((ui_metrics::THEME_DARK_BACKGROUND == metrics().get_color_theme()) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_SOURCE, MF_BYCOMMAND | (m_views[VIEW_IDX_SOURCE]->is_visible() ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_DISASM, MF_BYCOMMAND | (m_views[VIEW_IDX_DISASM]->is_visible() ? MF_CHECKED : MF_UNCHECKED));
}


bool consolewin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	if (HIWORD(wparam) == 0)
	{
		if (LOWORD(wparam) >= ID_DEVICE_OPTIONS)
		{
			uint32_t const devid = (LOWORD(wparam) - ID_DEVICE_OPTIONS) / DEVOPTION_MAX;
			image_interface_enumerator iter(machine().root_device());
			device_image_interface *const img = iter.byindex(devid);
			if (img != nullptr)
			{
				switch ((LOWORD(wparam) - ID_DEVICE_OPTIONS) % DEVOPTION_MAX)
				{
				case DEVOPTION_ITEM:
					// TODO: this is supposed to show a software list item picker - it never worked properly
					return true;
				case DEVOPTION_OPEN :
					open_image_file(*img);
					return true;
				case DEVOPTION_CREATE:
					create_image_file(*img);
					return true;
				case DEVOPTION_CLOSE:
					img->unload();
					return true;
				}
				if (img->device().type() == CASSETTE)
				{
					auto *const cassette = downcast<cassette_image_device *>(&img->device());
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
						return true;
					case DEVOPTION_CASSETTE_MOTOR:
						s = ((cassette->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED);
						cassette->change_state(s ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
						return true;
					case DEVOPTION_CASSETTE_SOUND:
						s = ((cassette->get_state() & CASSETTE_MASK_SPEAKER) == CASSETTE_SPEAKER_MUTED);
						cassette->change_state(s ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);
						return true;
					}
				}
			}
		}
		else switch (LOWORD(wparam))
		{
		case ID_SAVE_WINDOWS:
			debugger().set_save_window_arrangement(!debugger().get_save_window_arrangement());
			return true;
		case ID_GROUP_WINDOWS:
			debugger().set_group_windows_setting(!debugger().get_group_windows_setting());
			return true;
		case ID_LIGHT_BACKGROUND:
			debugger().set_color_theme(ui_metrics::THEME_LIGHT_BACKGROUND);
			return true;
		case ID_DARK_BACKGROUND:
			debugger().set_color_theme(ui_metrics::THEME_DARK_BACKGROUND);
			return true;
		case ID_SHOW_SOURCE:
			if (show_src_window())
			{
				m_views[VIEW_IDX_DISASM]->hide();
				machine().debug_view().update_all(DVT_SOURCE);
			}
			return true;
		case ID_SHOW_DISASM:
			hide_src_window();
			m_views[VIEW_IDX_DISASM]->show();
			machine().debug_view().update_all(DVT_DISASSEMBLY);
			return true;
		}
	}

	return sourcewin_info::handle_command(wparam, lparam);
}


void consolewin_info::save_configuration_to_node(util::xml::data_node &node)
{
	sourcewin_info::save_configuration_to_node(node);
	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_CONSOLE);
}


void consolewin_info::process_string(std::string const &string)
{
	if (string.empty()) // an empty string is a single step
		machine().debugger().console().get_visible_cpu()->debug()->single_step();
	else                // otherwise, just process the command
		machine().debugger().console().execute_command(string, true);

	// clear the edit text box
	set_editwnd_text("");
}


void consolewin_info::open_image_file(device_image_interface &device)
{
	choose_image(
			device,
			window(),
			CLSID_FileOpenDialog,
			true,
			[this, &device] (std::string_view selection)
			{
				auto [err, message] = device.load(selection);
				if (err)
					machine().debugger().console().printf("Error mounting image file: %s\n", !message.empty() ? message : err.message());
			});
}


void consolewin_info::create_image_file(device_image_interface &device)
{
	choose_image(
			device,
			window(),
			CLSID_FileSaveDialog,
			false,
			[this, &device] (std::string_view selection)
			{
				auto [err, message] = device.create(selection, device.device_get_indexed_creatable_format(0), nullptr);
				if (err)
					machine().debugger().console().printf("Error creating image file: %s\n", !message.empty() ? message : err.message());
			});
}


bool consolewin_info::get_softlist_info(device_image_interface &device)
{
	bool has_software = false;
	bool passes_tests = false;
	std::string sl_dir, opt_name = device.instance_name();

	// Get the path to suitable software
	for (software_list_device &swlist : software_list_device_enumerator(machine().root_device()))
	{
		for (const software_info &swinfo : swlist.get_info())
		{
			const software_part &part = swinfo.parts().front();
			if (swlist.is_compatible(part) == SOFTWARE_IS_COMPATIBLE)
			{
				for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
				{
					if (!image.user_loadable())
						continue;
					if (!has_software && (opt_name == image.instance_name()))
					{
						const char *intf = image.image_interface();
						if (intf && part.matches_interface(intf))
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
			if (osd::directory::open(test_path))
			{
				passes_tests = true;
				slmap[opt_name] = test_path;
			}
			sl_root = strtok(NULL, ";");
		}
	}

	return passes_tests;
}

} // namespace osd::debugger::win
