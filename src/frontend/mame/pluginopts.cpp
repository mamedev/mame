// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/
#include <fstream>
#include "emu.h"
#include "pluginopts.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

//**************************************************************************
//  PLUGIN OPTIONS
//**************************************************************************

const options_entry plugin_options::s_option_entries[] =
{
	{ nullptr,  nullptr,  OPTION_HEADER,  "PLUGINS OPTIONS" },
	{ nullptr }
};

//-------------------------------------------------
//  plugin_options - constructor
//-------------------------------------------------

plugin_options::plugin_options()
: core_options()
{
	add_entries(plugin_options::s_option_entries);
}


void plugin_options::parse_json(std::string path)
{
	// first try to open as a directory
	osd_directory *directory = osd_opendir(path.c_str());
	if (directory != nullptr)
	{
		// iterate over all files in the directory
		for (const osd_directory_entry *entry = osd_readdir(directory); entry != nullptr; entry = osd_readdir(directory))
		{
			if (entry->type == ENTTYPE_FILE)
			{
				std::string name = entry->name;
				if (name == "plugin.json")
				{
					std::string curfile = std::string(path).append(PATH_SEPARATOR).append(entry->name);
					std::ifstream ifs(curfile);
					rapidjson::IStreamWrapper isw(ifs);
					rapidjson::Document document;
					document.ParseStream<0>(isw);

					if (document.HasParseError()) {
						std::string error(GetParseError_En(document.GetParseError()));
						osd_printf_error("Unable to parse plugin definition file %s. Errors returned:\n", curfile.c_str());
						osd_printf_error("%s\n", error.c_str());
						return;
					}

					if (document["plugin"].IsObject())
					{
						std::string name = document["plugin"]["name"].GetString();
						std::string description = document["plugin"]["description"].GetString();
						std::string type = document["plugin"]["type"].GetString();
						bool start = false;
						if (document["plugin"].HasMember("start") && (std::string(document["plugin"]["start"].GetString()) == "true"))
							start = true;

						if (type=="plugin")
						{
							add_entry(core_strdup(name.c_str()),core_strdup(description.c_str()), OPTION_BOOLEAN, start ? "1" : "0");
						}
					}

				}
			}
			else if (entry->type == ENTTYPE_DIR)
			{
				std::string name = entry->name;
				if (!(name == "." || name == ".."))
				{
					parse_json(path + PATH_SEPARATOR + name);
				}
			}
		}

		// close the directory and be done
		osd_closedir(directory);
	}
}
