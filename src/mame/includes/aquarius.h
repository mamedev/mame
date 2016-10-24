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
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
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

	uint8_t m_scrambler;
	tilemap_t *m_tilemap;

	void aquarius_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aquarius_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cassette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cassette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsync_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mapper_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t printer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void printer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scrambler_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cartridge_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_aquarius();
	void aquarius_gettileinfo(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_aquarius(palette_device &palette);
	uint32_t screen_update_aquarius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void aquarius_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
};
#endif /* AQUARIUS_H_ */
