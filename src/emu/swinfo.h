// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    swinfo.h

    Software and software list information.

*********************************************************************/

#ifndef MAME_SWINFO_H
#define MAME_SWINFO_H

#include "cstrpool.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SOFTWARE_SUPPORTED_YES      0
#define SOFTWARE_SUPPORTED_PARTIAL  1
#define SOFTWARE_SUPPORTED_NO       2

enum software_compatibility
{
	SOFTWARE_IS_COMPATIBLE,
	SOFTWARE_IS_INCOMPATIBLE,
	SOFTWARE_NOT_COMPATIBLE
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct rom_entry;
class software_info;
class device_image_interface;
class parsed_software_list;

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
	feature_list_item *next() const { return m_next; }
	const std::string &name() const { return m_name; }
	const std::string &value() const { return m_value; }

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
	software_part *next() const { return m_next; }
	software_info &info() const { return m_info; }
	const std::string &name() const { return m_name; }
	const std::string &interface() const { return m_interface; }
	const std::list<feature_list_item> &featurelist() const { return m_featurelist; }
	const rom_entry *romdata(unsigned int index = 0) const { return (index < m_romdata.size()) ? &m_romdata[index] : nullptr; }

	// helpers
	software_compatibility is_compatible(const char *filter) const;
	bool matches_interface(const char *interface) const;
	const char *feature(const std::string &feature_name) const;
	device_image_interface *find_mountable_image(const machine_config &mconfig) const;

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
	software_info(parsed_software_list &list, std::string &&name, std::string &&parent, const char *supported);
	software_info(software_info const &) = delete;
	software_info(software_info &&) = delete;
	software_info& operator=(software_info const &) = delete;
	software_info& operator=(software_info &&) = delete;

	// getters
	software_info *next() const { return m_next; }
	parsed_software_list &list() const { return m_list; }
	const std::string &shortname() const { return m_shortname; }
	const std::string &longname() const { return m_longname; }
	const std::string &parentname() const { return m_parentname; }
	const std::string &year() const { return m_year; }
	const std::string &publisher() const { return m_publisher; }
	const std::list<feature_list_item> &other_info() const { return m_other_info; }
	const std::list<feature_list_item> &shared_info() const { return m_shared_info; }
	UINT32 supported() const { return m_supported; }
	const std::list<software_part> &parts() const { return m_partdata; }

	// additional operations
	const software_part *find_part(const char *partname, const char *interface = nullptr) const;
	bool has_multiple_parts(const char *interface) const;

private:
	// internal state
	software_info *         m_next;
	parsed_software_list &  m_list;
	UINT32                  m_supported;
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


// ======================> parsed_software_list

// a fully parsed software list (independent of device state)
class parsed_software_list
{
	friend class softlist_parser;

public:
	// construction/destruction
	parsed_software_list(const char *path, const char *list_name, bool report_not_found = true);
	~parsed_software_list();

	// getters
	const char *filename() const { return m_file.filename(); }
	const char *description() const { return m_description.c_str(); }
	bool valid() const { return !m_infolist.empty(); }
	const char *errors_string() const { return m_errors.c_str(); }
	const std::list<software_info> &get_info() const { return m_infolist; }

	// operations
	const software_info *find(const char *look_for) const;
	void internal_validity_check(validity_checker &valid) ATTR_COLD;

private:
	// string pool helpers
	const char *add_string(const char *string) { return m_stringpool.add(string); }

	// internal state
	emu_file                    m_file;
	std::string                 m_description;
	std::string                 m_errors;
	std::list<software_info>    m_infolist;
	const_string_pool           m_stringpool;
};



#endif
