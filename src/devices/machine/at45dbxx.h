// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Atmel Serial DataFlash

    (c) 2001-2007 Tim Schuerewegen

    AT45DB041 -  528 KByte
    AT45DB081 - 1056 KByte
    AT45DB161 - 2112 KByte

*/

#ifndef MAME_MACHINE_AT45DBXX_H
#define MAME_MACHINE_AT45DBXX_H

#pragma once


// ======================> at45db041_device

class at45db041_device : public device_t,
							public device_nvram_interface
{
public:
	at45db041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(cs_w);
	DECLARE_WRITE_LINE_MEMBER(sck_w);
	DECLARE_WRITE_LINE_MEMBER(si_w);
	DECLARE_READ_LINE_MEMBER(so_r);

	uint8_t *get_ptr() {  return &m_data[0];  }

	auto so_callback() { return write_so.bind(); }

protected:
	at45db041_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual int num_pages() const { return 2048; }
	virtual int page_size() const { return 264; }
	virtual uint8_t device_id() const { return 0x18; }

	uint8_t read_byte();
	void flash_set_io(uint8_t* data, uint32_t size, uint32_t pos);
	virtual uint32_t flash_get_page_addr();
	virtual uint32_t flash_get_byte_addr();
	void write_byte(uint8_t data);

	devcb_write_line write_so;

	// internal state
	std::vector<uint8_t> m_data;
	uint32_t      m_size;
	uint8_t       m_mode;
	uint8_t       m_status;
	std::vector<uint8_t> m_buffer1;
	//std::vector<uint8_t> m_buffer2;
	uint8_t       m_si_byte;
	uint8_t       m_si_bits;
	uint8_t       m_so_byte;
	uint8_t       m_so_bits;

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
		uint8_t *data;
		uint32_t size;
		uint32_t pos;
	} m_io;

	struct AT45DBXX_CMD
	{
		uint8_t data[8];
		uint8_t size;
	} m_cmd;
};

// ======================> at45db081_device

class at45db081_device : public at45db041_device
{
public:
	at45db081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int num_pages() const override { return 4096; }
	virtual int page_size() const override { return 264;  }
	virtual uint8_t device_id() const override { return 0x20; }

	virtual uint32_t flash_get_page_addr() override;
};

// ======================> at45db161_device

class at45db161_device : public at45db041_device
{
public:
	at45db161_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int num_pages() const override { return 4096; }
	virtual int page_size() const override { return 528;  }
	virtual uint8_t device_id() const override { return 0x28; }

	virtual uint32_t flash_get_page_addr() override;
	virtual uint32_t flash_get_byte_addr() override;
};


// device type definition
DECLARE_DEVICE_TYPE(AT45DB041, at45db041_device)
DECLARE_DEVICE_TYPE(AT45DB081, at45db081_device)
DECLARE_DEVICE_TYPE(AT45DB161, at45db161_device)

#endif // MAME_MACHINE_AT45DBXX_H
