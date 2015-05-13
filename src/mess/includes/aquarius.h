// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*****************************************************************************
 *
 * includes/aquarius.h
 *
 ****************************************************************************/

#ifndef __AQUARIUS__
#define __AQUARIUS__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tea1002.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class aquarius_state : public driver_device
{
public:
	aquarius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_cassette(*this, "cassette"),
			m_speaker(*this, "speaker"),
			m_cart(*this, "cartslot"),
			m_ram(*this, RAM_TAG),
			m_videoram(*this, "videoram"),
			m_colorram(*this, "colorram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_tea1002(*this, "encoder"),
			m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tea1002_device> m_tea1002;
	required_device<palette_device> m_palette;

	UINT8 m_scrambler;
	tilemap_t *m_tilemap;

	DECLARE_WRITE8_MEMBER(aquarius_videoram_w);
	DECLARE_WRITE8_MEMBER(aquarius_colorram_w);
	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(cassette_w);
	DECLARE_READ8_MEMBER(vsync_r);
	DECLARE_WRITE8_MEMBER(mapper_w);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(scrambler_w);
	DECLARE_READ8_MEMBER(cartridge_r);
	DECLARE_DRIVER_INIT(aquarius);
	TILE_GET_INFO_MEMBER(aquarius_gettileinfo);
	virtual void video_start();
	DECLARE_PALETTE_INIT(aquarius);
	UINT32 screen_update_aquarius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_INPUT_CHANGED_MEMBER(aquarius_reset);
};
#endif /* AQUARIUS_H_ */
