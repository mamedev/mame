// license:BSD-3-Clause
// copyright-holders:smf,Carl
/*
 * ds1205.h
 *
 * MultiKey
 *
 */

#ifndef MAME_MACHINE_DS1205_H
#define MAME_MACHINE_DS1205_H

#pragma once


class ds1205_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	ds1205_device( const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	DECLARE_WRITE_LINE_MEMBER( write_rst );
	DECLARE_WRITE_LINE_MEMBER( write_clk );
	DECLARE_WRITE_LINE_MEMBER( write_dq );
	DECLARE_READ_LINE_MEMBER( read_dq );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read( util::read_stream &file ) override;
	virtual bool nvram_write( util::write_stream &file ) override;

private:
	inline void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, const char *s_fmt, ... );
	void new_state(int state);
	void writebit(u8 *buffer);
	void readbit(u8 *buffer);

	enum state_t
	{
		STATE_STOP,
		STATE_PROTOCOL,
		STATE_READ_IDENTIFICATION,
		STATE_WRITE_IDENTIFICATION,
		STATE_WRITE_COMPARE_REGISTER,
		STATE_WRITE_SECURITY_MATCH,
		STATE_READ_DATA,
		STATE_WRITE_DATA,
		STATE_READ_SCRATCH,
		STATE_WRITE_SCRATCH,
		STATE_OUTPUT_GARBLED_DATA
	};

	enum command_t
	{
		COMMAND_SET_SCRATCHPAD = 0x96,
		COMMAND_GET_SCRATCHPAD = 0x69,
		COMMAND_SET_DATA = 0x99,
		COMMAND_GET_DATA = 0x66,
		COMMAND_SET_SECURITY = 0x5a,
		COMMAND_MOVE_BLOCK = 0x3c
	};

	enum cycle_t
	{
		CYCLE_NORMAL = 1,
		CYCLE_PROGRAM = 2,
		CYCLE_MASK = 3
	};

	static const int DQ_HIGH_IMPEDANCE = -1;

	optional_memory_region m_region;

	int m_rst;
	int m_clk;
	int m_dqw;
	int m_dqr;
	int m_state;
	int m_bit;
	u8 m_command[3];
	u8 m_compare_register[8];
	u8 m_scratchpad[64];
	u8 m_identification[3][8];
	u8 m_security_match[3][8];
	u8 m_secure_memory[3][48];
};


// device type definition
DECLARE_DEVICE_TYPE(DS1205, ds1205_device)

#endif // MAME_MACHINE_DS1205_H
