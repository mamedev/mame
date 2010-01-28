/***************************************************************************

    devintrf.h

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVINTRF_H__
#define __DEVINTRF_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

// forward references
struct rom_entry;
union machine_config_token;


// device classes
enum device_class
{
	DEVICE_CLASS_GENERAL = 0,			// default class for general devices
	DEVICE_CLASS_PERIPHERAL,			// peripheral devices: PIAs, timers, etc.
	DEVICE_CLASS_AUDIO,					// audio devices (not sound chips), including speakers
	DEVICE_CLASS_VIDEO,					// video devices, including screens
	DEVICE_CLASS_CPU_CHIP,				// CPU chips; only CPU cores should return this class
	DEVICE_CLASS_SOUND_CHIP,			// sound chips; only sound cores should return this class
	DEVICE_CLASS_TIMER,					// timer devices
	DEVICE_CLASS_OTHER					// anything else (the list may expand in the future)
};


// state constants passed to the device_get_config_func
enum
{
	// --- the following bits of info are returned as 64-bit signed integers ---
	DEVINFO_INT_FIRST = 0x00000,

		DEVINFO_INT_TOKEN_BYTES = DEVINFO_INT_FIRST,	// R/O: bytes to allocate for the token
		DEVINFO_INT_INLINE_CONFIG_BYTES,				// R/O: bytes to allocate for the inline configuration
		DEVINFO_INT_CLASS,								// R/O: the device's class

		DEVINFO_INT_ENDIANNESS,							// R/O: either ENDIANNESS_BIG or ENDIANNESS_LITTLE
		DEVINFO_INT_DATABUS_WIDTH,						// R/O: data bus size for each address space (8,16,32,64)
		DEVINFO_INT_DATABUS_WIDTH_0 = DEVINFO_INT_DATABUS_WIDTH + 0,
		DEVINFO_INT_DATABUS_WIDTH_1 = DEVINFO_INT_DATABUS_WIDTH + 1,
		DEVINFO_INT_DATABUS_WIDTH_2 = DEVINFO_INT_DATABUS_WIDTH + 2,
		DEVINFO_INT_DATABUS_WIDTH_3 = DEVINFO_INT_DATABUS_WIDTH + 3,
		DEVINFO_INT_DATABUS_WIDTH_LAST = DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACES - 1,
		DEVINFO_INT_ADDRBUS_WIDTH,						// R/O: address bus size for each address space (12-32)
		DEVINFO_INT_ADDRBUS_WIDTH_0 = DEVINFO_INT_ADDRBUS_WIDTH + 0,
		DEVINFO_INT_ADDRBUS_WIDTH_1 = DEVINFO_INT_ADDRBUS_WIDTH + 1,
		DEVINFO_INT_ADDRBUS_WIDTH_2 = DEVINFO_INT_ADDRBUS_WIDTH + 2,
		DEVINFO_INT_ADDRBUS_WIDTH_3 = DEVINFO_INT_ADDRBUS_WIDTH + 3,
		DEVINFO_INT_ADDRBUS_WIDTH_LAST = DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACES - 1,
		DEVINFO_INT_ADDRBUS_SHIFT,						// R/O: shift applied to addresses each address space (+3 means >>3, -1 means <<1)
		DEVINFO_INT_ADDRBUS_SHIFT_0 = DEVINFO_INT_ADDRBUS_SHIFT + 0,
		DEVINFO_INT_ADDRBUS_SHIFT_1 = DEVINFO_INT_ADDRBUS_SHIFT + 1,
		DEVINFO_INT_ADDRBUS_SHIFT_2 = DEVINFO_INT_ADDRBUS_SHIFT + 2,
		DEVINFO_INT_ADDRBUS_SHIFT_3 = DEVINFO_INT_ADDRBUS_SHIFT + 3,
		DEVINFO_INT_ADDRBUS_SHIFT_LAST = DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACES - 1,

	DEVINFO_INT_CLASS_SPECIFIC = 0x04000,				// R/W: device-specific values start here
	DEVINFO_INT_DEVICE_SPECIFIC = 0x08000,				// R/W: device-specific values start here
	DEVINFO_INT_LAST = 0x0ffff,

	// --- the following bits of info are returned as pointers ---
	DEVINFO_PTR_FIRST = 0x10000,

		DEVINFO_PTR_ROM_REGION = DEVINFO_PTR_FIRST,		// R/O: pointer to device-specific ROM region
		DEVINFO_PTR_MACHINE_CONFIG,						// R/O: pointer to device-specific machine config

		DEVINFO_PTR_INTERNAL_MEMORY_MAP,				// R/O: const addrmap_token *map
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_0 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 0,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_1 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 1,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_2 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 2,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_3 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 3,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_LAST = DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,

		DEVINFO_PTR_DEFAULT_MEMORY_MAP,					// R/O: const addrmap_token *map
		DEVINFO_PTR_DEFAULT_MEMORY_MAP_0 = DEVINFO_PTR_DEFAULT_MEMORY_MAP + 0,
		DEVINFO_PTR_DEFAULT_MEMORY_MAP_1 = DEVINFO_PTR_DEFAULT_MEMORY_MAP + 1,
		DEVINFO_PTR_DEFAULT_MEMORY_MAP_2 = DEVINFO_PTR_DEFAULT_MEMORY_MAP + 2,
		DEVINFO_PTR_DEFAULT_MEMORY_MAP_3 = DEVINFO_PTR_DEFAULT_MEMORY_MAP + 3,
		DEVINFO_PTR_DEFAULT_MEMORY_MAP_LAST = DEVINFO_PTR_DEFAULT_MEMORY_MAP + ADDRESS_SPACES - 1,

	DEVINFO_PTR_CLASS_SPECIFIC = 0x14000,				// R/W: device-specific values start here
	DEVINFO_PTR_DEVICE_SPECIFIC = 0x18000,				// R/W: device-specific values start here
	DEVINFO_PTR_LAST = 0x1ffff,

	// --- the following bits of info are returned as pointers to functions ---
	DEVINFO_FCT_FIRST = 0x20000,

		DEVINFO_FCT_START = DEVINFO_FCT_FIRST,			// R/O: device_start_func
		DEVINFO_FCT_STOP,								// R/O: device_stop_func
		DEVINFO_FCT_RESET,								// R/O: device_reset_func
		DEVINFO_FCT_EXECUTE,							// R/O: device_execute_func
		DEVINFO_FCT_VALIDITY_CHECK,						// R/O: device_validity_check_func
		DEVINFO_FCT_NVRAM,								// R/O: device_nvram_func
		DEVINFO_FCT_CUSTOM_CONFIG,						// R/O: device_custom_config_func
		DEVINFO_FCT_GET_RUNTIME_INFO,					// R/O: device_get_runtime_info_func

	DEVINFO_FCT_CLASS_SPECIFIC = 0x24000,				// R/W: device-specific values start here
	DEVINFO_FCT_DEVICE_SPECIFIC = 0x28000,				// R/W: device-specific values start here
	DEVINFO_FCT_LAST = 0x2ffff,

	// --- the following bits of info are returned as NULL-terminated strings ---
	DEVINFO_STR_FIRST = 0x30000,

		DEVINFO_STR_NAME = DEVINFO_STR_FIRST,			// R/O: name of the device
		DEVINFO_STR_FAMILY,								// R/O: family of the device
		DEVINFO_STR_VERSION,							// R/O: version of the device
		DEVINFO_STR_SOURCE_FILE,						// R/O: file containing the device implementation
		DEVINFO_STR_CREDITS,							// R/O: credits for the device implementation

	DEVINFO_STR_CLASS_SPECIFIC = 0x34000,				// R/W: device-specific values start here
	DEVINFO_STR_DEVICE_SPECIFIC = 0x38000,				// R/W: device-specific values start here
	DEVINFO_STR_LAST = 0x3ffff
};



/***************************************************************************
    MACROS
***************************************************************************/

#define DEVICE_GET_INFO_NAME(name)	device_get_config_##name
#define DEVICE_GET_INFO(name)		void DEVICE_GET_INFO_NAME(name)(const device_config *device, UINT32 state, deviceinfo *info)
#define DEVICE_GET_INFO_CALL(name)	DEVICE_GET_INFO_NAME(name)(device, state, info)

#define DEVICE_VALIDITY_CHECK_NAME(name)	device_validity_check_##name
#define DEVICE_VALIDITY_CHECK(name)			int DEVICE_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const device_config *device)
#define DEVICE_VALIDITY_CHECK_CALL(name)	DEVICE_VALIDITY_CHECK_NAME(name)(driver, device)

#define DEVICE_CUSTOM_CONFIG_NAME(name)		device_custom_config_##name
#define DEVICE_CUSTOM_CONFIG(name)			const machine_config_token *DEVICE_CUSTOM_CONFIG_NAME(name)(const device_config *device, UINT32 entrytype, const machine_config_token *tokens)
#define DEVICE_CUSTOM_CONFIG_CALL(name)		DEVICE_CUSTOM_CONFIG_NAME(name)(device, entrytype, tokens)


#define DEVICE_GET_RUNTIME_INFO_NAME(name)	device_get_runtime_info_##name
#define DEVICE_GET_RUNTIME_INFO(name)		void DEVICE_GET_RUNTIME_INFO_NAME(name)(running_device *device, UINT32 state, deviceinfo *info)
#define DEVICE_GET_RUNTIME_INFO_CALL(name)	DEVICE_GET_RUNTIME_INFO_NAME(name)(device, state, info)

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			void DEVICE_START_NAME(name)(running_device *device)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(running_device *device)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(device)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(running_device *device)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(device)

#define DEVICE_EXECUTE_NAME(name)	device_execute_##name
#define DEVICE_EXECUTE(name)		INT32 DEVICE_EXECUTE_NAME(name)(running_device *device, INT32 clocks)
#define DEVICE_EXECUTE_CALL(name)	DEVICE_EXECUTE_NAME(name)(device, clocks)

#define DEVICE_NVRAM_NAME(name)		device_nvram_##name
#define DEVICE_NVRAM(name)			void DEVICE_NVRAM_NAME(name)(running_device *device, mame_file *file, int read_or_write)
#define DEVICE_NVRAM_CALL(name)		DEVICE_NVRAM_NAME(name)(device, file, read_or_write)


// macro for specifying a clock derived from an owning device
#define DERIVED_CLOCK(num, den)		(0xff000000 | ((num) << 12) | ((den) << 0))


// shorthand for accessing devices by machine/type/tag
#define devtag_get_device(mach,tag)							(mach)->device(tag)

#define devtag_reset(mach,tag)								(mach)->device(tag)->reset()



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// forward-declare these types
class region_info;
class device_config;
class running_device;
union deviceinfo;


// exception types
class device_missing_dependencies : public emu_exception { };


// device interface function types
typedef void (*device_get_config_func)(const device_config *device, UINT32 state, deviceinfo *info);
typedef int (*device_validity_check_func)(const game_driver *driver, const device_config *device);
typedef const machine_config_token *(*device_custom_config_func)(const device_config *device, UINT32 entrytype, const machine_config_token *tokens);

typedef void (*device_start_func)(running_device *device);
typedef void (*device_stop_func)(running_device *device);
typedef INT32 (*device_execute_func)(running_device *device, INT32 clocks);
typedef void (*device_reset_func)(running_device *device);
typedef void (*device_nvram_func)(running_device *device, mame_file *file, int read_or_write);
typedef void (*device_get_runtime_info_func)(running_device *device, UINT32 state, deviceinfo *info);


// a device_type is simply a pointer to its get_info function
typedef device_get_config_func device_type;


// tagged_device_list is a tagged_list with additional searching based on type or class
template<class T> class tagged_device_list : public tagged_list<T>
{
	typedef tagged_list<T> super;

public:
	// pull the generic forms forward
	using super::first;
	using super::count;
	using super::index;
	using super::find;

	// provide type-specific overrides
	T *first(device_type type) const
	{
		T *cur;
		for (cur = super::first(); cur != NULL && cur->type != type; cur = cur->next) ;
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
		return (object != NULL && object->type == type) ? index(type, object) : -1;
	}

	T *find(device_type type, int index) const
	{
		for (T *cur = first(type); cur != NULL; cur = cur->typenext())
			if (index-- == 0) return cur;
		return NULL;
	}

	// provide class-specific overrides
	T *first(device_class devclass) const
	{
		T *cur;
		for (cur = super::first(); cur != NULL && cur->devclass != devclass; cur = cur->next) ;
		return cur;
	}

	int count(device_class devclass) const
	{
		int num = 0;
		for (const T *curdev = first(devclass); curdev != NULL; curdev = curdev->classnext()) num++;
		return num;
	}

	int index(device_class devclass, T *object) const
	{
		int num = 0;
		for (T *cur = first(devclass); cur != NULL; cur = cur->classnext(), num++)
			if (cur == object) return num;
		return -1;
	}

	int index(device_class devclass, const char *tag) const
	{
		T *object = find(tag);
		return (object != NULL && object->devclass == devclass) ? index(devclass, object) : -1;
	}

	T *find(device_class devclass, int index) const
	{
		for (T *cur = first(devclass); cur != NULL; cur = cur->classnext())
			if (index-- == 0) return cur;
		return NULL;
	}
};


// device_config_list manages a list of device_configs
typedef tagged_device_list<device_config> device_config_list;


// device_list manages  a list of running_devices
class device_list : public tagged_device_list<running_device>
{
	running_machine *machine;

	static void static_reset(running_machine *machine);
	static void static_stop(running_machine *machine);

public:
	device_list();
	void import_config_list(const device_config_list &list, running_machine &machine);

	void start_all();
	void reset_all();
	void stop_all();
};



// the actual deviceinfo union
union deviceinfo
{
	INT64					i;							// generic integers
	void *					p;							// generic pointers
	genf *  				f;							// generic function pointers
	char *					s;							// generic strings

	device_start_func		start;						// DEVINFO_FCT_START
	device_stop_func		stop;						// DEVINFO_FCT_STOP
	device_reset_func		reset;						// DEVINFO_FCT_RESET
	device_execute_func 	execute;					// DEVINFO_FCT_EXECUTE
	device_validity_check_func validity_check;			// DEVINFO_FCT_VALIDITY_CHECK
	device_custom_config_func custom_config;			// DEVINFO_FCT_CUSTOM_CONFIG
	device_nvram_func		nvram;						// DEVINFO_FCT_NVRAM
	device_get_runtime_info_func get_runtime_info;		// DEVINFO_FCT_GET_RUNTIME_INFO
	const rom_entry *		romregion;					// DEVINFO_PTR_ROM_REGION
	const machine_config_token *machine_config;			// DEVINFO_PTR_MACHINE_CONFIG
	const addrmap8_token *	internal_map8;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap16_token *	internal_map16;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap32_token *	internal_map32;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap64_token *	internal_map64;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap8_token *	default_map8;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap16_token *	default_map16;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap32_token *	default_map32;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap64_token *	default_map64;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
};


// the configuration for a general device
enum device_space
{
	AS_PROGRAM = 0,
	AS_DATA = 1,
	AS_IO = 2
};


// device_config represents a device configuration that is attached to a machine_config
class device_config
{
	DISABLE_COPYING(device_config);

public:
	device_config(const device_config *owner, device_type type, const char *tag, UINT32 clock);
	virtual ~device_config();

	device_config *typenext() const
	{
		device_config *cur;
		for (cur = this->next; cur != NULL && cur->type != type; cur = cur->next) ;
		return cur;
	}

	device_config *classnext() const
	{
		device_config *cur;
		for (cur = this->next; cur != NULL && cur->devclass != devclass; cur = cur->next) ;
		return cur;
	}

	// get state from a device config
	virtual endianness_t endianness() const { return static_cast<endianness_t>(get_config_int(DEVINFO_INT_ENDIANNESS)); }
	virtual UINT8 databus_width(int spacenum = 0) const { return get_config_int(DEVINFO_INT_DATABUS_WIDTH + spacenum); }
	virtual UINT8 addrbus_width(int spacenum = 0) const { return get_config_int(DEVINFO_INT_ADDRBUS_WIDTH + spacenum); }
	virtual INT8 addrbus_shift(int spacenum = 0) const { return get_config_int(DEVINFO_INT_ADDRBUS_SHIFT + spacenum); }

	virtual const rom_entry *rom_region() const { return reinterpret_cast<const rom_entry *>(get_config_ptr(DEVINFO_PTR_ROM_REGION)); }
	virtual const machine_config_token *machine_config_tokens() const { return reinterpret_cast<const machine_config_token *>(get_config_ptr(DEVINFO_PTR_MACHINE_CONFIG)); }
	virtual const addrmap_token *internal_map(int spacenum = 0) const { return reinterpret_cast<const addrmap_token *>(get_config_ptr(DEVINFO_PTR_INTERNAL_MEMORY_MAP + spacenum)); }
	virtual const addrmap_token *default_map(int spacenum = 0) const { return reinterpret_cast<const addrmap_token *>(get_config_ptr(DEVINFO_PTR_DEFAULT_MEMORY_MAP + spacenum)); }

	virtual const char *name() const { return get_config_string(DEVINFO_STR_NAME); }
	virtual const char *family() const { return get_config_string(DEVINFO_STR_FAMILY); }
	virtual const char *version() const { return get_config_string(DEVINFO_STR_VERSION); }
	virtual const char *source_file() const { return get_config_string(DEVINFO_STR_SOURCE_FILE); }
	virtual const char *credits() const { return get_config_string(DEVINFO_STR_CREDITS); }

	INT64 get_config_int(UINT32 state) const;
	void *get_config_ptr(UINT32 state) const;
	genf *get_config_fct(UINT32 state) const;
	const char *get_config_string(UINT32 state) const;

	astring &subtag(astring &dest, const char *tag) const;
	astring &siblingtag(astring &dest, const char *tag) const;

	// device relationships
	device_config *			next;					// next device (of any type/class)
	device_config *			owner;					// device that owns us, or NULL if nobody

	// device properties
	astring 				tag;					// tag for this instance
	device_type				type;					// device type
	device_class			devclass;				// device class

	// device configuration
	UINT32					clock;					// device clock
	const addrmap_token *	address_map[ADDRESS_SPACES]; // address maps for each address space
	const void *			static_config;			// static device configuration
	void *					inline_config;			// inline device configuration
};


// running_device represents a device that is live and attached to a running_machine
class running_device
{
	DISABLE_COPYING(running_device);

	const device_config &	m_baseconfig;

public:	// private eventually
	const address_space *	addrspace[ADDRESS_SPACES];	// auto-discovered address spaces

public:
	running_device(running_machine &machine, const device_config &config);
	virtual ~running_device();

	const device_config &baseconfig() const { return m_baseconfig; }

	inline const address_space *space(int index = 0) const;
	inline const address_space *space(device_space index) const;

	astring &subtag(astring &dest, const char *tag) const { return m_baseconfig.subtag(dest, tag); }
	astring &siblingtag(astring &dest, const char *tag) const { return m_baseconfig.siblingtag(dest, tag); }

	const region_info *subregion(const char *tag) const;
	running_device *subdevice(const char *tag) const;

	running_device *typenext() const
	{
		running_device *cur;
		for (cur = this->next; cur != NULL && cur->type != type; cur = cur->next) ;
		return cur;
	}

	running_device *classnext() const
	{
		running_device *cur;
		for (cur = this->next; cur != NULL && cur->devclass != devclass; cur = cur->next) ;
		return cur;
	}

	void start();
	void reset();
	void stop();
	void set_clock(UINT32 clock);

	// get state from a device config
	endianness_t endianness() const { return m_baseconfig.endianness(); }
	UINT8 databus_width(int spacenum = 0) const { return m_baseconfig.databus_width(spacenum); }
	UINT8 addrbus_width(int spacenum = 0) const { return m_baseconfig.addrbus_width(spacenum); }
	INT8 addrbus_shift(int spacenum = 0) const { return m_baseconfig.addrbus_shift(spacenum); }

	const rom_entry *rom_region() const { return m_baseconfig.rom_region(); }
	const machine_config_token *machine_config_tokens() const { return m_baseconfig.machine_config_tokens(); }
	const addrmap_token *internal_map(int spacenum = 0) const { return m_baseconfig.internal_map(spacenum); }
	const addrmap_token *default_map(int spacenum = 0) const { return m_baseconfig.default_map(spacenum); }

	const char *name() const { return m_baseconfig.name(); }
	const char *family() const { return m_baseconfig.family(); }
	const char *version() const { return m_baseconfig.version(); }
	const char *source_file() const { return m_baseconfig.source_file(); }
	const char *credits() const { return m_baseconfig.credits(); }

	INT64 get_config_int(UINT32 state) const { return m_baseconfig.get_config_int(state); }
	void *get_config_ptr(UINT32 state) const { return m_baseconfig.get_config_ptr(state); }
	genf *get_config_fct(UINT32 state) const { return m_baseconfig.get_config_fct(state); }
	const char *get_config_string(UINT32 state) const { return m_baseconfig.get_config_string(state); }

	INT64 get_runtime_int(UINT32 state);
	void *get_runtime_ptr(UINT32 state);
	const char *get_runtime_string(UINT32 state);

	void set_address_space(int spacenum, const address_space *space);

	// these fields are only valid once the device is attached to a machine
	running_machine *		machine;				// machine if device is live

	// device relationships
	running_device *		next;					// next device (of any type/class)
	running_device *		owner;					// device that owns us, or NULL if nobody

	// device properties
	astring 				tag;					// tag for this instance
	device_type				type;					// device type
	device_class			devclass;				// device class

	// device configuration
	UINT32					clock;					// device clock

	// these fields are only valid if the device is live
	bool					started;				// true if the start function has succeeded
	void *					token;					// token if device is live
	UINT32					tokenbytes;				// size of the token data allocated
	const region_info *		region;					// our device-local region

	device_execute_func 	execute;				// quick pointer to execute callback
	device_get_runtime_info_func get_runtime_info;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    space - return an address space within a
    device
-------------------------------------------------*/

inline const address_space *running_device::space(int index) const
{
	return addrspace[index];
}

inline const address_space *running_device::space(device_space index) const
{
	return space(static_cast<int>(index));
}



#endif	/* __DEVINTRF_H__ */
