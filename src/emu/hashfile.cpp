// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    hashfile.cpp

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#include "emu.h"
#include "hashfile.h"

#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"

#include "hash.h"

#include <pugixml.hpp>


/*-------------------------------------------------
    hashfile_lookup
-------------------------------------------------*/

static bool read_hash_config(const char *hash_path, const util::hash_collection &hashes, const char *sysname, std::string &result)
{
	/* open a file */
	emu_file file(hash_path, OPEN_FLAG_READ);
	if (file.open(std::string(sysname) + ".hsi"))
		return false;

	pugi::xml_document doc;

	pugi::xml_parse_result res = doc.load_file(file.fullpath());
	if (res)
	{
		// Do search by CRC32 and SHA1
		std::string query = "/hashfile/hash[";
		auto crc = hashes.internal_string().substr(1,8);
		auto sha1 = hashes.internal_string().substr(10, 40);
		query += "@crc32='" + crc + "' and @sha1='" + sha1 + "']/extrainfo";
		pugi::xpath_node_set tools = doc.select_nodes(query.c_str());
		for (pugi::xpath_node_set::const_iterator it = tools.begin(); it != tools.end(); ++it)
		{
			result = it->node().first_child().value();
			return true;
		}

		// Try search by CRC32 only
		query = "/hashfile/hash[";
		query += "@crc32='" + crc + "']/extrainfo";
		tools = doc.select_nodes(query.c_str());
		for (pugi::xpath_node_set::const_iterator it = tools.begin(); it != tools.end(); ++it)
		{
			result = it->node().first_child().value();
			return true;
		}

	}
	return false;
}


bool hashfile_extrainfo(const char *hash_path, const game_driver &driver, const util::hash_collection &hashes, std::string &result)
{
	/* now read the hash file */
	int drv = driver_list::find(driver);
	int compat, open = drv;
	bool hashfound;
	do
	{
		hashfound = read_hash_config(hash_path, hashes, driver_list::driver(open).name, result);
		// first check if there are compatible systems
		compat = driver_list::compatible_with(open);
		// if so, try to open its hashfile
		if (compat != -1)
			open = compat;
		// otherwise, try with the parent
		else
		{
			drv = driver_list::clone(drv);
			open = drv;
		}
	}
	// if no extrainfo has been found but we can try a compatible or a parent set, go back
	while (!hashfound && open != -1);
	return hashfound;
}



bool hashfile_extrainfo(device_image_interface &image, std::string &result)
{
	return hashfile_extrainfo(
		image.device().mconfig().options().hash_path(),
		image.device().mconfig().gamedrv(),
		image.hash(),
		result);
}


