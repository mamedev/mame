// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Maurizio Petrarota
/*********************************************************************

    ui/miscmenu.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "ui/miscmenu.h"

#include "ui/inifile.h"
#include "ui/selector.h"
#include "ui/submenu.h"
#include "ui/ui.h"

#include "infoxml.h"
#include "mame.h"

#include "osdnet.h"
#include "mameopts.h"
#include "pluginopts.h"
#include "drivenum.h"
#include "romload.h"

#include "uiinput.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>


namespace ui {

/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    menu_bios_selection - populates the main
    bios selection menu
-------------------------------------------------*/

menu_bios_selection::menu_bios_selection(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_bios_selection::populate(float &customtop, float &custombottom)
{
	// cycle through all devices for this system
	for (device_t &device : device_enumerator(machine().root_device()))
	{
		device_t const *const parent(device.owner());
		device_slot_interface const *const slot(dynamic_cast<device_slot_interface const *>(parent));
		if (!parent || (slot && (slot->get_card_device() == &device)))
		{
			tiny_rom_entry const *rom(device.rom_region());
			if (rom && !ROMENTRY_ISEND(rom))
			{
				char const *val = nullptr;
				for ( ; !ROMENTRY_ISEND(rom) && !val; rom++)
				{
					if (ROMENTRY_ISSYSTEM_BIOS(rom) && ROM_GETBIOSFLAGS(rom) == device.system_bios())
						val = rom->hashdata;
				}
				if (val)
					item_append(!parent ? "driver" : (device.tag() + 1), val, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, (void *)&device);
			}
		}
	}

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset"), 0, (void *)1);
}

menu_bios_selection::~menu_bios_selection()
{
}

/*-------------------------------------------------
    menu_bios_selection - menu that
-------------------------------------------------*/

void menu_bios_selection::handle()
{
	/* process the menu */
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if ((uintptr_t)menu_event->itemref == 1 && menu_event->iptkey == IPT_UI_SELECT)
			machine().schedule_hard_reset();
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			device_t *dev = (device_t *)menu_event->itemref;
			int const cnt = ([bioses = romload::entries(dev->rom_region()).get_system_bioses()] () { return std::distance(bioses.begin(), bioses.end()); })();
			int val = dev->system_bios() + ((menu_event->iptkey == IPT_UI_LEFT) ? -1 : +1);
			if (val < 1)
				val = cnt;
			if (val > cnt)
				val = 1;
			dev->set_system_bios(val);
			if (strcmp(dev->tag(),":")==0) {
				machine().options().set_value("bios", val-1, OPTION_PRIORITY_CMDLINE);
			} else {
				const char *slot_option_name = dev->owner()->tag() + 1;
				machine().options().slot_option(slot_option_name).set_bios(string_format("%d", val - 1));
			}
			reset(reset_options::REMEMBER_REF);
		}
	}
}



menu_network_devices::menu_network_devices(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_network_devices::~menu_network_devices()
{
}

/*-------------------------------------------------
    menu_network_devices_populate - populates the main
    network device menu
-------------------------------------------------*/

void menu_network_devices::populate(float &customtop, float &custombottom)
{
	/* cycle through all devices for this system */
	for (device_network_interface &network : network_interface_enumerator(machine().root_device()))
	{
		int curr = network.get_interface();
		const char *title = nullptr;
		for(auto &entry : get_netdev_list())
		{
			if(entry->id==curr) {
				title = entry->description;
				break;
			}
		}

		item_append(network.device().tag(),  (title) ? title : "------", FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, (void *)&network);
	}
}

/*-------------------------------------------------
    menu_network_devices - menu that
-------------------------------------------------*/

void menu_network_devices::handle()
{
	/* process the menu */
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT) {
			device_network_interface *network = (device_network_interface *)menu_event->itemref;
			int curr = network->get_interface();
			if (menu_event->iptkey == IPT_UI_LEFT) curr--; else curr++;
			if (curr==-2) curr = netdev_count() - 1;
			network->set_interface(curr);
			reset(reset_options::REMEMBER_REF);
		}
	}
}


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/

menu_bookkeeping::menu_bookkeeping(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_bookkeeping::~menu_bookkeeping()
{
}

void menu_bookkeeping::handle()
{
	/* process the menu */
	process(0);

	/* if the time has rolled over another second, regenerate */
	attotime const curtime = machine().time();
	if (prevtime.seconds() != curtime.seconds())
		reset(reset_options::REMEMBER_POSITION);
}

void menu_bookkeeping::populate(float &customtop, float &custombottom)
{
	int tickets = machine().bookkeeping().get_dispensed_tickets();
	std::ostringstream tempstring;
	int ctrnum;

	/* show total time first */
	prevtime = machine().time();
	if (prevtime.seconds() >= (60 * 60))
		util::stream_format(tempstring, _("Uptime: %1$d:%2$02d:%3$02d\n\n"), prevtime.seconds() / (60 * 60), (prevtime.seconds() / 60) % 60, prevtime.seconds() % 60);
	else
		util::stream_format(tempstring, _("Uptime: %1$d:%2$02d\n\n"), (prevtime.seconds() / 60) % 60, prevtime.seconds() % 60);

	/* show tickets at the top */
	if (tickets > 0)
		util::stream_format(tempstring, _("Tickets dispensed: %1$d\n\n"), tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < bookkeeping_manager::COIN_COUNTERS; ctrnum++)
	{
		int count = machine().bookkeeping().coin_counter_get_count(ctrnum);

		/* display the coin counter number */
		/* display how many coins */
		/* display whether or not we are locked out */
		util::stream_format(tempstring,
				(count == 0) ? _("Coin %1$c: NA%3$s\n") : _("Coin %1$c: %2$d%3$s\n"),
				ctrnum + 'A',
				count,
				machine().bookkeeping().coin_lockout_get_state(ctrnum) ? _(" (locked)") : "");
	}

	/* append the single item */
	item_append(tempstring.str(), FLAG_MULTILINE, nullptr);
}

/*-------------------------------------------------
    menu_crosshair - handle the crosshair settings
    menu
-------------------------------------------------*/

void menu_crosshair::handle()
{
	// process the menu
	event const *const menu_event(process(PROCESS_LR_REPEAT));

	// handle events
	if (menu_event && menu_event->itemref)
	{
		crosshair_item_data &data(*reinterpret_cast<crosshair_item_data *>(menu_event->itemref));
		bool changed(false);
		int newval(data.cur);

		switch (menu_event->iptkey)
		{
		// if selected, reset to default value
		case IPT_UI_SELECT:
			newval = data.defvalue;
			break;

		// left decrements
		case IPT_UI_LEFT:
			newval -= machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
			break;

		// right increments
		case IPT_UI_RIGHT:
			newval += machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
			break;
		}

		// clamp to range
		if (newval < data.min)
			newval = data.min;
		if (newval > data.max)
			newval = data.max;

		// if things changed, update
		if (newval != data.cur)
		{
			switch (data.type)
			{
			// visibility state
			case CROSSHAIR_ITEM_VIS:
				data.crosshair->set_mode(newval);
				// set visibility as specified by mode - auto mode starts with visibility off
				data.crosshair->set_visible(newval == CROSSHAIR_VISIBILITY_ON);
				changed = true;
				break;

			// auto time
			case CROSSHAIR_ITEM_AUTO_TIME:
				machine().crosshair().set_auto_time(newval);
				changed = true;
				break;
			}
		}

		// crosshair graphic name
		if (data.type == CROSSHAIR_ITEM_PIC)
		{
			switch (menu_event->iptkey)
			{
			case IPT_UI_SELECT:
				{
					std::vector<std::string> sel;
					sel.reserve(m_pics.size() + 1);
					sel.push_back("DEFAULT");
					std::copy(m_pics.begin(), m_pics.end(), std::back_inserter(sel));
					menu::stack_push<menu_selector>(
							ui(), container(), std::move(sel), data.cur,
							[this, &data] (int selection)
							{
								if (!selection)
									data.crosshair->set_default_bitmap();
								else
									data.crosshair->set_bitmap_name(m_pics[selection - 1].c_str());
								reset(reset_options::REMEMBER_REF);
							});
				}
				break;

			case IPT_UI_LEFT:
				data.crosshair->set_bitmap_name(data.last_name.c_str());
				changed = true;
				break;

			case IPT_UI_RIGHT:
				data.crosshair->set_bitmap_name(data.next_name.c_str());
				changed = true;
				break;
			}
		}

		if (changed)
			reset(reset_options::REMEMBER_REF); // rebuild the menu
	}
}


/*-------------------------------------------------
    menu_crosshair_populate - populate the
    crosshair settings menu
-------------------------------------------------*/

menu_crosshair::menu_crosshair(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_crosshair::populate(float &customtop, float &custombottom)
{
	if (m_data.empty())
	{
		// loop over player and add the manual items
		for (int player = 0; player < MAX_PLAYERS; player++)
		{
			// get the user settings
			render_crosshair &crosshair(machine().crosshair().get_crosshair(player));

			// add menu items for usable crosshairs
			if (crosshair.is_used())
			{
				// CROSSHAIR_ITEM_VIS - allocate a data item and fill it
				crosshair_item_data &visdata(m_data.emplace_back());
				visdata.crosshair = &crosshair;
				visdata.type = CROSSHAIR_ITEM_VIS;
				visdata.player = player;
				visdata.min = CROSSHAIR_VISIBILITY_OFF;
				visdata.max = CROSSHAIR_VISIBILITY_AUTO;
				visdata.defvalue = CROSSHAIR_VISIBILITY_DEFAULT;

				// CROSSHAIR_ITEM_PIC - allocate a data item and fill it
				crosshair_item_data &picdata(m_data.emplace_back());
				picdata.crosshair = &crosshair;
				picdata.type = CROSSHAIR_ITEM_PIC;
				picdata.player = player;
				// other data item not used by this menu
			}
		}

		// CROSSHAIR_ITEM_AUTO_TIME - allocate a data item and fill it
		crosshair_item_data &timedata(m_data.emplace_back());
		timedata.type = CROSSHAIR_ITEM_AUTO_TIME;
		timedata.min = CROSSHAIR_VISIBILITY_AUTOTIME_MIN;
		timedata.max = CROSSHAIR_VISIBILITY_AUTOTIME_MAX;
		timedata.defvalue = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;
	}

	if (m_pics.empty())
	{
		// open a path to the crosshairs
		file_enumerator path(machine().options().crosshair_path());
		for (osd::directory::entry const *dir = path.next(); dir; dir = path.next())
		{
			// look for files ending in .png
			size_t const length(std::strlen(dir->name));
			if ((length > 4) && core_filename_ends_with(dir->name, ".png"))
				m_pics.emplace_back(dir->name, length - 4);
		}
		std::stable_sort(
				m_pics.begin(),
				m_pics.end(),
				[] (std::string const &a, std::string const &b) { return 0 > core_stricmp(a.c_str(), b.c_str()); });
	}

	// Make sure to keep these matched to the CROSSHAIR_VISIBILITY_xxx types
	static char const *const vis_text[] = { "Off", "On", "Auto" };

	bool use_auto = false;
	for (crosshair_item_data &data : m_data)
	{
		switch (data.type)
		{
		case CROSSHAIR_ITEM_VIS:
			{
				// track if we need the auto time menu
				if (data.crosshair->mode() == CROSSHAIR_VISIBILITY_AUTO)
					use_auto = true;

				data.cur = data.crosshair->mode();

				// put on arrows
				uint32_t flags(0U);
				if (data.cur > data.min)
					flags |= FLAG_LEFT_ARROW;
				if (data.cur < data.max)
					flags |= FLAG_RIGHT_ARROW;

				// add CROSSHAIR_ITEM_VIS menu */
				item_append(util::string_format(_("P%d Visibility"), data.player + 1), vis_text[data.crosshair->mode()], flags, &data);
			}
			break;

		case CROSSHAIR_ITEM_PIC:
			// search for crosshair graphics
			{
				// reset search flags
				bool const using_default(*data.crosshair->bitmap_name() == '\0');
				bool finished(false);
				bool found(false);
				data.cur = using_default ? 0U : 1U;
				data.last_name.clear();
				data.next_name.clear();

				// look for the current name, then remember the name before and find the next name
				for (auto it = m_pics.begin(); it != m_pics.end() && !finished; ++it)
				{
					// if we are using the default, then we just need to find the first in the list
					if (found || using_default)
					{
						// get the next name
						data.next_name = *it;
						finished = true;
					}
					else if (data.crosshair->bitmap_name() == *it)
					{
						// we found the current name so loop once more to find the next name
						found = true;
					}
					else
					{
						// remember last name - we will do it here in case files get added to the directory
						++data.cur;
						data.last_name = *it;
					}
				}

				// if name not found then next item is DEFAULT
				if (!found && !using_default)
				{
					data.cur = 0U;
					data.next_name.clear();
					finished = true;
				}

				// set up the selection flags
				uint32_t flags(0U);
				if (finished)
					flags |= FLAG_RIGHT_ARROW;
				if (found)
					flags |= FLAG_LEFT_ARROW;

				// add CROSSHAIR_ITEM_PIC menu
				item_append(util::string_format(_("P%d Crosshair"), data.player + 1), using_default ? "DEFAULT" : data.crosshair->bitmap_name(), flags, &data);
			}
			break;

		case CROSSHAIR_ITEM_AUTO_TIME:
			if (use_auto)
			{
				data.cur = machine().crosshair().auto_time();

				// put on arrows in visible menu
				uint32_t flags(0U);
				if (data.cur > data.min)
					flags |= FLAG_LEFT_ARROW;
				if (data.cur < data.max)
					flags |= FLAG_RIGHT_ARROW;

				// add CROSSHAIR_ITEM_AUTO_TIME menu
				item_append(_("Visible Delay"), util::string_format("%d", data.cur), flags, &data);
			}
			else
			{
				// leave a blank filler line when not in auto time so size does not rescale
				//item_append("", "", nullptr, nullptr);
			}
			break;
		}
	}
}

menu_crosshair::~menu_crosshair()
{
}

/*-------------------------------------------------
    menu_quit_game - handle the "menu" for
    quitting the game
-------------------------------------------------*/

menu_quit_game::menu_quit_game(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_quit_game::~menu_quit_game()
{
}

void menu_quit_game::populate(float &customtop, float &custombottom)
{
}

void menu_quit_game::handle()
{
	/* request a reset */
	machine().schedule_exit();

	/* reset the menu stack */
	stack_reset();
}

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_export::menu_export(mame_ui_manager &mui, render_container &container, std::vector<const game_driver *> &&drvlist)
	: menu(mui, container), m_list(std::move(drvlist))
{
}

menu_export::~menu_export()
{
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void menu_export::handle()
{
	// process the menu
	process_parent();
	const event *const menu_event = process(PROCESS_NOIMAGE);
	if (menu_event && menu_event->itemref)
	{
		switch (uintptr_t(menu_event->itemref))
		{
		case 1:
		case 3:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				std::string filename("exported");
				emu_file infile(ui().options().ui_path(), OPEN_FLAG_READ);
				if (infile.open(filename + ".xml") == osd_file::error::NONE)
					for (int seq = 0; ; ++seq)
					{
						const std::string seqtext = string_format("%s_%04d", filename, seq);
						if (infile.open(seqtext + ".xml") != osd_file::error::NONE)
						{
							filename = seqtext;
							break;
						}
					}

				// attempt to open the output file
				emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
				if (file.open(filename + ".xml") == osd_file::error::NONE)
				{
					const std::string fullpath(file.fullpath());
					file.close();
					std::ofstream pfile(fullpath);

					// prepare a filter for the drivers we want to show
					std::unordered_set<const game_driver *> driver_list(m_list.begin(), m_list.end());
					auto filter = [&driver_list](const char *shortname, bool &)
					{
						auto iter = std::find_if(
							driver_list.begin(),
							driver_list.end(),
							[shortname](const game_driver *driver) { return !strcmp(shortname, driver->name); });
						return iter != driver_list.end();
					};

					// do we want to show devices?
					bool include_devices = uintptr_t(menu_event->itemref) == 1;

					// and do the dirty work
					info_xml_creator creator(machine().options());
					creator.output(pfile, filter, include_devices);
					machine().popmessage(_("%s.xml saved under ui folder."), filename);
				}
			}
			break;
		case 2:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				std::string filename("exported");
				emu_file infile(ui().options().ui_path(), OPEN_FLAG_READ);
				if (infile.open(filename + ".txt") == osd_file::error::NONE)
					for (int seq = 0; ; ++seq)
					{
						const std::string seqtext = string_format("%s_%04d", filename, seq);
						if (infile.open(seqtext + ".txt") != osd_file::error::NONE)
						{
							filename = seqtext;
							break;
						}
					}

				// attempt to open the output file
				emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
				if (file.open(filename + ".txt") == osd_file::error::NONE)
				{
					// print the header
					std::ostringstream buffer;
					buffer << _("Name:             Description:\n");
					driver_enumerator drvlist(machine().options());
					drvlist.exclude_all();
					for (auto & elem : m_list)
						drvlist.include(driver_list::find(*elem));

					// iterate through drivers and output the info
					while (drvlist.next())
						util::stream_format(buffer, "%-18s\"%s\"\n", drvlist.driver().name, drvlist.driver().type.fullname());
					file.puts(buffer.str());
					file.close();
					machine().popmessage(_("%s.txt saved under ui folder."), filename);
				}
			}
			break;
		default:
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_export::populate(float &customtop, float &custombottom)
{
	// add options items
	item_append(_("Export list in XML format (like -listxml)"), 0, (void *)(uintptr_t)1);
	item_append(_("Export list in XML format (like -listxml, but exclude devices)"), 0, (void *)(uintptr_t)3);
	item_append(_("Export list in TXT format (like -listfull)"), 0, (void *)(uintptr_t)2);
	item_append(menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_machine_configure::menu_machine_configure(
		mame_ui_manager &mui,
		render_container &container,
		game_driver const &drv,
		std::function<void (bool, bool)> &&handler,
		float x0, float y0)
	: menu(mui, container)
	, m_handler(std::move(handler))
	, m_drv(drv)
	, m_x0(x0)
	, m_y0(y0)
	, m_curbios(0)
	, m_was_favorite(mame_machine_manager::instance()->favorite().is_favorite_system(drv))
	, m_want_favorite(m_was_favorite)
{
	// parse the INI file
	std::ostringstream error;
	osd_setup_osd_specific_emu_options(m_opts);
	mame_options::parse_standard_inis(m_opts, error, &m_drv);
	setup_bios();
}

menu_machine_configure::~menu_machine_configure()
{
	if (m_was_favorite != m_want_favorite)
	{
		if (m_want_favorite)
			mame_machine_manager::instance()->favorite().add_favorite_system(m_drv);
		else
			mame_machine_manager::instance()->favorite().remove_favorite_system(m_drv);
	}

	if (m_handler)
		m_handler(m_want_favorite, m_was_favorite != m_want_favorite);
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void menu_machine_configure::handle()
{
	// process the menu
	process_parent();
	const event *menu_event = process(PROCESS_NOIMAGE, m_x0, m_y0);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			switch ((uintptr_t)menu_event->itemref)
			{
			case SAVE:
				{
					const std::string filename(m_drv.name);
					emu_file file(machine().options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
					osd_file::error filerr = file.open(filename + ".ini");
					if (filerr == osd_file::error::NONE)
					{
						std::string inistring = m_opts.output_ini();
						file.puts(inistring);
						ui().popup_time(2, "%s", _("\n    Configuration saved    \n\n"));
					}
				}
				break;
			case ADDFAV:
				m_want_favorite = true;
				reset(reset_options::REMEMBER_POSITION);
				break;
			case DELFAV:
				m_want_favorite = false;
				reset(reset_options::REMEMBER_POSITION);
				break;
			case CONTROLLER:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<submenu>(ui(), container(), submenu::control_options, &m_drv, &m_opts);
				break;
			case VIDEO:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<submenu>(ui(), container(), submenu::video_options, &m_drv, &m_opts);
				break;
			case ADVANCED:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<submenu>(ui(), container(), submenu::advanced_options, &m_drv, &m_opts);
				break;
			default:
				break;
			}
		}
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			(menu_event->iptkey == IPT_UI_LEFT) ? --m_curbios : ++m_curbios;
			m_opts.set_value(OPTION_BIOS, m_bios[m_curbios].second, OPTION_PRIORITY_CMDLINE);
			reset(reset_options::REMEMBER_POSITION);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_machine_configure::populate(float &customtop, float &custombottom)
{
	// add options items
	item_append(_("BIOS"), FLAG_DISABLE | FLAG_UI_HEADING, nullptr);
	if (!m_bios.empty())
	{
		uint32_t arrows = get_arrow_flags(std::size_t(0), m_bios.size() - 1, m_curbios);
		item_append(_("Driver"), m_bios[m_curbios].first, arrows, (void *)(uintptr_t)BIOS);
	}
	else
		item_append(_("This machine has no BIOS."), FLAG_DISABLE, nullptr);

	item_append(menu_item_type::SEPARATOR);
	item_append(_(submenu::advanced_options[0].description), 0, (void *)(uintptr_t)ADVANCED);
	item_append(_(submenu::video_options[0].description), 0, (void *)(uintptr_t)VIDEO);
	item_append(_(submenu::control_options[0].description), 0, (void *)(uintptr_t)CONTROLLER);
	item_append(menu_item_type::SEPARATOR);

	if (!m_want_favorite)
		item_append(_("Add To Favorites"), 0, (void *)ADDFAV);
	else
		item_append(_("Remove From Favorites"), 0, (void *)DELFAV);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Save machine configuration"), 0, (void *)(uintptr_t)SAVE);
	item_append(menu_item_type::SEPARATOR);
	customtop = 2.0f * ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_machine_configure::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("Configure machine:"), m_drv.type.fullname() };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}

void menu_machine_configure::setup_bios()
{
	if (!m_drv.rom)
		return;

	std::string specbios(m_opts.bios());
	char const *default_name(nullptr);
	for (tiny_rom_entry const *rom = m_drv.rom; !ROMENTRY_ISEND(rom); ++rom)
	{
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			default_name = rom->name;
	}

	std::size_t bios_count = 0;
	for (romload::system_bios const &bios : romload::entries(m_drv.rom).get_system_bioses())
	{
		std::string name(bios.get_description());
		u32 const bios_flags(bios.get_value());
		std::string const bios_number(std::to_string(bios_flags - 1));

		// check biosnumber and name
		if ((bios_number == specbios) || (specbios == bios.get_name()))
			m_curbios = bios_count;

		if (default_name && !std::strcmp(bios.get_name(), default_name))
		{
			name.append(_(" (default)"));
			if (specbios == "default")
				m_curbios = bios_count;
		}

		m_bios.emplace_back(std::move(name), bios_flags - 1);
		bios_count++;
	}
}

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_plugins_configure::menu_plugins_configure(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
}

menu_plugins_configure::~menu_plugins_configure()
{
	emu_file file_plugin(machine().options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file_plugin.open("plugin.ini") != osd_file::error::NONE)
		// Can't throw in a destructor, so let's ignore silently for
		// now.  We shouldn't write files in a destructor in any case.
		//
		// throw emu_fatalerror("Unable to create file plugin.ini\n");
		return;
	// generate the updated INI
	file_plugin.puts(mame_machine_manager::instance()->plugins().output_ini());
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void menu_plugins_configure::handle()
{
	// process the menu
	bool changed = false;
	plugin_options& plugins = mame_machine_manager::instance()->plugins();
	process_parent();
	const event *menu_event = process(PROCESS_NOIMAGE);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
		{
			plugin_options::plugin *p = plugins.find((const char*)menu_event->itemref);
			if (p)
			{
				p->m_start = !p->m_start;
				changed = true;
			}
		}
	}
	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_plugins_configure::populate(float &customtop, float &custombottom)
{
	plugin_options& plugins = mame_machine_manager::instance()->plugins();

	for (auto &curentry : plugins.plugins())
	{
		bool enabled = curentry.m_start;
		item_append_on_off(curentry.m_description, enabled, 0, (void *)(uintptr_t)curentry.m_name.c_str());
	}
	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + (3.0f * ui().box_tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_plugins_configure::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const toptext[] = { _("Plugins") };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}

} // namespace ui
