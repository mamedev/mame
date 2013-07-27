/***************************************************************************

    eepromser.h

    Serial EEPROM devices.

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

#ifndef __EEPROMSER_H__
#define __EEPROMSER_H__

#include "eeprom.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SERIAL_EEPROM_ADD(_tag, _cells, _cellbits, _interface) \
	MCFG_DEVICE_ADD(_tag, SERIAL_EEPROM, 0) \
	MCFG_EEPROM_SIZE(_cells, _cellbits) \
	serial_eeprom_device::static_set_interface(*device, _interface);

#define MCFG_SERIAL_EEPROM_DATA(_data, _size) \
	serial_eeprom_device::static_set_default_data(*device, _data, _size);
#define MCFG_SERIAL_EEPROM_DEFAULT_VALUE(_value) \
	serial_eeprom_device::static_set_default_value(*device, _value);

#define MCFG_EEPROM_93C46_ADD(_tag) \
	MCFG_SERIAL_EEPROM_ADD(_tag, 64, 16, eeprom_interface_93C46_93C66B)

#define MCFG_EEPROM_93C46_8BIT_ADD(_tag) \
	MCFG_SERIAL_EEPROM_ADD(_tag, 128, 8, eeprom_interface_93C46_93C66B)

#define MCFG_EEPROM_93C66B_ADD(_tag) \
	MCFG_SERIAL_EEPROM_ADD(_tag, 256, 16, eeprom_interface_93C46_93C66B)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> serial_eeprom_interface

struct serial_eeprom_interface
{
	const char *m_cmd_read;             //   read command string, e.g. "0110"
	const char *m_cmd_write;            //  write command string, e.g. "0111"
	const char *m_cmd_erase;            //  erase command string, or 0 if n/a
	const char *m_cmd_lock;             //   lock command string, or 0 if n/a
	const char *m_cmd_unlock;           // unlock command string, or 0 if n/a
	bool        m_enable_multi_read;    // set to 1 to enable multiple values to be read from one read command
	int         m_reset_delay;          // number of times eeprom_read_bit() should return 0 after a reset,
										// before starting to return 1.
};



// ======================> serial_eeprom_device

class serial_eeprom_device :   	public base_eeprom_device,
								public serial_eeprom_interface
{
public:
	// construction/destruction
	serial_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_interface(device_t &device, const serial_eeprom_interface &interface);

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( write_bit );
	DECLARE_READ_LINE_MEMBER( read_bit );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );
	DECLARE_WRITE_LINE_MEMBER( set_clock_line );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal helpers
	UINT32 decode_value(int numbits, int bitsfromend = 0);
	void fill_data_buffer(offs_t address);
	void write(int bit);
	bool command_match(const char *cmd, int ignorebits = 0);

	static const int SERIAL_BUFFER_LENGTH = 40;

	// runtime state
	int           m_serial_count;
	char          m_serial_buffer[SERIAL_BUFFER_LENGTH];
	int           m_data_buffer;
	int           m_read_address;
	int           m_clock_count;
	int           m_latch;
	int           m_reset_line;
	int           m_clock_line;
	bool          m_sending;
	bool          m_locked;
	int           m_reset_counter;
};


// device type definition
extern const device_type SERIAL_EEPROM;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const serial_eeprom_interface eeprom_interface_93C46_93C66B;


#endif
