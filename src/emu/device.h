/***************************************************************************

    device.h

    Device interface functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVINTRF_H__
#define __DEVINTRF_H__



//**************************************************************************
//  MACROS
//**************************************************************************

// macro for specifying a clock derived from an owning device
#define DERIVED_CLOCK(num, den)		(0xff000000 | ((num) << 12) | ((den) << 0))

// shorthand for accessing devices by machine/type/tag
#define devtag_reset(mach,tag)								(mach).device(tag)->reset()



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices
#define MCFG_DEVICE_CONFIG(_config) \
	device_t::static_set_static_config(*device, &(_config)); \

#define MCFG_DEVICE_CONFIG_CLEAR() \
	device_t::static_set_static_config(*device, NULL); \

#define MCFG_DEVICE_CLOCK(_clock) \
	device_t::static_set_clock(*device, _clock); \

#define MCFG_DEVICE_INPUT_DEFAULTS(_config) \
	device_t::static_set_input_default(*device, DEVICE_INPUT_DEFAULTS_NAME(_config)); \


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
struct rom_entry;
class machine_config;
class emu_timer;
typedef struct _input_device_default input_device_default;


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


// read/write types for I/O lines (similar to read/write handlers but no offset)
typedef int (*read_line_device_func)(device_t *device);
typedef void (*write_line_device_func)(device_t *device, int state);



// ======================> tagged_device_list

// tagged_device_list is a tagged_list with additional searching based on type
class device_list : public tagged_list<device_t>
{
	typedef tagged_list<device_t> super;

public:
	// construction/destruction
	device_list(resource_pool &pool = global_resource_pool);

	// getters
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	// bulk operations
	void set_machine_all(running_machine &machine);
	void start_all();
	void start_new_devices();
	void reset_all();
	void stop_all();

	// pull the generic forms forward
	using super::first;
	using super::count;
	using super::indexof;
	using super::find;

	// provide type-specific overrides
	device_t *first(device_type type) const;
	int count(device_type type) const;
	int indexof(device_type type, device_t &object) const;
	int indexof(device_type type, const char *tag) const;
	device_t *find(device_type type, int index) const;

	// provide interface-specific overrides
	template<class _InterfaceClass>
	bool first(_InterfaceClass *&intf) const;

private:
	// internal helpers
	void exit();
	void presave_all();
	void postload_all();

	// internal state
	running_machine *m_machine;
};



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

protected:
	// construction/destruction
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock);
	virtual ~device_t();

public:
	// getters
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	device_type type() const { return m_type; }
	UINT32 configured_clock() const { return m_configured_clock; }
	const char *name() const { return m_name; }
	const char *shortname() const { return m_shortname; }
	const char *searchpath() const { return m_searchpath; }
	const char *tag() const { return m_tag; }
	const void *static_config() const { return m_static_config; }
	const machine_config &mconfig() const { return m_machine_config; }
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	const rom_entry *rom_region() const { return device_rom_region(); }
	machine_config_constructor machine_config_additions() const { return device_mconfig_additions(); }
	ioport_constructor input_ports() const { return device_input_ports(); }

	// iteration helpers
	device_t *next() const { return m_next; }
	device_t *typenext() const;
	device_t *owner() const { return m_owner; }

	// interface helpers
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) { intf = dynamic_cast<_DeviceClass *>(this); return (intf != NULL); }
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) const { intf = dynamic_cast<const _DeviceClass *>(this); return (intf != NULL); }
	template<class _DeviceClass> bool next(_DeviceClass *&intf) const
	{
		for (device_t *cur = m_next; cur != NULL; cur = cur->m_next)
			if (cur->interface(intf))
				return true;
		return false;
	}

	// specialized helpers for common core interfaces
	bool interface(device_execute_interface *&intf) { intf = m_execute; return (intf != NULL); }
	bool interface(device_execute_interface *&intf) const { intf = m_execute; return (intf != NULL); }
	bool interface(device_memory_interface *&intf) { intf = m_memory; return (intf != NULL); }
	bool interface(device_memory_interface *&intf) const { intf = m_memory; return (intf != NULL); }
	bool interface(device_state_interface *&intf) { intf = m_state; return (intf != NULL); }
	bool interface(device_state_interface *&intf) const { intf = m_state; return (intf != NULL); }
	device_execute_interface &execute() const { assert(m_execute != NULL); return *m_execute; }
	device_memory_interface &memory() const { assert(m_memory != NULL); return *m_memory; }

	// owned object helpers
	astring &subtag(astring &dest, const char *tag) const;
	astring &siblingtag(astring &dest, const char *tag) const;
	const memory_region *subregion(const char *tag) const;
	device_t *subdevice(const char *tag) const;
	device_t *siblingdevice(const char *tag) const;
	template<class _DeviceClass> inline _DeviceClass *subdevice(const char *tag) { return downcast<_DeviceClass *>(subdevice(tag)); }
	template<class _DeviceClass> inline _DeviceClass *siblingdevice(const char *tag) { return downcast<_DeviceClass *>(siblingdevice(tag)); }
	const memory_region *region() const { return m_region; }

	// configuration helpers
	static void static_set_clock(device_t &device, UINT32 clock);
	static void static_set_static_config(device_t &device, const void *config) { device.m_static_config = config; }
	static void static_set_input_default(device_t &device, const input_device_default *config) { device.m_input_defaults = config; }

	// state helpers
	void config_complete();
	bool configured() const { return m_config_complete; }
	bool validity_check(emu_options &options, const game_driver &driver) const;
	bool started() const { return m_started; }
	void reset();

	// clock/timing accessors
	UINT32 clock() const { return m_clock; }
	UINT32 unscaled_clock() const { return m_unscaled_clock; }
	void set_unscaled_clock(UINT32 clock);
	double clock_scale() const { return m_clock_scale; }
	void set_clock_scale(double clockscale);
	attotime clocks_to_attotime(UINT64 clocks) const;
	UINT64 attotime_to_clocks(attotime duration) const;

	// timer interfaces
	emu_timer *timer_alloc(device_timer_id id = 0, void *ptr = NULL);
	void timer_set(attotime duration, device_timer_id id = 0, int param = 0, void *ptr = NULL);
	void synchronize(device_timer_id id = 0, int param = 0, void *ptr = NULL) { timer_set(attotime::zero, id, param, ptr); }
	void timer_expired(emu_timer &timer, device_timer_id id, int param, void *ptr) { device_timer(timer, id, param, ptr); }

	// state saving interfaces
	template<typename _ItemType>
	void ATTR_COLD save_item(_ItemType &value, const char *valname, int index = 0) { assert(m_save != NULL); m_save->save_item(name(), tag(), index, value, valname); }
	template<typename _ItemType>
	void ATTR_COLD save_pointer(_ItemType *value, const char *valname, UINT32 count, int index = 0) { assert(m_save != NULL); m_save->save_pointer(name(), tag(), index, value, valname, count); }

	// debugging
	device_debug *debug() const { return m_debug; }

protected:
	// miscellaneous helpers
	void set_machine(running_machine &machine);
	void start();
	void stop();
	void debug_setup();
	void pre_save();
	void post_load();
	void notify_clock_changed();

	//------------------- begin derived class overrides

	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_config_complete();
	virtual bool device_validity_check(emu_options &options, const game_driver &driver) const ATTR_COLD;
	virtual void device_start() ATTR_COLD = 0;
	virtual void device_stop() ATTR_COLD;
	virtual void device_reset() ATTR_COLD;
	virtual void device_pre_save() ATTR_COLD;
	virtual void device_post_load() ATTR_COLD;
	virtual void device_clock_changed();
	virtual void device_debug_setup();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	//------------------- end derived class overrides

	device_debug *			m_debug;

	// core device interfaces for speed
	device_execute_interface *m_execute;
	device_memory_interface *m_memory;
	device_state_interface *m_state;

	// device relationships
	device_t *				m_next;					// next device (of any type/class)
	device_t *				m_owner;				// device that owns us, or NULL if nobody
	device_interface *		m_interface_list;		// head of interface list

	const device_type		m_type;					// device type
	UINT32					m_configured_clock;		// originally configured device clock

	const machine_config &	m_machine_config;		// reference to the machine's configuration
	const void *			m_static_config;		// static device configuration
	const input_device_default *m_input_defaults;   // devices input ports default overrides

	astring					m_name;					// name of the device
	astring					m_shortname;			// short name of the device
	astring					m_searchpath;			// search path, used for media loading

	bool					m_started;				// true if the start function has succeeded
	UINT32					m_clock;				// device clock
	const memory_region *	m_region;				// our device-local region

	UINT32					m_unscaled_clock;		// unscaled clock
	double					m_clock_scale;			// clock scale factor
	attoseconds_t			m_attoseconds_per_clock;// period in attoseconds

	// helper class to request auto-object discovery in the constructor of a derived class
	class auto_finder_base
	{
	public:
		// construction/destruction
		auto_finder_base(device_t &base, const char *tag);
		virtual ~auto_finder_base();

		// getters
		virtual void findit(device_t &base) = 0;

		// helpers
		device_t *find_device(device_t &device, const char *tag);
		void *find_shared_ptr(device_t &device, const char *tag);
		size_t find_shared_size(device_t &device, const char *tag);

		// internal state
		auto_finder_base *m_next;
		const char *m_tag;
	};

	// templated version bound to a specific type
	template<typename _TargetType, bool _Required>
	class auto_finder_type : public auto_finder_base
	{
	public:
		// construction/destruction
		auto_finder_type(device_t &base, const char *tag)
			: auto_finder_base(base, tag),
			  m_target(0) { }

		// operators to make use transparent
		operator _TargetType() { return m_target; }
		operator _TargetType() const { return m_target; }
		_TargetType operator->() { return m_target; }

		// getter for explicit fetching
		_TargetType target() const { return m_target; }

		// setter for setting the object
		void set_target(_TargetType target)
		{
			m_target = target;
			if (target == 0 && _Required)
				throw emu_fatalerror("Unable to find required object '%s'", this->m_tag);
		}

		// internal state
		_TargetType m_target;
	};

	// optional device finder
	template<class _DeviceClass>
	class optional_device : public auto_finder_type<_DeviceClass *, false>
	{
	public:
		optional_device(device_t &base, const char *tag) : auto_finder_type<_DeviceClass *, false>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(downcast<_DeviceClass *>(this->find_device(base, this->m_tag))); }
	};

	// required devices are similar but throw an error if they are not found
	template<class _DeviceClass>
	class required_device : public auto_finder_type<_DeviceClass *, true>
	{
	public:
		required_device(device_t &base, const char *tag) : auto_finder_type<_DeviceClass *, true>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(downcast<_DeviceClass *>(this->find_device(base, this->m_tag))); }
	};

	// optional shared pointer finder
	template<typename _PointerType>
	class optional_shared_ptr : public auto_finder_type<_PointerType *, false>
	{
	public:
		optional_shared_ptr(device_t &base, const char *tag) : auto_finder_type<_PointerType *, false>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(reinterpret_cast<_PointerType *>(this->find_shared_ptr(base, this->m_tag))); }
	};

	// required shared pointer finder
	template<typename _PointerType>
	class required_shared_ptr : public auto_finder_type<_PointerType *, true>
	{
	public:
		required_shared_ptr(device_t &base, const char *tag) : auto_finder_type<_PointerType *, true>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(reinterpret_cast<_PointerType *>(this->find_shared_ptr(base, this->m_tag))); }
	};

	// optional shared pointer size finder
	class optional_shared_size : public auto_finder_type<size_t, false>
	{
	public:
		optional_shared_size(device_t &base, const char *tag) : auto_finder_type<size_t, false>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(find_shared_size(base, this->m_tag)); }
	};

	// required shared pointer size finder
	class required_shared_size : public auto_finder_type<size_t, true>
	{
	public:
		required_shared_size(device_t &base, const char *tag) : auto_finder_type<size_t, true>(base, tag) { }
		virtual void findit(device_t &base) { this->set_target(find_shared_size(base, this->m_tag)); }
	};

	// internal helpers
	void register_auto_finder(auto_finder_base &autodev);

	auto_finder_base *		m_auto_finder_list;

private:
	// private state; accessor use required
	running_machine *		m_machine;
	save_manager *			m_save;
	astring 				m_tag;					// tag for this instance
	bool					m_config_complete;		// have we completed our configuration?
};



// ======================> device_interface

// device_interface represents runtime information for a particular device interface
class device_interface
{
	DISABLE_COPYING(device_interface);

protected:
	// construction/destruction
	device_interface(device_t &device);
	virtual ~device_interface();

public:
	// casting helpers
	device_t &device() { return m_device; }
	const device_t &device() const { return m_device; }
	operator device_t &() { return m_device; }
	operator device_t *() { return &m_device; }

	// iteration helpers
	device_interface *interface_next() const { return m_interface_next; }
	template<class _InterfaceClass> bool next(_InterfaceClass *&intf) const { return m_device.next(intf); }

	// optional operation overrides
	virtual void interface_config_complete();
	virtual bool interface_validity_check(emu_options &options, const game_driver &driver) const;
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
	device_interface *		m_interface_next;
	device_t &				m_device;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************


// ======================> device config helpers

// find the next device_t of the same type
inline device_t *device_t::typenext() const
{
	device_t *cur;
	for (cur = m_next; cur != NULL && cur->m_type != m_type; cur = cur->m_next) ;
	return cur;
}

// create a tag for an object that is owned by this device
inline astring &device_t::subtag(astring &dest, const char *_tag) const
{
	// temp. for now: don't include the root tag in the full tag name
	return (this != NULL && m_owner != NULL) ? dest.cpy(m_tag).cat(":").cat(_tag) : dest.cpy(_tag);
}

// create a tag for an object that a sibling to this device
inline astring &device_t::siblingtag(astring &dest, const char *_tag) const
{
	return (this != NULL && m_owner != NULL) ? m_owner->subtag(dest, _tag) : dest.cpy(_tag);
}


template<class _InterfaceClass>
bool device_list::first(_InterfaceClass *&intf) const
{
	for (device_t *cur = super::first(); cur != NULL; cur = cur->next())
		if (cur->interface(intf))
			return true;
	return false;
}

#endif	/* __DEVINTRF_H__ */
