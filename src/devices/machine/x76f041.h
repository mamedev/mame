// license:BSD-3-Clause
// copyright-holders:smf
/*
 * x76f041.h
 *
 * Secure SerialFlash
 *
 */

#ifndef MAME_MACHINE_X76F041_H
#define MAME_MACHINE_X76F041_H

#pragma once


class x76f041_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	x76f041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write_cs(int state);
	void write_rst(int state);
	void write_scl(int state);
	void write_sda(int state);
	int read_sda();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	inline void ATTR_PRINTF(3, 4) verboselog(int n_level, const char *s_fmt, ...);
	uint8_t *password();
	void password_ok();
	void load_address();
	int data_offset();

	enum configuration_register_t
	{
		// If set to 1, retry counter is incremented when an invalid password is provided
		CR_RETRY_COUNTER_ENABLE_BIT = 0x04,

		// If set to 1, retry counter will be reset when a correct password is provided
		CR_RETRY_COUNTER_RESET_BIT = 0x08,

		// 10 = If retry counter is enabled, deny all commands when retry register equals retry counter
		// 00, 01, 11 = If retry counter is enabled, allow only configuration commands when retry register equals retry counter
		CR_UNAUTHORIZED_ACCESS_BITS = 0xc0,
	};

	enum configuration_registers_t
	{
		CONFIG_BCR1 = 0, // Array Control Register
		CONFIG_BCR2 = 1, // Array Control Register 2
		CONFIG_CR   = 2, // Configuration Register
		CONFIG_RR   = 3, // Retry Register
		CONFIG_RC   = 4  // Reset Counter
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
		STATE_CONFIGURATION_WRITE_DATA,
		STATE_READ_CONFIGURATION_REGISTERS,
		STATE_WRITE_CONFIGURATION_REGISTERS,

		STATE_PROGRAM_WRITE_PASSWORD,
		STATE_PROGRAM_READ_PASSWORD,
		STATE_PROGRAM_CONFIGURATION_PASSWORD,

		STATE_RESET_WRITE_PASSWORD,
		STATE_RESET_READ_PASSWORD,
		STATE_MASS_PROGRAM,
		STATE_MASS_ERASE
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
	int m_address;
	bool m_is_password_accepted;
	uint8_t m_write_buffer[8];
	uint8_t m_response_to_reset[4];
	uint8_t m_write_password[8];
	uint8_t m_read_password[8];
	uint8_t m_configuration_password[8];
	uint8_t m_configuration_registers[8];
	uint8_t m_data[512];
	uint8_t m_password_temp[16];
};


// device type definition
DECLARE_DEVICE_TYPE(X76F041, x76f041_device)

#endif // MAME_MACHINE_X76F041_H
