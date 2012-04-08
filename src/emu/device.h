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
class validity_checker;
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

protected:
	// construction/destruction
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock);
	virtual ~device_t();

public:
	// getters
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	const char *tag() const { return m_tag; }
	const char *basetag() const { return m_basetag; }
	device_type type() const { return m_type; }
	const char *name() const { return m_name; }
	const char *shortname() const { return m_shortname; }
	const char *searchpath() const { return m_searchpath; }
	device_t *owner() const { return m_owner; }
	device_t *next() const { return m_next; }
	UINT32 configured_clock() const { return m_configured_clock; }
	const void *static_config() const { return m_static_config; }
	const machine_config &mconfig() const { return m_machine_config; }
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	const rom_entry *rom_region() const { return device_rom_region(); }
	machine_config_constructor machine_config_additions() const { return device_mconfig_additions(); }
	ioport_constructor input_ports() const { return device_input_ports(); }

	// interface helpers
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) { intf = dynamic_cast<_DeviceClass *>(this); return (intf != NULL); }
	template<class _DeviceClass> bool interface(_DeviceClass *&intf) const { intf = dynamic_cast<const _DeviceClass *>(this); return (intf != NULL); }

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
	device_t *first_subdevice() const { return m_subdevice_list.first(); }
	astring &subtag(astring &dest, const char *tag) const;
	astring &siblingtag(astring &dest, const char *tag) const { return (this != NULL && m_owner != NULL) ? m_owner->subtag(dest, tag) : dest.cpy(tag); }
	const memory_region *subregion(const char *tag) const;
	device_t *subdevice(const char *tag) const;
	device_t *siblingdevice(const char *tag) const;
	template<class _DeviceClass> inline _DeviceClass *subdevice(const char *tag) const { return downcast<_DeviceClass *>(subdevice(tag)); }
	template<class _DeviceClass> inline _DeviceClass *siblingdevice(const char *tag) const { return downcast<_DeviceClass *>(siblingdevice(tag)); }
	const memory_region *region() const { return m_region; }

	// configuration helpers
	static void static_set_clock(device_t &device, UINT32 clock);
	static void static_set_static_config(device_t &device, const void *config) { device.m_static_config = config; }
	static void static_set_input_default(device_t &device, const input_device_default *config) { device.m_input_defaults = config; }

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
	const device_type		m_type;					// device type
	astring					m_name;					// name of the device
	astring					m_shortname;			// short name of the device
	astring					m_searchpath;			// search path, used for media loading

	// device relationships
	device_t *				m_owner;				// device that owns us
	device_t *				m_next;					// next device by the same owner (of any type/class)
	simple_list<device_t>	m_subdevice_list;		// list of sub-devices we own
	mutable tagmap_t<device_t *> m_device_map;		// map of device names looked up and found

	// device interfaces
	device_interface *		m_interface_list;		// head of interface list
	device_execute_interface *m_execute;			// pre-cached pointer to execute interface
	device_memory_interface *m_memory;				// pre-cached pointer to memory interface
	device_state_interface *m_state;				// pre-cached pointer to state interface

	// device clocks
	UINT32					m_configured_clock;		// originally configured device clock
	UINT32					m_unscaled_clock;		// current unscaled device clock
	UINT32					m_clock;				// current device clock, after scaling
	double					m_clock_scale;			// clock scale factor
	attoseconds_t			m_attoseconds_per_clock;// period in attoseconds

	device_debug *			m_debug;
	const memory_region *	m_region;				// our device-local region
	const machine_config &	m_machine_config;		// reference to the machine's configuration
	const void *			m_static_config;		// static device configuration
	const input_device_default *m_input_defaults;   // devices input ports default overrides

	// helper class to request auto-object discovery in the constructor of a derived class
	class finder_base
	{
		friend class device_t;

	public:
		// construction/destruction
		finder_base(device_t &base, const char *tag);
		virtual ~finder_base();

		// getters
		virtual void findit() = 0;

	protected:
		// static helpers
		void *find_memory(UINT8 width, size_t &bytes, bool required);

		// internal state
		finder_base *m_next;
		device_t &m_base;
		const char *m_tag;
	};

	// device finder template
	template<typename _DeviceClass, bool _Required>
	class device_finder : public finder_base
	{
	public:
		// construction/destruction
		device_finder(device_t &base, const char *tag)
			: finder_base(base, tag),
			  m_target(0) { }

		// operators to make use transparent
		operator _DeviceClass *() { return m_target; }
		operator _DeviceClass *() const { return m_target; }
		_DeviceClass *operator->() { return m_target; }

		// getter for explicit fetching
		_DeviceClass *target() const { return m_target; }

		// setter for setting the object
		void set_target(_DeviceClass *target)
		{
			m_target = target;
			if (target == 0 && _Required)
				throw emu_fatalerror("Unable to find required device '%s'", this->m_tag);
		}

		// finder
		virtual void findit() { set_target(m_base.subdevice<_DeviceClass>(m_tag)); }

	protected:
		// internal state
		_DeviceClass *m_target;
	};

	// optional device finder
	template<class _DeviceClass>
	class optional_device : public device_finder<_DeviceClass, false>
	{
	public:
		optional_device(device_t &base, const char *tag) : device_finder<_DeviceClass, false>(base, tag) { }
	};

	// required devices are similar but throw an error if they are not found
	template<class _DeviceClass>
	class required_device : public device_finder<_DeviceClass, true>
	{
	public:
		required_device(device_t &base, const char *tag) : device_finder<_DeviceClass, true>(base, tag) { }
	};

	// shared pointer finder template
	template<typename _PointerType, bool _Required>
	class shared_ptr_finder : public finder_base
	{
	public:
		// construction/destruction
		shared_ptr_finder(device_t &base, const char *tag, UINT8 width = 0)
			: finder_base(base, tag),
			  m_target(0),
			  m_bytes(0),
			  m_allocated(false),
			  m_width((width != 0) ? width : sizeof(_PointerType) * 8) { }

		virtual ~shared_ptr_finder() { if (m_allocated) global_free(m_target); }

		// operators to make use transparent
		operator _PointerType *() { return m_target; }
		operator _PointerType *() const { return m_target; }
		_PointerType *operator->() { return m_target; }

		// getter for explicit fetching
		_PointerType *target() const { return m_target; }
		UINT32 bytes() const { return m_bytes; }

		// setter for setting the object
		void set_target(_PointerType *target, size_t bytes)
		{
			m_target = target;
			m_bytes = bytes;
			if (target == 0 && _Required)
				throw emu_fatalerror("Unable to find required shared pointer '%s'", this->m_tag);
		}

		// dynamic allocation of a shared pointer
		void allocate(UINT32 entries)
		{
			assert(!m_allocated);
			m_allocated = true;
			m_target = global_alloc_array_clear(_PointerType, entries);
			m_bytes = entries * sizeof(_PointerType);
			m_base.save_pointer(m_target, m_tag, entries);
		}

		// finder
		virtual void findit() { m_target = reinterpret_cast<_PointerType *>(find_memory(m_width, m_bytes, _Required)); }

	protected:
		// internal state
		_PointerType *m_target;
		size_t m_bytes;
		bool m_allocated;
		UINT8 m_width;
	};

	// optional device finder
	template<class _PointerType>
	class optional_shared_ptr : public shared_ptr_finder<_PointerType, false>
	{
	public:
		optional_shared_ptr(device_t &base, const char *tag, UINT8 width = 0) : shared_ptr_finder<_PointerType, false>(base, tag, width) { }
	};

	// required devices are similar but throw an error if they are not found
	template<class _PointerType>
	class required_shared_ptr : public shared_ptr_finder<_PointerType, true>
	{
	public:
		required_shared_ptr(device_t &base, const char *tag, UINT8 width = 0) : shared_ptr_finder<_PointerType, true>(base, tag, width) { }
	};

	// internal helpers
	finder_base *register_auto_finder(finder_base &autodev);

	finder_base *		m_auto_finder_list;

private:
	// private helpers
	device_t *add_subdevice(device_type type, const char *tag, UINT32 clock);
	device_t *replace_subdevice(device_t &old, device_type type, const char *tag, UINT32 clock);
	void remove_subdevice(device_t &device);
	device_t *subdevice_slow(const char *tag) const;

	// private state; accessor use required
	running_machine *		m_machine;
	save_manager *			m_save;
	astring 				m_tag;					// full tag for this instance
	astring 				m_basetag;				// base part of the tag
	bool					m_config_complete;		// have we completed our configuration?
	bool					m_started;				// true if the start function has succeeded
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
	device_interface *		m_interface_next;
	device_t &				m_device;
};


// ======================> device_iterator

// helper class to iterate over the hierarchy of devices depth-first
class device_iterator
{
public:
	// construction
	device_iterator(device_t &root, int maxdepth = 255)
		: m_root(&root),
		  m_current(NULL),
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
		if (start == NULL)
			return NULL;

		// search down first
		if (m_curdepth < m_maxdepth)
		{
			m_current = start->first_subdevice();
			if (m_current != NULL)
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
			if (m_current != NULL)
				return m_current;

			// no? try our parent
			start = start->owner();
			m_curdepth--;
		}

		// returned to the top; we're done
		return m_current = NULL;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (device_t *item = first(); item != NULL; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(device_t &device)
	{
		int index = 0;
		for (device_t *item = first(); item != NULL; item = next(), index++)
			if (item == &device)
				return index;
		return -1;
	}

	// return the indexed item in the list
	device_t *byindex(int index)
	{
		for (device_t *item = first(); item != NULL; item = next(), index--)
			if (index == 0)
				return item;
		return NULL;
	}

private:
	// internal state
	device_t *		m_root;
	device_t *		m_current;
	int				m_curdepth;
	int				m_maxdepth;
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
		for (device_t *device = m_iterator.first(); device != NULL; device = m_iterator.next())
			if (device->type() == _DeviceType)
				return downcast<_DeviceClass *>(device);
		return NULL;
	}

	// advance depth-first
	_DeviceClass *next()
	{
		for (device_t *device = m_iterator.next(); device != NULL; device = m_iterator.next())
			if (device->type() == _DeviceType)
				return downcast<_DeviceClass *>(device);
		return NULL;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (_DeviceClass *item = first(); item != NULL; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(_DeviceClass &device)
	{
		int index = 0;
		for (_DeviceClass *item = first(); item != NULL; item = next(), index++)
			if (item == &device)
				return index;
		return -1;
	}

	// return the indexed item in the list
	_DeviceClass *byindex(int index)
	{
		for (_DeviceClass *item = first(); item != NULL; item = next(), index--)
			if (index == 0)
				return item;
		return NULL;
	}

private:
	// internal state
	device_iterator 	m_iterator;
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
		  m_current(NULL) { }

	// getters
	_InterfaceClass *current() const { return m_current; }

	// reset and return first item
	_InterfaceClass *first()
	{
		for (device_t *device = m_iterator.first(); device != NULL; device = m_iterator.next())
			if (device->interface(m_current))
				return m_current;
		return NULL;
	}

	// advance depth-first
	_InterfaceClass *next()
	{
		for (device_t *device = m_iterator.next(); device != NULL; device = m_iterator.next())
			if (device->interface(m_current))
				return m_current;
		return NULL;
	}

	// return the number of items available
	int count()
	{
		int result = 0;
		for (_InterfaceClass *item = first(); item != NULL; item = next())
			result++;
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(_InterfaceClass &intrf)
	{
		int index = 0;
		for (_InterfaceClass *item = first(); item != NULL; item = next(), index++)
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
	device_iterator 	m_iterator;
	_InterfaceClass *	m_current;
};


// ======================> device_delegate

// device_delegate is a delegate that wraps with a device tag and can be easily
// late bound without replicating logic everywhere
template<typename _Signature>
class device_delegate : public delegate<_Signature>
{
	typedef delegate<_Signature> basetype;

public:
	// provide same set of constructors as the base class, with additional device name
	// parameter
	device_delegate() : basetype(), m_device_name(NULL) { }
	device_delegate(const basetype &src) : basetype(src), m_device_name(src.m_device_name) { }
	device_delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object), m_device_name(src.m_device_name) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	device_delegate(typename basetype::template traits<device_t>::static_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), m_device_name(NULL) { }
	device_delegate(typename basetype::template traits<device_t>::static_ref_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), m_device_name(NULL) { }
	device_delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; m_device_name = src.m_device_name; return *this; }

	// perform the binding
	void bind_relative_to(device_t &search_root);

private:
	// internal state
	const char *m_device_name;
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
	if (this == NULL)
		return NULL;

	// empty string or NULL means this device
	if (tag == NULL || *tag == 0)
		return const_cast<device_t *>(this);

	// do a quick lookup and return that if possible
	device_t *quick = m_device_map.find(tag);
	return (quick != NULL) ? quick : subdevice_slow(tag);
}


//-------------------------------------------------
//  siblingdevice - given a tag, find the device
//  by name relative to this device's parent
//-------------------------------------------------

inline device_t *device_t::siblingdevice(const char *tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// empty string or NULL means this device
	if (tag == NULL || *tag == 0)
		return const_cast<device_t *>(this);

	// leading caret implies the owner, just skip it
	if (tag[0] == '^') tag++;

	// query relative to the parent
	return (m_owner != NULL) ? m_owner->subdevice(tag) : NULL;
}


//-------------------------------------------------
//  bind_relative_to - perform a late binding of
//  a device_delegate
//-------------------------------------------------

template<typename _Signature>
void device_delegate<_Signature>::bind_relative_to(device_t &search_root)
{
	if (!basetype::isnull())
	{
		device_t *device = search_root.subdevice(m_device_name);
		if (device == NULL)
			throw emu_fatalerror("Unable to locate device '%s' relative to '%s'\n", m_device_name, search_root.tag());
		basetype::late_bind(*device);
	}
}

#endif	/* __DEVINTRF_H__ */
