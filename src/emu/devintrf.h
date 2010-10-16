/***************************************************************************

    devintrf.h

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
#define devtag_reset(mach,tag)								(mach)->device(tag)->reset()

// often derived devices need only a different name and a simple parameter to differentiate them
// these are provided as macros because you can't pass string literals to templates, annoyingly enough
// use this to declare the existence of a derived device in the header file
#define DECLARE_TRIVIAL_DERIVED_DEVICE(_ConfigClass, _ConfigBase, _DeviceClass, _DeviceBase) \
typedef _DeviceBase _DeviceClass;														\
class _ConfigClass;																		\
																						\
class _ConfigClass : public _ConfigBase 												\
{																						\
protected:																				\
	_ConfigClass(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock, UINT32 param = 0); \
																						\
public: 																				\
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock); \
	virtual device_t *alloc_device(running_machine &machine) const;						\
};																						\

// use this macro to define the actual implementation in the source file
#define DEFINE_TRIVIAL_DERIVED_DEVICE(_ConfigClass, _ConfigBase, _DeviceClass, _DeviceBase, _Name, _Param) \
_ConfigClass::_ConfigClass(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock, UINT32 param) \
	: _ConfigBase(mconfig, static_alloc_device_config, _Name, tag, owner, clock, param)	\
{																						\
}																						\
																						\
device_config *_ConfigClass::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
{																						\
	return global_alloc(_ConfigClass(mconfig, tag, owner, clock, _Param));				\
}																						\
																						\
device_t *_ConfigClass::alloc_device(running_machine &machine) const					\
{																						\
	return auto_alloc(&machine, _DeviceClass(machine, *this));							\
}																						\



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices
#define MDRV_DEVICE_CONFIG(_config) \
	device_config::static_set_static_config(device, &(_config)); \

#define MDRV_DEVICE_CONFIG_CLEAR() \
	device_config::static_set_static_config(device, NULL); \

#define MDRV_DEVICE_CLOCK(_clock) \
	device_config::static_set_clock(device, _clock); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class region_info;
class device_debug;
class device_config;
class device_config_interface;
class device_t;
class device_interface;
class device_execute_interface;
class device_memory_interface;
class device_state_interface;
struct rom_entry;
class machine_config;
class emu_timer;


// exception classes
class device_missing_dependencies : public emu_exception { };


// a device_type is simply a pointer to its alloc function
typedef device_config *(*device_type)(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);


// timer IDs for devices
typedef UINT32 device_timer_id;


// read/write types for I/O lines (similar to read/write handlers but no offset)
typedef int (*read_line_device_func)(device_t *device);
typedef void (*write_line_device_func)(device_t *device, int state);



// ======================> tagged_device_list

// tagged_device_list is a tagged_list with additional searching based on type
template<class T>
class tagged_device_list : public tagged_list<T>
{
	typedef tagged_list<T> super;

public:
	tagged_device_list(resource_pool &pool = global_resource_pool)
		: tagged_list<T>(pool) { }

	// pull the generic forms forward
	using super::first;
	using super::count;
	using super::index;
	using super::find;

	// provide type-specific overrides
	T *first(device_type type) const
	{
		T *cur;
		for (cur = super::first(); cur != NULL && cur->type() != type; cur = cur->next()) ;
		return cur;
	}

	int count(device_type type) const
	{
		int num = 0;
		for (const T *curdev = first(type); curdev != NULL; curdev = curdev->typenext()) num++;
		return num;
	}

	int index(device_type type, T *object) const
	{
		int num = 0;
		for (T *cur = first(type); cur != NULL; cur = cur->typenext(), num++)
			if (cur == object) return num;
		return -1;
	}

	int index(device_type type, const char *tag) const
	{
		T *object = find(tag);
		return (object != NULL && object->type() == type) ? index(type, object) : -1;
	}

	T *find(device_type type, int index) const
	{
		for (T *cur = first(type); cur != NULL; cur = cur->typenext())
			if (index-- == 0) return cur;
		return NULL;
	}

	// provide interface-specific overrides
	template<class I>
	bool first(I *&intf) const
	{
		for (T *cur = super::first(); cur != NULL; cur = cur->next())
			if (cur->interface(intf))
				return true;
		return false;
	}
};



// ======================> device_config_list

// device_config_list manages a list of device_configs
typedef tagged_device_list<device_config> device_config_list;



// ======================> device_list

// device_list manages a list of device_ts
class device_list : public tagged_device_list<device_t>
{
	running_machine *m_machine;

	static void static_reset(running_machine &machine);
	static void static_exit(running_machine &machine);
	static void static_pre_save(running_machine *machine, void *param);
	static void static_post_load(running_machine *machine, void *param);

public:
	device_list(resource_pool &pool = global_resource_pool);
	void import_config_list(const device_config_list &list, running_machine &machine);

	void start_all();
	void reset_all();
};



// ======================> device_config

// device_config represents a device configuration that is attached to a machine_config
class device_config
{
	DISABLE_COPYING(device_config);

	friend class machine_config;
	friend class device_t;
	friend class device_config_interface;
	template<class T> friend class tagged_list;

protected:
	// construction/destruction
	device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 param = 0);
	virtual ~device_config();

public:
	// iteration helpers
	device_config *next() const { return m_next; }
	device_config *typenext() const;
	device_config *owner() const { return m_owner; }

	// interface helpers
	template<class T> bool interface(const T *&intf) const { intf = dynamic_cast<const T *>(this); return (intf != NULL); }
	template<class T> bool next(T *&intf) const
	{
		for (device_config *cur = m_next; cur != NULL; cur = cur->m_next)
			if (cur->interface(intf))
				return true;
		return false;
	}

	// owned object helpers
	astring &subtag(astring &dest, const char *tag) const;
	astring &siblingtag(astring &dest, const char *tag) const;

	// basic information getters
	device_type type() const { return m_type; }
	UINT32 clock() const { return m_clock; }
	const char *name() const { return m_name; }
	const char *tag() const { return m_tag; }
	const void *static_config() const { return m_static_config; }
	const machine_config &mconfig() const { return m_machine_config; }

	// methods that wrap both interface-level and device-level behavior
	void config_complete();
	bool validity_check(const game_driver &driver) const;

	// configuration helpers
	static void static_set_clock(device_config *device, UINT32 clock) { device->m_clock = clock; }
	static void static_set_static_config(device_config *device, const void *config) { device->m_static_config = config; }

	//------------------- begin derived class overrides

	// required operation overrides
	virtual device_t *alloc_device(running_machine &machine) const = 0;

	// optional operation overrides
protected:
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;

public:
	// optional information overrides
	virtual const rom_entry *rom_region() const;
	virtual machine_config_constructor machine_config_additions() const;

	//------------------- end derived class overrides

protected:
	// device relationships
	device_config *			m_next;					// next device (of any type/class)
	device_config *			m_owner;				// device that owns us, or NULL if nobody
	device_config_interface *m_interface_list;		// head of interface list

	const device_type		m_type;					// device type
	UINT32					m_clock;				// device clock

	const machine_config &	m_machine_config;		// reference to the machine's configuration
	const void *			m_static_config;		// static device configuration

	astring					m_name;					// name of the device

private:
	astring 				m_tag;					// tag for this instance
	bool					m_config_complete;		// have we completed our configuration?
};



// ======================> device_config_interface

// device_config_interface represents configuration information for a particular device interface
class device_config_interface
{
	DISABLE_COPYING(device_config_interface);

protected:
	// construction/destruction
	device_config_interface(const machine_config &mconfig, device_config &devconfig);
	virtual ~device_config_interface();

public:
	// casting helpers
	const device_config &devconfig() const { return m_device_config; }
	operator const device_config &() const { return m_device_config; }
	operator const device_config *() const { return &m_device_config; }

	// iteration helpers
	device_config_interface *interface_next() const { return m_interface_next; }
	template<class T> bool next(T *&intf) const { return m_device_config.next(intf); }

	// optional operation overrides
	virtual void interface_config_complete();
	virtual bool interface_validity_check(const game_driver &driver) const;

protected:
	const device_config &		m_device_config;
	const machine_config &		m_machine_config;
	device_config_interface *	m_interface_next;
};



// ======================> device_t

// device_t represents a device that is live and attached to a running_machine
class device_t : public bindable_object
{
	DISABLE_COPYING(device_t);

	friend class device_interface;
	template<class T> friend class tagged_list;
	friend class device_list;

protected:
	// construction/destruction
	device_t(running_machine &machine, const device_config &config);
	virtual ~device_t();

public:
	// iteration helpers
	device_t *next() const { return m_next; }
	device_t *typenext() const;
	device_t *owner() const { return m_owner; }

	// interface helpers
	template<class T> bool interface(T *&intf) { intf = dynamic_cast<T *>(this); return (intf != NULL); }
	template<class T> bool next(T *&intf)
	{
		for (device_t *cur = m_next; cur != NULL; cur = cur->m_next)
			if (cur->interface(intf))
				return true;
		return false;
	}

	// specialized helpers
	bool interface(device_execute_interface *&intf) { intf = m_execute; return (intf != NULL); }
	bool interface(device_memory_interface *&intf) { intf = m_memory; return (intf != NULL); }
	bool interface(device_state_interface *&intf) { intf = m_state; return (intf != NULL); }

	// owned object helpers
	astring &subtag(astring &dest, const char *tag) const { return m_baseconfig.subtag(dest, tag); }
	astring &siblingtag(astring &dest, const char *tag) const { return m_baseconfig.siblingtag(dest, tag); }
	const region_info *subregion(const char *tag) const;
	device_t *subdevice(const char *tag) const;
	device_t *siblingdevice(const char *tag) const;
	template<class T> inline T *subdevice(const char *tag) { return downcast<T *>(subdevice(tag)); }
	template<class T> inline T *siblingdevice(const char *tag) { return downcast<T *>(siblingdevice(tag)); }

	// configuration helpers
	const device_config &baseconfig() const { return m_baseconfig; }
	const region_info *region() const { return m_region; }

	// state helpers
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
	void timer_fired(emu_timer &timer, device_timer_id id, int param, void *ptr) { device_timer(timer, id, param, ptr); }

	// debugging
	device_debug *debug() const { return m_debug; }

	// basic information getters ... pass through to underlying config
	device_type type() const { return m_baseconfig.type(); }
	const char *name() const { return m_baseconfig.name(); }
	const char *tag() const { return m_baseconfig.tag(); }

	// machine and ROM configuration getters ... pass through to underlying config
	const rom_entry *rom_region() const { return m_baseconfig.rom_region(); }
	machine_config_constructor machine_config_additions() const { return m_baseconfig.machine_config_additions(); }

public:
	running_machine *		machine;

protected:
	// miscellaneous helpers
	void find_interfaces();
	void start();
	void debug_setup();
	void pre_save();
	void post_load();
	void notify_clock_changed();

	//------------------- begin derived class overrides

	// device-level overrides
	virtual void device_start() = 0;
	virtual void device_reset();
	virtual void device_pre_save();
	virtual void device_post_load();
	virtual void device_clock_changed();
	virtual void device_debug_setup();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	//------------------- end derived class overrides

	running_machine &		m_machine;
	device_debug *			m_debug;

	// for speed
	device_execute_interface *m_execute;
	device_memory_interface *m_memory;
	device_state_interface *m_state;

	// device relationships
	device_t *				m_next;					// next device (of any type/class)
	device_t *				m_owner;				// device that owns us, or NULL if nobody
	device_interface *		m_interface_list;		// head of interface list

	bool					m_started;				// true if the start function has succeeded
	UINT32					m_clock;				// device clock
	const region_info *		m_region;				// our device-local region

	const device_config &	m_baseconfig;			// reference to our device_config
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
};


// running_device is an alias for device_t for now
typedef device_t running_device;



// ======================> device_interface

// device_interface represents runtime information for a particular device interface
class device_interface
{
	DISABLE_COPYING(device_interface);

protected:
	// construction/destruction
	device_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_interface();

public:
	// casting helpers
	device_t &device() { return m_device; }
	const device_t &device() const { return m_device; }
	operator device_t &() { return m_device; }
	operator device_t *() { return &m_device; }

	// iteration helpers
	device_interface *interface_next() const { return m_interface_next; }
	template<class T> bool next(T *&intf) const { return m_device.next(intf); }

	// optional operation overrides
	virtual void interface_pre_start();
	virtual void interface_post_start();
	virtual void interface_pre_reset();
	virtual void interface_post_reset();
	virtual void interface_pre_save();
	virtual void interface_post_load();
	virtual void interface_clock_changed();
	virtual void interface_debug_setup();

protected:
	device_interface *		m_interface_next;
	device_t &				m_device;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************


// ======================> device config helpers

// find the next device_config of the same type
inline device_config *device_config::typenext() const
{
	device_config *cur;
	for (cur = m_next; cur != NULL && cur->m_type != m_type; cur = cur->m_next) ;
	return cur;
}

// create a tag for an object that is owned by this device
inline astring &device_config::subtag(astring &dest, const char *_tag) const
{
	// temp. for now: don't include the root tag in the full tag name
	return (this != NULL && m_owner != NULL) ? dest.cpy(m_tag).cat(":").cat(_tag) : dest.cpy(_tag);
}

// create a tag for an object that a sibling to this device
inline astring &device_config::siblingtag(astring &dest, const char *_tag) const
{
	return (this != NULL && m_owner != NULL) ? m_owner->subtag(dest, _tag) : dest.cpy(_tag);
}



// ======================> running device helpers

// find the next device_t of the same type
inline device_t *device_t::typenext() const
{
	device_t *cur;
	for (cur = m_next; cur != NULL && cur->type() != type(); cur = cur->m_next) ;
	return cur;
}


#endif	/* __DEVINTRF_H__ */
