// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/busicom.h
 *
 ****************************************************************************/

#ifndef BUSICOM_H_
#define BUSICOM_H_

#include "cpu/i4004/i4004.h"

class busicom_state : public driver_device
{
public:
	busicom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	uint8_t m_drum_index;
	uint16_t m_keyboard_shifter;
	uint32_t m_printer_shifter;
	uint8_t m_timer;
	uint8_t m_printer_line[11][17];
	uint8_t m_printer_line_color[11];
	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t printer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shifter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void printer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void printer_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_busicom(palette_device &palette);
	uint32_t screen_update_busicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void timer_callback(timer_device &timer, void *ptr, int32_t param);
	required_device<i4004_cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	uint8_t get_bit_selected(uint32_t val,int num);
};

#endif /* BUSICOM_H_ */
