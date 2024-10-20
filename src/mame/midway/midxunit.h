// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/
#ifndef MAME_MIDWAY_MIDXUNIT_H
#define MAME_MIDWAY_MIDXUNIT_H

#pragma once

#include "midtunit_v.h"

#include "dcs.h"

#include "cpu/tms34010/tms34010.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/nvram.h"

#include "emupal.h"


class midxunit_state : public driver_device
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "video")
		, m_dcs(*this, "dcs")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "pic")
		, m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
		, m_gun_led(*this, "Player%u_Gun_LED", 1U)
	{ }

	void midxunit(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t cmos_r(offs_t offset);
	void cmos_w(offs_t offset, uint8_t data);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void adc_int_w(int state);
	uint32_t status_r();
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);
	uint32_t security_r();
	void security_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void security_clock_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dcs_output_full(int state);
	uint32_t dma_r(offs_t offset, uint32_t mem_mask = ~0);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;

	required_device<tms340x0_device> m_maincpu;
	required_device<midtunit_video_device> m_video;
	required_device<dcs_audio_device> m_dcs;
	required_device<palette_device> m_palette;

	required_device<nvram_device> m_nvram;
	required_device<pic16c57_device> m_pic;
	output_finder<3> m_gun_recoil;
	output_finder<3> m_gun_led;

	uint8_t m_cmos_write_enable = 0;
	uint16_t m_iodata[8] = {};
	uint8_t m_uart[8] = {};
	bool m_adc_int = false;
	std::unique_ptr<uint8_t[]> m_nvram_data;

	uint8_t m_pic_command = 0;
	uint8_t m_pic_data = 0;
	uint8_t m_pic_clk = 0;
	uint8_t m_pic_status = 0;
};

#endif // MAME_MIDWAY_MIDXUNIT_H
