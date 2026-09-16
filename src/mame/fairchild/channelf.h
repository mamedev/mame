// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
/*****************************************************************************
 *
 * includes/channelf.h
 *
 ****************************************************************************/
#ifndef MAME_FAIRCHILD_CHANNELF_H
#define MAME_FAIRCHILD_CHANNELF_H

#pragma once

#include "cpu/f8/f8.h"
#include "channelf_a.h"

#include "bus/chanf/slot.h"
#include "bus/chanf/rom.h"

#include "emupal.h"


class channelf_state : public driver_device
{
public:
	channelf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_custom(*this,"custom")
		, m_cart(*this, "cartslot")
	{ }

	uint8_t port_0_r();
	uint8_t port_1_r();
	uint8_t port_4_r();
	uint8_t port_5_r();
	void port_0_w(uint8_t data);
	void port_1_w(uint8_t data);
	void port_4_w(uint8_t data);
	void port_5_w(uint8_t data);
	uint8_t *m_p_videoram = nullptr;
	uint8_t m_latch[6]{};
	uint8_t m_val_reg = 0U;
	uint8_t m_row_reg = 0U;
	uint8_t m_col_reg = 0U;
	uint8_t port_read_with_latch(uint8_t ext, uint8_t latch_state);
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void channelf_palette(palette_device &palette) const;
	uint32_t screen_update_ntsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_channelf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_rpt);
	required_device<cpu_device> m_maincpu;
	required_device<channelf_sound_device> m_custom;
	required_device<channelf_cart_slot_device> m_cart;
	int recalc_palette_offset(int reg1, int reg2);
	void channelf_cart(machine_config &config);
	void channelf(machine_config &config);
	void sabavpl2(machine_config &config);
	void sabavdpl(machine_config &config);
	void channlf2(machine_config &config);
	void channelf_io(address_map &map) ATTR_COLD;
	void channelf_map(address_map &map) ATTR_COLD;
};

#endif // MAME_FAIRCHILD_CHANNELF_H
