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
		if (readbuf[0] == '[')
		{
			size_t found = readbuf.find("]");
			if (found == std::string::npos)
				return;

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
	std::string	 carriage("\r\n");

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
		// get the base name
		if (image->basename() != NULL)
		{
			const software_info *swinfo = image->software_entry();
			ui_software_info tmpmatches;
			if (swinfo->shortname()) tmpmatches.shortname.assign(swinfo->shortname());
			if (swinfo->longname()) tmpmatches.longname.assign(swinfo->longname());
			if (swinfo->parentname()) tmpmatches.parentname.assign(swinfo->parentname());
			if (swinfo->year()) tmpmatches.year.assign(swinfo->year());
			if (swinfo->publisher()) tmpmatches.publisher.assign(swinfo->publisher());
			tmpmatches.supported = swinfo->supported();
			if (image->part_entry()->name()) tmpmatches.part.assign(image->part_entry()->name());
			tmpmatches.driver = &machine().system();
			if (image->software_list_name()) tmpmatches.listname.assign(image->software_list_name());
			if (image->part_entry()->interface()) tmpmatches.interface.assign(image->part_entry()->interface());
			tmpmatches.instance.clear();
			tmpmatches.startempty = 0;
			tmpmatches.parentlongname.clear();
			tmpmatches.usage.clear();
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
		if (image->filename() != NULL)
		{
			image_loaded = true;
			const software_info *swinfo = image->software_entry();
			const software_part *swpart = image->part_entry();

			for (size_t current = 0; current < favorite_list.size(); current++)
				if (favorite_list[current].shortname.compare(swinfo->shortname()) == 0)
					if (swpart != NULL)
						if (swpart->name() != NULL && favorite_list[current].part.compare(swpart->name()) == 0)
						{
							current_favorite = current;
							return true;
						}
		}
	}

	if (!image_loaded)
		for (size_t current = 0; current < favorite_list.size(); current++)
			if (favorite_list[current].driver == &machine().system())
			{
				current_favorite = current;
				return true;
			}

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
	if (parseOpen(favorite_filename))
	{
		std::ifstream myfile(fullpath.c_str());
		std::string readbuf;

		std::getline(myfile, readbuf);
		while (readbuf[0] == '[')
			std::getline(myfile, readbuf);

		while (std::getline(myfile, readbuf))
		{
			ui_software_info tmpmatches;
			tmpmatches.shortname = readbuf;
			std::getline(myfile, tmpmatches.longname);
			std::getline(myfile, tmpmatches.parentname);
			std::getline(myfile, tmpmatches.year);
			std::getline(myfile, tmpmatches.publisher);
			myfile >> tmpmatches.supported;
			std::getline(myfile, tmpmatches.part);
			std::getline(myfile, readbuf);
			int dx = driver_list::find(readbuf.c_str());
			tmpmatches.driver = &driver_list::driver(dx);
			std::getline(myfile, tmpmatches.listname);
			std::getline(myfile, tmpmatches.interface);
			std::getline(myfile, tmpmatches.instance);
			myfile >> tmpmatches.startempty;
			std::getline(myfile, tmpmatches.parentlongname);
			std::getline(myfile, tmpmatches.usage);
			std::getline(myfile, tmpmatches.devicetype);
			myfile >> tmpmatches.available;
			favorite_list.push_back(tmpmatches);
		}
		myfile.close();
	}
}

//-------------------------------------------------
//  save favorite
//-------------------------------------------------

void favorite_manager::save_favorite_games()
{
	// if the list is empty, then deletes the file
	if (favorite_list.empty())
	{
		if (parseOpen(favorite_filename))
			remove(fullpath.c_str());
		return;
	}

	// attempt to open the output file
	emu_file file(MEWUI_DIR, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	if (file.open(favorite_filename) == FILERR_NONE)
	{
		std::string filename(file.fullpath());
		file.close();

		std::ofstream myfile(filename.c_str());

		// generate the favorite INI
		std::string favtext;
		std::string headtext("[ROOT_FOLDER]\n[Favorite]\n\n");
		myfile << headtext;

		for (size_t current = 0; current < favorite_list.size(); current++)
		{
			myfile << favorite_list[current].shortname << "\n";
			myfile << favorite_list[current].longname << "\n";
			myfile << favorite_list[current].parentname << "\n";
			myfile << favorite_list[current].year << "\n";
			myfile << favorite_list[current].publisher << "\n";
			myfile << favorite_list[current].supported;
			myfile << favorite_list[current].part << "\n";
			myfile << favorite_list[current].driver->name << "\n";
			myfile << favorite_list[current].listname << "\n";
			myfile << favorite_list[current].interface << "\n";
			myfile << favorite_list[current].instance << "\n";
			myfile << favorite_list[current].startempty;
			myfile << favorite_list[current].parentlongname << "\n";
			myfile << favorite_list[current].usage << "\n";
			myfile << favorite_list[current].devicetype << "\n";
			myfile << favorite_list[current].available;
		}
		myfile.close();
	}
}

//-------------------------------------------------
//  open up file for reading
//-------------------------------------------------

//-------------------------------------------------
//  open up file for reading
//-------------------------------------------------

bool favorite_manager::parseOpen(const char *filename)
{
	// Open file up in binary mode
	emu_file fp(MEWUI_DIR, OPEN_FLAG_READ);

	if (fp.open(filename) == FILERR_NONE)
	{
		fullpath.assign(fp.fullpath());
		fp.close();
		return true;
	}

	return false;
}
