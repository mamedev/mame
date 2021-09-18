// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_SPI_SDCARD_H
#define MAME_MACHINE_SPI_SDCARD_H

#pragma once

#include "imagedev/harddriv.h"

class spi_sdcard_device : public device_t
{
public:
	// SPI 4-wire interface
	auto spi_miso_callback() { return write_miso.bind(); }
	void spi_clock_w(int state);
	void spi_ss_w(int state) { m_ss = state; }
	void spi_mosi_w(int state) { m_in_bit = state; }

	bool get_card_present() { return !(m_harddisk == nullptr); }

	devcb_write_line write_miso;

protected:
	enum
	{
		SD_TYPE_V2 = 0,
		SD_TYPE_HC
	};

	spi_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<harddisk_image_device> m_image;

	int m_type;

private:
	enum
	{
		SD_STATE_IDLE = 0,
		SD_STATE_WRITE_WAITFE,
		SD_STATE_WRITE_DATA
	};

	void send_data(int count);
	void do_command();

	u8 m_data[520], m_cmd[6];
	hard_disk_file *m_harddisk;

	u8 m_in_latch, m_out_latch;
	int m_cmd_ptr, m_state, m_out_ptr, m_out_count, m_ss, m_in_bit, m_cur_bit, m_write_ptr, m_blksize;
	bool m_bACMD;
};

class spi_sdcard_sdhc_device : public spi_sdcard_device
{
public:
	spi_sdcard_sdhc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class spi_sdcard_sdv2_device : public spi_sdcard_device
{
public:
	spi_sdcard_sdv2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(SPI_SDCARD, spi_sdcard_sdhc_device)
DECLARE_DEVICE_TYPE(SPI_SDCARDV2, spi_sdcard_sdv2_device)

#endif // MAME_MACHINE_SPI_SDCARD_H
