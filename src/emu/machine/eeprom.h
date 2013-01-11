/***************************************************************************

    eeprom.h

    Serial eeproms.

***************************************************************************/

#pragma once

#ifndef __EEPROMDEV_H__
#define __EEPROMDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EEPROM_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, EEPROM, 0) \
	eeprom_device::static_set_interface(*device, _interface); \

#define MCFG_EEPROM_93C46_ADD(_tag) \
	MCFG_EEPROM_ADD(_tag, eeprom_interface_93C46)

#define MCFG_EEPROM_93C46_8BIT_ADD(_tag) \
	MCFG_EEPROM_ADD(_tag, eeprom_interface_93C46_8bit)

#define MCFG_EEPROM_93C66B_ADD(_tag) \
	MCFG_EEPROM_ADD(_tag, eeprom_interface_93C66B)

#define MCFG_EEPROM_DATA(_data, _size) \
	eeprom_device::static_set_default_data(*device, _data, _size); \

#define MCFG_EEPROM_DEFAULT_VALUE(_value) \
	eeprom_device::static_set_default_value(*device, _value); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> eeprom_interface

struct eeprom_interface
{
	UINT8       m_address_bits;         // EEPROM has 2^address_bits cells
	UINT8       m_data_bits;            // every cell has this many bits (8 or 16)
	const char *m_cmd_read;             //   read command string, e.g. "0110"
	const char *m_cmd_write;            //  write command string, e.g. "0111"
	const char *m_cmd_erase;            //  erase command string, or 0 if n/a
	const char *m_cmd_lock;             //   lock command string, or 0 if n/a
	const char *m_cmd_unlock;           // unlock command string, or 0 if n/a
	bool        m_enable_multi_read;    // set to 1 to enable multiple values to be read from one read command
	int         m_reset_delay;          // number of times eeprom_read_bit() should return 0 after a reset,
										// before starting to return 1.
};



// ======================> eeprom_device

class eeprom_device :   public device_t,
						public device_memory_interface,
						public device_nvram_interface,
						public eeprom_interface
{
public:
	// construction/destruction
	eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_interface(device_t &device, const eeprom_interface &interface);
	static void static_set_default_data(device_t &device, const UINT8 *data, UINT32 size);
	static void static_set_default_data(device_t &device, const UINT16 *data, UINT32 size);
	static void static_set_default_value(device_t &device, UINT16 value);

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( write_bit );
	DECLARE_READ_LINE_MEMBER( read_bit );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );
	DECLARE_WRITE_LINE_MEMBER( set_clock_line );

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

	// internal helpers
	void write(int bit);
	bool command_match(const char *buf, const char *cmd, int len);

	static const int SERIAL_BUFFER_LENGTH = 40;

	// configuration state
	address_space_config    m_space_config;
	generic_ptr             m_default_data;
	int                     m_default_data_size;
	UINT32                  m_default_value;

	// runtime state
	int                     m_serial_count;
	UINT8                   m_serial_buffer[SERIAL_BUFFER_LENGTH];
	int                     m_data_buffer;
	int                     m_read_address;
	int                     m_clock_count;
	int                     m_latch;
	int                     m_reset_line;
	int                     m_clock_line;
	int                     m_sending;
	int                     m_locked;
	int                     m_reset_counter;
};


// device type definition
extern const device_type EEPROM;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const eeprom_interface eeprom_interface_93C46;
extern const eeprom_interface eeprom_interface_93C46_8bit;
extern const eeprom_interface eeprom_interface_93C66B;


#endif
