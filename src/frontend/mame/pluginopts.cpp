// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/

#include "emu.h"
#include "pluginopts.h"
#include "options.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <fstream>


//**************************************************************************
//  PLUGIN OPTIONS
//**************************************************************************

//-------------------------------------------------
//  plugin_options - constructor
//-------------------------------------------------

plugin_options::plugin_options()
{
}


//-------------------------------------------------
//  scan_directory
//-------------------------------------------------

void plugin_options::scan_directory(const std::string &path, bool recursive)
{
	// first try to open as a directory
	osd::directory::ptr directory = osd::directory::open(path);
	if (directory)
	{
		// iterate over all files in the directory
		for (const osd::directory::entry *entry = directory->read(); entry != nullptr; entry = directory->read())
		{
			if (entry->type == osd::directory::entry::entry_type::FILE && !strcmp(entry->name, "plugin.json"))
			{
				std::string curfile = std::string(path).append(PATH_SEPARATOR).append(entry->name);
				load_plugin(curfile);
			}
			else if (entry->type == osd::directory::entry::entry_type::DIR)
			{
				if (recursive && strcmp(entry->name, ".") && strcmp(entry->name, ".."))
					scan_directory(path + PATH_SEPARATOR + entry->name, recursive);
			}
		}
	}
}


//-------------------------------------------------
//  load_plugin
//-------------------------------------------------

bool plugin_options::load_plugin(const std::string &path)
{
	std::ifstream ifs(path);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document document;
	document.ParseStream<0>(isw);

	if (document.HasParseError())
	{
		std::string error(GetParseError_En(document.GetParseError()));
		osd_printf_error("Unable to parse plugin definition file %s. Errors returned:\n", path.c_str());
		osd_printf_error("%s\n", error.c_str());
		return false;
	}

	if (!document["plugin"].IsObject())
	{
		osd_printf_error("Bad plugin definition file %s:\n", path.c_str());
		return false;
	}

	size_t last_path_sep = path.find_last_of(PATH_SEPARATOR[0]);
	std::string dir = last_path_sep != std::string::npos
		? path.substr(0, last_path_sep)
		: ".";

	plugin p;
	p.m_name        = document["plugin"]["name"].GetString();
	p.m_description = document["plugin"]["description"].GetString();
	p.m_type        = document["plugin"]["type"].GetString();
	p.m_directory   = std::move(dir);
	p.m_start       = false;
	if (document["plugin"].HasMember("start") && (std::string(document["plugin"]["start"].GetString()) == "true"))
		p.m_start = true;

	m_plugins.push_back(std::move(p));
	return true;
}


//-------------------------------------------------
//  find
//-------------------------------------------------

plugin *plugin_options::find(const std::string &name)
{
	auto iter = std::find_if(
		m_plugins.begin(),
		m_plugins.end(),
		[&name](const plugin &p) { return name == p.m_name; });

	return iter != m_plugins.end()
		? &*iter
		: nullptr;
}


//-------------------------------------------------
//  create_core_options
//-------------------------------------------------

static void create_core_options(core_options &opts, const plugin_options &plugin_opts)
{
	// we're sort of abusing core_options to just get INI file parsing, so we'll build a
	// core_options structure for the sole purpose of parsing an INI file, and then reflect
	// the data back
	static const options_entry s_option_entries[] =
	{
		{ nullptr, nullptr, OPTION_HEADER, "PLUGINS OPTIONS" },
		{ nullptr }
	};
	opts.add_entries(s_option_entries);

	// create an entry for each option
	for (const plugin &p : plugin_opts.plugins())
	{
		opts.add_entry(
			{ p.m_name },
			nullptr,
			core_options::option_type::BOOLEAN,
			p.m_start ? "1" : "0");
	}
}


//-------------------------------------------------
//  parse_ini_file
//-------------------------------------------------

void plugin_options::parse_ini_file(util::core_file &inifile)
{
	core_options opts;
	create_core_options(opts, *this);

	// parse the INI file
	opts.parse_ini_file(inifile, OPTION_PRIORITY_NORMAL, true, true);

	// and reflect these options back
	for (plugin &p : m_plugins)
		p.m_start = opts.bool_value(p.m_name.c_str());
}


//-------------------------------------------------
//  output_ini
//-------------------------------------------------

std::string plugin_options::output_ini() const
{
	core_options opts;
	create_core_options(opts, *this);
	return opts.output_ini();
}
