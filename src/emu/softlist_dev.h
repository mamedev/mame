// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    softlist_dev.h

    Software and software list information.

*********************************************************************/

#ifndef MAME_EMU_SOFTLIST_DEV_H
#define MAME_EMU_SOFTLIST_DEV_H

#pragma once


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
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> software_list_loader

class software_list_loader
{
public:
	virtual bool load_software(device_image_interface &image, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const = 0;
};


// ======================> false_software_list_loader

class false_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &image, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
	static const software_list_loader &instance() { return s_instance; }

private:
	static false_software_list_loader s_instance;
};


// ======================> rom_software_list_loader

class rom_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &image, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
	static const software_list_loader &instance() { return s_instance; }

private:
	static rom_software_list_loader s_instance;
};


// ======================> image_software_list_loader

class image_software_list_loader : public software_list_loader
{
public:
	virtual bool load_software(device_image_interface &image, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const override;
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
	software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// inline configuration helpers
	software_list_device &set_type(const char *list, softlist_type list_type) { m_list_name.assign(list); m_list_type = list_type; return *this; }
	software_list_device &set_original(const char *list) { return set_type(list, SOFTWARE_LIST_ORIGINAL_SYSTEM); }
	software_list_device &set_compatible(const char *list) { return set_type(list, SOFTWARE_LIST_COMPATIBLE_SYSTEM); }
	software_list_device &set_filter(const char *filter) { m_filter = filter; return *this; }

	// getters
	const std::string &list_name() const { return m_list_name; }
	softlist_type list_type() const { return m_list_type; }
	const char *filter() const { return m_filter; }
	const char *filename() { return m_file.filename(); }

	// getters that may trigger a parse
	const std::string &description() { if (!m_parsed) parse(); return m_description; }
	bool valid() { if (!m_parsed) parse(); return !m_infolist.empty(); }
	const char *errors_string() { if (!m_parsed) parse(); return m_errors.c_str(); }
	const std::list<software_info> &get_info() { if (!m_parsed) parse(); return m_infolist; }

	// operations
	const software_info *find(const std::string &look_for);
	void find_approx_matches(const std::string &name, int matches, const software_info **list, const char *interface);
	void release();
	software_compatibility is_compatible(const software_part &part) const;

	// static helpers
	static software_list_device *find_by_name(const machine_config &mconfig, const std::string &name);
	static void display_matches(const machine_config &config, const char *interface, const std::string &name);
	static device_image_interface *find_mountable_image(const machine_config &mconfig, const software_part &part, std::function<bool (const device_image_interface &)> filter);
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
DECLARE_DEVICE_TYPE(SOFTWARE_LIST, software_list_device)

// device type iterator
typedef device_type_iterator<software_list_device> software_list_device_iterator;


#endif // MAME_EMU_SOFTLIST_DEV_H
