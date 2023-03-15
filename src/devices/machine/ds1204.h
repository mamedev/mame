// license:BSD-3-Clause
// copyright-holders:smf
/*
 * ds1204.h
 *
 * Electronic Key
 *
 */

#ifndef MAME_MACHINE_DS1204_H
#define MAME_MACHINE_DS1204_H

#pragma once


class ds1204_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	ds1204_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0 );

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
	void writebit(uint8_t *buffer);
	void readbit(uint8_t *buffer);

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

	optional_memory_region m_region;

	int m_rst;
	int m_clk;
	int m_dqw;
	int m_dqr;
	int m_state;
	int m_bit;
	uint8_t m_command[3];
	uint8_t m_compare_register[8];
	uint8_t m_unique_pattern[2];
	uint8_t m_identification[8];
	uint8_t m_security_match[8];
	uint8_t m_secure_memory[16];
};


// device type definition
DECLARE_DEVICE_TYPE(DS1204, ds1204_device)

#endif // MAME_MACHINE_DS1204_H
