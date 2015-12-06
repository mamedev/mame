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

	UINT8 m_drum_index;
	UINT16 m_keyboard_shifter;
	UINT32 m_printer_shifter;
	UINT8 m_timer;
	UINT8 m_printer_line[11][17];
	UINT8 m_printer_line_color[11];
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(shifter_w);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_WRITE8_MEMBER(printer_ctrl_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(busicom);
	UINT32 screen_update_busicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);
	required_device<i4004_cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	UINT8 get_bit_selected(UINT32 val,int num);
};

#endif /* BUSICOM_H_ */
