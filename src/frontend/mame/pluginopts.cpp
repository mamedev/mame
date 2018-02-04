// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/
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
	osd_subst_env(path, path);
	osd::directory::ptr directory = osd::directory::open(path);
	if (directory)
	{
		// iterate over all files in the directory
		for (const osd::directory::entry *entry = directory->read(); entry != nullptr; entry = directory->read())
		{
			if (entry->type == osd::directory::entry::entry_type::FILE)
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
						std::string plugin_name = document["plugin"]["name"].GetString();
						std::string description = document["plugin"]["description"].GetString();
						std::string type = document["plugin"]["type"].GetString();
						bool start = false;
						if (document["plugin"].HasMember("start") && (std::string(document["plugin"]["start"].GetString()) == "true"))
							start = true;

						if (type=="plugin")
						{
							auto const it = m_descriptions.emplace(m_descriptions.end(), std::move(description));
							add_entry({ std::move(plugin_name) }, it->c_str(), option_type::BOOLEAN, start ? "1" : "0");
						}
					}

				}
			}
			else if (entry->type == osd::directory::entry::entry_type::DIR)
			{
				std::string name = entry->name;
				if (!(name == "." || name == ".."))
				{
					parse_json(path + PATH_SEPARATOR + name);
				}
			}
		}
	}
}
