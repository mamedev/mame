// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERIC_SPI_FLASH_H
#define MAME_MACHINE_GENERIC_SPI_FLASH_H

#pragma once

class generic_spi_flash_device : public device_t,
							  public device_nvram_interface
{
public:
	generic_spi_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_rom_ptr(u8 *rom) { m_spiptr = rom; }
	void set_rom_size(size_t size) { m_length = size; }

	u8 read()
	{
		return m_spi_latch;
	}

	void set_ready()
	{
		m_spi_state = READY_FOR_COMMAND;
	}

	void write(u8 data);

	void set_single_byte_status_read() { m_multibyte_status_read = 0; };
	void set_single_byte_status_writes() { m_multibyte_status_write = 0; };

	void set_jedec_manufacturer(u8 byte) { m_idbytes[0] = byte; }
	void set_jedec_memtype(u8 byte) { m_idbytes[1] = byte; }
	void set_jedec_capacity(u8 byte) { m_idbytes[2] = byte; }


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

		COMMAND_01_WRSR = 0x01,
		COMMAND_02_PP = 0x02,
		COMMAND_03_READ = 0x03,
		COMMAND_04_WRDI = 0x04,
		COMMAND_05_RDSR = 0x05,
		COMMAND_06_WREN = 0x06,

		COMMAND_0B_FAST_READ = 0x0b,

		COMMAND_11_UNKNOWN = 0x11, // wiwcs

		COMMAND_15_RDCR = 0x15,

		COMMAND_20_SE = 0x20,

		COMMAND_31_UNKNOWN = 0x31,

		COMMAND_35_RDSR2 = 0x35,

		COMMAND_66_ENABLE_RESET = 0x66,

		COMMAND_90_REMS = 0x90,

		COMMAND_99_RESET = 0x99,

		COMMAND_9F_RDID = 0x9f,

		COMMAND_AB_RDP = 0xab,

		COMMAND_B9_DP = 0xb9,

		COMMAND_EB_4READ = 0xeb,

		COMMAND_EC_UNKNOWN = 0xec,
		COMMAND_FF_CRMR = 0xff,
	};

	void get_command(u8 data);
	void process_hsread_command(u8 data);
	void process_read_command(u8 data);
	void process_read4_command(u8 data);
	void process_write_command(u8 data);
	void process_sector_erase_command(u8 data);
	void process_status_write_command(u8 data);
	void process_status_rems_command(u8 data);
	void process_status_read_command(u8 data);
	void process_status2_read_command(u8 data);
	void process_status_rdid_command(u8 data);
	void process_config_read_command(u8 data);

	u32 m_spi_addr;
	u8 m_spi_state;
	u8 m_spi_latch;
	u8 m_spi_state_step;
	u8 m_spi_statusreg;
	u8 m_spi_configreg;

	// config
	u8 *m_spiptr;
	size_t m_length;
	u8 m_multibyte_status_read;
	u8 m_multibyte_status_write;
	u8 m_idbytes[3];
};

DECLARE_DEVICE_TYPE(GENERIC_SPI_FLASH, generic_spi_flash_device)

#endif // MAME_MACHINE_GENERIC_SPI_FLASH_H
