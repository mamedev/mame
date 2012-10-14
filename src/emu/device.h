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


// macros for defining read_line/write_line functions
#define READ_LINE_DEVICE_HANDLER(name)		int  name(ATTR_UNUSED device_t *device)
#define WRITE_LINE_DEVICE_HANDLER(name) 	void name(ATTR_UNUSED device_t *device, ATTR_UNUSED int state)

#define DECLARE_READ_LINE_MEMBER(name)		int  name()
#define READ_LINE_MEMBER(name)				int  name()
#define DECLARE_WRITE_LINE_MEMBER(name) 	void name(ATTR_UNUSED int state)
#define WRITE_LINE_MEMBER(name)				void name(ATTR_UNUSED int state)



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
	UINT8 default_bios() const { return m_default_bios; }
	UINT8 system_bios() const { return m_system_bios; }

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
	device_state_interface &state() const { assert(m_state != NULL); return *m_state; }

	// owned object helpers
	device_t *first_subdevice() const { return m_subdevice_list.first(); }
	astring &subtag(astring &dest, const char *tag) const;
	astring &siblingtag(astring &dest, const char *tag) const { return (this != NULL && m_owner != NULL) ? m_owner->subtag(dest, tag) : dest.cpy(tag); }
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
	offs_t safe_pc();
	offs_t safe_pcbase();

	void set_default_bios(UINT8 bios) { m_default_bios = bios; }
	void set_system_bios(UINT8 bios) { m_system_bios = bios; }

protected:
	// internal helper classes (defined below)
	class finder_base;
	template<class _ObjectClass> class object_finder_base;
	template<class _DeviceClass, bool _Required> class device_finder;
	template<class _DeviceClass> class optional_device;
	template<class _DeviceClass> class required_device;
	template<bool _Required> class memory_region_finder;
	class optional_memory_region;
	class required_memory_region;
	template<bool _Required> class memory_bank_finder;
	class optional_memory_bank;
	class required_memory_bank;
	template<bool _Required> class ioport_finder;
	class optional_ioport;
	class required_ioport;
	template<typename _PointerType, bool _Required> class shared_ptr_finder;
	template<typename _PointerType> class optional_shared_ptr;
	template<typename _PointerType> class required_shared_ptr;
	template<typename _PointerType, int _Count, bool _Required> class shared_ptr_array_finder;
	template<typename _PointerType, int _Count> class optional_shared_ptr_array;
	template<typename _PointerType, int _Count> class required_shared_ptr_array;

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
	memory_region *			m_region;				// our device-local region
	const machine_config &	m_machine_config;		// reference to the machine's configuration
	const void *			m_static_config;		// static device configuration
	const input_device_default *m_input_defaults;   // devices input ports default overrides

	UINT8					m_system_bios;			// the system BIOS we wish to load
	UINT8					m_default_bios;			// the default system BIOS
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
	finder_base *			m_auto_finder_list;		// list of objects to auto-find
};


// ======================> finder_base

// helper class to request auto-object discovery in the constructor of a derived class
class device_t::finder_base
{
	friend class device_t;

public:
	// construction/destruction
	finder_base(device_t &base, const char *tag);
	virtual ~finder_base();

	// getters
	virtual bool findit() = 0;

protected:
	// helpers
	void *find_memory(UINT8 width, size_t &bytes, bool required);
	bool report_missing(bool found, const char *objname, bool required);

	// internal state
	finder_base *m_next;
	device_t &m_base;
	const char *m_tag;
};


// ======================> object_finder_base

// helper class to find objects of a particular type
template<class _ObjectClass>
class device_t::object_finder_base : public device_t::finder_base
{
public:
	// construction/destruction
	object_finder_base(device_t &base, const char *tag)
		: finder_base(base, tag),
		  m_target(NULL) { }

	// operators to make use transparent
	operator _ObjectClass *() const { return m_target; }
	_ObjectClass *operator->() const { assert(m_target != NULL); return m_target; }

	// getter for explicit fetching
	_ObjectClass *target() const { return m_target; }

	// setter for setting the object
	void set_target(_ObjectClass *target) { m_target = target; }

protected:
	// internal state
	_ObjectClass *m_target;
};


// ======================> device_finder

// device finder template
template<class _DeviceClass, bool _Required>
class device_t::device_finder : public device_t::object_finder_base<_DeviceClass>
{
public:
	// construction/destruction
	device_finder(device_t &base, const char *tag)
		: object_finder_base<_DeviceClass>(base, tag) { }

	// make reference use transparent as well
	operator _DeviceClass &() { assert(object_finder_base<_DeviceClass>::m_target != NULL); return *object_finder_base<_DeviceClass>::m_target; }

	// finder
	virtual bool findit()
	{
		device_t *device = this->m_base.subdevice(this->m_tag);
		this->m_target = dynamic_cast<_DeviceClass *>(device);
		if (device != NULL && this->m_target == NULL)
		{
			void mame_printf_warning(const char *format, ...) ATTR_PRINTF(1,2);
			mame_printf_warning("Device '%s' found but is of incorrect type\n", this->m_tag);
		}
		return this->report_missing(this->m_target != NULL, "device", _Required);
	}
};

// optional device finder
template<class _DeviceClass>
class device_t::optional_device : public device_t::device_finder<_DeviceClass, false>
{
public:
	optional_device(device_t &base, const char *tag) : device_finder<_DeviceClass, false>(base, tag) { }
};

// required devices are similar but throw an error if they are not found
template<class _DeviceClass>
class device_t::required_device : public device_t::device_finder<_DeviceClass, true>
{
public:
	required_device(device_t &base, const char *tag) : device_finder<_DeviceClass, true>(base, tag) { }
};


// ======================> memory_region_finder

// device finder template
template<bool _Required>
class device_t::memory_region_finder : public device_t::object_finder_base<memory_region>
{
public:
	// construction/destruction
	memory_region_finder(device_t &base, const char *tag)
		: object_finder_base<memory_region>(base, tag) { }

	// make reference use transparent as well
	operator memory_region &() { assert(object_finder_base<memory_region>::m_target != NULL); return *object_finder_base<memory_region>::m_target; }

	// finder
	virtual bool findit()
	{
		m_target = m_base.memregion(m_tag);
		return this->report_missing(m_target != NULL, "memory region", _Required);
	}
};

// optional device finder
class device_t::optional_memory_region : public device_t::memory_region_finder<false>
{
public:
	optional_memory_region(device_t &base, const char *tag) : memory_region_finder<false>(base, tag) { }
};

// required devices are similar but throw an error if they are not found
class device_t::required_memory_region : public device_t::memory_region_finder<true>
{
public:
	required_memory_region(device_t &base, const char *tag) : memory_region_finder<true>(base, tag) { }
};


// ======================> memory_bank_finder

// device finder template
template<bool _Required>
class device_t::memory_bank_finder : public device_t::object_finder_base<memory_bank>
{
public:
	// construction/destruction
	memory_bank_finder(device_t &base, const char *tag)
		: object_finder_base<memory_bank>(base, tag) { }

	// make reference use transparent as well
	operator memory_bank &() { assert(object_finder_base<memory_bank>::m_target != NULL); return *object_finder_base<memory_bank>::m_target; }

	// finder
	virtual bool findit()
	{
		m_target = m_base.membank(m_tag);
		return this->report_missing(m_target != NULL, "memory bank", _Required);
	}
};

// optional device finder
class device_t::optional_memory_bank : public device_t::memory_bank_finder<false>
{
public:
	optional_memory_bank(device_t &base, const char *tag) : memory_bank_finder<false>(base, tag) { }
};

// required devices are similar but throw an error if they are not found
class device_t::required_memory_bank : public device_t::memory_bank_finder<true>
{
public:
	required_memory_bank(device_t &base, const char *tag) : memory_bank_finder<true>(base, tag) { }
};


// ======================> ioport_finder

// device finder template
template<bool _Required>
class device_t::ioport_finder : public device_t::object_finder_base<ioport_port>
{
public:
	// construction/destruction
	ioport_finder(device_t &base, const char *tag)
		: object_finder_base<ioport_port>(base, tag) { }

	// make reference use transparent as well
	operator ioport_port &() { assert(object_finder_base<ioport_port>::m_target != NULL); return *object_finder_base<ioport_port>::m_target; }

	// finder
	virtual bool findit()
	{
		m_target = m_base.ioport(m_tag);
		return this->report_missing(m_target != NULL, "I/O port", _Required);
	}
};

// optional device finder
class device_t::optional_ioport : public device_t::ioport_finder<false>
{
public:
	optional_ioport(device_t &base, const char *tag) : ioport_finder<false>(base, tag) { }
};

// required devices are similar but throw an error if they are not found
class device_t::required_ioport : public device_t::ioport_finder<true>
{
public:
	required_ioport(device_t &base, const char *tag) : ioport_finder<true>(base, tag) { }
};


// ======================> shared_ptr_finder

// shared pointer finder template
template<typename _PointerType, bool _Required>
class device_t::shared_ptr_finder : public device_t::object_finder_base<_PointerType>
{
public:
	// construction/destruction
	shared_ptr_finder(device_t &base, const char *tag, UINT8 width = sizeof(_PointerType) * 8)
		: object_finder_base<_PointerType>(base, tag),
		  m_bytes(0),
		  m_allocated(false),
		  m_width(width) { }

	virtual ~shared_ptr_finder() { if (m_allocated) global_free(this->m_target); }

	// operators to make use transparent
	_PointerType operator[](int index) const { return this->m_target[index]; }
	_PointerType &operator[](int index) { return this->m_target[index]; }

	// getter for explicit fetching
	UINT32 bytes() const { return m_bytes; }

	// setter for setting the object
	void set_target(_PointerType *target, size_t bytes) { this->m_target = target; m_bytes = bytes; }

	// dynamic allocation of a shared pointer
	void allocate(UINT32 entries)
	{
		assert(!m_allocated);
		m_allocated = true;
		this->m_target = global_alloc_array_clear(_PointerType, entries);
		m_bytes = entries * sizeof(_PointerType);
		this->m_base.save_pointer(this->m_target, this->m_tag, entries);
	}

	// finder
	virtual bool findit()
	{
		this->m_target = reinterpret_cast<_PointerType *>(this->find_memory(m_width, m_bytes, _Required));
		return this->report_missing(this->m_target != NULL, "shared pointer", _Required);
	}

protected:
	// internal state
	size_t m_bytes;
	bool m_allocated;
	UINT8 m_width;
};

// optional shared pointer finder
template<class _PointerType>
class device_t::optional_shared_ptr : public device_t::shared_ptr_finder<_PointerType, false>
{
public:
	optional_shared_ptr(device_t &base, const char *tag, UINT8 width = sizeof(_PointerType) * 8) : shared_ptr_finder<_PointerType, false>(base, tag, width) { }
};

// required shared pointer finder
template<class _PointerType>
class device_t::required_shared_ptr : public device_t::shared_ptr_finder<_PointerType, true>
{
public:
	required_shared_ptr(device_t &base, const char *tag, UINT8 width = sizeof(_PointerType) * 8) : shared_ptr_finder<_PointerType, true>(base, tag, width) { }
};


// ======================> shared_ptr_array_finder

// shared pointer array finder template
template<typename _PointerType, int _Count, bool _Required>
class device_t::shared_ptr_array_finder
{
	typedef shared_ptr_finder<_PointerType, _Required> shared_ptr_type;

public:
	// construction/destruction
	shared_ptr_array_finder(device_t &base, const char *basetag, UINT8 width = sizeof(_PointerType) * 8)
	{
		for (int index = 0; index < _Count; index++)
			m_array[index] = global_alloc(shared_ptr_type(base, m_tag[index].format("%s.%d", basetag, index), width));
	}

	virtual ~shared_ptr_array_finder()
	{
		for (int index = 0; index < _Count; index++)
			global_free(m_array[index]);
	}

	// array accessors
	const shared_ptr_type &operator[](int index) const { assert(index < _Count); return *m_array[index]; }
	shared_ptr_type &operator[](int index) { assert(index < _Count); return *m_array[index]; }

protected:
	// internal state
	shared_ptr_type *m_array[_Count+1];
	astring m_tag[_Count+1];
};

// optional shared pointer array finder
template<class _PointerType, int _Count>
class device_t::optional_shared_ptr_array : public device_t::shared_ptr_array_finder<_PointerType, _Count, false>
{
public:
	optional_shared_ptr_array(device_t &base, const char *tag, UINT8 width = sizeof(_PointerType) * 8) : shared_ptr_array_finder<_PointerType, _Count, false>(base, tag, width) { }
};

// required shared pointer array finder
template<class _PointerType, int _Count>
class device_t::required_shared_ptr_array : public device_t::shared_ptr_array_finder<_PointerType, _Count, true>
{
public:
	required_shared_ptr_array(device_t &base, const char *tag, UINT8 width = sizeof(_PointerType) * 8) : shared_ptr_array_finder<_PointerType, _Count, true>(base, tag, width) { }
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

	// query relative to the parent, if we have one
	if (m_owner != NULL)
		return m_owner->subdevice(tag);

	// otherwise, it's NULL unless the tag is absolute
	return (tag[0] == ':') ? subdevice(tag) : NULL;
}

#endif	/* __DEVINTRF_H__ */
