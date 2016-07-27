// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    softlist.h

    Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_H_
#define __SOFTLIST_H_



//**************************************************************************
//  CONSTANTS
//**************************************************************************

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

class software_info;
class parsed_software_list;


// ======================> software_list_device

// device holding a software list
class software_list_device : public device_t
{
public:
	// construction/destruction
	software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, const char *list, softlist_type list_type);
	static void static_set_filter(device_t &device, const char *filter);

	// getters
	const char *list_name() const { return m_list_name.c_str(); }
	softlist_type list_type() const { return m_list_type; }
	const char *filter() const { return m_filter; }
	bool is_parsed() const { return m_parsed != nullptr; }
	const parsed_software_list &parsed() const { assert(m_parsed != nullptr); return *m_parsed; }

	// operations
	const software_info *find(const char *look_for);
	void find_approx_matches(const char *name, int matches, const software_info **list, const char *interface);
	void parse();
	void release();

	// static helpers
	static software_list_device *find_by_name(const machine_config &mconfig, const char *name);
	static void display_matches(const machine_config &config, const char *interface, const char *name);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;

	// configuration state
	std::string                 m_list_name;
	softlist_type               m_list_type;
	const char *                m_filter;

	// internal state
	std::unique_ptr<parsed_software_list> m_parsed;
};


// device type definition
extern const device_type SOFTWARE_LIST;

// device type iterator
typedef device_type_iterator<&device_creator<software_list_device>, software_list_device> software_list_device_iterator;


#endif
