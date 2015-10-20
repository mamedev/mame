// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    er2055.h

    GI 512 bit electrically alterable read-only memory.

***************************************************************************/

#pragma once

#ifndef __ER2055_H__
#define __ER2055_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ER2055_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ER2055, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> er2055_device

class er2055_device :   public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	er2055_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	UINT8 data() const { return m_data; }
	void set_address(UINT8 address) { m_address = address & 0x3f; }
	void set_data(UINT8 data) { m_data = data; }

	// control lines -- all lines are specified as active-high (even CS2)
	void set_control(UINT8 cs1, UINT8 cs2, UINT8 c1, UINT8 c2, UINT8 ck);

protected:
	// device-level overrides
	virtual void device_start();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	static const int SIZE_DATA = 0x40;

	static const UINT8 CK  = 0x01;
	static const UINT8 C1  = 0x02;
	static const UINT8 C2  = 0x04;
	static const UINT8 CS1 = 0x08;
	static const UINT8 CS2 = 0x10;

	// configuration state
	address_space_config        m_space_config;

	// internal state
	UINT8       m_control_state;
	UINT8       m_address;
	UINT8       m_data;
};


// device type definition
extern const device_type ER2055;


#endif
