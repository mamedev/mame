// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/
#ifndef MAME_INCLUDES_MIDXUNIT_H
#define MAME_INCLUDES_MIDXUNIT_H

#pragma once

#include "audio/dcs.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/nvram.h"
#include "video/midtunit.h"
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
		, m_gfxrom(*this, "gfxrom")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "pic")
		, m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
		, m_gun_led(*this, "Player%u_Gun_LED", 1U)
	{ }

	void midxunit(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t midxunit_cmos_r(offs_t offset);
	void midxunit_cmos_w(offs_t offset, uint8_t data);
	void midxunit_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midxunit_unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(adc_int_w);
	uint32_t midxunit_status_r();
	uint8_t midxunit_uart_r(offs_t offset);
	void midxunit_uart_w(offs_t offset, uint8_t data);
	uint32_t midxunit_security_r();
	void midxunit_security_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void midxunit_security_clock_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(midxunit_dcs_output_full);
	uint32_t midxunit_dma_r(offs_t offset, uint32_t mem_mask = ~0);
	void midxunit_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void main_map(address_map &map);

	required_device<tms340x0_device> m_maincpu;
	required_device<midtunit_video_device> m_video;
	required_device<dcs_audio_device> m_dcs;
	required_device<palette_device> m_palette;
	required_memory_region m_gfxrom;

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

#endif // MAME_INCLUDES_MIDXUNIT_H
