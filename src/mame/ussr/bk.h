// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/*****************************************************************************
 *
 * includes/bk.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_BK_H
#define MAME_USSR_BK_H

#pragma once

#include "1801vp014.h"
#include "vm1timer.h"

#include "bus/bk/parallel.h"
#include "bus/bk/carts.h"
#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/pdp11.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"


class bk_state : public driver_device
{
public:
	bk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_config(*this, "CONFIG")
		, m_palette(*this, "palette")
		, m_cassette(*this, "cassette")
		, m_dac(*this, "dac")
		, m_timer(*this, "timer")
		, m_kbd(*this, "keyboard")
		, m_qbus(*this, "qbus")
		, m_up(*this, "up")
		, m_ram(*this, "ram%u", 0U)
		, m_view1(*this, "view1")
		, m_view2(*this, "view2")
	{ }

	void bk0010(machine_config &config);
	void bk0010fd(machine_config &config);
	void bk0011(machine_config &config);
	void bk0011m(machine_config &config);
	void update_monitor(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum sel1_bits : u8
	{
		SEL1_UPDATED = 0004,
		SEL1_RX_SER  = 0020,
		SEL1_RX_CAS  = 0040,
		SEL1_KEYDOWN = 0100,
		SEL1_RDY_SER = 0200,
		SEL1_MOTOR   = 0200,
	};

	uint16_t m_scroll;
	uint8_t m_sel1;
	uint8_t m_misc;
	int m_monitor;
	int m_video_page;
	int m_stop_disabled;
	bitmap_rgb32 m_tmpbmp;

	uint16_t vid_scroll_r();
	uint16_t sel1_r();
	uint16_t bk11_sel1_r();
	uint16_t trap_r();
	void vid_scroll_w(uint16_t data);
	void sel1_w(uint16_t data);
	void bk11_sel1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bk11m_sel1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void trap_w(uint16_t data);
	void reset_w(int state);

	uint32_t screen_update_bk10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bk11(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	required_device<k1801vm1_device> m_maincpu;
	required_ioport m_config;
	required_device<palette_device> m_palette;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_bit_interface> m_dac;
	required_device<k1801vm1_timer_device> m_timer;
	required_device<k1801vp014_device> m_kbd;
	required_device<qbus_device> m_qbus;
	required_device<bk_parallel_slot_device> m_up;
	optional_shared_ptr_array<uint16_t, 8> m_ram;

	memory_view m_view1, m_view2;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback_bk11);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void bk0010_mem(address_map &map) ATTR_COLD;
	void bk0010fd_mem(address_map &map) ATTR_COLD;
	void bk0011_mem(address_map &map) ATTR_COLD;
	void bk0011m_mem(address_map &map) ATTR_COLD;

	void bk0010_palette(palette_device &palette);
	void bk0011_palette(palette_device &palette);
};

#endif // MAME_USSR_BK_H
