// license:BSD-3-Clause
// copyright-holders:smf
/*
 * zs01.h
 *
 * Secure SerialFlash
 *
 */

#pragma once

#ifndef __ZS01_H__
#define __ZS01_H__

#include "machine/ds2401.h"

#define MCFG_ZS01_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, ZS01, 0 )
#define MCFG_ZS01_DS2401( ds2401_tag ) \
	zs01_device::static_set_ds2401_tag( *device, ds2401_tag );

class zs01_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	zs01_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock );

	// inline configuration helpers
	static void static_set_ds2401_tag( device_t &device, const char *ds2401_tag ) { downcast<zs01_device &>( device ).m_ds2401_tag = ds2401_tag; }

	DECLARE_WRITE_LINE_MEMBER( write_cs );
	DECLARE_WRITE_LINE_MEMBER( write_rst );
	DECLARE_WRITE_LINE_MEMBER( write_scl );
	DECLARE_WRITE_LINE_MEMBER( write_sda );
	DECLARE_READ_LINE_MEMBER( read_sda );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read( emu_file &file ) override;
	virtual void nvram_write( emu_file &file ) override;

private:
	inline void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, const char *s_fmt, ... );

	void decrypt( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte );
	void decrypt2( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte );
	void encrypt( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT32 previous_byte );
	UINT16 calc_crc( UINT8 *buffer, UINT32 length );
	int data_offset();

	enum size_t
	{
		SIZE_DATA_BUFFER = 8
	};

	enum command_t
	{
		COMMAND_WRITE = 0x00,
		COMMAND_READ = 0x01
	};

	enum state_t
	{
		STATE_STOP,
		STATE_RESPONSE_TO_RESET,
		STATE_LOAD_COMMAND,
		STATE_READ_DATA
	};

	// internal state
	const char *m_ds2401_tag;

	int m_cs;
	int m_rst;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_shift;
	int m_bit;
	int m_byte;
	UINT8 m_write_buffer[ 12 ];
	UINT8 m_read_buffer[ 12 ];
	UINT8 m_response_key[ 8 ];
	UINT8 m_response_to_reset[ 4 ];
	UINT8 m_command_key[ 8 ];
	UINT8 m_data_key[ 8 ];
	UINT8 m_data[ 4096 ];
	ds2401_device *m_ds2401;
};


// device type definition
extern const device_type ZS01;

#endif
