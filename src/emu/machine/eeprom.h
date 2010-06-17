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

#define MDRV_EEPROM_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, EEPROM, 0) \
	MDRV_DEVICE_INLINE_DATAPTR(eeprom_device_config::INLINE_INTERFACE, &_interface)

#define MDRV_EEPROM_93C46_ADD(_tag) \
	MDRV_EEPROM_ADD(_tag, eeprom_interface_93C46)

#define MDRV_EEPROM_93C66B_ADD(_tag) \
	MDRV_EEPROM_ADD(_tag, eeprom_interface_93C66B)

#define MDRV_EEPROM_DATA(_data, _size) \
	MDRV_DEVICE_INLINE_DATAPTR(eeprom_device_config::INLINE_DATAPTR, &_data) \
	MDRV_DEVICE_INLINE_DATA16(eeprom_device_config::INLINE_DATASIZE, _size)

#define MDRV_EEPROM_DEFAULT_VALUE(_value) \
	MDRV_DEVICE_INLINE_DATA32(eeprom_device_config::INLINE_DEFVALUE, 0x10000 | ((_value) & 0xffff))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> eeprom_interface

struct eeprom_interface
{
	UINT8		m_address_bits;			// EEPROM has 2^address_bits cells
	UINT8		m_data_bits;			// every cell has this many bits (8 or 16)
	const char *m_cmd_read;				//   read command string, e.g. "0110"
	const char *m_cmd_write;			//  write command string, e.g. "0111"
	const char *m_cmd_erase;			//  erase command string, or 0 if n/a
	const char *m_cmd_lock;				//   lock command string, or 0 if n/a
	const char *m_cmd_unlock;			// unlock command string, or 0 if n/a
	bool		m_enable_multi_read;	// set to 1 to enable multiple values to be read from one read command
	int			m_reset_delay;			// number of times eeprom_read_bit() should return 0 after a reset,
										// before starting to return 1.
};



// ======================> eeprom_device_config

class eeprom_device_config :	public device_config,
								public device_config_memory_interface,
								public device_config_nvram_interface,
								public eeprom_interface
{
	friend class eeprom_device;

	// construction/destruction
	eeprom_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// basic information getters
	virtual const char *name() const { return "EEPROM"; }

	// inline configuration indexes
	enum
	{
		INLINE_INTERFACE,
		INLINE_DATAPTR,
		INLINE_DATASIZE,
		INLINE_DEFVALUE
	};

protected:
	// device_config overrides
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// device-specific configuration
	const UINT8 *				m_default_data;
	int 						m_default_data_size;
	UINT32						m_default_value;
	address_space_config		m_space_config;
};


// ======================> eeprom_device

class eeprom_device :	public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
	friend class eeprom_device_config;

	// construction/destruction
	eeprom_device(running_machine &_machine, const eeprom_device_config &config);

public:
	// I/O operations
	void write_bit(int state);
	int read_bit();
	void set_cs_line(int state);
	void set_clock_line(int state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	// internal helpers
	void write(int bit);
	bool command_match(const char *buf, const char *cmd, int len);

	static const int SERIAL_BUFFER_LENGTH = 40;

	// internal state
	const eeprom_device_config &m_config;

	int 		m_serial_count;
	UINT8		m_serial_buffer[SERIAL_BUFFER_LENGTH];
	int 		m_data_bits;
	int 		m_read_address;
	int 		m_clock_count;
	int 		m_latch;
	int			m_reset_line;
	int			m_clock_line;
	int			m_sending;
	int 		m_locked;
	int 		m_reset_delay;
};


// device type definition
const device_type EEPROM = eeprom_device_config::static_alloc_device_config;




//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const eeprom_interface eeprom_interface_93C46;
extern const eeprom_interface eeprom_interface_93C66B;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( eeprom_write_bit );
READ_LINE_DEVICE_HANDLER( eeprom_read_bit );
WRITE_LINE_DEVICE_HANDLER( eeprom_set_cs_line );
WRITE_LINE_DEVICE_HANDLER( eeprom_set_clock_line );

#endif
