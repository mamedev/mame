/*
 * zs01.h
 *
 * Secure SerialFlash
 *
 */

#ifndef __ZS01_H__
#define __ZS01_H__

#define MCFG_ZS01_ADD(_tag, ds2401_tag) \
	MCFG_DEVICE_ADD(_tag, ZS01, 0) \
	zs01_device::static_set_ds2401_tag(*device, ds2401_tag);
#include "machine/secflash.h"

class zs01_device : public device_secure_serial_flash
{
public:
	// construction/destruction
	zs01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_ds2401_tag(device_t &device, const char *ds2401_tag);

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
	const char *ds2401_tag;

	enum {
		SIZE_WRITE_BUFFER = 12,
		SIZE_READ_BUFFER = 12,
		SIZE_DATA_BUFFER = 8,
		SIZE_RESPONSE_TO_RESET = 4,
		SIZE_KEY = 8,
		SIZE_DATA = 4096,

		COMMAND_WRITE = 0x00,
		COMMAND_READ = 0x01
	};

	enum {
		STATE_STOP,
		STATE_RESPONSE_TO_RESET,
		STATE_LOAD_COMMAND,
		STATE_READ_DATA
	};

	int state, bit, byte;
	UINT8 shift;
	UINT8 write_buffer[SIZE_WRITE_BUFFER];
	UINT8 read_buffer[SIZE_READ_BUFFER];
	UINT8 response_key[SIZE_KEY];
	UINT8 response_to_reset[SIZE_RESPONSE_TO_RESET];
	UINT8 command_key[SIZE_KEY];
	UINT8 data_key[SIZE_KEY];
	UINT8 data[SIZE_DATA];

	void decrypt(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte);
	void decrypt2(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte);
	void encrypt(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT32 previous_byte);
	UINT16 do_crc(UINT8 *buffer, UINT32 length);
	int data_offset();

private:
	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ...);
};


// device type definition
extern const device_type ZS01;

#endif
