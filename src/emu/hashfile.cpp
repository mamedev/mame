// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    hashfile.c

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#include "hashfile.h"
#include "pool.h"
#include "emuopts.h"
#include "hash.h"
#include "drivenum.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.hpp"

/*-------------------------------------------------
    hashfile_lookup
-------------------------------------------------*/

bool read_hash_config(device_image_interface &image, const char *sysname, std::string &result)
{
	/* open a file */
	emu_file file(image.device().mconfig().options().hash_path(), OPEN_FLAG_READ);
	if (file.open(sysname, ".hsi") != osd_file::error::NONE)
	{
		return false;
	}

	pugi::xml_document doc;

	pugi::xml_parse_result res = doc.load_file(file.fullpath());
	if (res) 
	{
		// Do search by CRC32 and SHA1
		std::string query = "/hashfile/hash[";
		auto crc = image.hash().internal_string().substr(1,8);
		auto sha1 = image.hash().internal_string().substr(10, 40);
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

bool hashfile_extrainfo(device_image_interface &image, std::string &result)
{
	/* now read the hash file */
	image.crc();
	int drv = driver_list::find(image.device().mconfig().gamedrv());
	int compat, open = drv;
	bool hashfound;
	do
	{
		hashfound = read_hash_config(image, driver_list::driver(open).name, result);
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
