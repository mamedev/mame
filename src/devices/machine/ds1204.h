// license:BSD-3-Clause
// copyright-holders:smf
/*
 * ds1204.h
 *
 * Electronic Key
 *
 */

#pragma once

#ifndef __DS1204_H__
#define __DS1204_H__

#include "emu.h"

#define MCFG_DS1204_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, DS1204, 0 )

class ds1204_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	ds1204_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock );

	DECLARE_WRITE_LINE_MEMBER( write_rst );
	DECLARE_WRITE_LINE_MEMBER( write_clk );
	DECLARE_WRITE_LINE_MEMBER( write_dq );
	DECLARE_READ_LINE_MEMBER( read_dq );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read( emu_file &file ) override;
	virtual void nvram_write( emu_file &file ) override;

private:
	inline void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, const char *s_fmt, ... );
	void new_state(int state);
	void writebit(UINT8 *buffer);
	void readbit(UINT8 *buffer);

	enum state_t
	{
		STATE_STOP,
		STATE_PROTOCOL,
		STATE_READ_IDENTIFICATION,
		STATE_WRITE_IDENTIFICATION,
		STATE_WRITE_COMPARE_REGISTER,
		STATE_WRITE_SECURITY_MATCH,
		STATE_READ_SECURE_MEMORY,
		STATE_WRITE_SECURE_MEMORY,
		STATE_OUTPUT_GARBLED_DATA
	};

	enum command_t
	{
		COMMAND_READ = 0x62,
		COMMAND_WRITE = 0x9d
	};

	enum cycle_t
	{
		CYCLE_NORMAL = 1,
		CYCLE_PROGRAM = 2,
		CYCLE_MASK = 3
	};

	static const int DQ_HIGH_IMPEDANCE = -1;

	int m_rst;
	int m_clk;
	int m_dqw;
	int m_dqr;
	int m_state;
	int m_bit;
	UINT8 m_command[3];
	UINT8 m_compare_register[8];
	UINT8 m_unique_pattern[2];
	UINT8 m_identification[8];
	UINT8 m_security_match[8];
	UINT8 m_secure_memory[16];
};


// device type definition
extern const device_type DS1204;

#endif
