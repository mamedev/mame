/***************************************************************************

    eeprom.h

    Base class for EEPROM devices.

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

#ifndef __EEPROM_H__
#define __EEPROM_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EEPROM_SIZE(_cells, _cellbits) \
	eeprom_base_device::static_set_size(*device, _cells, _cellbits);
#define MCFG_EEPROM_DATA(_data, _size) \
	eeprom_base_device::static_set_default_data(*device, _data, _size);
#define MCFG_EEPROM_DEFAULT_VALUE(_value) \
	eeprom_base_device::static_set_default_value(*device, _value);

#define MCFG_EEPROM_WRITE_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::WRITE_TIME, _value);
#define MCFG_EEPROM_WRITE_ALL_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::WRITE_ALL_TIME, _value);
#define MCFG_EEPROM_ERASE_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::ERASE_TIME, _value);
#define MCFG_EEPROM_ERASE_ALL_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::ERASE_ALL_TIME, _value);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> eeprom_base_device

class eeprom_base_device : 	public device_t,
							public device_memory_interface,
							public device_nvram_interface
{
protected:
	// construction/destruction
	eeprom_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, const char *shortname, const char *file);

public:
	// timing constants
	enum timing_type
	{
		WRITE_TIME,			// default = 2ms
		WRITE_ALL_TIME,		// default = 8ms
		ERASE_TIME,			// default = 1ms
		ERASE_ALL_TIME,		// default = 8ms
		TIMING_COUNT
	};

	// inline configuration helpers
	static void static_set_size(device_t &device, int cells, int cellbits);
	static void static_set_default_data(device_t &device, const UINT8 *data, UINT32 size);
	static void static_set_default_data(device_t &device, const UINT16 *data, UINT32 size);
	static void static_set_default_value(device_t &device, UINT32 value);
	static void static_set_timing(device_t &device, timing_type type, attotime duration);

	// read/write/erase data
	UINT32 read(offs_t address);
	void write(offs_t address, UINT32 data);
	void write_all(UINT32 data);
	void erase(offs_t address);
	void erase_all();
	
	// status
	bool ready() const { return machine().time() >= m_completion_time; }
	
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	// internal read/write without side-effects
	UINT32 internal_read(offs_t address);
	void internal_write(offs_t address, UINT32 data);

	// configuration state
	UINT32					m_cells;
	UINT8					m_address_bits;
	UINT8					m_data_bits;
	address_space_config    m_space_config;
	generic_ptr             m_default_data;
	UINT32                  m_default_data_size;
	UINT32                  m_default_value;
	bool                    m_default_value_set;
	attotime				m_operation_time[TIMING_COUNT];

	// live state
	attotime				m_completion_time;
};


#endif
