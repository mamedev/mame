// license:BSD-3-Clause
// copyright-holders:smf
/*
 * x76f100.h
 *
 * Secure SerialFlash
 *
 */

#ifndef MAME_MACHINE_X76F100_H
#define MAME_MACHINE_X76F100_H

#pragma once


class x76f100_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	x76f100_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_WRITE_LINE_MEMBER( write_cs );
	DECLARE_WRITE_LINE_MEMBER( write_rst );
	DECLARE_WRITE_LINE_MEMBER( write_scl );
	DECLARE_WRITE_LINE_MEMBER( write_sda );
	DECLARE_READ_LINE_MEMBER( read_sda );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read( util::read_stream &file ) override;
	virtual bool nvram_write( util::write_stream &file ) override;

private:
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);

	uint8_t *password();
	void password_ok();
	int data_offset();

	enum command_t
	{
		COMMAND_WRITE = 0x80,
		COMMAND_READ = 0x81,
		COMMAND_CHANGE_WRITE_PASSWORD = 0xfc,
		COMMAND_CHANGE_READ_PASSWORD = 0xfe,
		COMMAND_ACK_PASSWORD = 0x55
	};

	enum state_t
	{
		STATE_STOP,
		STATE_RESPONSE_TO_RESET,
		STATE_LOAD_COMMAND,
		STATE_LOAD_PASSWORD,
		STATE_VERIFY_PASSWORD,
		STATE_READ_DATA,
		STATE_WRITE_DATA
	};

	optional_memory_region m_region;

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
	int m_password_retry_counter;
	bool m_is_password_accepted;
	uint8_t m_write_buffer[ 8 ];
	uint8_t m_response_to_reset[ 4 ];
	uint8_t m_write_password[ 8 ];
	uint8_t m_read_password[ 8 ];
	uint8_t m_data[ 112 ];
};

// device type definition
DECLARE_DEVICE_TYPE(X76F100, x76f100_device)

#endif // MAME_MACHINE_X76F100_H
