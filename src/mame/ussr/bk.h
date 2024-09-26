// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/bk.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_BK_H
#define MAME_USSR_BK_H

#pragma once

#include "1801vp014.h"

#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"
#include "sound/dac.h"

class bk_state : public driver_device
{
public:
	bk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_dac(*this, "dac")
		, m_kbd(*this, "keyboard")
		, m_qbus(*this, "qbus")
	{ }

	void bk0010(machine_config &config);
	void bk0010fd(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum sel1_bits : u16
	{
		SEL1_UPDATED = 0004,
		SEL1_RX_SER  = 0020,
		SEL1_RX_CAS  = 0040,
		SEL1_KEYDOWN = 0100,
		SEL1_RDY_SER = 0200,
		SEL1_MOTOR   = 0200,
	};

	uint16_t vid_scroll_r();
	uint16_t sel1_r();
	uint16_t trap_r();
	void vid_scroll_w(uint16_t data);
	void sel1_w(uint16_t data);
	void trap_w(uint16_t data);
	uint16_t floppy_cmd_r();
	void floppy_cmd_w(uint16_t data);
	uint16_t floppy_data_r();
	void floppy_data_w(uint16_t data);
	void reset_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bk0010_mem(address_map &map) ATTR_COLD;
	void bk0010fd_mem(address_map &map) ATTR_COLD;

	required_shared_ptr<uint16_t> m_vram;
	required_device<k1801vm1_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_bit_interface> m_dac;
	required_device<k1801vp014_device> m_kbd;
	required_device<qbus_device> m_qbus;

	uint16_t m_scroll = 0U;
	uint16_t m_sel1 = 0U;
	uint16_t m_drive = 0U;
};

#endif // MAME_USSR_BK_H
