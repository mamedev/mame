// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    softlist.h

    Software list file format.

*********************************************************************/

#ifndef MAME_EMU_SOFTLIST_H
#define MAME_EMU_SOFTLIST_H

#pragma once


#include "emucore.h"
#include "romentry.h"
#include "corefile.h"

#include <list>


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

namespace detail { class softlist_parser; }


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum class software_support
{
	SUPPORTED,
	PARTIALLY_SUPPORTED,
	UNSUPPORTED
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// an item in a list of name/value pairs
class feature_list_item
{
public:
	// construction/destruction
	feature_list_item(std::string &&name, std::string &&value);
	feature_list_item(const std::string &name, const std::string &value);
	feature_list_item(feature_list_item const &) = delete;
	feature_list_item(feature_list_item &&) = delete;
	feature_list_item& operator=(feature_list_item const &) = delete;
	feature_list_item& operator=(feature_list_item &&) = delete;

	// getters
	const std::string &name() const noexcept { return m_name; }
	const std::string &value() const noexcept { return m_value; }

private:
	// internal state
	std::string         m_name;
	std::string         m_value;
};


// a single part of a software item
class software_part
{
	friend class detail::softlist_parser;

public:
	// construction/destruction
	software_part(software_info &info, std::string &&name, std::string &&interface);
	software_part(software_part const &) = delete;
	software_part(software_part &&) = delete;
	software_part& operator=(software_part const &) = delete;
	software_part& operator=(software_part &&) = delete;

	// getters
	software_info &info() const noexcept { return m_info; }
	const std::string &name() const noexcept { return m_name; }
	const std::string &interface() const noexcept { return m_interface; }
	const std::list<feature_list_item> &featurelist() const noexcept { return m_featurelist; }
	const std::vector<rom_entry> &romdata() const noexcept { return m_romdata; }

	// helpers
	bool matches_interface(const char *interface_list) const noexcept;
	const char *feature(std::string_view feature_name) const noexcept;

private:
	// internal state
	software_info &                 m_info;
	std::string                     m_name;
	std::string                     m_interface;
	std::list<feature_list_item>    m_featurelist;
	std::vector<rom_entry>          m_romdata;
};


// a single software item
class software_info
{
	friend class detail::softlist_parser;

public:
	// construction/destruction
	software_info(std::string &&name, std::string &&parent, std::string_view supported);
	software_info(software_info const &) = delete;
	software_info(software_info &&) = delete;
	software_info& operator=(software_info const &) = delete;
	software_info& operator=(software_info &&) = delete;

	// getters
	const std::string &shortname() const { return m_shortname; }
	const std::string &longname() const { return m_longname; }
	const std::string &parentname() const { return m_parentname; }
	const std::string &year() const { return m_year; }
	const std::string &publisher() const { return m_publisher; }
	const std::list<feature_list_item> &other_info() const { return m_other_info; }
	const std::list<feature_list_item> &shared_info() const { return m_shared_info; }
	software_support supported() const { return m_supported; }
	const std::list<software_part> &parts() const { return m_partdata; }

	// additional operations
	const software_part *find_part(std::string_view part_name, const char *interface = nullptr) const;
	bool has_multiple_parts(const char *interface) const;

private:
	// internal state
	software_support        m_supported;
	std::string             m_shortname;
	std::string             m_longname;
	std::string             m_parentname;
	std::string             m_year;           // Copyright year on title screen, actual release dates can be tracked in external resources
	std::string             m_publisher;
	std::list<feature_list_item> m_other_info;   // Here we store info like developer, serial #, etc. which belong to the software entry as a whole
	std::list<feature_list_item> m_shared_info;  // Here we store info like TV standard compatibility, or add-on requirements, etc. which get inherited
												// by each part of this software entry (after loading these are stored in partdata->featurelist)
	std::list<software_part> m_partdata;
};


// ----- Helpers -----

// parses a software list
void parse_software_list(
		util::core_file &file,
		std::string_view filename,
		std::string &listname,
		std::string &description,
		std::list<software_info> &infolist,
		std::ostream &errors);

// parses a software identifier (e.g. - 'apple2e:agentusa:flop1') into its constituent parts (returns false if cannot parse)
bool software_name_parse(std::string_view identifier, std::string *list_name = nullptr, std::string *software_name = nullptr, std::string *part_name = nullptr);

#endif // MAME_EMU_SOFTLIST_H
