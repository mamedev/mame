// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Atmel Serial DataFlash

    (c) 2001-2007 Tim Schuerewegen

    AT45DB041 -  528 KByte
    AT45DB081 - 1056 KByte
    AT45DB161 - 2112 KByte

*/

#ifndef _AT45DBXX_H_
#define _AT45DBXX_H_

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AT45DB041_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB041, 0)

#define MCFG_AT45DB081_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB081, 0)

#define MCFG_AT45DB161_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB161, 0)

#define MCFG_AT45DBXXX_SO_CALLBACK(_cb) \
	devcb = &at45db041_device::set_so_cb(*device, DEVCB_##_cb);


// ======================> at45db041_device

class at45db041_device : public device_t,
							public device_nvram_interface
{
public:
	at45db041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	at45db041_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_WRITE_LINE_MEMBER(cs_w);
	DECLARE_WRITE_LINE_MEMBER(sck_w);
	DECLARE_WRITE_LINE_MEMBER(si_w);
	DECLARE_READ_LINE_MEMBER(so_r);

	UINT8 *get_ptr() {  return &m_data[0];  }

	template<class _Object> static devcb_base &set_so_cb(device_t &device, _Object object) { return downcast<at45db041_device &>(device).write_so.set_callback(object); }
	devcb_write_line write_so;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

protected:
	virtual int num_pages() const { return 2048; }
	virtual int page_size() const { return 264; }
	virtual UINT8 device_id() const { return 0x18; }

	UINT8 read_byte();
	void flash_set_io(UINT8* data, UINT32 size, UINT32 pos);
	virtual UINT32 flash_get_page_addr();
	virtual UINT32 flash_get_byte_addr();
	void write_byte(UINT8 data);

	// internal state
	dynamic_buffer m_data;
	UINT32      m_size;
	UINT8       m_mode;
	UINT8       m_status;
	dynamic_buffer m_buffer1;
	//dynamic_buffer m_buffer2;
	UINT8       m_si_byte;
	UINT8       m_si_bits;
	UINT8       m_so_byte;
	UINT8       m_so_bits;

	struct AT45DBXX_PINS
	{
		int cs;    // chip select
		int sck;   // serial clock
		int si;    // serial input
		int so;    // serial output
		int wp;    // write protect
		int reset; // reset
		int busy;  // busy
	} m_pin;

	struct AT45DBXX_IO
	{
		UINT8 *data;
		UINT32 size;
		UINT32 pos;
	} m_io;

	struct AT45DBXX_CMD
	{
		UINT8 data[8];
		UINT8 size;
	} m_cmd;
};

// ======================> at45db081_device

class at45db081_device : public at45db041_device
{
public:
	at45db081_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int num_pages() const override { return 4096; }
	virtual int page_size() const override { return 264;  }
	virtual UINT8 device_id() const override { return 0x20; }

	virtual UINT32 flash_get_page_addr() override;
};

// ======================> at45db161_device

class at45db161_device : public at45db041_device
{
public:
	at45db161_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int num_pages() const override { return 4096; }
	virtual int page_size() const override { return 528;  }
	virtual UINT8 device_id() const override { return 0x28; }

	virtual UINT32 flash_get_page_addr() override;
	virtual UINT32 flash_get_byte_addr() override;
};


// device type definition
extern const device_type AT45DB041;
extern const device_type AT45DB081;
extern const device_type AT45DB161;

#endif
