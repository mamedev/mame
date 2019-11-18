// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    Mr. Flea

*************************************************************************/
#ifndef MAME_INCLUDES_MRFLEA_H
#define MAME_INCLUDES_MRFLEA_H

#pragma once

#include "machine/pic8259.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class mrflea_state : public driver_device
{
public:
	mrflea_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_pic(*this, "pic"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void mrflea(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	int     m_gfx_bank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<pic8259_device> m_pic;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(mrflea_data1_w);
	DECLARE_WRITE8_MEMBER(mrflea_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(mrflea_videoram_w);
	DECLARE_WRITE8_MEMBER(mrflea_spriteram_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_mrflea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mrflea_slave_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mrflea_master_io_map(address_map &map);
	void mrflea_master_map(address_map &map);
	void mrflea_slave_io_map(address_map &map);
	void mrflea_slave_map(address_map &map);
};

#endif // MAME_INCLUDES_MRFLEA_H
