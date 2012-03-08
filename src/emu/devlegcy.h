/***************************************************************************

    devlegcy.h

    Legacy device helpers.

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

#ifndef __DEVLEGCY_H__
#define __DEVLEGCY_H__

#include <stddef.h>

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define DEVINFO_IMAGE_CREATE_OPTMAX   32

// state constants passed to the device_get_config_func
enum
{
	// --- the following bits of info are returned as 64-bit signed integers ---
	DEVINFO_INT_FIRST = 0x00000,

		DEVINFO_INT_TOKEN_BYTES = DEVINFO_INT_FIRST,	// R/O: bytes to allocate for the token
		DEVINFO_INT_INLINE_CONFIG_BYTES,				// R/O: bytes to allocate for the inline configuration

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
		DEVINFO_PTR_INPUT_PORTS,

		DEVINFO_PTR_INTERNAL_MEMORY_MAP,				// R/O: address_map_constructor map
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_0 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 0,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_1 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 1,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_2 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 2,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_3 = DEVINFO_PTR_INTERNAL_MEMORY_MAP + 3,
		DEVINFO_PTR_INTERNAL_MEMORY_MAP_LAST = DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,

		DEVINFO_PTR_DEFAULT_MEMORY_MAP,					// R/O: address_map_constructor map
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
		DEVINFO_FCT_NVRAM,								// R/O: device_nvram_func
		DEVINFO_FCT_VALIDITY_CHECK,						// R/O: device_validity_check_func

	DEVINFO_FCT_CLASS_SPECIFIC = 0x24000,				// R/W: device-specific values start here
	DEVINFO_FCT_DEVICE_SPECIFIC = 0x28000,				// R/W: device-specific values start here
	DEVINFO_FCT_LAST = 0x2ffff,

	// --- the following bits of info are returned as NULL-terminated strings ---
	DEVINFO_STR_FIRST = 0x30000,

		DEVINFO_STR_NAME = DEVINFO_STR_FIRST,			// R/O: name of the device
		DEVINFO_STR_SHORTNAME,							// R/O: search path of device, used for media loading
		DEVINFO_STR_FAMILY,								// R/O: family of the device
		DEVINFO_STR_VERSION,							// R/O: version of the device
		DEVINFO_STR_SOURCE_FILE,						// R/O: file containing the device implementation
		DEVINFO_STR_CREDITS,							// R/O: credits for the device implementation

	DEVINFO_STR_CLASS_SPECIFIC = 0x34000,				// R/W: device-specific values start here
	DEVINFO_STR_DEVICE_SPECIFIC = 0x38000,				// R/W: device-specific values start here
	DEVINFO_STR_LAST = 0x3ffff
};

//**************************************************************************
//  MACROS
//**************************************************************************

// macro for declaring the configuration and device classes of a legacy device
#define _DECLARE_LEGACY_DEVICE(name, basename, deviceclass, basedeviceclass)	\
																				\
DEVICE_GET_INFO( basename );													\
																				\
class deviceclass : public basedeviceclass										\
{																				\
public:																			\
	deviceclass(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);	\
};																				\
																				\
extern const device_type name

// macro for defining the implementation needed for configuration and device classes
#define _DEFINE_LEGACY_DEVICE(name, basename, deviceclass, basedeviceclass) 	\
																				\
deviceclass::deviceclass(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)	\
	: basedeviceclass(mconfig, type, tag, owner, clock, DEVICE_GET_INFO_NAME(basename))	\
{																				\
}																				\
																				\
const device_type name = &legacy_device_creator<deviceclass>

// this template function creates a stub which constructs a device
template<class _DeviceClass>
device_t *legacy_device_creator(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
{
	return global_alloc(_DeviceClass(mconfig, &legacy_device_creator<_DeviceClass>, tag, owner, clock));
}


// reduced macros that are easier to use, and map to the above two macros
#define DECLARE_LEGACY_DEVICE(name, basename) _DECLARE_LEGACY_DEVICE(name, basename, basename##_device, legacy_device_base)
#define DECLARE_LEGACY_SOUND_DEVICE(name, basename) _DECLARE_LEGACY_DEVICE(name, basename, basename##_device, legacy_sound_device_base)

#define DEFINE_LEGACY_DEVICE(name, basename) _DEFINE_LEGACY_DEVICE(name, basename, basename##_device, legacy_device_base)
#define DEFINE_LEGACY_SOUND_DEVICE(name, basename) _DEFINE_LEGACY_DEVICE(name, basename, basename##_device, legacy_sound_device_base)


// macros to wrap legacy device functions
#define DEVICE_GET_INFO_NAME(name)	device_get_config_##name
#define DEVICE_GET_INFO(name)		void DEVICE_GET_INFO_NAME(name)(const device_t *device, UINT32 state, deviceinfo *info)
#define DEVICE_GET_INFO_CALL(name)	DEVICE_GET_INFO_NAME(name)(device, state, info)

#define DEVICE_VALIDITY_CHECK_NAME(name)	device_validity_check_##name
#define DEVICE_VALIDITY_CHECK(name)			int DEVICE_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const device_t *device, emu_options &options)
#define DEVICE_VALIDITY_CHECK_CALL(name)	DEVICE_VALIDITY_CHECK_NAME(name)(driver, device)

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			void DEVICE_START_NAME(name)(device_t *device)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(device_t *device)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(device)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(device_t *device)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(device)

#define DEVICE_EXECUTE_NAME(name)	device_execute_##name
#define DEVICE_EXECUTE(name)		INT32 DEVICE_EXECUTE_NAME(name)(device_t *device, INT32 clocks)
#define DEVICE_EXECUTE_CALL(name)	DEVICE_EXECUTE_NAME(name)(device, clocks)


//**************************************************************************
//  DEVICE_CONFIGURATION_MACROS
//**************************************************************************

#define structsizeof(_struct, _field) sizeof(((_struct *)NULL)->_field)

// inline device configurations that require 32 bits of storage in the token
#define MCFG_DEVICE_CONFIG_DATA32_EXPLICIT(_size, _offset, _val) \
	legacy_device_base::static_set_inline32(*device, _offset, _size, (UINT32)(_val));

#define MCFG_DEVICE_CONFIG_DATA32(_struct, _field, _val) \
	MCFG_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_struct, _field), offsetof(_struct, _field), _val)

#define MCFG_DEVICE_CONFIG_DATA32_ARRAY(_struct, _field, _index, _val) \
	MCFG_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_struct, _field[0]), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]), _val)

#define MCFG_DEVICE_CONFIG_DATA32_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) \
	MCFG_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_memstruct, _member), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]) + offsetof(_memstruct, _member), _val)


// inline device configurations that require 64 bits of storage in the token
#define MCFG_DEVICE_CONFIG_DATA64_EXPLICIT(_size, _offset, _val) \
	legacy_device_base::static_set_inline64(*device, _offset, _size, (UINT64)(_val));

#define MCFG_DEVICE_CONFIG_DATA64(_struct, _field, _val) \
	MCFG_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_struct, _field), offsetof(_struct, _field), _val)

#define MCFG_DEVICE_CONFIG_DATA64_ARRAY(_struct, _field, _index, _val) \
	MCFG_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_struct, _field[0]), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]), _val)

#define MCFG_DEVICE_CONFIG_DATA64_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) \
	MCFG_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_memstruct, _member), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]) + offsetof(_memstruct, _member), _val)


// inline device configurations that require a pointer-sized amount of storage in the token
#ifdef PTR64
#define MCFG_DEVICE_CONFIG_DATAPTR_EXPLICIT(_struct, _size, _offset) MCFG_DEVICE_CONFIG_DATA64_EXPLICIT(_struct, _size, _offset)
#define MCFG_DEVICE_CONFIG_DATAPTR(_struct, _field, _val) MCFG_DEVICE_CONFIG_DATA64(_struct, _field, _val)
#define MCFG_DEVICE_CONFIG_DATAPTR_ARRAY(_struct, _field, _index, _val) MCFG_DEVICE_CONFIG_DATA64_ARRAY(_struct, _field, _index, _val)
#define MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) MCFG_DEVICE_CONFIG_DATA64_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val)
#else
#define MCFG_DEVICE_CONFIG_DATAPTR_EXPLICIT(_struct, _size, _offset) MCFG_DEVICE_CONFIG_DATA32_EXPLICIT(_struct, _size, _offset)
#define MCFG_DEVICE_CONFIG_DATAPTR(_struct, _field, _val) MCFG_DEVICE_CONFIG_DATA32(_struct, _field, _val)
#define MCFG_DEVICE_CONFIG_DATAPTR_ARRAY(_struct, _field, _index, _val) MCFG_DEVICE_CONFIG_DATA32_ARRAY(_struct, _field, _index, _val)
#define MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) MCFG_DEVICE_CONFIG_DATA32_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val)
#endif



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

union deviceinfo;
class machine_config;
class device_t;

char *get_temp_string_buffer(void);
resource_pool &machine_get_pool(running_machine &machine);


// device interface function types
typedef void (*device_get_config_func)(const device_t *device, UINT32 state, deviceinfo *info);
typedef int (*device_validity_check_func)(const game_driver *driver, const device_t *device, emu_options &options);

typedef void (*device_start_func)(device_t *device);
typedef void (*device_stop_func)(device_t *device);
typedef INT32 (*device_execute_func)(device_t *device, INT32 clocks);
typedef void (*device_reset_func)(device_t *device);
typedef void (*device_nvram_func)(device_t *device, emu_file *file, int read_or_write);

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
	device_nvram_func		nvram;						// DEVINFO_FCT_NVRAM
	const rom_entry *		romregion;					// DEVINFO_PTR_ROM_REGION
	machine_config_constructor machine_config;			// DEVINFO_PTR_MACHINE_CONFIG
	ioport_constructor ipt;								// DEVINFO_PTR_INPUT_PORTS
	address_map_constructor	internal_map8;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	address_map_constructor	internal_map16;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	address_map_constructor	internal_map32;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	address_map_constructor	internal_map64;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	address_map_constructor	default_map8;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	address_map_constructor	default_map16;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	address_map_constructor	default_map32;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	address_map_constructor	default_map64;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
};


// ======================> legacy_device_base

// legacy_device_base serves as a common base class for legacy devices
class legacy_device_base : public device_t
{
protected:
	// construction/destruction
	legacy_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config);
	virtual ~legacy_device_base();

public:
	// access to legacy inline configuartion
	void *inline_config() const { return m_inline_config; }

	// inline configuration helpers
	static void static_set_inline32(device_t &device, UINT32 offset, UINT32 size, UINT32 value);
	static void static_set_inline64(device_t &device, UINT32 offset, UINT32 size, UINT64 value);
	static void static_set_inline_float(device_t &device, UINT32 offset, UINT32 size, float value);

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const { return reinterpret_cast<const rom_entry *>(get_legacy_ptr(DEVINFO_PTR_ROM_REGION)); }
	virtual machine_config_constructor device_mconfig_additions() const { return reinterpret_cast<machine_config_constructor>(get_legacy_ptr(DEVINFO_PTR_MACHINE_CONFIG)); }
	virtual ioport_constructor device_input_ports() const { return reinterpret_cast<ioport_constructor>(get_legacy_ptr(DEVINFO_PTR_INPUT_PORTS)); }
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

	// access to legacy configuration info
	INT64 get_legacy_int(UINT32 state) const;
	void *get_legacy_ptr(UINT32 state) const;
	genf *get_legacy_fct(UINT32 state) const;
	const char *get_legacy_string(UINT32 state) const;

	// configuration state
	device_get_config_func		m_get_config_func;
	void *						m_inline_config;

	// internal state
	void *						m_token;
};



// ======================> legacy_sound_device_base

// legacy_sound_device is a legacy_device_base with a sound interface
class legacy_sound_device_base :	public legacy_device_base,
									public device_sound_interface
{
protected:
	// construction/destruction
	legacy_sound_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};



#endif	/* __DEVLEGCY_H__ */
