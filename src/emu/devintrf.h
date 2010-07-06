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
#define devtag_get_device(mach,tag)							(mach)->device(tag)
#define devtag_reset(mach,tag)								(mach)->device(tag)->reset()



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices
#define MDRV_DEVICE_CONFIG(_config) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_CONFIG, 8), \
	TOKEN_PTR(voidptr, &(_config)),

#define MDRV_DEVICE_CONFIG_CLEAR() \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_CONFIG, 8), \
	TOKEN_PTR(voidptr, NULL),

#define MDRV_DEVICE_CLOCK(_clock) \
	TOKEN_UINT64_PACK2(MCONFIG_TOKEN_DEVICE_CLOCK, 8, _clock, 32),

#define MDRV_DEVICE_INLINE_DATA16(_index, _data) \
	TOKEN_UINT32_PACK3(MCONFIG_TOKEN_DEVICE_INLINE_DATA16, 8, _index, 8, (UINT16)(_data), 16), \

#define MDRV_DEVICE_INLINE_DATA32(_index, _data) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_DEVICE_INLINE_DATA32, 8, _index, 8), \
	TOKEN_UINT32((UINT32)(_data)),

#define MDRV_DEVICE_INLINE_DATA64(_index, _data) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_DEVICE_INLINE_DATA64, 8, _index, 8), \
	TOKEN_UINT64((UINT64)(_data)),

#ifdef PTR64
#define MDRV_DEVICE_INLINE_DATAPTR(_index, _data) MDRV_DEVICE_INLINE_DATA64(_index, (FPTR)(_data))
#else
#define MDRV_DEVICE_INLINE_DATAPTR(_index, _data) MDRV_DEVICE_INLINE_DATA32(_index, (FPTR)(_data))
#endif



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
union machine_config_token;
class machine_config;


// exception classes
class device_missing_dependencies : public emu_exception { };


// a device_type is simply a pointer to its alloc function
typedef device_config *(*device_type)(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);



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
	void debug_setup_all();
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
	device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock);
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

	// methods that wrap both interface-level and device-level behavior
	void process_token(UINT32 entrytype, const machine_config_token *&tokens);
	void config_complete();
	bool validity_check(const game_driver &driver) const;

	//------------------- begin derived class overrides

	// required operation overrides
	virtual device_t *alloc_device(running_machine &machine) const = 0;

	// optional operation overrides
protected:
	virtual bool device_process_token(UINT32 entrytype, const machine_config_token *&tokens);
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;

public:
	// optional information overrides
	virtual const rom_entry *rom_region() const;
	virtual const machine_config_token *machine_config_tokens() const;

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
	UINT64					m_inline_data[16];		// array of inline configuration values

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
	virtual bool interface_process_token(UINT32 entrytype, const machine_config_token *&tokens);
	virtual bool interface_validity_check(const game_driver &driver) const;

protected:
	const device_config &		m_device_config;
	const machine_config &		m_machine_config;
	device_config_interface *	m_interface_next;
};



// ======================> device_t

// device_t represents a device that is live and attached to a running_machine
class device_t
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

	// debugging
	device_debug *debug() const { return m_debug; }
	void set_debug(device_debug &debug) { m_debug = &debug; }

	// basic information getters ... pass through to underlying config
	device_type type() const { return m_baseconfig.type(); }
	const char *name() const { return m_baseconfig.name(); }
	const char *tag() const { return m_baseconfig.tag(); }

	// machine and ROM configuration getters ... pass through to underlying config
	const rom_entry *rom_region() const { return m_baseconfig.rom_region(); }
	const machine_config_token *machine_config_tokens() const { return m_baseconfig.machine_config_tokens(); }

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
	return (this != NULL) ? dest.cpy(m_tag).cat(":").cat(_tag) : dest.cpy(_tag);
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



// ======================> device clock management

// returns the current device's unscaled running clock speed
inline int device_get_clock(device_t *device)
{
	return device->unscaled_clock();
}

// sets the current device's clock speed and then adjusts for scaling
inline void device_set_clock(device_t *device, int clock)
{
	device->set_unscaled_clock(clock);
}

// returns the current scaling factor for a device's clock speed
inline double device_get_clock_scale(device_t *device)
{
	return device->clock_scale();
}

// sets the current scaling factor for a device's clock speed
inline void device_set_clock_scale(device_t *device, double clockscale)
{
	device->set_clock_scale(clockscale);
}


#endif	/* __DEVINTRF_H__ */
