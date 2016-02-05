// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/mainmenu.c

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "crsshair.h"
#include "emuopts.h"
#include "rendutil.h"
#include "cheat.h"
#include "ui/menu.h"
#include "ui/filemngr.h"
#include "ui/barcode.h"
#include "ui/cheatopt.h"
#include "ui/info.h"
#include "ui/info_pty.h"
#include "ui/inputmap.h"
#include "ui/mainmenu.h"
#include "ui/miscmenu.h"
#include "ui/selgame.h"
#include "ui/simpleselgame.h"
#include "ui/sliders.h"
#include "ui/slotopt.h"
#include "ui/tapectrl.h"
#include "ui/videoopt.h"
#include "imagedev/cassette.h"
#include "machine/bcreader.h"
#include "ui/datfile.h"
#include "ui/inifile.h"
#include "ui/datmenu.h"


/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ui_menu_main constructor - populate the main menu
-------------------------------------------------*/

ui_menu_main::ui_menu_main(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_main::populate()
{
	std::string menu_text;

	/* add input menu items */
	item_append("Input (general)", nullptr, 0, (void *)INPUT_GROUPS);

	strprintf(menu_text, "Input (this %s)", emulator_info::get_capstartgamenoun());
	item_append(menu_text.c_str(), nullptr, 0, (void *)INPUT_SPECIFIC);

	/* add optional input-related menus */
	if (machine().ioport().has_analog())
		item_append("Analog Controls", nullptr, 0, (void *)ANALOG);
	if (machine().ioport().has_dips())
		item_append("Dip Switches", nullptr, 0, (void *)SETTINGS_DIP_SWITCHES);
	if (machine().ioport().has_configs())
	{
		strprintf(menu_text, "%s Configuration", emulator_info::get_capstartgamenoun());
		item_append(menu_text.c_str(), nullptr, 0, (void *)SETTINGS_DRIVER_CONFIG);
	}

	/* add bookkeeping menu */
	item_append("Bookkeeping Info", nullptr, 0, (void *)BOOKKEEPING);

	/* add game info menu */
	strprintf(menu_text, "%s Information", emulator_info::get_capstartgamenoun());
	item_append(menu_text.c_str(), nullptr, 0, (void *)GAME_INFO);

	image_interface_iterator imgiter(machine().root_device());
	if (imgiter.first() != nullptr)
	{
		/* add image info menu */
		item_append("Image Information", nullptr, 0, (void *)IMAGE_MENU_IMAGE_INFO);

		/* add file manager menu */
		item_append("File Manager", nullptr, 0, (void *)IMAGE_MENU_FILE_MANAGER);

		/* add tape control menu */
		cassette_device_iterator cassiter(machine().root_device());
		if (cassiter.first() != nullptr)
			item_append("Tape Control", nullptr, 0, (void *)TAPE_CONTROL);
	}

		pty_interface_iterator ptyiter(machine().root_device());
		if (ptyiter.first() != nullptr) {
			item_append("Pseudo terminals", nullptr, 0, (void *)PTY_INFO);
		}
	if (machine().ioport().has_bioses())
		item_append("Bios Selection", nullptr, 0, (void *)BIOS_SELECTION);

	slot_interface_iterator slotiter(machine().root_device());
	if (slotiter.first() != nullptr)
	{
		/* add slot info menu */
		item_append("Slot Devices", nullptr, 0, (void *)SLOT_DEVICES);
	}

	barcode_reader_device_iterator bcriter(machine().root_device());
	if (bcriter.first() != nullptr)
	{
		/* add slot info menu */
		item_append("Barcode Reader", nullptr, 0, (void *)BARCODE_READ);
	}

	network_interface_iterator netiter(machine().root_device());
	if (netiter.first() != nullptr)
	{
		/* add image info menu */
		item_append("Network Devices", nullptr, 0, (void*)NETWORK_DEVICES);
	}

	/* add keyboard mode menu */
	if (machine().ioport().has_keyboard() && machine().ioport().natkeyboard().can_post())
		item_append("Keyboard Mode", nullptr, 0, (void *)KEYBOARD_MODE);

	/* add sliders menu */
	item_append("Slider Controls", nullptr, 0, (void *)SLIDERS);

	/* add video options menu */
	item_append("Video Options", nullptr, 0, (machine().render().target_by_index(1) != nullptr) ? (void *)VIDEO_TARGETS : (void *)VIDEO_OPTIONS);

	/* add crosshair options menu */
	if (machine().crosshair().get_usage())
		item_append("Crosshair Options", nullptr, 0, (void *)CROSSHAIR);

	/* add cheat menu */
	if (machine().options().cheat() && machine().cheat().first() != nullptr)
		item_append("Cheat", nullptr, 0, (void *)CHEAT);

	/* add history menu */
	if (machine().ui().options().enabled_dats())
		item_append("History Info", nullptr, 0, (void *)HISTORY);

	// add software history menu
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) == 0 && machine().ui().options().enabled_dats())
	{
		image_interface_iterator iter(machine().root_device());
		for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
		{
			const char *name = image->filename();
			if (name != nullptr)
			{
				item_append("Software History Info", nullptr, 0, (void *)SW_HISTORY);
				break;
			}
		}
	}

	/* add mameinfo / messinfo menu */
	if (machine().ui().options().enabled_dats())
	{
		if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0)
			item_append("MameInfo", nullptr, 0, (void *)MAMEINFO);
		else if ((machine().system().flags & MACHINE_TYPE_ARCADE) == 0)
			item_append("MessInfo", nullptr, 0, (void *)MAMEINFO);
	}

	/* add sysinfo menu */
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) == 0 && machine().ui().options().enabled_dats())
		item_append("SysInfo", nullptr, 0, (void *)SYSINFO);

	/* add command list menu */
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0 && machine().ui().options().enabled_dats())
		item_append("Commands Info", nullptr, 0, (void *)COMMAND);

	/* add story menu */
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0 && machine().ui().options().enabled_dats())
		item_append("Mamescores", nullptr, 0, (void *)STORYINFO);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	/* add favorite menu */
    if (!machine().favorite().isgame_favorite())
		item_append("Add To Favorites", nullptr, 0, (void *)ADD_FAVORITE);
	else
		item_append("Remove From Favorites", nullptr, 0, (void *)REMOVE_FAVORITE);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

//	menu_text.assign("Quit from ").append(emulator_info::get_capstartgamenoun());
//	item_append(menu_text.c_str(), nullptr, 0, (void *)QUIT_GAME);

	/* add reset and exit menus */
	strprintf(menu_text, "Select New %s", emulator_info::get_capstartgamenoun());
	item_append(menu_text.c_str(), nullptr, 0, (void *)SELECT_GAME);
}

ui_menu_main::~ui_menu_main()
{
}

/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

void ui_menu_main::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->iptkey == IPT_UI_SELECT) {
		switch((long long)(menu_event->itemref)) {
		case INPUT_GROUPS:
			ui_menu::stack_push(global_alloc_clear<ui_menu_input_groups>(machine(), container));
			break;

		case INPUT_SPECIFIC:
			ui_menu::stack_push(global_alloc_clear<ui_menu_input_specific>(machine(), container));
			break;

		case SETTINGS_DIP_SWITCHES:
			ui_menu::stack_push(global_alloc_clear<ui_menu_settings_dip_switches>(machine(), container));
			break;

		case SETTINGS_DRIVER_CONFIG:
			ui_menu::stack_push(global_alloc_clear<ui_menu_settings_driver_config>(machine(), container));
			break;

		case ANALOG:
			ui_menu::stack_push(global_alloc_clear<ui_menu_analog>(machine(), container));
			break;

		case BOOKKEEPING:
			ui_menu::stack_push(global_alloc_clear<ui_menu_bookkeeping>(machine(), container));
			break;

		case GAME_INFO:
			ui_menu::stack_push(global_alloc_clear<ui_menu_game_info>(machine(), container));
			break;

		case IMAGE_MENU_IMAGE_INFO:
			ui_menu::stack_push(global_alloc_clear<ui_menu_image_info>(machine(), container));
			break;

		case IMAGE_MENU_FILE_MANAGER:
			ui_menu::stack_push(global_alloc_clear<ui_menu_file_manager>(machine(), container, nullptr));
			break;

		case TAPE_CONTROL:
			ui_menu::stack_push(global_alloc_clear<ui_menu_tape_control>(machine(), container, nullptr));
			break;

		case PTY_INFO:
			ui_menu::stack_push(global_alloc_clear<ui_menu_pty_info>(machine(), container));
			break;

		case SLOT_DEVICES:
			ui_menu::stack_push(global_alloc_clear<ui_menu_slot_devices>(machine(), container));
			break;

		case NETWORK_DEVICES:
			ui_menu::stack_push(global_alloc_clear<ui_menu_network_devices>(machine(), container));
			break;

		case KEYBOARD_MODE:
			ui_menu::stack_push(global_alloc_clear<ui_menu_keyboard_mode>(machine(), container));
			break;

		case SLIDERS:
			ui_menu::stack_push(global_alloc_clear<ui_menu_sliders>(machine(), container, false));
			break;

		case VIDEO_TARGETS:
			ui_menu::stack_push(global_alloc_clear<ui_menu_video_targets>(machine(), container));
			break;

		case VIDEO_OPTIONS:
			ui_menu::stack_push(global_alloc_clear<ui_menu_video_options>(machine(), container, machine().render().first_target()));
			break;

		case CROSSHAIR:
			ui_menu::stack_push(global_alloc_clear<ui_menu_crosshair>(machine(), container));
			break;

		case CHEAT:
			ui_menu::stack_push(global_alloc_clear<ui_menu_cheat>(machine(), container));
			break;

		case SELECT_GAME:
			if (strcmp(machine().options().ui(),"simple")==0) {
				ui_menu::stack_push(global_alloc_clear<ui_simple_menu_select_game>(machine(), container, nullptr));
			} else {
				ui_menu::stack_push(global_alloc_clear<ui_menu_select_game>(machine(), container, nullptr));
			}		
			break;

		case BIOS_SELECTION:
			ui_menu::stack_push(global_alloc_clear<ui_menu_bios_selection>(machine(), container));
			break;

		case BARCODE_READ:
			ui_menu::stack_push(global_alloc_clear<ui_menu_barcode_reader>(machine(), container, nullptr));
			break;

		case HISTORY:
			ui_menu::stack_push(global_alloc_clear<ui_menu_dats>(machine(), container, UI_HISTORY_LOAD));
			break;

		case MAMEINFO:
			if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0)
				ui_menu::stack_push(global_alloc_clear<ui_menu_dats>(machine(), container, UI_MAMEINFO_LOAD));
			else
				ui_menu::stack_push(global_alloc_clear<ui_menu_dats>(machine(), container, UI_MESSINFO_LOAD));
			break;

		case SYSINFO:
			ui_menu::stack_push(global_alloc_clear<ui_menu_dats>(machine(), container, UI_SYSINFO_LOAD));
			break;

		case COMMAND:
			ui_menu::stack_push(global_alloc_clear<ui_menu_command>(machine(), container));
			break;

		case STORYINFO:
			ui_menu::stack_push(global_alloc_clear<ui_menu_dats>(machine(), container, UI_STORY_LOAD));
			break;

		case ADD_FAVORITE:
            machine().favorite().add_favorite_game();
			reset(UI_MENU_RESET_REMEMBER_POSITION);
			break;

		case REMOVE_FAVORITE:
            machine().favorite().remove_favorite_game();
			reset(UI_MENU_RESET_REMEMBER_POSITION);
			break;

		case SW_HISTORY:
			ui_menu::stack_push(global_alloc_clear<ui_menu_history_sw>(machine(), container));
			break;

		case QUIT_GAME:
			ui_menu::stack_pop(machine());
			machine().ui().request_quit();
			break;

		default:
			fatalerror("ui_menu_main::handle - unknown reference\n");
		}
	}
}
