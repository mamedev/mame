/***************************************************************************

    er2055.h

    GI 512 bit electrically alterable read-only memory.

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

#ifndef __ER2055_H__
#define __ER2055_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ER2055_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ER2055, 0) \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> er2055_device_config

class er2055_device_config :	public device_config,
								public device_config_memory_interface,
								public device_config_nvram_interface
{
	friend class er2055_device;

	// construction/destruction
	er2055_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// device-specific configuration
	address_space_config		m_space_config;
};


// ======================> er2055_device

class er2055_device :	public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
	friend class er2055_device_config;

	// construction/destruction
	er2055_device(running_machine &_machine, const er2055_device_config &config);

public:
	// I/O operations
	UINT8 data() const { return m_data; }
	void set_address(UINT8 address) { m_address = address & 0x3f; }
	void set_data(UINT8 data) { m_data = data; }

	// control lines -- all lines are specified as active-high (even CS2)
	void set_control(UINT8 cs1, UINT8 cs2, UINT8 c1, UINT8 c2, UINT8 ck);

protected:
	// device-level overrides
	virtual void device_start();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	static const int SIZE_DATA = 0x40;

	static const UINT8 CK  = 0x01;
	static const UINT8 C1  = 0x02;
	static const UINT8 C2  = 0x04;
	static const UINT8 CS1 = 0x08;
	static const UINT8 CS2 = 0x10;

	// internal state
	const er2055_device_config &m_config;

	UINT8		m_control_state;
	UINT8		m_address;
	UINT8		m_data;
};


// device type definition
extern const device_type ER2055;


#endif
