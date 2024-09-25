// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

        Sanyo LC7985NA/LC7985ND LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_LC7985_H
#define MAME_VIDEO_LC7985_H

#pragma once

class lc7985_device : public device_t
{
public:
	lc7985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ir_w(u8 data);
	u8 status_r();
	void dr_w(u8 data);
	u8 dr_r();

	// 5 bits used per byte, blocks of 16 lines, 80 blocks
	const u8 *render();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	optional_region_ptr<u8> m_cgrom_region; // internal chargen ROM
	u8 m_render_buffer[16*80];
	u8 m_ddram[80];
	u8 m_cgram[64];
	const u8 *m_cgrom;
	emu_timer *m_busy_timer;
	u8 m_ddac;
	u8 m_cgac;
	u8 m_shift;
	u8 m_function;
	u8 m_cds;
	u8 m_display;
	u8 m_entry;

	bool m_busy_flag;
	bool m_access_ddram;

	void inc_ddac();
	void dec_ddac();
	void shift_left();
	void shift_right();

	void busy(attotime tm);
};

DECLARE_DEVICE_TYPE(LC7985, lc7985_device)

#endif // MAME_VIDEO_LC7985_H
