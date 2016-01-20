// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
/*****************************************************************************
 *
 * includes/channelf.h
 *
 ****************************************************************************/

#ifndef CHANNELF_H_
#define CHANNELF_H_

#include "emu.h"
#include "cpu/f8/f8.h"
#include "audio/channelf.h"

#include "bus/chanf/slot.h"
#include "bus/chanf/rom.h"


class channelf_state : public driver_device
{
public:
	channelf_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_custom(*this,"custom"),
			m_cart(*this, "cartslot")
	{ }

	DECLARE_READ8_MEMBER(port_0_r);
	DECLARE_READ8_MEMBER(port_1_r);
	DECLARE_READ8_MEMBER(port_4_r);
	DECLARE_READ8_MEMBER(port_5_r);
	DECLARE_WRITE8_MEMBER(port_0_w);
	DECLARE_WRITE8_MEMBER(port_1_w);
	DECLARE_WRITE8_MEMBER(port_4_w);
	DECLARE_WRITE8_MEMBER(port_5_w);
	UINT8 *m_p_videoram;
	UINT8 m_latch[6];
	UINT8 m_val_reg;
	UINT8 m_row_reg;
	UINT8 m_col_reg;
	UINT8 port_read_with_latch(UINT8 ext, UINT8 latch_state);
	virtual void video_start() override;
	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(channelf);
	UINT32 screen_update_channelf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<channelf_sound_device> m_custom;
	required_device<channelf_cart_slot_device> m_cart;
	int recalc_palette_offset(int reg1, int reg2);
};

#endif /* CHANNELF_H_ */
