// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/busicom.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_BUSICOM_H
#define MAME_INCLUDES_BUSICOM_H

#pragma once

#include "cpu/mcs40/mcs40.h"
#include "machine/timer.h"
#include "emupal.h"

class busicom_state : public driver_device
{
public:
	busicom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_input_lines(*this, "LINE%u", 0)
	{
	}

	void busicom(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(shifter_w);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_WRITE8_MEMBER(printer_ctrl_w);
	void busicom_palette(palette_device &palette) const;
	uint32_t screen_update_busicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);
	uint8_t get_bit_selected(uint32_t val,int num);

	void busicom_mem(address_map &map);
	void busicom_mp(address_map &map);
	void busicom_rom(address_map &map);
	void busicom_rp(address_map &map);
	void busicom_stat(address_map &map);

	uint8_t m_drum_index;
	uint16_t m_keyboard_shifter;
	uint32_t m_printer_shifter;
	uint8_t m_timer;
	uint8_t m_printer_line[11][17];
	uint8_t m_printer_line_color[11];

	required_device<i4004_cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_ioport_array<10> m_input_lines;
};

#endif // MAME_INCLUDES_BUSICOM_H
