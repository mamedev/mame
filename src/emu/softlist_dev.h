// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

	softlist_dev.h

	Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_DEV_H_
#define __SOFTLIST_DEV_H_

#include "softlist.h"


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

enum software_compatibility
{
	SOFTWARE_IS_COMPATIBLE,
	SOFTWARE_IS_INCOMPATIBLE,
	SOFTWARE_NOT_COMPATIBLE
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

class device_image_interface;
class software_list_device;


// ======================> software_list_loader

class software_list_loader
{
public:
	virtual bool load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const = 0;
};


// ======================> false_software_list_loader

class false_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
	static const software_list_loader &instance() { return s_instance; }

private:
	static false_software_list_loader s_instance;
};


// ======================> rom_software_list_loader

class rom_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
	static const software_list_loader &instance() { return s_instance; }

private:
	static rom_software_list_loader s_instance;
};


// ======================> image_software_list_loader

class image_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
	static const software_list_loader &instance() { return s_instance; }

private:
	static image_software_list_loader s_instance;
};


// ======================> software_list_device

// device representing a software list
class software_list_device : public device_t
{
	friend class softlist_parser;

public:
	// construction/destruction
	software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, const char *list, softlist_type list_type);
	static void static_set_filter(device_t &device, const char *filter);

	// getters
	const std::string &list_name() const { return m_list_name; }
	softlist_type list_type() const { return m_list_type; }
	const char *filter() const { return m_filter; }
	const char *filename() { return m_file.filename(); }

	// getters that may trigger a parse
	const char *description() { if (!m_parsed) parse(); return m_description.c_str(); }
	bool valid() { if (!m_parsed) parse(); return !m_infolist.empty(); }
	const char *errors_string() { if (!m_parsed) parse(); return m_errors.c_str(); }
	const std::list<software_info> &get_info() { if (!m_parsed) parse(); return m_infolist; }

	// operations
	const software_info *find(const char *look_for);
	void find_approx_matches(const std::string &name, int matches, const software_info **list, const char *interface);
	void release();
	software_compatibility is_compatible(const software_part &part) const;

	// static helpers
	static software_list_device *find_by_name(const machine_config &mconfig, const std::string &name);
	static void display_matches(const machine_config &config, const char *interface, const std::string &name);
	static device_image_interface *find_mountable_image(const machine_config &mconfig, const software_part &part);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;

private:
	// internal helpers
	void parse();
	void internal_validity_check(validity_checker &valid) ATTR_COLD;

	// configuration state
	std::string                 m_list_name;
	softlist_type               m_list_type;
	const char *                m_filter;

	// internal state
	bool                        m_parsed;
	emu_file                    m_file;
	std::string                 m_description;
	std::string                 m_errors;
	std::list<software_info>    m_infolist;
};


// device type definition
extern const device_type SOFTWARE_LIST;

// device type iterator
typedef device_type_iterator<&device_creator<software_list_device>, software_list_device> software_list_device_iterator;


#endif // __SOFTLIST_DEV_H_
