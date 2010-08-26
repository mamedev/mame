/***************************************************************************

    dimemory.h

    Device memory interfaces.

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

#ifndef __DIMEMORY_H__
#define __DIMEMORY_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// the configuration for a general device
enum device_space
{
	AS_PROGRAM = 0,
	AS_DATA = 1,
	AS_IO = 2
};


// Translation intentions
const int TRANSLATE_TYPE_MASK		= 0x03;		// read write or fetch
const int TRANSLATE_USER_MASK		= 0x04;		// user mode or fully privileged
const int TRANSLATE_DEBUG_MASK		= 0x08;		// debug mode (no side effects)

const int TRANSLATE_READ			= 0;		// translate for read
const int TRANSLATE_WRITE			= 1;		// translate for write
const int TRANSLATE_FETCH			= 2;		// translate for instruction fetch
const int TRANSLATE_READ_USER		= (TRANSLATE_READ | TRANSLATE_USER_MASK);
const int TRANSLATE_WRITE_USER		= (TRANSLATE_WRITE | TRANSLATE_USER_MASK);
const int TRANSLATE_FETCH_USER		= (TRANSLATE_FETCH | TRANSLATE_USER_MASK);
const int TRANSLATE_READ_DEBUG		= (TRANSLATE_READ | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_WRITE_DEBUG		= (TRANSLATE_WRITE | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_FETCH_DEBUG		= (TRANSLATE_FETCH | TRANSLATE_DEBUG_MASK);



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_DEVICE_ADDRESS_MAP(_space, _map) \
	device_config_memory_interface::static_set_addrmap(device, _space, ADDRESS_MAP_NAME(_map));

#define MDRV_DEVICE_PROGRAM_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(AS_PROGRAM, _map)

#define MDRV_DEVICE_DATA_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(AS_DATA, _map)

#define MDRV_DEVICE_IO_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(AS_IO, _map)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_config_memory_interface

// class representing interface-specific configuration state
class device_config_memory_interface : public device_config_interface
{
	friend class device_memory_interface;

public:
	// construction/destruction
	device_config_memory_interface(const machine_config &mconfig, device_config &devconfig);
	virtual ~device_config_memory_interface();

	// basic information getters
	address_map_constructor address_map(int spacenum = 0) const { return (spacenum < ARRAY_LENGTH(m_address_map)) ? m_address_map[spacenum] : NULL; }
	const address_space_config *space_config(int spacenum = 0) const { return memory_space_config(spacenum); }

	// static inline helpers
	static void static_set_addrmap(device_config *device, int spacenum, address_map_constructor map);

protected:
	// required overrides
	virtual const address_space_config *memory_space_config(int spacenum) const = 0;

	// optional operation overrides
	virtual bool interface_validity_check(const game_driver &driver) const;

	address_map_constructor	m_address_map[ADDRESS_SPACES]; // address maps for each address space
};



// ======================> device_memory_interface

class device_memory_interface : public device_interface
{
	friend class device_scheduler;

public:
	// construction/destruction
	device_memory_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_memory_interface();

	// configuration access
	const device_config_memory_interface &memory_config() const { return m_memory_config; }

	// basic information getters
	const address_space_config *space_config(int spacenum = 0) const { return m_memory_config.space_config(spacenum); }
	address_space *space(int index = 0) const { return m_addrspace[index]; }
	address_space *space(device_space index) const { return m_addrspace[static_cast<int>(index)]; }

	// address space accessors
	void set_address_space(int spacenum, address_space &space);

	// address translation
	bool translate(int spacenum, int intention, offs_t &address) { return memory_translate(spacenum, intention, address); }

	// read/write access
	bool read(int spacenum, offs_t offset, int size, UINT64 &value) { return memory_read(spacenum, offset, size, value); }
	bool write(int spacenum, offs_t offset, int size, UINT64 value) { return memory_write(spacenum, offset, size, value); }
	bool readop(offs_t offset, int size, UINT64 &value) { return memory_readop(offset, size, value); }

protected:
	// optional operation overrides
	virtual bool memory_translate(int spacenum, int intention, offs_t &address);
	virtual bool memory_read(int spacenum, offs_t offset, int size, UINT64 &value);
	virtual bool memory_write(int spacenum, offs_t offset, int size, UINT64 value);
	virtual bool memory_readop(offs_t offset, int size, UINT64 &value);

	// interface-level overrides

	// configuration
	const device_config_memory_interface &m_memory_config;	// reference to our device_config_execute_interface
	address_space *		m_addrspace[ADDRESS_SPACES]; // reported address spaces
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  device_memory - return a pointer to the device
//  memory interface for this device
//-------------------------------------------------

inline device_memory_interface *device_memory(device_t *device)
{
	device_memory_interface *intf;
	if (!device->interface(intf))
		throw emu_fatalerror("Device '%s' does not have memory interface", device->tag());
	return intf;
}


//-------------------------------------------------
//  device_get_space - return a pointer to the
//  given address space on this device
//-------------------------------------------------

inline address_space *device_get_space(device_t *device, int spacenum = 0)
{
	return device_memory(device)->space(spacenum);
}


//-------------------------------------------------
//  devconfig_get_space_config - return a pointer
//  to sthe given address space's configuration
//-------------------------------------------------

inline const address_space_config *devconfig_get_space_config(const device_config &devconfig, int spacenum = 0)
{
	const device_config_memory_interface *intf;
	if (!devconfig.interface(intf))
		throw emu_fatalerror("Device '%s' does not have memory interface", devconfig.tag());
	return intf->space_config(spacenum);
}


#endif	/* __DIMEMORY_H__ */
