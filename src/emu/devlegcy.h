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

// state constants passed to the device_get_config_func
enum
{
	// --- the following bits of info are returned as 64-bit signed integers ---
	DEVINFO_INT_FIRST = 0x00000,

		DEVINFO_INT_TOKEN_BYTES = DEVINFO_INT_FIRST,	// R/O: bytes to allocate for the token

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

// this template function creates a stub which constructs a device
template<class _DeviceClass>
device_t *legacy_device_creator(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
{
	return global_alloc(_DeviceClass(mconfig, &legacy_device_creator<_DeviceClass>, tag, owner, clock));
}

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			void DEVICE_START_NAME(name)(device_t *device)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(device_t *device)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(device)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(device_t *device)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(device)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_t;

char *get_temp_string_buffer(void);
typedef void (*device_start_func)(device_t *device);
typedef void (*device_stop_func)(device_t *device);
typedef void (*device_reset_func)(device_t *device);

#endif	/* __DEVLEGCY_H__ */
