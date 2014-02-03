/***************************************************************************

    emenubar.c

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/emenubar.h"
#include "ui/selgame.h"
#include "ui/miscmenu.h"
#include "ui/filesel.h"
#include "ui/imginfo.h"
#include "ui/tapectrl.h"
#include "ui/bbcontrl.h"
#include "ui/swlist.h"
#include "ui/viewgfx.h"
#include "softlist.h"
#include "cheat.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#ifdef MAME_PROFILER
#define HAS_PROFILER			1
#else // !MAME_PROFILER
#define HAS_PROFILER			0
#endif // MAME_PROFILER


//**************************************************************************
//  MENUBAR IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_emu_menubar::ui_emu_menubar(running_machine &machine)
	: ui_menubar(machine)
{
}


//-------------------------------------------------
//  start_menu
//-------------------------------------------------

template<class _Menu>
void ui_emu_menubar::start_menu()
{
	machine().ui().set_handler(ui_menu::ui_handler, 0);
	ui_menu::stack_push(auto_alloc_clear(machine(), _Menu(machine(), container())));
}


//-------------------------------------------------
//  menubar_draw_ui_elements
//-------------------------------------------------

void ui_emu_menubar::menubar_draw_ui_elements()
{
	// first draw the FPS counter 
	if (machine().ui().show_fps_counter())
	{
		astring tempstring;
		machine().ui().draw_text_full(container(), machine().video().speed_text(tempstring), 0.0f, 0.0f, 1.0f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}

	// draw the profiler if visible 
	if (machine().ui().show_profiler())
	{
		const char *text = g_profiler.text(machine());
		machine().ui().draw_text_full(container(), text, 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}

	// check for fast forward 
	if (machine().ioport().type_pressed(IPT_UI_FAST_FORWARD))
	{
		machine().video().set_fastforward(true);
		machine().ui().show_fps_temp(0.5);
	}
	else
		machine().video().set_fastforward(false);

}


//-------------------------------------------------
//  menubar_build_menus
//-------------------------------------------------

void ui_emu_menubar::menubar_build_menus()
{
	build_file_menu();
	build_images_menu();
	build_options_menu();
	build_settings_menu();
	build_help_menu();
}


//-------------------------------------------------
//  build_file_menu
//-------------------------------------------------

void ui_emu_menubar::build_file_menu()
{
	astring menu_text;
	menu_item &file_menu = root_menu().append("File");

	// show gfx
	if (ui_gfx_is_relevant(machine()))
		file_menu.append("Show Graphics/Palette...", &ui_emu_menubar::set_ui_handler, *this, ui_gfx_ui_handler, (UINT32) machine().paused(), IPT_UI_SHOW_GFX);

	// save screen snapshot
	file_menu.append("Save Screen Snapshot(s)", &video_manager::save_active_screen_snapshots, machine().video(), IPT_UI_SNAPSHOT);

	// record movie
	menu_item &record_movie_menu = file_menu.append("Record Movie", &video_manager::toggle_record_movie, machine().video(), IPT_UI_RECORD_MOVIE);
	record_movie_menu.set_checked(machine().video().is_recording());

	// save state
	file_menu.append("Save State...", &ui_emu_menubar::set_ui_handler, *this, ui_manager::ui_handler_load_save, (UINT32) LOADSAVE_SAVE, IPT_UI_SAVE_STATE);

	// load state
	file_menu.append("Load State...", &ui_emu_menubar::set_ui_handler, *this, ui_manager::ui_handler_load_save, (UINT32) LOADSAVE_LOAD, IPT_UI_LOAD_STATE);

	// separator
	file_menu.append_separator();

	// paste
	if (machine().ioport().has_keyboard() && machine().ioport().natkeyboard().can_post())
	{
		menu_item &paste_menu = file_menu.append("Paste", &ui_manager::paste, machine().ui(), IPT_UI_PASTE);
		paste_menu.set_enabled(machine().ui().can_paste());
	}

	// pause
	menu_item &pause_menu = file_menu.append("Pause", &running_machine::toggle_pause, machine(), IPT_UI_PAUSE);
	pause_menu.set_checked(machine().paused());

	// reset
	menu_item &reset_menu = file_menu.append("Reset");
	reset_menu.append("Hard", &running_machine::schedule_hard_reset, machine(), IPT_UI_RESET_MACHINE);
	reset_menu.append("Soft", &running_machine::schedule_soft_reset, machine(), IPT_UI_SOFT_RESET);

	// separator
	file_menu.append_separator();

	// select new game
	menu_text.printf("Select New %s...", emulator_info::get_capstartgamenoun());
	file_menu.append(menu_text, &ui_emu_menubar::select_new_game, *this);

	// exit
	file_menu.append("Exit", &running_machine::schedule_exit, machine());
}


//-------------------------------------------------
//  build_images_menu
//-------------------------------------------------

void ui_emu_menubar::build_images_menu()
{
	// we only have an images menu if we have image devices
	image_interface_iterator iter(machine().root_device());
	if (iter.first() != NULL)
	{
		// we have image slots; add the menu and start iterating
		menu_item &images_menu = root_menu().append("Images");
		for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
		{
			bool is_loaded = image->basename() != NULL;

			astring buffer;
			buffer.printf("%s (%s): \t%s",
				image->device().name(),
				image->brief_instance_name(),
				is_loaded ? image->basename() : "[empty]");

			// append the menu item for this device
			menu_item &menu = images_menu.append(buffer);

			// software list
			if (image->image_interface() != NULL)
			{
				if (build_software_list_menus(menu, image))
					menu.append_separator();
			}

			// load
			menu.append("Load...", &ui_emu_menubar::load, *this, image);

			// unload
			menu_item &unload_menu = menu.append("Unload", &device_image_interface::unload, *image);
			unload_menu.set_enabled(is_loaded);

			// tape control
			cassette_image_device *cassette = dynamic_cast<cassette_image_device *>(image);
			if (cassette != NULL)
			{
				menu_item &control_menu = menu.append("Tape Control...", &ui_emu_menubar::tape_control, *this, cassette);
				control_menu.set_enabled(is_loaded);
			}

			// bitbanger control
			bitbanger_device *bitbanger = dynamic_cast<bitbanger_device *>(image);
			if (bitbanger != NULL)
			{
				menu_item &control_menu = menu.append("Bitbanger Control...", &ui_emu_menubar::bitbanger_control, *this, bitbanger);
				control_menu.set_enabled(is_loaded);
			}
		}
	}
}


//-------------------------------------------------
//  build_software_list_menus
//-------------------------------------------------

bool ui_emu_menubar::build_software_list_menus(menu_item &menu, device_image_interface *image)
{
	int item_count = 0;
	menu_item *last_menu_item = NULL;
	astring description;
	softlist_type types[] = { SOFTWARE_LIST_ORIGINAL_SYSTEM, SOFTWARE_LIST_COMPATIBLE_SYSTEM };
	software_list_device_iterator softlist_iter(machine().config().root_device());

	// first do "original system" softlists, then do compatible ones
	for (int typenum = 0; typenum < ARRAY_LENGTH(types); typenum++)
	{
		for (const software_list_device *swlist = softlist_iter.first(); swlist != NULL; swlist = softlist_iter.next())
		{
			if ((swlist->list_type() == types[typenum]) && is_softlist_relevant(swlist, image->image_interface(), description))
			{
				// we've found a softlist; append the menu item
				last_menu_item = &menu.append(description, &ui_emu_menubar::select_from_software_list, *this, image, swlist);
				item_count++;
			}
		}
	}

	// if we only had one list, lets use a generic name
	if (last_menu_item != NULL && (item_count == 1))
		last_menu_item->set_text("Software list...");

	return item_count > 0;
}


//-------------------------------------------------
//  build_options_menu
//-------------------------------------------------

void ui_emu_menubar::build_options_menu()
{
	astring menu_text;
	menu_item &options_menu = root_menu().append("Options");

	// throttle
	float throttle_rates[] = { 10.0f, 5.0f, 2.0f, 1.0f, 0.5f, 0.2f, 0.1f, 0.0f };
	menu_item &throttle_menu = options_menu.append("Throttle");
	for (int i = 0; i < ARRAY_LENGTH(throttle_rates); i++)
	{
		const char *item = "Unthrottled";
		if (throttle_rates[i] != 0)
		{
			menu_text.printf("%d%%", (int) (throttle_rates[i] * 100));
			item = menu_text;
		}

		menu_item &menu = throttle_menu.append(item, &video_manager::set_throttle_rate, machine().video(), throttle_rates[i]);
		menu.set_checked(machine().video().throttle_rate() == throttle_rates[i]);
	}

	// frame skip
	menu_item &frameskip_menu = options_menu.append("Frame Skip");
	for (int i = -1; i <= MAX_FRAMESKIP; i++)
	{
		const char *item = "Auto";
		if (i >= 0)
		{
			menu_text.printf("%d", i);
			item = menu_text;
		}

		menu_item &menu = frameskip_menu.append(item, &video_manager::set_frameskip, machine().video(), i);
		menu.set_checked(machine().video().frameskip() == i);
	}

	// separator
	frameskip_menu.append_separator();

	// increase
	frameskip_menu.append("Increase", &ui_manager::increase_frameskip, machine().ui(), IPT_UI_FRAMESKIP_INC);

	// decrease
	frameskip_menu.append("Decrease", &ui_manager::decrease_frameskip, machine().ui(), IPT_UI_FRAMESKIP_DEC);

	// show fps
	options_menu.append("Show Frames Per Second", &ui_manager::set_show_fps, &ui_manager::show_fps, machine().ui(), IPT_UI_SHOW_FPS);

	// show profiler
	if (HAS_PROFILER)
		options_menu.append("Show Profiler", &ui_manager::set_show_profiler, &ui_manager::show_profiler, machine().ui(), IPT_UI_SHOW_PROFILER);

	// video
	// do different things if we actually have multiple render targets
	menu_item &video_menu = options_menu.append("Video");
	if (machine().render().target_by_index(1) != NULL)
	{
		// multiple targets
		int targetnum = 0;
		render_target *target;
		while((target = machine().render().target_by_index(targetnum)) != NULL)
		{
			astring buffer;
			buffer.printf("Screen #%d", targetnum++);
			menu_item &target_menu = options_menu.append(buffer);
			build_video_target_menu(target_menu, *target);
		}
	}
	else
	{
		// single target
		build_video_target_menu(video_menu, *machine().render().first_target());
	}

	// separator
	options_menu.append_separator();

	// slot devices
	slot_interface_iterator slotiter(machine().root_device());
	if (slotiter.first() != NULL)
		options_menu.append<ui_emu_menubar>("Slot Devices...", &ui_emu_menubar::start_menu<ui_menu_slot_devices>, *this);

	// network devices
	network_interface_iterator netiter(machine().root_device());
	if (netiter.first() != NULL)
		options_menu.append<ui_emu_menubar>("Network Devices...", &ui_emu_menubar::start_menu<ui_menu_network_devices>, *this);

	// keyboard
	if (machine().ioport().has_keyboard() && machine().ioport().natkeyboard().can_post())
	{
		menu_item &keyboard_menu = options_menu.append("Keyboard");
		keyboard_menu.append("Emulated", &ui_manager::set_use_natural_keyboard, &ui_manager::use_natural_keyboard, machine().ui(), false);
		keyboard_menu.append("Natural",  &ui_manager::set_use_natural_keyboard, &ui_manager::use_natural_keyboard, machine().ui(), true);
	}

	// crosshair options
	if (crosshair_get_usage(machine()))
		options_menu.append<ui_emu_menubar>("Crosshair Options...", &ui_emu_menubar::start_menu<ui_menu_crosshair>, *this);

	// memory card
	if (machine().config().m_memcard_handler != NULL)
		options_menu.append<ui_emu_menubar>("Memory Card...", &ui_emu_menubar::start_menu<ui_menu_memory_card>, *this);

	// cheat
	if (machine().options().cheat() && machine().cheat().first() != NULL)
	{
		options_menu.append_separator();
		options_menu.append("Cheats enabled", &cheat_manager::set_enable, &cheat_manager::enabled, machine().cheat(), IPT_UI_TOGGLE_CHEAT);
		options_menu.append<ui_emu_menubar>("Cheat...", &ui_emu_menubar::start_menu<ui_menu_cheat>, *this);
	}
}


//-------------------------------------------------
//  build_video_target_menu
//-------------------------------------------------

void ui_emu_menubar::build_video_target_menu(menu_item &target_menu, render_target &target)
{
	astring tempstring;
	const char *view_name;

	// add the menu items for each view
	for(int viewnum = 0; (view_name = target.view_name(viewnum)) != NULL; viewnum++)
	{
		// replace spaces with underscores
		tempstring.cpy(view_name).replace(0, "_", " ");

		// append the menu
		target_menu.append(tempstring, &render_target::set_view, &render_target::view, target, viewnum);
	}

	// separator
	target_menu.append_separator();

	// rotation
	menu_item &rotation_menu = target_menu.append("Rotation");
	rotation_menu.append("None",				&render_target::set_orientation, &render_target::orientation, target, ROT0);
	rotation_menu.append("Clockwise 90",		&render_target::set_orientation, &render_target::orientation, target, ROT90);
	rotation_menu.append("180",					&render_target::set_orientation, &render_target::orientation, target, ROT180);
	rotation_menu.append("Counterclockwise 90",	&render_target::set_orientation, &render_target::orientation, target, ROT270);

	// show backdrops
	target_menu.append("Show Backdrops", &render_target::set_backdrops_enabled, &render_target::backdrops_enabled, target);

	// show overlay
	target_menu.append("Show Overlays", &render_target::set_overlays_enabled, &render_target::overlays_enabled, target);

	// show bezel
	target_menu.append("Show Bezels", &render_target::set_bezels_enabled, &render_target::bezels_enabled, target);

	// show cpanel
	target_menu.append("Show CPanels", &render_target::set_cpanels_enabled, &render_target::cpanels_enabled, target);

	// show marquee
	target_menu.append("Show Marquees", &render_target::set_marquees_enabled, &render_target::marquees_enabled, target);

	// view
	menu_item &view_menu = target_menu.append("View");
	view_menu.append("Cropped", &render_target::set_zoom_to_screen, &render_target::zoom_to_screen, target, true);
	view_menu.append("Full", &render_target::set_zoom_to_screen, &render_target::zoom_to_screen, target, false);
}


//-------------------------------------------------
//  build_settings_menu
//-------------------------------------------------

void ui_emu_menubar::build_settings_menu()
{
	astring menu_text;
	menu_item &settings_menu = root_menu().append("Settings");

	// general input
	// TODO - BREAK THIS APART?
	settings_menu.append<ui_emu_menubar>("General Input...", &ui_emu_menubar::start_menu<ui_menu_input_groups>, *this);

	// game input
	menu_text.printf("%s Input...", emulator_info::get_capstartgamenoun());
	settings_menu.append<ui_emu_menubar>(menu_text, &ui_emu_menubar::start_menu<ui_menu_input_specific>, *this);

	// analog controls
	if (machine().ioport().has_analog())
		settings_menu.append<ui_emu_menubar>("Analog Controls...", &ui_emu_menubar::start_menu<ui_menu_analog>, *this);

	// dip switches
	if (machine().ioport().has_dips())
		settings_menu.append<ui_emu_menubar>("Dip Switches...", &ui_emu_menubar::start_menu<ui_menu_settings_dip_switches>, *this);

	// driver configuration
	if (machine().ioport().has_configs())
	{
		menu_text.printf("%s Configuration...", emulator_info::get_capstartgamenoun());
		settings_menu.append<ui_emu_menubar>(menu_text, &ui_emu_menubar::start_menu<ui_menu_settings_driver_config>, *this);
	}

	// bios selection
	if (machine().ioport().has_bioses())
		settings_menu.append<ui_emu_menubar>("Bios Selection...", &ui_emu_menubar::start_menu<ui_menu_bios_selection>, *this);

	// sliders
	settings_menu.append<ui_emu_menubar>("Sliders...", &ui_emu_menubar::start_menu<ui_menu_sliders>, *this, IPT_UI_ON_SCREEN_DISPLAY);
}


//-------------------------------------------------
//  build_help_menu
//-------------------------------------------------

void ui_emu_menubar::build_help_menu()
{
	astring menu_text;
	menu_item &help_menu = root_menu().append("Help");

	// bookkeeping info
	help_menu.append<ui_emu_menubar>("Bookkeeping info...", &ui_emu_menubar::start_menu<ui_menu_bookkeeping>, *this);

	// game info
	menu_text.printf("%s Information...", emulator_info::get_capstartgamenoun());
	help_menu.append<ui_emu_menubar>(menu_text, &ui_emu_menubar::start_menu<ui_menu_game_info>, *this);

	// image information
	image_interface_iterator imgiter(machine().root_device());
	if (imgiter.first() != NULL)
		help_menu.append<ui_emu_menubar>("Image Information...", &ui_emu_menubar::start_menu<ui_menu_image_info>, *this);
}


//-------------------------------------------------
//  is_softlist_relevant
//-------------------------------------------------

bool ui_emu_menubar::is_softlist_relevant(const software_list_device *swlist, const char *interface, astring &list_description)
{
	bool result = false;

	const software_list *list = software_list_open(machine().options(), swlist->list_name(), false, NULL);
	if (list != NULL)
	{
		for (const software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
		{
			const software_part *part = software_find_part(swinfo, NULL, NULL);
			if (softlist_contain_interface(interface, part->interface_))
			{
				list_description.printf("%s...", list->description);
				result = true;
				break;
			}
		}
		software_list_close(list);
	}
	return result;
}


//-------------------------------------------------
//  set_ui_handler
//-------------------------------------------------

void ui_emu_menubar::set_ui_handler(ui_callback callback, UINT32 param)
{
	// first pause
	machine().pause();

	// and transfer control
	machine().ui().set_handler(callback, param);
}


//-------------------------------------------------
//  select_new_game
//-------------------------------------------------

void ui_emu_menubar::select_new_game()
{
	ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_game(machine(), container(), machine().system().name)));
}


//-------------------------------------------------
//  select_from_software_list
//-------------------------------------------------

void ui_emu_menubar::select_from_software_list(device_image_interface *image, const software_list_device *swlist)
{
	ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_list(machine(), container(), swlist, image)));
}


//-------------------------------------------------
//  tape_control
//-------------------------------------------------

void ui_emu_menubar::tape_control(cassette_image_device *image)
{
	ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_mess_tape_control(machine(), container(), image)));
}


//-------------------------------------------------
//  bitbanger_control
//-------------------------------------------------

void ui_emu_menubar::bitbanger_control(bitbanger_device *image)
{
	ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_mess_bitbanger_control(machine(), container(), image)));
}


//-------------------------------------------------
//  load
//-------------------------------------------------

void ui_emu_menubar::load(device_image_interface *image)
{
	ui_menu::stack_push(image->get_selection_menu(machine(), container()));
}
