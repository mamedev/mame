/*
 * x76f041.h
 *
 * Secure SerialFlash
 *
 */

#ifndef __X76F041_H__
#define __X76F041_H__

#define MCFG_X76F041_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X76F041, 0)

#include "machine/secflash.h"

class x76f041_device : public device_secure_serial_flash
{
public:
	// construction/destruction
	x76f041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	// device_secure_serial_flash implementations
	virtual void cs_0();
	virtual void cs_1();
	virtual void rst_0();
	virtual void rst_1();
	virtual void scl_0();
	virtual void scl_1();
	virtual void sda_0();
	virtual void sda_1();

	// internal state
	enum {
		SIZE_WRITE_BUFFER = 8,
		SIZE_RESPONSE_TO_RESET = 4,
		SIZE_WRITE_PASSWORD = 8,
		SIZE_READ_PASSWORD = 8,
		SIZE_CONFIGURATION_PASSWORD = 8,
		SIZE_CONFIGURATION_REGISTERS = 8,
		SIZE_DATA = 512,

		CONFIG_BCR1 = 0,
		CONFIG_BCR2 = 1,
		CONFIG_CR = 2,
		CONFIG_RR = 3,
		CONFIG_RC = 4,

		BCR_X = 8,
		BCR_Y = 4,
		BCR_Z = 2,
		BCR_T = 1,

		COMMAND_WRITE = 0x00,
		COMMAND_READ = 0x20,
		COMMAND_WRITE_USE_CONFIGURATION_PASSWORD = 0x40,
		COMMAND_READ_USE_CONFIGURATION_PASSWORD = 0x60,
		COMMAND_CONFIGURATION = 0x80,

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

	enum {
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

	int state, bit, byte, address;
	UINT8 command, shift;
	UINT8 write_buffer[SIZE_WRITE_BUFFER];
	UINT8 response_to_reset[SIZE_RESPONSE_TO_RESET];
	UINT8 write_password[SIZE_WRITE_PASSWORD];
	UINT8 read_password[SIZE_READ_PASSWORD];
	UINT8 configuration_password[SIZE_CONFIGURATION_PASSWORD];
	UINT8 configuration_registers[SIZE_CONFIGURATION_REGISTERS];
	UINT8 data[SIZE_DATA];

	UINT8 *password();
	void password_ok();
	void load_address();
	int data_offset();

private:
	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ...);
};


// device type definition
extern const device_type X76F041;

#endif
