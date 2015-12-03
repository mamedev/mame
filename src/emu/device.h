// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.h

    Device interface functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVICE_H__
#define __DEVICE_H__



//**************************************************************************
//  MACROS
//**************************************************************************

// macro for specifying a clock derived from an owning device
#define DERIVED_CLOCK(num, den)     (0xff000000 | ((num) << 12) | ((den) << 0))



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices
#define MCFG_DEVICE_CONFIG(_config) \
	device_t::static_set_static_config(*device, &(_config));
#define MCFG_DEVICE_CLOCK(_clock) \
	device_t::static_set_clock(*device, _clock);
#define MCFG_DEVICE_INPUT_DEFAULTS(_config) \
	device_t::static_set_input_default(*device, DEVICE_INPUT_DEFAULTS_NAME(_config));

#define DECLARE_READ_LINE_MEMBER(name)      int  name()
#define READ_LINE_MEMBER(name)              int  name()
#define DECLARE_WRITE_LINE_MEMBER(name)     void name(ATTR_UNUSED int state)
#define WRITE_LINE_MEMBER(name)             void name(ATTR_UNUSED int state)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class memory_region;
class device_debug;
class device_t;
class device_interface;
class device_execute_interface;
class device_memory_interface;
class device_state_interface;
class validity_checker;
struct rom_entry;
class machine_config;
class emu_timer;
struct input_device_default;
class finder_base;


// exception classes
class device_missing_dependencies : public emu_exception { };


// a device_type is simply a pointer to its alloc function
typedef device_t *(*device_type)(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


// this template function creates a stub which constructs a device
template<class _DeviceClass>
device_t *device_creator(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
{
	return global_alloc(_DeviceClass(mconfig, tag, owner, clock));
}


// timer IDs for devices
typedef UINT32 device_timer_id;

// ======================> device_t

// device_t represents a device
class device_t : public delegate_late_bind
{
	DISABLE_COPYING(device_t);

	friend class device_interface;
	friend class device_memory_interface;
	friend class device_state_interface;
	friend class device_execute_interface;
	friend class simple_list<device_t>;
	friend class device_list;
	friend class machine_config;
	friend class running_machine;
	friend class finder_base;

protected:
	// construction/destruction
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
public:
	virtual ~device_t();

	// getters
	running_machine &machine() const { /*assert(m_machine != NULL);*/ return *m_machine; }
	const char *tag() const { return m_tag.c_str(); }
	const char *basetag() const { return m_basetag.c_str(); }
	device_type type() const { return m_type; }
	const char *name() const { return m_name.c_str(); }
	const char *shortname() const { return m_shortname.c_str(); }
	const char *searchpath() const { return m_searchpath.c_str(); }
	const char *source() const { return m_source.c_str(); }
	device_t *owner() const { return m_owner; }
	device_t *next() const { return m_next; }
	UINT32 configured_clock() const { return m_configured_clock; }
	const void *static_config() const { return m_static_config; }
	const machine_config &mconfig() const { return m_machine_config; }
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	const rom_entry *rom_region() const { return device_rom_region(); }
	machine_config_constructor machine_config_additions() const { return device_mconfig_additions(); }
	ioport_constructor input_ports() const { return device_input_ports(); }
	UINT8 default_bios() const { return m_default_bios; }
	UINT8 system_bios() const { return m_system_bios; }
	std::string default_bios_tag() const { return m_default_bios_tag; }
	std::string parameter(const char *tag) const;

	// interface helpers
	device_interface *first_interface() const { return m_interface_list; }
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) { intf = dynamic_cast<_DeviceClass *>(this); return (intf != nullptr); }
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) const { intf = dynamic_cast<const _DeviceClass *>(this); return (intf != nullptr); }

	// specialized helpers for common core interfaces
	bool interface(device_execute_interface *&intf) { intf = m_execute; return (intf != nullptr); }
	bool interface(device_execute_interface *&intf) const { intf = m_execute; return (intf != nullptr); }
	bool interface(device_memory_interface *&intf) { intf = m_memory; return (intf != nullptr); }
	bool interface(device_memory_interface *&intf) const { intf = m_memory; return (intf != nullptr); }
	bool interface(device_state_interface *&intf) { intf = m_state; return (intf != nullptr); }
	bool interface(device_state_interface *&intf) const { intf = m_state; return (intf != nullptr); }
	device_execute_interface &execute() const { assert(m_execute != nullptr); return *m_execute; }
	device_memory_interface &memory() const { assert(m_memory != nullptr); return *m_memory; }
	device_state_interface &state() const { assert(m_state != nullptr); return *m_state; }

	// owned object helpers
	device_t *first_subdevice() const { return m_subdevice_list.first(); }
	std::string subtag(const char *tag) const;
	std::string siblingtag(const char *tag) const { return (this != nullptr && m_owner != nullptr) ? m_owner->subtag(tag) : std::string(tag); }
	memory_region *memregion(const char *tag) const;
	memory_share *memshare(const char *tag) const;
	memory_bank *membank(const char *tag) const;
	ioport_port *ioport(const char *tag) const;
	device_t *subdevice(const char *tag) const;
	device_t *siblingdevice(const char *tag) const;
	template<class _DeviceClass> inline _DeviceClass *subdevice(const char *tag) const { return downcast<_DeviceClass *>(subdevice(tag)); }
	template<class _DeviceClass> inline _DeviceClass *siblingdevice(const char *tag) const { return downcast<_DeviceClass *>(siblingdevice(tag)); }
	memory_region *region() const { return m_region; }

	// configuration helpers
	static void static_set_clock(device_t &device, UINT32 clock);
	static void static_set_static_config(device_t &device, const void *config) { device.m_static_config = config; }
	static void static_set_input_default(device_t &device, const input_device_default *config) { device.m_input_defaults = config; }
	static void static_set_default_bios_tag(device_t &device, const char *tag) { std::string default_bios_tag(tag); device.m_default_bios_tag = default_bios_tag; }

	// state helpers
	void config_complete();
	bool configured() const { return m_config_complete; }
	void validity_check(validity_checker &valid) const;
	bool started() const { return m_started; }
	void reset();

	// clock/timing accessors
	UINT32 clock() const { return m_clock; }
	UINT32 unscaled_clock() const { return m_unscaled_clock; }
	void set_unscaled_clock(UINT32 clock);
	double clock_scale() const { return m_clock_scale; }
	void set_clock_scale(double clockscale);
	attotime clocks_to_attotime(UINT64 clocks) const;
	UINT64 attotime_to_clocks(const attotime &duration) const;

	// timer interfaces
	emu_timer *timer_alloc(device_timer_id id = 0, void *ptr = nullptr);
	void timer_set(const attotime &duration, device_timer_id id = 0, int param = 0, void *ptr = nullptr);
	void synchronize(device_timer_id id = 0, int param = 0, void *ptr = nullptr) { timer_set(attotime::zero, id, param, ptr); }
	void timer_expired(emu_timer &timer, device_timer_id id, int param, void *ptr) { device_timer(timer, id, param, ptr); }

	// state saving interfaces
	template<typename _ItemType>
	void ATTR_COLD save_item(_ItemType &value, const char *valname, int index = 0) { assert(m_save != nullptr); m_save->save_item(this, name(), tag(), index, value, valname); }
	template<typename _ItemType>
	void ATTR_COLD save_pointer(_ItemType *value, const char *valname, UINT32 count, int index = 0) { assert(m_save != nullptr); m_save->save_pointer(this, name(), tag(), index, value, valname, count); }

	// debugging
	device_debug *debug() const { return m_debug.get(); }
	offs_t safe_pc() const;
	offs_t safe_pcbase() const;

	void set_default_bios(UINT8 bios) { m_default_bios = bios; }
	void set_system_bios(UINT8 bios) { m_system_bios = bios; }
	bool findit(bool isvalidation = false) const;

	// misc
	void popmessage(const char *format, ...) const;
	void logerror(const char *format, ...) const;
	void vlogerror(const char *format, va_list args) const;

protected:
	// miscellaneous helpers
	void set_machine(running_machine &machine);
	void start();
	void stop();
	void debug_setup();
	void pre_save();
	void post_load();
	void notify_clock_changed();
	finder_base *register_auto_finder(finder_base &autodev);

	//------------------- begin derived class overrides

	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const ATTR_COLD;
	virtual void device_start() ATTR_COLD = 0;
	virtual void device_stop() ATTR_COLD;
	virtual void device_reset() ATTR_COLD;
	virtual void device_reset_after_children() ATTR_COLD;
	virtual void device_pre_save() ATTR_COLD;
	virtual void device_post_load() ATTR_COLD;
	virtual void device_clock_changed();
	virtual void device_debug_setup();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	//------------------- end derived class overrides

	// core device properties
	const device_type       m_type;                 // device type
	std::string             m_name;                 // name of the device
	std::string             m_shortname;            // short name of the device
	std::string             m_searchpath;           // search path, used for media loading
	std::string             m_source;               // device source file name

	// device relationships
	device_t *              m_owner;                // device that owns us
	device_t *              m_next;                 // next device by the same owner (of any type/class)
	simple_list<device_t>   m_subdevice_list;       // list of sub-devices we own
	mutable tagmap_t<device_t *> m_device_map;      // map of device names looked up and found

	// device interfaces
	device_interface *      m_interface_list;       // head of interface list
	device_execute_interface *m_execute;            // pre-cached pointer to execute interface
	device_memory_interface *m_memory;              // pre-cached pointer to memory interface
	device_state_interface *m_state;                // pre-cached pointer to state interface

	// device clocks
	UINT32                  m_configured_clock;     // originally configured device clock
	UINT32                  m_unscaled_clock;       // current unscaled device clock
	UINT32                  m_clock;                // current device clock, after scaling
	double                  m_clock_scale;          // clock scale factor
	attoseconds_t           m_attoseconds_per_clock;// period in attoseconds

	std::unique_ptr<device_debug> m_debug;
	memory_region *         m_region;               // our device-local region
	const machine_config &  m_machine_config;       // reference to the machine's configuration
	const void *            m_static_config;        // static device configuration
	const input_device_default *m_input_defaults;   // devices input ports default overrides

	UINT8                   m_system_bios;          // the system BIOS we wish to load
	UINT8                   m_default_bios;         // the default system BIOS
	std::string             m_default_bios_tag;     // tag of the default system BIOS

private:
	// private helpers
	device_t *add_subdevice(device_type type, const char *tag, UINT32 clock);
	device_t *replace_subdevice(device_t &old, device_type type, const char *tag, UINT32 clock);
	void remove_subdevice(device_t &device);
	device_t *subdevice_slow(const char *tag) const;

	// private state; accessor use required
	running_machine *       m_machine;
	save_manager *          m_save;
	std::string             m_tag;                  // full tag for this instance
	std::string             m_basetag;              // base part of the tag
	bool                    m_config_complete;      // have we completed our configuration?
	bool                    m_started;              // true if the start function has succeeded
	finder_base *           m_auto_finder_list;     // list of objects to auto-find
};


// ======================> device_interface

// device_interface represents runtime information for a particular device interface
class device_interface
{
	DISABLE_COPYING(device_interface);

protected:
	// construction/destruction
	device_interface(device_t &device, const char *type);
	virtual ~device_interface();

public:
	const char *interface_type() const { return m_type; }

	// casting helpers
	device_t &device() { return m_device; }
	const device_t &device() const { return m_device; }
	operator device_t &() { return m_device; }
	operator device_t *() { return &m_device; }

	// iteration helpers
	device_interface *interface_next() const { return m_interface_next; }
	template<class _InterfaceClass> bool next(_InterfaceClass *&intf) const { return m_device.next(intf); }

	// optional operation overrides
	//
	// WARNING: interface_pre_start must be callable multiple times in
	// case another interface throws a missing dependency.  In
	// particular, state saving registrations should be done in post.
	virtual void interface_config_complete();
	virtual void interface_validity_check(validity_checker &valid) const;
	virtual void interface_pre_start();
	virtual void interface_post_start();
	virtual void interface_pre_reset();
	virtual void interface_post_reset();
	virtual void interface_pre_stop();
	virtual void interface_post_stop();
	virtual void interface_pre_save();
	virtual void interface_post_load();
	virtual void interface_clock_changed();
	virtual void interface_debug_setup();

protected:
	// internal state
	device_interface *      m_interface_next;
	device_t &              m_device;
	const char *            m_type;
};


// ======================> device_iterator

// helper class to iterate over the hierarchy of devices depth-first
class device_iterator
{
public:
	// construction
	device_iterator(device_t &root, int maxdepth = 255)
		: m_root(&root),
			m_current(nullptr),
			m_curdepth(0),
			m_maxdepth(maxdepth) { }

	// getters
	device_t *current() const { return m_current; }

	// reset and return first item
	device_t *first()
	{
		m_current = m_root;
		return m_current;
	}

	// advance depth-first
	device_t *next()
	{
		// remember our starting position, and end immediately if we're NULL
		device_t *start = m_current;
		if (start == nullptr)
			return nullptr;

		// search down first
		if (m_curdepth < m_maxdepth)
		{
			m_current = start->first_subdevice();
			if (m_current != nullptr)
			{
				m_curdepth++;
				return m_current;
			}
		}

		// search next for neighbors up the ownership chain
		while (m_curdepth > 0)
		{
			// found a neighbor? great!
			m_current = start->next();
			if (m_current != nullptr)
				return m_current;

			// no? try our parent
			start = start->owner();
			m_curdepth--;
		}

		// returned to the top; we're done
		return m_current = nullptr;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (device_t *item = first(); item != nullptr; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(device_t &device)
	{
		int index = 0;
		for (device_t *item = first(); item != nullptr; item = next(), index++)
			if (item == &device)
				return index;
		return -1;
	}

	// return the indexed item in the list
	device_t *byindex(int index)
	{
		for (device_t *item = first(); item != nullptr; item = next(), index--)
			if (index == 0)
				return item;
		return nullptr;
	}

private:
	// internal state
	device_t *      m_root;
	device_t *      m_current;
	int             m_curdepth;
	int             m_maxdepth;
};


// ======================> device_type_iterator

// helper class to find devices of a given type in the device hierarchy
template<device_type _DeviceType, class _DeviceClass = device_t>
class device_type_iterator
{
public:
	// construction
	device_type_iterator(device_t &root, int maxdepth = 255)
		: m_iterator(root, maxdepth) { }

	// getters
	_DeviceClass *current() const { return downcast<_DeviceClass *>(m_iterator.current()); }

	// reset and return first item
	_DeviceClass *first()
	{
		for (device_t *device = m_iterator.first(); device != nullptr; device = m_iterator.next())
			if (device->type() == _DeviceType)
				return downcast<_DeviceClass *>(device);
		return nullptr;
	}

	// advance depth-first
	_DeviceClass *next()
	{
		for (device_t *device = m_iterator.next(); device != nullptr; device = m_iterator.next())
			if (device->type() == _DeviceType)
				return downcast<_DeviceClass *>(device);
		return nullptr;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (_DeviceClass *item = first(); item != nullptr; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(_DeviceClass &device)
	{
		int index = 0;
		for (_DeviceClass *item = first(); item != nullptr; item = next(), index++)
			if (item == &device)
				return index;
		return -1;
	}

	// return the indexed item in the list
	_DeviceClass *byindex(int index)
	{
		for (_DeviceClass *item = first(); item != nullptr; item = next(), index--)
			if (index == 0)
				return item;
		return nullptr;
	}

private:
	// internal state
	device_iterator     m_iterator;
};


// ======================> device_interface_iterator

// helper class to find devices with a given interface in the device hierarchy
// also works for findnig devices derived from a given subclass
template<class _InterfaceClass>
class device_interface_iterator
{
public:
	// construction
	device_interface_iterator(device_t &root, int maxdepth = 255)
		: m_iterator(root, maxdepth),
			m_current(nullptr) { }

	// getters
	_InterfaceClass *current() const { return m_current; }

	// reset and return first item
	_InterfaceClass *first()
	{
		for (device_t *device = m_iterator.first(); device != nullptr; device = m_iterator.next())
			if (device->interface(m_current))
				return m_current;
		return nullptr;
	}

	// advance depth-first
	_InterfaceClass *next()
	{
		for (device_t *device = m_iterator.next(); device != nullptr; device = m_iterator.next())
			if (device->interface(m_current))
				return m_current;
		return nullptr;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (_InterfaceClass *item = first(); item != nullptr; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(_InterfaceClass &intrf)
	{
		int index = 0;
		for (_InterfaceClass *item = first(); item != nullptr; item = next(), index++)
			if (item == &intrf)
				return index;
		return -1;
	}

	// return the indexed item in the list
	_InterfaceClass *byindex(int index)
	{
		for (_InterfaceClass *item = first(); item != NULL; item = next(), index--)
			if (index == 0)
				return item;
		return NULL;
	}

private:
	// internal state
	device_iterator     m_iterator;
	_InterfaceClass *   m_current;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  subdevice - given a tag, find the device by
//  name relative to this device
//-------------------------------------------------

inline device_t *device_t::subdevice(const char *tag) const
{
	// safety first
	if (this == nullptr)
		return nullptr;

	// empty string or NULL means this device
	if (tag == nullptr || *tag == 0)
		return const_cast<device_t *>(this);

	// do a quick lookup and return that if possible
	device_t *quick = m_device_map.find(tag);
	return (quick != nullptr) ? quick : subdevice_slow(tag);
}


//-------------------------------------------------
//  siblingdevice - given a tag, find the device
//  by name relative to this device's parent
//-------------------------------------------------

inline device_t *device_t::siblingdevice(const char *tag) const
{
	// safety first
	if (this == nullptr)
		return nullptr;

	// empty string or NULL means this device
	if (tag == nullptr || *tag == 0)
		return const_cast<device_t *>(this);

	// leading caret implies the owner, just skip it
	if (tag[0] == '^') tag++;

	// query relative to the parent, if we have one
	if (m_owner != nullptr)
		return m_owner->subdevice(tag);

	// otherwise, it's NULL unless the tag is absolute
	return (tag[0] == ':') ? subdevice(tag) : nullptr;
}

#endif  /* __DEVICE_H__ */
