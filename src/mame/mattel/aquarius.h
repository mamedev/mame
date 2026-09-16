// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Nigel Barnes
/*****************************************************************************
 *
 * aquarius.h
 *
 ****************************************************************************/
#ifndef MAME_MATTEL_AQUARIUS_H
#define MAME_MATTEL_AQUARIUS_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/tea1002.h"

#include "bus/aquarius/slot.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"

#include "formats/aquarius_caq.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class aquarius_state : public driver_device
{
public:
	aquarius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_exp(*this, "exp")
		, m_printer(*this, "printer")
		, m_mapper(*this, "mapper")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_gfx(*this, "gfx1")
		, m_y(*this, "Y%u", 0U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_tea1002(*this, "encoder")
		, m_palette(*this, "palette")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(aquarius_reset);
	DECLARE_INPUT_CHANGED_MEMBER(gfx_changed);

	void aquarius(machine_config &config);
	void aquariusp(machine_config &config);
	void aquarius_ar(machine_config &config);

	static void cfg_ram16(device_t* device);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<aquarius_cartridge_slot_device> m_exp;
	required_device<rs232_port_device> m_printer;
	memory_view m_mapper;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_gfx;
	required_ioport_array<8> m_y;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tea1002_device> m_tea1002;
	required_device<palette_device> m_palette;

	uint8_t m_scrambler = 0U;
	uint8_t m_gfx_bank = 0U;
	tilemap_t *m_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t cassette_r();
	void cassette_w(uint8_t data);
	uint8_t vsync_r();
	void mapper_w(uint8_t data);
	uint8_t printer_r();
	void printer_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	void scrambler_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void aquarius_palette(palette_device &palette) const;
	uint32_t screen_update_aquarius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void aquarius_io(address_map &map) ATTR_COLD;
	void aquarius_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_MATTEL_AQUARIUS_H
