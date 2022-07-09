// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    Munch Mobile

*************************************************************************/
#ifndef MAME_INCLUDES_MUNCHMO_H
#define MAME_INCLUDES_MUNCHMO_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "emupal.h"

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_sprite_xpos(*this, "sprite_xpos")
		, m_sprite_tile(*this, "sprite_tile")
		, m_sprite_attr(*this, "sprite_attr")
		, m_videoram(*this, "videoram")
		, m_status_vram(*this, "status_vram")
		, m_vreg(*this, "vreg")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mainlatch(*this, "mainlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_ay8910(*this, "ay%u", 1U)
	{
	}

	void mnchmobl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	void nmi_ack_w(uint8_t data);
	void sound_nmi_ack_w(uint8_t data);

	uint8_t ay1reset_r();
	uint8_t ay2reset_r();

	DECLARE_WRITE_LINE_MEMBER(palette_bank_0_w);
	DECLARE_WRITE_LINE_MEMBER(palette_bank_1_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	void munchmo_palette(palette_device &palette) const;
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	IRQ_CALLBACK_MEMBER(generic_irq_ack);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void mnchmobl_map(address_map &map);
	void sound_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_sprite_xpos;
	required_shared_ptr<uint8_t> m_sprite_tile;
	required_shared_ptr<uint8_t> m_sprite_attr;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_status_vram;
	required_shared_ptr<uint8_t> m_vreg;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	int          m_palette_bank = 0;
	int          m_flipscreen = 0;

	/* misc */
	int          m_nmi_enable = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ls259_device> m_mainlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device_array<ay8910_device, 2> m_ay8910;
};

#endif // MAME_INCLUDES_MUNCHMO_H
