/***************************************************************************

    mewui/inifile.c

    MEWUI inifile system.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"
#include "mewui/inifile.h"
#include "mewui/utils.h"
#include <fstream>

//-------------------------------------------------
//  GLOBAL VARIABLES
//-------------------------------------------------

static const char *favorite_filename = "favorites.ini";
UINT16 inifile_manager::current_category = 0;
UINT16 inifile_manager::current_file = 0;

//-------------------------------------------------
//  ctor
//-------------------------------------------------

inifile_manager::inifile_manager(running_machine &machine)
	: m_machine(machine)
{
	ini_index.clear();
	directory_scan();
}

//-------------------------------------------------
//  scan directories and create index
//-------------------------------------------------

void inifile_manager::directory_scan()
{
	// open extra INIs folder
	file_enumerator path(machine().options().extraini_path());
	const osd_directory_entry *dir;

	// loop into folder's file
	while ((dir = path.next()) != NULL)
	{
		int length = strlen(dir->name);
		std::string file_name(dir->name);

		// skip mewui_favorite file
		if (!core_stricmp("mewui_favorite.ini", file_name.c_str()))
			continue;

		// check .ini file ending
		if ((length > 4) && dir->name[length - 4] == '.' && tolower((UINT8)dir->name[length - 3]) == 'i' &&
			tolower((UINT8)dir->name[length - 2]) == 'n' && tolower((UINT8)dir->name[length - 1]) == 'i')
		{
			// try to open file and indexing
			if (ParseOpen(file_name.c_str()))
			{
				std::vector<IniCategoryIndex> tmp;
				init_category(tmp, fullpath);
				if (!tmp.empty())
				{
					IniFileIndex tfile;
					tfile.name.assign(file_name);
					tfile.category = tmp;
					ini_index.push_back(tfile);
				}
			}
		}
	}
}

//-------------------------------------------------
//  initialize category
//-------------------------------------------------

void inifile_manager::init_category(std::vector<IniCategoryIndex> &index, std::string &filename)
{
	std::string readbuf;
	std::ifstream myfile(filename.c_str(), std::ifstream::binary);
	while (std::getline(myfile, readbuf))
	{
		if (!readbuf.empty() && readbuf[0] == '[')
		{
			size_t found = readbuf.find("]");
			std::string name = readbuf.substr(1, found - 1);
			if (name.compare("FOLDER_SETTINGS") == 0 || name.compare("ROOT_FOLDER") == 0)
				continue;
			else
			{
				IniCategoryIndex tmp;
				tmp.name.assign(name);
				tmp.offset = myfile.tellg();
				index.push_back(tmp);
			}
		}
	}
	myfile.close();
}


//-------------------------------------------------
//  closes the existing opened file (if any)
//-------------------------------------------------

void inifile_manager::load_ini_category(std::vector<int> &temp_filter)
{
	if (ini_index.empty())
		return;

	bool search_clones = false;
	std::string file_name(ini_index[current_file].name);
	long offset = ini_index[current_file].category[current_category].offset;
	std::string carriage("\r\n");

	if (!core_stricmp(file_name.c_str(), "category.ini") || !core_stricmp(file_name.c_str(), "alltime.ini"))
		search_clones = true;

	if (ParseOpen(file_name.c_str()))
	{
		std::ifstream myfile(fullpath.c_str(), std::ifstream::binary);
		int num_game = driver_list::total();
		std::string readbuf;
		myfile.seekg(offset, myfile.beg);
		while (std::getline(myfile, readbuf))
		{
			if (readbuf[0] == '[') break;
			std::string name;
			size_t found = readbuf.find_last_not_of(carriage);
			name.assign(readbuf.substr(0, found + 1));
			if (name.empty()) break;
			int dfind = driver_list::find(name.c_str());
			if (dfind != -1 && search_clones)
			{
				temp_filter.push_back(dfind);
				int clone_of = driver_list::non_bios_clone(dfind);
				if (clone_of == -1)
				{
					for (int x = 0; x < num_game; x++)
						if (name.compare(driver_list::driver(x).parent) == 0 && name.compare(driver_list::driver(x).name) != 0)
							temp_filter.push_back(x);
				}
			}
			else if (dfind != -1)
				temp_filter.push_back(dfind);
		}
		myfile.close();
	}
}

//-------------------------------------------------
//  open up file for reading
//-------------------------------------------------

bool inifile_manager::ParseOpen(const char *filename)
{
	// Open file up in binary mode
	emu_file fp(machine().options().extraini_path(), OPEN_FLAG_READ);

	if (fp.open(filename) == FILERR_NONE)
	{
		fullpath.assign(fp.fullpath());
		fp.close();
		return true;
	}

	return false;
}

/**************************************************************************
    FAVORITE MANAGER
**************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

favorite_manager::favorite_manager(running_machine &machine)
	: m_machine(machine)
{
	parse_favorite();
}

//-------------------------------------------------
//  add a game
//-------------------------------------------------

void favorite_manager::add_favorite_game(const game_driver *driver)
{
	ui_software_info tmpmatches;
	tmpmatches.shortname.assign(driver->name);
	tmpmatches.longname.assign(driver->description);
	tmpmatches.parentname.clear();
	tmpmatches.year.clear();
	tmpmatches.publisher.clear();
	tmpmatches.supported = 0;
	tmpmatches.part.clear();
	tmpmatches.driver = driver;
	tmpmatches.listname.clear();
	tmpmatches.interface.clear();
	tmpmatches.instance.clear();
	tmpmatches.startempty = 1;
	tmpmatches.parentlongname.clear();
	tmpmatches.usage.clear();
	tmpmatches.devicetype.clear();
	tmpmatches.available = true;
	favorite_list.push_back(tmpmatches);
	save_favorite_games();
}

//-------------------------------------------------
//  add a system
//-------------------------------------------------

void favorite_manager::add_favorite_game(ui_software_info &swinfo)
{
	favorite_list.push_back(swinfo);
	save_favorite_games();
}

//-------------------------------------------------
//  add a game / system
//-------------------------------------------------

void favorite_manager::add_favorite_game()
{
	if ((machine().system().flags & GAME_TYPE_ARCADE) != 0)
	{
		add_favorite_game(&machine().system());
		return;
	}

	bool software_avail = false;
	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		if (image->exists() && image->software_entry())
		{
			const software_info *swinfo = image->software_entry();
			ui_software_info tmpmatches;
			if (swinfo->shortname()) tmpmatches.shortname.assign(swinfo->shortname());
			if (image->longname()) tmpmatches.longname.assign(image->longname());
			if (swinfo->parentname()) tmpmatches.parentname.assign(swinfo->parentname());
			if (image->year()) tmpmatches.year.assign(image->year());
			if (image->manufacturer()) tmpmatches.publisher.assign(image->manufacturer());
			tmpmatches.supported = image->supported();
			if (image->part_entry()->name()) tmpmatches.part.assign(image->part_entry()->name());
			tmpmatches.driver = &machine().system();
			if (image->software_list_name()) tmpmatches.listname.assign(image->software_list_name());
			if (image->part_entry()->interface()) tmpmatches.interface.assign(image->part_entry()->interface());
			if (image->instance_name()) tmpmatches.instance.assign(image->instance_name());
			tmpmatches.startempty = 0;
			tmpmatches.parentlongname.clear();
			if (swinfo->parentname())
			{
				software_list_device *swlist = software_list_device::find_by_name(machine().config(), image->software_list_name());
				for (software_info *c_swinfo = swlist->first_software_info(); c_swinfo != NULL; c_swinfo = c_swinfo->next())
				{
					std::string c_parent(c_swinfo->parentname());
					if (!c_parent.empty() && !c_parent.compare(swinfo->shortname()))
						{
							tmpmatches.parentlongname.assign(c_swinfo->longname());
							break;
						}
				}
			}

			tmpmatches.usage.clear();
			for (feature_list_item *flist = swinfo->other_info(); flist != NULL; flist = flist->next())
				if (!strcmp(flist->name(), "usage"))
					tmpmatches.usage.assign(flist->value());

			if (image->image_type_name()) tmpmatches.devicetype.assign(image->image_type_name());
			tmpmatches.available = true;
			software_avail = true;
			favorite_list.push_back(tmpmatches);
			save_favorite_games();
		}
	}

	if (!software_avail)
		add_favorite_game(&machine().system());
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game(ui_software_info &swinfo)
{
	for (size_t x = 0; x < favorite_list.size(); x++)
		if (favorite_list[x] == swinfo)
		{
			favorite_list.erase(favorite_list.begin() + x);
			break;
		}

	save_favorite_games();
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game()
{
	favorite_list.erase(favorite_list.begin() + current_favorite);
	save_favorite_games();
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite()
{
	if ((machine().system().flags & GAME_TYPE_ARCADE) != 0)
		return isgame_favorite(&machine().system());

	image_interface_iterator iter(machine().root_device());
	bool image_loaded = false;

	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		if (image->exists() && image->software_entry())
		{
			image_loaded = true;
			const software_info *swinfo = image->software_entry();

			for (size_t current = 0; current < favorite_list.size(); current++)
				if (!favorite_list[current].shortname.compare(swinfo->shortname()) &&
				    !favorite_list[current].listname.compare(image->software_list_name()))
				{
					current_favorite = current;
					return true;
				}
		}
	}

	if (!image_loaded)
		return isgame_favorite(&machine().system());

	current_favorite = -1;
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(const game_driver *driver)
{
	for (size_t x = 0; x < favorite_list.size(); x++)
		if (favorite_list[x].driver == driver && favorite_list[x].shortname.compare(driver->name) == 0)
		{
			current_favorite = x;
			return true;
		}

	current_favorite = -1;
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(ui_software_info &swinfo)
{
	for (size_t x = 0; x < favorite_list.size(); x++)
		if (favorite_list[x] == swinfo)
		{
			current_favorite = x;
			return true;
		}

	current_favorite = -1;
	return false;
}

//-------------------------------------------------
//  parse favorite file
//-------------------------------------------------

void favorite_manager::parse_favorite()
{
	emu_file file(machine().options().mewui_path(), OPEN_FLAG_READ);

	if (file.open(favorite_filename) == FILERR_NONE)
	{
		char readbuf[1024];
		std::string text;

		file.gets(readbuf, 1024);
		while (readbuf[0] == '[')
			file.gets(readbuf, 1024);

		while (file.gets(readbuf, 1024))
		{
			ui_software_info tmpmatches;
			tmpmatches.shortname = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.longname = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.parentname = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.year = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.publisher = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.supported = atoi(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.part = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			text = strtrimspace(text.assign(readbuf));
			int dx = driver_list::find(text.c_str());
			if (dx == -1) continue;
			tmpmatches.driver = &driver_list::driver(dx);
			file.gets(readbuf, 1024);
			tmpmatches.listname = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.interface = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.instance = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.startempty = atoi(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.parentlongname = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.usage = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.devicetype = strtrimspace(text.assign(readbuf));
			file.gets(readbuf, 1024);
			tmpmatches.available = atoi(readbuf);
			favorite_list.push_back(tmpmatches);
		}
		file.close();
	}
}

//-------------------------------------------------
//  save favorite
//-------------------------------------------------

void favorite_manager::save_favorite_games()
{
	// attempt to open the output file
	emu_file file(machine().options().mewui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	if (file.open(favorite_filename) == FILERR_NONE)
	{
		if (favorite_list.empty())
		{
			file.remove_on_close();
			file.close();
			return;
		}

		// generate the favorite INI
		std::string text("[ROOT_FOLDER]\n[Favorite]\n\n");

		for (size_t current = 0; current < favorite_list.size(); current++)
		{
			text += favorite_list[current].shortname + "\n";
			text += favorite_list[current].longname + "\n";
			text += favorite_list[current].parentname + "\n";
			text += favorite_list[current].year + "\n";
			text += favorite_list[current].publisher + "\n";
			strcatprintf(text, "%d\n", favorite_list[current].supported);
			text += favorite_list[current].part + "\n";
			strcatprintf(text, "%s\n", favorite_list[current].driver->name);
			text += favorite_list[current].listname + "\n";
			text += favorite_list[current].interface + "\n";
			text += favorite_list[current].instance + "\n";
			strcatprintf(text, "%d\n", favorite_list[current].startempty);
			text += favorite_list[current].parentlongname + "\n";
			text += favorite_list[current].usage + "\n";
			text += favorite_list[current].devicetype + "\n";
			strcatprintf(text, "%d\n", favorite_list[current].available);
		}
		file.puts(text.c_str());
		file.close();
	}
}
