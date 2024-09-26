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

	bool get_card_present() { return m_image->exists(); }

	devcb_write_line write_miso;

protected:
	enum sd_type : u8
	{
		SD_TYPE_V2 = 0,
		SD_TYPE_HC
	};

	spi_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<harddisk_image_device> m_image;

	sd_type m_type;

private:
	enum sd_state : u8
	{
		//REF Table 4-1:Overview of Card States vs. Operation Mode
		SD_STATE_IDLE = 0,
		SD_STATE_READY,
		SD_STATE_IDENT,
		SD_STATE_STBY,
		SD_STATE_TRAN,
		SD_STATE_DATA,
		SD_STATE_DATA_MULTI, // synthetical state for this implementation
		SD_STATE_RCV,
		SD_STATE_PRG,
		SD_STATE_DIS,
		SD_STATE_INA,

		//FIXME Existing states which must be revisited
		SD_STATE_WRITE_WAITFE,
		SD_STATE_WRITE_DATA
	};
	sd_state m_state;

	// MMFS for Acorn machines expect dummy byte before response
	static constexpr int SPI_DELAY_RESPONSE = 1;

	void send_data(u16 count, sd_state new_state);
	void do_command();
	void change_state(sd_state new_state);

	void latch_in();
	void shift_out();

	u8 m_data[520], m_cmd[6];

	int m_ss, m_in_bit, m_clk_state;
	u8 m_in_latch, m_out_latch, m_cur_bit;
	u16 m_out_count, m_out_ptr, m_write_ptr, m_blksize;
	u32 m_blknext;
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
