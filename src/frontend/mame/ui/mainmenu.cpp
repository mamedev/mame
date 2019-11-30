// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/mainmenu.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "ui/mainmenu.h"

#include "ui/analogipt.h"
#include "ui/barcode.h"
#include "ui/cheatopt.h"
#include "ui/confswitch.h"
#include "ui/datmenu.h"
#include "ui/filemngr.h"
#include "ui/info.h"
#include "ui/info_pty.h"
#include "ui/inifile.h"
#include "ui/inputmap.h"
#include "ui/miscmenu.h"
#include "ui/pluginopt.h"
#include "ui/selgame.h"
#include "ui/simpleselgame.h"
#include "ui/sliders.h"
#include "ui/slotopt.h"
#include "ui/tapectrl.h"
#include "ui/videoopt.h"

#include "mame.h"
#include "luaengine.h"

#include "machine/bcreader.h"
#include "imagedev/cassette.h"

#include "crsshair.h"
#include "dipty.h"
#include "emuopts.h"
#include "natkeyboard.h"


namespace ui {

/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    menu_main constructor - populate the main menu
-------------------------------------------------*/

menu_main::menu_main(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_main::populate(float &customtop, float &custombottom)
{
	/* add main menu items */
	item_append(_("Input (general)"), "", 0, (void *)INPUT_GROUPS);

	item_append(_("Input (this Machine)"), "", 0, (void *)INPUT_SPECIFIC);

	if (ui().machine_info().has_analog())
		item_append(_("Analog Controls"), "", 0, (void *)ANALOG);
	if (ui().machine_info().has_dips())
		item_append(_("Dip Switches"), "", 0, (void *)SETTINGS_DIP_SWITCHES);
	if (ui().machine_info().has_configs())
		item_append(_("Machine Configuration"), "", 0, (void *)SETTINGS_DRIVER_CONFIG);

	item_append(_("Bookkeeping Info"), "", 0, (void *)BOOKKEEPING);

	item_append(_("Machine Information"), "", 0, (void *)GAME_INFO);

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		if (image.user_loadable())
		{
			item_append(_("Image Information"), "", 0, (void *)IMAGE_MENU_IMAGE_INFO);

			item_append(_("File Manager"), "", 0, (void *)IMAGE_MENU_FILE_MANAGER);

			break;
		}
	}

	if (cassette_device_iterator(machine().root_device()).first() != nullptr)
		item_append(_("Tape Control"), "", 0, (void *)TAPE_CONTROL);

	if (pty_interface_iterator(machine().root_device()).first() != nullptr)
		item_append(_("Pseudo terminals"), "", 0, (void *)PTY_INFO);

	if (ui().machine_info().has_bioses())
		item_append(_("BIOS Selection"), "", 0, (void *)BIOS_SELECTION);

	if (slot_interface_iterator(machine().root_device()).first() != nullptr)
		item_append(_("Slot Devices"), "", 0, (void *)SLOT_DEVICES);

	if (barcode_reader_device_iterator(machine().root_device()).first() != nullptr)
		item_append(_("Barcode Reader"), "", 0, (void *)BARCODE_READ);

	if (network_interface_iterator(machine().root_device()).first() != nullptr)
		item_append(_("Network Devices"), "", 0, (void*)NETWORK_DEVICES);

	if (ui().machine_info().has_keyboard() && machine().ioport().natkeyboard().can_post())
		item_append(_("Keyboard Mode"), "", 0, (void *)KEYBOARD_MODE);

	item_append(_("Slider Controls"), "", 0, (void *)SLIDERS);

	item_append(_("Video Options"), "", 0, (machine().render().target_by_index(1) != nullptr) ? (void *)VIDEO_TARGETS : (void *)VIDEO_OPTIONS);

	if (machine().crosshair().get_usage())
		item_append(_("Crosshair Options"), "", 0, (void *)CROSSHAIR);

	if (machine().options().cheat())
		item_append(_("Cheat"), "", 0, (void *)CHEAT);

	if (machine().options().plugins())
		item_append(_("Plugin Options"), "", 0, (void *)PLUGINS);

	if (mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", "", true))
		item_append(_("External DAT View"), "", 0, (void *)EXTERNAL_DATS);

	item_append(menu_item_type::SEPARATOR);

	if (!mame_machine_manager::instance()->favorite().is_favorite(machine()))
		item_append(_("Add To Favorites"), "", 0, (void *)ADD_FAVORITE);
	else
		item_append(_("Remove From Favorites"), "", 0, (void *)REMOVE_FAVORITE);

	item_append(menu_item_type::SEPARATOR);

//  item_append(_("Quit from Machine"), nullptr, 0, (void *)QUIT_GAME);

	item_append(_("Select New Machine"), "", 0, (void *)SELECT_GAME);
}

menu_main::~menu_main()
{
}

/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

void menu_main::handle()
{
	/* process the menu */
	const event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->iptkey == IPT_UI_SELECT) {
		switch((long long)(menu_event->itemref)) {
		case INPUT_GROUPS:
			menu::stack_push<menu_input_groups>(ui(), container());
			break;

		case INPUT_SPECIFIC:
			menu::stack_push<menu_input_specific>(ui(), container());
			break;

		case SETTINGS_DIP_SWITCHES:
			menu::stack_push<menu_settings_dip_switches>(ui(), container());
			break;

		case SETTINGS_DRIVER_CONFIG:
			menu::stack_push<menu_settings_machine_config>(ui(), container());
			break;

		case ANALOG:
			menu::stack_push<menu_analog>(ui(), container());
			break;

		case BOOKKEEPING:
			menu::stack_push<menu_bookkeeping>(ui(), container());
			break;

		case GAME_INFO:
			menu::stack_push<menu_game_info>(ui(), container());
			break;

		case IMAGE_MENU_IMAGE_INFO:
			menu::stack_push<menu_image_info>(ui(), container());
			break;

		case IMAGE_MENU_FILE_MANAGER:
			menu::stack_push<menu_file_manager>(ui(), container(), nullptr);
			break;

		case TAPE_CONTROL:
			menu::stack_push<menu_tape_control>(ui(), container(), nullptr);
			break;

		case PTY_INFO:
			menu::stack_push<menu_pty_info>(ui(), container());
			break;

		case SLOT_DEVICES:
			menu::stack_push<menu_slot_devices>(ui(), container());
			break;

		case NETWORK_DEVICES:
			menu::stack_push<menu_network_devices>(ui(), container());
			break;

		case KEYBOARD_MODE:
			menu::stack_push<menu_keyboard_mode>(ui(), container());
			break;

		case SLIDERS:
			menu::stack_push<menu_sliders>(ui(), container(), false);
			break;

		case VIDEO_TARGETS:
			menu::stack_push<menu_video_targets>(ui(), container());
			break;

		case VIDEO_OPTIONS:
			menu::stack_push<menu_video_options>(ui(), container(), machine().render().first_target());
			break;

		case CROSSHAIR:
			menu::stack_push<menu_crosshair>(ui(), container());
			break;

		case CHEAT:
			menu::stack_push<menu_cheat>(ui(), container());
			break;

		case PLUGINS:
			menu::stack_push<menu_plugin>(ui(), container());
			break;

		case SELECT_GAME:
			if (machine().options().ui() == emu_options::UI_SIMPLE)
				menu::stack_push<simple_menu_select_game>(ui(), container(), nullptr);
			else
				menu::stack_push<menu_select_game>(ui(), container(), nullptr);
			break;

		case BIOS_SELECTION:
			menu::stack_push<menu_bios_selection>(ui(), container());
			break;

		case BARCODE_READ:
			menu::stack_push<menu_barcode_reader>(ui(), container(), nullptr);
			break;

		case EXTERNAL_DATS:
			menu::stack_push<menu_dats_view>(ui(), container());
			break;

		case ADD_FAVORITE:
			mame_machine_manager::instance()->favorite().add_favorite(machine());
			reset(reset_options::REMEMBER_POSITION);
			break;

		case REMOVE_FAVORITE:
			mame_machine_manager::instance()->favorite().remove_favorite(machine());
			reset(reset_options::REMEMBER_POSITION);
			break;

		case QUIT_GAME:
			stack_pop();
			ui().request_quit();
			break;

		default:
			fatalerror("ui::menu_main::handle - unknown reference\n");
		}
	}
}

} // namespace ui
