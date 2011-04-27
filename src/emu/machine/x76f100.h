/*
 * x76f100.h
 *
 * Secure SerialFlash
 *
 */

#ifndef __X76F100_H__
#define __X76F100_H__

#define MCFG_X76F100_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X76F100, 0)

#include "machine/secflash.h"

class x76f100_device : public device_secure_serial_flash
{
public:
	// construction/destruction
	x76f100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
		SIZE_DATA = 112,

		COMMAND_WRITE = 0x80,
		COMMAND_READ = 0x81,
		COMMAND_CHANGE_WRITE_PASSWORD = 0xfc,
		COMMAND_CHANGE_READ_PASSWORD = 0xfe,
		COMMAND_ACK_PASSWORD = 0x55
	};

	enum {
		STATE_STOP,
		STATE_RESPONSE_TO_RESET,
		STATE_LOAD_COMMAND,
		STATE_LOAD_PASSWORD,
		STATE_VERIFY_PASSWORD,
		STATE_READ_DATA,
		STATE_WRITE_DATA
	};

	int state, bit, byte;
	UINT8 command, shift;
	UINT8 write_buffer[SIZE_WRITE_BUFFER];
	UINT8 response_to_reset[SIZE_RESPONSE_TO_RESET];
	UINT8 write_password[SIZE_WRITE_PASSWORD];
	UINT8 read_password[SIZE_READ_PASSWORD];
	UINT8 data[SIZE_DATA];

	UINT8 *password();
	void password_ok();
	int data_offset();

private:
	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ...);
};


// device type definition
extern const device_type X76F100;

#endif
