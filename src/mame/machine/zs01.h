// license:BSD-3-Clause
// copyright-holders:smf
/*
 * zs01.h
 *
 * Secure SerialFlash
 *
 */
#ifndef MAME_MACHINE_ZS01_H
#define MAME_MACHINE_ZS01_H

#pragma once


#include "machine/ds2401.h"

class zs01_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	zs01_device( const machine_config &mconfig, const char *tag, device_t *owner)
		: zs01_device(mconfig, tag, owner, uint32_t(0))
	{
	}

	zs01_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock );

	// inline configuration helpers
	void set_ds2401_tag( const char *ds2401_tag ) { m_ds2401_tag = ds2401_tag; }

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

	void decrypt( uint8_t *destination, uint8_t *source, int length, uint8_t *key, uint8_t previous_byte );
	void decrypt2( uint8_t *destination, uint8_t *source, int length, uint8_t *key, uint8_t previous_byte );
	void encrypt( uint8_t *destination, uint8_t *source, int length, uint8_t *key, uint32_t previous_byte );
	uint16_t calc_crc( uint8_t *buffer, uint32_t length );
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
	optional_memory_region m_region;

	int m_cs;
	int m_rst;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_shift;
	int m_bit;
	int m_byte;
	uint8_t m_write_buffer[ 12 ];
	uint8_t m_read_buffer[ 12 ];
	uint8_t m_response_key[ 8 ];
	uint8_t m_response_to_reset[ 4 ];
	uint8_t m_command_key[ 8 ];
	uint8_t m_data_key[ 8 ];
	uint8_t m_data[ 4096 ];
	ds2401_device *m_ds2401;
};


// device type definition
DECLARE_DEVICE_TYPE(ZS01, zs01_device)

#endif // MAME_MACHINE_ZS01_H
