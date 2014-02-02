/***************************************************************************

    emenubar.c

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/emenubar.h"
#include "ui/selgame.h"
#include "uimain.h"
#include "uiimage.h"
#include "cheat.h"
#include "imagedev/cassette.h"
#include "imagedev/bitbngr.h"


//**************************************************************************
//  MENUBAR IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_emu_menubar::ui_emu_menubar(running_machine &machine, render_container *container)
	: ui_menubar(machine, container)
{
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

	// pause
	menu_item &pause_menu = file_menu.append("Pause", &running_machine::toggle_pause, machine());
	pause_menu.set_checked(machine().paused());

	// reset
	menu_item &reset_menu = file_menu.append("Reset");
	reset_menu.append("Hard", &running_machine::schedule_hard_reset, machine());
	reset_menu.append("Soft", &running_machine::schedule_soft_reset, machine());

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
	image_interface_iterator imgiter(machine().root_device());
	if (imgiter.first() != NULL)
	{
		// TODO - this is not the final list but it will bootstrap the effort
		menu_item &images_menu = root_menu().append("Images");

		// image information
		images_menu.append<ui_menubar>("Image Information...", &ui_menubar::push_menu<ui_menu_image_info>, *this);

		// file manager
		images_menu.append<ui_menubar>("File Manager...", &ui_menubar::push_menu<ui_menu_file_manager>, *this);

		// tape control
		cassette_device_iterator cassiter(machine().root_device());
		if (cassiter.first() != NULL)
			images_menu.append<ui_menubar>("Tape Control...", &ui_menubar::push_menu<ui_menu_mess_tape_control>, *this);

		// bitbanger
		bitbanger_device_iterator bititer(machine().root_device());
		if (bititer.first() != NULL)
			images_menu.append<ui_menubar>("Bitbanger Control...", &ui_menubar::push_menu<ui_menu_mess_bitbanger_control>, *this);
	}
}


//-------------------------------------------------
//  build_options_menu
//-------------------------------------------------

void ui_emu_menubar::build_options_menu()
{
	menu_item &options_menu = root_menu().append("Options");

	// throttle
	menu_item &throttle_menu = options_menu.append("Throttle");
	throttle_menu.append("200%", &ui_emu_menubar::throttle, *this, 2.0f);
	throttle_menu.append("100%", &ui_emu_menubar::throttle, *this, 1.0f);
	throttle_menu.append("50%", &ui_emu_menubar::throttle, *this, 0.5f);
	throttle_menu.append("20%", &ui_emu_menubar::throttle, *this, 0.2f);
	throttle_menu.append("10%", &ui_emu_menubar::throttle, *this, 0.1f);
	throttle_menu.append("Unthrottled", &ui_emu_menubar::throttle, *this, 0.0f);

	// video
	// TODO - DOES THIS MAKE SENSE WITH A REAL PULL DOWN MENU?
	options_menu.append("Video...", &ui_emu_menubar::video_options, *this);

	// slot devices
	slot_interface_iterator slotiter(machine().root_device());
	if (slotiter.first() != NULL)
		options_menu.append<ui_menubar>("Slot Devices...", &ui_menubar::push_menu<ui_menu_slot_devices>, *this);

	// network devices
	network_interface_iterator netiter(machine().root_device());
	if (netiter.first() != NULL)
		options_menu.append<ui_menubar>("Network Devices...", &ui_menubar::push_menu<ui_menu_network_devices>, *this);

	// keyboard
	if (machine().ioport().has_keyboard() && machine().ioport().natkeyboard().can_post())
	{
		bool use_natural_keyboard = ui_get_use_natural_keyboard(machine());
		menu_item &keyboard_menu = options_menu.append("Keyboard");
		keyboard_menu.append("Emulated", &ui_emu_menubar::set_natural_keyboard, *this, false).set_checked(!use_natural_keyboard);
		keyboard_menu.append("Natural", &ui_emu_menubar::set_natural_keyboard, *this, true).set_checked(use_natural_keyboard);
	}

	// crosshair options
	if (crosshair_get_usage(machine()))
		options_menu.append<ui_menubar>("Crosshair Options...", &ui_menubar::push_menu<ui_menu_crosshair>, *this);

	// memory card
	if (machine().config().m_memcard_handler != NULL)
		options_menu.append<ui_menubar>("Memory Card...", &ui_menubar::push_menu<ui_menu_memory_card>, *this);

	// cheat
	if (machine().options().cheat() && machine().cheat().first() != NULL)
		options_menu.append<ui_menubar>("Cheat...", &ui_menubar::push_menu<ui_menu_cheat>, *this);
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
	settings_menu.append<ui_menubar>("General Input...", &ui_menubar::push_menu<ui_menu_input_groups>, *this);

	// game input
	menu_text.printf("%s Input...", emulator_info::get_capstartgamenoun());
	settings_menu.append<ui_menubar>(menu_text, &ui_menubar::push_menu<ui_menu_input_specific>, *this);

	// analog controls
	if (machine().ioport().has_analog())
		settings_menu.append<ui_menubar>("Analog Controls...", &ui_menubar::push_menu<ui_menu_analog>, *this);

	// dip switches
	if (machine().ioport().has_dips())
		settings_menu.append<ui_menubar>("Dip Switches...", &ui_menubar::push_menu<ui_menu_settings_dip_switches>, *this);

	// driver configuration
	if (machine().ioport().has_configs())
	{
		menu_text.printf("%s Configuration...", emulator_info::get_capstartgamenoun());
		settings_menu.append<ui_menubar>(menu_text, &ui_menubar::push_menu<ui_menu_settings_driver_config>, *this);
	}

	// bios selection
	if (machine().ioport().has_bioses())
		settings_menu.append<ui_menubar>("Bios Selection...", &ui_menubar::push_menu<ui_menu_bios_selection>, *this);

	// sliders
	settings_menu.append<ui_menubar>("Sliders...", &ui_menubar::push_menu<ui_menu_sliders>, *this);
}


//-------------------------------------------------
//  build_help_menu
//-------------------------------------------------

void ui_emu_menubar::build_help_menu()
{
	astring menu_text;
	menu_item &help_menu = root_menu().append("Help");

	// bookkeeping info
	help_menu.append<ui_menubar>("Bookkeeping info...", &ui_menubar::push_menu<ui_menu_bookkeeping>, *this);

	// game info
	menu_text.printf("%s Information...", emulator_info::get_capstartgamenoun());
	help_menu.append<ui_menubar>(menu_text, &ui_menubar::push_menu<ui_menu_game_info>, *this);
}


//-------------------------------------------------
//  select_new_game
//-------------------------------------------------

void ui_emu_menubar::select_new_game()
{
	stack_push(auto_alloc_clear(machine(), ui_menu_select_game(machine(), container, machine().system().name)));
}


//-------------------------------------------------
//  throttle
//-------------------------------------------------

void ui_emu_menubar::throttle(float rate)
{
	// TODO - IMPLEMENT THIS
}


//-------------------------------------------------
//  set_natural_keyboard
//-------------------------------------------------

void ui_emu_menubar::set_natural_keyboard(bool use_natural_keyboard)
{
	ui_set_use_natural_keyboard(machine(), use_natural_keyboard);
}


//-------------------------------------------------
//  video_options
//-------------------------------------------------

void ui_emu_menubar::video_options()
{
	// do different things if we actually have multiple render targets
	if (machine().render().target_by_index(1) != NULL)
		stack_push(auto_alloc_clear(machine(), ui_menu_video_targets(machine(), container)));
	else
		stack_push(auto_alloc_clear(machine(), ui_menu_video_options(machine(), container, machine().render().first_target())));
}
