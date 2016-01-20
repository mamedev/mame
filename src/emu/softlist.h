// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    softlist.h

    Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_H_
#define __SOFTLIST_H_

#include "cstrpool.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SOFTWARE_SUPPORTED_YES      0
#define SOFTWARE_SUPPORTED_PARTIAL  1
#define SOFTWARE_SUPPORTED_NO       2

enum softlist_type
{
	SOFTWARE_LIST_ORIGINAL_SYSTEM,
	SOFTWARE_LIST_COMPATIBLE_SYSTEM
};



//**************************************************************************
//  MACROS
//**************************************************************************

#define MCFG_SOFTWARE_LIST_CONFIG(_list,_list_type) \
	software_list_device::static_set_type(*device, _list, _list_type);

#define MCFG_SOFTWARE_LIST_ADD( _tag, _list ) \
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)

#define MCFG_SOFTWARE_LIST_COMPATIBLE_ADD( _tag, _list ) \
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)

#define MCFG_SOFTWARE_LIST_MODIFY( _tag, _list ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)

#define MCFG_SOFTWARE_LIST_COMPATIBLE_MODIFY( _tag, _list ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)

#define MCFG_SOFTWARE_LIST_FILTER( _tag, _filter ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	software_list_device::static_set_filter(*device, _filter);

#define MCFG_SOFTWARE_LIST_REMOVE( _tag ) \
	MCFG_DEVICE_REMOVE( _tag )



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> feature_list_item

// an item in a list of name/value pairs
class feature_list_item
{
	friend class simple_list<feature_list_item>;

public:
	// construction/destruction
	feature_list_item(const char *name = nullptr, const char *value = nullptr)
		: m_next(nullptr),
			m_name(name),
			m_value(value) { }

	// getters
	feature_list_item *next() const { return m_next; }
	const char *name() const { return m_name; }
	const char *value() const { return m_value; }

private:
	// internal state
	feature_list_item * m_next;
	const char *        m_name;
	const char *        m_value;
};


// ======================> software_part

// a single part of a software item
class software_part
{
	friend class softlist_parser;
	friend class simple_list<software_part>;

public:
	// construction/destruction
	software_part(software_info &info, const char *name = nullptr, const char *interface = nullptr);

	// getters
	software_part *next() const { return m_next; }
	software_info &info() const { return m_info; }
	const char *name() const { return m_name; }
	const char *interface() const { return m_interface; }
	feature_list_item *featurelist() const { return m_featurelist.first(); }
	rom_entry *romdata(unsigned int index = 0) { return (index < m_romdata.size()) ? &m_romdata[index] : nullptr; }

	// helpers
	bool is_compatible(const software_list_device &swlist) const;
	bool matches_interface(const char *interface) const;
	const char *feature(const char *feature_name) const;

private:
	// internal state
	software_part *     m_next;
	software_info &     m_info;
	const char *        m_name;
	const char *        m_interface;
	simple_list<feature_list_item> m_featurelist;
	std::vector<rom_entry>   m_romdata;
};


// ======================> software_info

// a single software item
class software_info
{
	friend class softlist_parser;
	friend class simple_list<software_info>;

public:
	// construction/destruction
	software_info(software_list_device &list, const char *name, const char *parent, const char *supported);

	// getters
	software_info *next() const { return m_next; }
	software_list_device &list() const { return m_list; }
	const char *shortname() const { return m_shortname; }
	const char *longname() const { return m_longname; }
	const char *parentname() const { return m_parentname; }
	const char *year() const { return m_year; }
	const char *publisher() const { return m_publisher; }
	feature_list_item *other_info() const { return m_other_info.first(); }
	feature_list_item *shared_info() const { return m_shared_info.first(); }
	UINT32 supported() const { return m_supported; }
	int num_parts() const { return m_partdata.count(); }
	software_part *first_part() const { return m_partdata.first(); }
	software_part *last_part() const { return m_partdata.last(); }

	// additional operations
	software_part *find_part(const char *partname, const char *interface = nullptr);
	bool has_multiple_parts(const char *interface) const;

private:
	// internal state
	software_info *         m_next;
	software_list_device &  m_list;
	UINT32                  m_supported;
	const char *            m_shortname;
	const char *            m_longname;
	const char *            m_parentname;
	const char *            m_year;           // Copyright year on title screen, actual release dates can be tracked in external resources
	const char *            m_publisher;
	simple_list<feature_list_item> m_other_info;   // Here we store info like developer, serial #, etc. which belong to the software entry as a whole
	simple_list<feature_list_item> m_shared_info;  // Here we store info like TV standard compatibility, or add-on requirements, etc. which get inherited
												// by each part of this software entry (after loading these are stored in partdata->featurelist)
	simple_list<software_part> m_partdata;
};


// ======================> software_list_device

// device representing a software list
class software_list_device : public device_t
{
	friend class softlist_parser;

public:
	// construction/destruction
	software_list_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, const char *list, softlist_type list_type);
	static void static_set_filter(device_t &device, const char *filter);

	// getters
	const char *list_name() const { return m_list_name.c_str(); }
	softlist_type list_type() const { return m_list_type; }
	const char *filter() const { return m_filter; }
	const char *filename() { return m_file.filename(); }

	// getters that may trigger a parse
	const char *description() { if (!m_parsed) parse(); return m_description; }
	bool valid() { if (!m_parsed) parse(); return m_infolist.count() > 0; }
	const char *errors_string() { if (!m_parsed) parse(); return m_errors.c_str(); }

	// operations
	software_info *find(const char *look_for, software_info *prev = nullptr);
	software_info *first_software_info() { if (!m_parsed) parse(); return m_infolist.first(); }
	void find_approx_matches(const char *name, int matches, software_info **list, const char *interface);
	void release();

	// string pool helpers
	const char *add_string(const char *string) { return m_stringpool.add(string); }
	bool string_pool_contains(const char *string) { return m_stringpool.contains(string); }

	// static helpers
	static software_list_device *find_by_name(const machine_config &mconfig, const char *name);
	static void display_matches(const machine_config &config, const char *interface, const char *name);

protected:
	// internal helpers
	void parse();
	void internal_validity_check(validity_checker &valid) ATTR_COLD;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;

	// configuration state
	std::string                 m_list_name;
	softlist_type               m_list_type;
	const char *                m_filter;

	// internal state
	bool                        m_parsed;
	emu_file                    m_file;
	const char *                m_description;
	std::string                 m_errors;
	simple_list<software_info>  m_infolist;
	const_string_pool           m_stringpool;
};


// device type definition
extern const device_type SOFTWARE_LIST;

// device type iterator
typedef device_type_iterator<&device_creator<software_list_device>, software_list_device> software_list_device_iterator;


#endif
