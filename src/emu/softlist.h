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
//  CONSTANTS
//**************************************************************************

#define SOFTWARE_SUPPORTED_YES      0
#define SOFTWARE_SUPPORTED_PARTIAL  1
#define SOFTWARE_SUPPORTED_NO       2


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> feature_list_item

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
	feature_list_item *next() const noexcept { return m_next; }
	const std::string &name() const noexcept { return m_name; }
	const std::string &value() const noexcept { return m_value; }

private:
	// internal state
	feature_list_item * m_next;
	std::string         m_name;
	std::string         m_value;
};


// ======================> software_part

// a single part of a software item
class software_part
{
	friend class softlist_parser;

public:
	// construction/destruction
	software_part(software_info &info, std::string &&name, std::string &&interface);
	software_part(software_part const &) = delete;
	software_part(software_part &&) = delete;
	software_part& operator=(software_part const &) = delete;
	software_part& operator=(software_part &&) = delete;

	// getters
	software_part *next() const noexcept { return m_next; }
	software_info &info() const noexcept { return m_info; }
	const std::string &name() const noexcept { return m_name; }
	const std::string &interface() const noexcept { return m_interface; }
	const std::list<feature_list_item> &featurelist() const noexcept { return m_featurelist; }
	const std::vector<rom_entry> &romdata() const noexcept { return m_romdata; }

	// helpers
	bool matches_interface(const char *interface_list) const noexcept;
	const char *feature(const std::string &feature_name) const noexcept;

private:
	// internal state
	software_part *                 m_next;
	software_info &                 m_info;
	std::string                     m_name;
	std::string                     m_interface;
	std::list<feature_list_item>    m_featurelist;
	std::vector<rom_entry>          m_romdata;
};


// ======================> software_info

// a single software item
class software_info
{
	friend class softlist_parser;

public:
	// construction/destruction
	software_info(std::string &&name, std::string &&parent, const std::string &supported);
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
	u32 supported() const { return m_supported; }
	const std::list<software_part> &parts() const { return m_partdata; }

	// additional operations
	const software_part *find_part(const std::string &part_name, const char *interface = nullptr) const;
	bool has_multiple_parts(const char *interface) const;

private:
	// internal state
	u32                     m_supported;
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


// ======================> softlist_parser

class softlist_parser
{
public:
	// construction (== execution)
	softlist_parser(util::core_file &file, const std::string &filename, std::string &description, std::list<software_info> &infolist, std::ostringstream &errors);

private:
	enum parse_position
	{
		POS_ROOT,
		POS_MAIN,
		POS_SOFT,
		POS_PART,
		POS_DATA
	};

	// internal parsing helpers
	const char *infoname() const { return (m_current_info != nullptr) ? m_current_info->shortname().c_str() : "???"; }
	int line() const;
	int column() const;
	const char *parser_error() const;

	// internal error helpers
	template <typename Format, typename... Params> void parse_error(Format &&fmt, Params &&... args);
	void unknown_tag(const char *tagname) { parse_error("Unknown tag: %s", tagname); }
	void unknown_attribute(const char *attrname) { parse_error("Unknown attribute: %s", attrname); }

	// internal helpers
	template <typename T> std::vector<std::string> parse_attributes(const char **attributes, const T &attrlist);
	bool parse_name_and_value(const char **attributes, std::string &name, std::string &value);
	void add_rom_entry(std::string &&name, std::string &&hashdata, u32 offset, u32 length, u32 flags);

	// expat callbacks
	static void start_handler(void *data, const char *tagname, const char **attributes);
	static void data_handler(void *data, const char *s, int len);
	static void end_handler(void *data, const char *name);

	// internal parsing
	void parse_root_start(const char *tagname, const char **attributes);
	void parse_main_start(const char *tagname, const char **attributes);
	void parse_soft_start(const char *tagname, const char **attributes);
	void parse_part_start(const char *tagname, const char **attributes);
	void parse_data_start(const char *tagname, const char **attributes);
	void parse_soft_end(const char *name);

	// internal parsing state
	util::core_file &                   m_file;
	std::string                         m_filename;
	std::list<software_info> &  m_infolist;
	std::ostringstream &        m_errors;
	struct XML_ParserStruct *   m_parser;
	bool                        m_done;
	std::string &               m_description;
	bool                    m_data_accum_expected;
	std::string             m_data_accum;
	software_info *         m_current_info;
	software_part *         m_current_part;
	parse_position          m_pos;
};


// ----- Helpers -----

// parses a software identifier (e.g. - 'apple2e:agentusa:flop1') into its constituent parts (returns false if cannot parse)
bool software_name_parse(const std::string &identifier, std::string *list_name = nullptr, std::string *software_name = nullptr, std::string *part_name = nullptr);

#endif // MAME_EMU_SOFTLIST_H
