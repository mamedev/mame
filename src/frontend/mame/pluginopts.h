// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/

#ifndef MAME_FRONTEND_PLUGINOPTS_H
#define MAME_FRONTEND_PLUGINOPTS_H

#pragma once

#include <list>
#include <string>


// ======================> plugin

struct plugin
{
	std::string m_name;
	std::string m_description;
	std::string m_type;
	std::string m_directory;
	bool        m_start;
};


// ======================> plugin_options

class plugin_options
{
public:
	plugin_options();

	// accessors
	std::list<plugin> &plugins() { return m_plugins; }
	const std::list<plugin> &plugins() const { return m_plugins; }

	// methods
	void scan_directory(const std::string &path, bool recursive);
	bool load_plugin(const std::string &path);
	plugin *find(const std::string &name);

	// INI functionality
	void parse_ini_file(util::core_file &inifile);
	std::string output_ini() const;

private:
	std::list<plugin> m_plugins;
};

#endif // MAME_FRONTEND_PLUGINOPTS_H
