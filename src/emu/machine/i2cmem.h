/***************************************************************************

    i2cmem.h

    I2C Memory

***************************************************************************/

#pragma once

#ifndef __I2CMEM_H__
#define __I2CMEM_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define I2CMEM_SLAVE_ADDRESS ( 0xa0 )
#define I2CMEM_SLAVE_ADDRESS_ALT ( 0xb0 )


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_I2CMEM_ADD( _tag, _interface ) \
	MDRV_DEVICE_ADD( _tag, I2CMEM, 0 ) \
	i2cmem_device_config::static_set_interface(device, _interface);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i2cmem_interface

struct i2cmem_interface
{
	int m_slave_address;
	int m_page_size;
	int m_data_size;
};


// ======================> i2cmem_device_config

class i2cmem_device_config :
	public device_config,
	public device_config_memory_interface,
	public device_config_nvram_interface,
	public i2cmem_interface
{
	friend class i2cmem_device;

	// construction/destruction
	i2cmem_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock );

public:
	// allocators
	static device_config *static_alloc_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock );
	virtual device_t *alloc_device( running_machine &machine ) const;

	// inline configuration
	static void static_set_interface(device_config *device, const i2cmem_interface &interface);

protected:
	// device_config overrides
	virtual void device_config_complete();
	virtual bool device_validity_check( const game_driver &driver ) const;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config( int spacenum = 0 ) const;

	// device-specific configuration
	address_space_config m_space_config;
	int m_address_bits;
};


// ======================> i2cmem_device

class i2cmem_device :
	public device_t,
	public device_memory_interface,
	public device_nvram_interface
{
	friend class i2cmem_device_config;

	// construction/destruction
	i2cmem_device( running_machine &_machine, const i2cmem_device_config &config );

public:
	// I/O operations
	void set_e0_line( int state );
	void set_e1_line( int state );
	void set_e2_line( int state );
	void set_sda_line( int state );
	void set_scl_line( int state );
	void set_wc_line( int state );
	int read_sda_line();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read( mame_file &file );
	virtual void nvram_write( mame_file &file );

	// internal helpers
	int address_mask();
	int select_device();
	int data_offset();

	// internal state
	const i2cmem_device_config &m_config;

	int m_scl;
	int m_sdaw;
	int m_e0;
	int m_e1;
	int m_e2;
	int m_wc;
	int m_sdar;
	int m_state;
	int m_bits;
	int m_shift;
	int m_devsel;
	int m_byteaddr;
	UINT8 *m_page;
	int m_page_offset;
};


// device type definition
extern const device_type I2CMEM;


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( i2cmem_e0_write );
WRITE_LINE_DEVICE_HANDLER( i2cmem_e1_write );
WRITE_LINE_DEVICE_HANDLER( i2cmem_e2_write );
WRITE_LINE_DEVICE_HANDLER( i2cmem_sda_write );
WRITE_LINE_DEVICE_HANDLER( i2cmem_scl_write );
WRITE_LINE_DEVICE_HANDLER( i2cmem_wc_write );
READ_LINE_DEVICE_HANDLER( i2cmem_sda_read );

#endif	/* __I2CMEM_H__ */
