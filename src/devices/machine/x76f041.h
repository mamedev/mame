// license:BSD-3-Clause
// copyright-holders:smf
/*
 * x76f041.h
 *
 * Secure SerialFlash
 *
 */

#pragma once

#ifndef __X76F041_H__
#define __X76F041_H__

#include "emu.h"

#define MCFG_X76F041_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, X76F041, 0 )

class x76f041_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	x76f041_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock );

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
	UINT8 *password();
	void password_ok();
	void load_address();
	int data_offset();

	enum configuration_register_t
	{
		CONFIG_BCR1 = 0,
		CONFIG_BCR2 = 1,
		CONFIG_CR = 2,
		CONFIG_RR = 3,
		CONFIG_RC = 4
	};

	enum bcr_t
	{
		BCR_X = 8,
		BCR_Y = 4,
		BCR_Z = 2,
		BCR_T = 1
	};

	enum command_t
	{
		COMMAND_WRITE = 0x00,
		COMMAND_READ = 0x20,
		COMMAND_WRITE_USE_CONFIGURATION_PASSWORD = 0x40,
		COMMAND_READ_USE_CONFIGURATION_PASSWORD = 0x60,
		COMMAND_CONFIGURATION = 0x80
	};

	enum configuration_t
	{
		CONFIGURATION_PROGRAM_WRITE_PASSWORD = 0x00,
		CONFIGURATION_PROGRAM_READ_PASSWORD = 0x10,
		CONFIGURATION_PROGRAM_CONFIGURATION_PASSWORD = 0x20,
		CONFIGURATION_RESET_WRITE_PASSWORD = 0x30,
		CONFIGURATION_RESET_READ_PASSWORD = 0x40,
		CONFIGURATION_PROGRAM_CONFIGURATION_REGISTERS = 0x50,
		CONFIGURATION_READ_CONFIGURATION_REGISTERS = 0x60,
		CONFIGURATION_MASS_PROGRAM = 0x70,
		CONFIGURATION_MASS_ERASE = 0x80
	};

	enum state_t
	{
		STATE_STOP,
		STATE_RESPONSE_TO_RESET,
		STATE_LOAD_COMMAND,
		STATE_LOAD_ADDRESS,
		STATE_LOAD_PASSWORD,
		STATE_VERIFY_PASSWORD,
		STATE_READ_DATA,
		STATE_WRITE_DATA,
		STATE_READ_CONFIGURATION_REGISTERS,
		STATE_WRITE_CONFIGURATION_REGISTERS
	};

	// internal state
	int m_cs;
	int m_rst;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_shift;
	int m_bit;
	int m_byte;
	int m_command;
	int m_address;
	UINT8 m_write_buffer[ 8 ];
	UINT8 m_response_to_reset[ 4 ];
	UINT8 m_write_password[ 8 ];
	UINT8 m_read_password[ 8 ];
	UINT8 m_configuration_password[ 8 ];
	UINT8 m_configuration_registers[ 8 ];
	UINT8 m_data[ 512 ];
};


// device type definition
extern const device_type X76F041;

#endif
