// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERIC_SPI_FLASH_H
#define MAME_MACHINE_GENERIC_SPI_FLASH_H

#pragma once

class generic_spi_flash_device : public device_t,
							  public device_nvram_interface
{
public:
	generic_spi_flash_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	void set_rom_ptr(uint8_t* rom) { m_spiptr = rom; }
	void set_rom_size(size_t size) { m_length = size; }

	uint8_t read()
	{
		return m_spilatch;
	}

	void set_ready()
	{
		m_spi_state = READY_FOR_COMMAND;
	}

	void dir_w(int state)
	{
		m_spidir = state;
	}

	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum
	{
		READY_FOR_COMMAND = 0x00,

		READY_FOR_ADDRESS2 = 0x01,
		READY_FOR_ADDRESS1 = 0x02,
		READY_FOR_ADDRESS0 = 0x03,

		READY_FOR_HSADDRESS2 = 0x04,
		READY_FOR_HSADDRESS1 = 0x05,
		READY_FOR_HSADDRESS0 = 0x06,
		READY_FOR_HSDUMMY = 0x07,

		READY_FOR_WRITEADDRESS2 = 0x08,
		READY_FOR_WRITEADDRESS1 = 0x09,
		READY_FOR_WRITEADDRESS0 = 0x0a,

		READY_FOR_SECTORERASEADDRESS2 = 0x0b,
		READY_FOR_SECTORERASEADDRESS1 = 0x0c,
		READY_FOR_SECTORERASEADDRESS0 = 0x0d,

		READY_FOR_WRITE = 0x0e,

		READY_FOR_READ = 0x0f,
		READY_FOR_STATUS_READ = 0x10,
	};

	uint32_t m_spiaddr;
	uint8_t m_spi_state;
	uint8_t m_spilatch;
	bool m_spidir;

	uint8_t* m_spiptr;
	size_t m_length;
};

DECLARE_DEVICE_TYPE(GENERIC_SPI_FLASH, generic_spi_flash_device)

#endif // MAME_MACHINE_GENERIC_SPI_FLASH_H
