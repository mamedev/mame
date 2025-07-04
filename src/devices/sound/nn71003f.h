// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Nippon Steel Corp NN71003F mpeg audio decoder

#ifndef MAME_SOUND_NN71003F_H
#define MAME_SOUND_NN71003F_H

#pragma once

#include "mpeg_audio.h"

class nn71003f_device : public device_t, public device_sound_interface
{
public:
	nn71003f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Serial audio interface
	void frm_w(int state);
	void dat_w(int state);
	void clk_w(int state);

	// Slave SPI interface
	void ss_w(int state);
	void sclk_w(int state);
	void mosi_w(int state);
	auto miso_cb() { return m_miso.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	devcb_write_line m_miso;
	u8 m_spi_byte, m_spi_cnt;
	int m_ss, m_sclk, m_mosi;
};

DECLARE_DEVICE_TYPE(NN71003F, nn71003f_device)

#endif // MAME_SOUND_NN71003F_H
