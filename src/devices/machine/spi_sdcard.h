// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_SPI_SDCARD_H
#define MAME_MACHINE_SPI_SDCARD_H

#pragma once

#include "imagedev/harddriv.h"

#include <vector>


class spi_sdcard_device : public device_t
{
public:
	spi_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~spi_sdcard_device();

	void set_prefer_sd() { m_preferred_type = SD_TYPE_V2; }
	void set_prefer_sdhc() { m_preferred_type = SD_TYPE_HC; }
	void set_ignore_stop_bit(bool ignore) { m_ignore_stop_bit = ignore; }

	// SPI 4-wire interface
	auto spi_miso_callback() { return write_miso.bind(); }
	void spi_clock_w(int state);
	void spi_ss_w(int state) { m_ss = state; }
	void spi_mosi_w(int state) { m_in_bit = state; }

	bool get_card_present() { return m_image->exists(); }

	devcb_write_line write_miso;

protected:
	spi_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<harddisk_image_device> m_image;

private:
	enum sd_type : u8
	{
		SD_TYPE_V2,
		SD_TYPE_HC
	};

	enum sd_state : u8;

	// MMFS for Acorn machines expect dummy byte before response
	static constexpr u8 SPI_DELAY_RESPONSE = 1;

	std::error_condition image_loaded(device_image_interface &image);
	void image_unloaded(device_image_interface &image);

	void send_data(u16 count, sd_state new_state, u8 delay = SPI_DELAY_RESPONSE);
	void do_command();
	void change_state(sd_state new_state);

	void latch_in();
	void shift_out();

	// configuration
	sd_type m_preferred_type;
	bool m_ignore_stop_bit;

	// mounted image info
	std::vector<u8> m_sectorbuf;
	u16 m_blksize;
	sd_type m_type;
	u8 m_csd[16];

	// live state
	std::unique_ptr<u8 []> m_data;
	u8 m_cmd[6];
	sd_state m_state;
	u8 m_ss, m_in_bit, m_clk_state;
	u8 m_in_latch, m_out_latch, m_cur_bit;
	u8 m_out_delay;
	u16 m_out_count, m_out_ptr, m_write_ptr;
	u16 m_xferblk;
	u32 m_blknext;
	bool m_bACMD;
};

DECLARE_DEVICE_TYPE(SPI_SDCARD, spi_sdcard_device)

#endif // MAME_MACHINE_SPI_SDCARD_H
