/***************************************************************************

    ATMEL AT28C16

    16K ( 2K x 8 ) Parallel EEPROM

***************************************************************************/

#pragma once

#ifndef __AT28C16_H__
#define __AT28C16_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AT28C16_ADD( _tag, _interface ) \
	MCFG_DEVICE_ADD( _tag, AT28C16, 0 )


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> at28c16_interface

struct at28c16_interface
{
};


// ======================> at28c16_device

class at28c16_device :
	public device_t,
	public device_memory_interface,
	public device_nvram_interface,
	public at28c16_interface
{
public:
	// construction/destruction
	at28c16_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock );

	// I/O operations
	void write( offs_t offset, UINT8 data );
	UINT8 read( offs_t offset );
	void set_a9_12v( int state );
	void set_oe_12v( int state );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config( address_spacenum spacenum = AS_0 ) const;

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read( emu_file &file );
	virtual void nvram_write( emu_file &file );

	// internal helpers
	static TIMER_CALLBACK( write_finished );

	// internal state
	address_space_config m_space_config;
	emu_timer *m_write_timer;
	int m_a9_12v;
	int m_oe_12v;
	int m_last_write;
};


// device type definition
extern const device_type AT28C16;


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

DECLARE_WRITE8_DEVICE_HANDLER( at28c16_w );
DECLARE_READ8_DEVICE_HANDLER( at28c16_r );
WRITE_LINE_DEVICE_HANDLER( at28c16_a9_12v );
WRITE_LINE_DEVICE_HANDLER( at28c16_oe_12v );

#endif
