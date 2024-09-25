// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/
#ifndef MAME_VSYSTEM_OJANKOHS_H
#define MAME_VSYSTEM_OJANKOHS_H

#pragma once

#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag) :
		ojankohs_state(mconfig, type, tag, 0x1000, 0x800)
	{ }

	void ojankohs(machine_config &config);

protected:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag, uint32_t vramsize, uint32_t pramsize) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram", vramsize, ENDIANNESS_LITTLE),
		m_colorram(*this, "colorram", 0x1000, ENDIANNESS_LITTLE),
		m_paletteram(*this, "paletteram", pramsize, ENDIANNESS_LITTLE),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_coin(*this, "coin"),
		m_inputs_p1(*this, "p1_%u", 0U),
		m_inputs_p2(*this, "p2_%u", 0U),
		m_inputs_p1_extra(*this, "p1_4"),
		m_inputs_p2_extra(*this, "p2_4"),
		m_dsw(*this, "dsw%u", 1U)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void common_state_saving();

	// memory pointers
	memory_share_creator<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_colorram;
	memory_share_creator<uint8_t> m_paletteram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_tilemap = nullptr;
	bitmap_ind16 m_tmpbitmap;
	uint8_t m_gfxreg = 0;
	uint8_t m_flipscreen = 0;
	uint8_t m_flipscreen_old = 0;
	int16_t m_scrollx = 0;
	int16_t m_scrolly = 0;
	uint8_t m_screen_refresh = 0;

	// misc
	uint8_t m_port_select = 0;
	uint8_t m_adpcm_reset = 0;
	int m_adpcm_data = 0;
	uint8_t m_vclk_left = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// I/O ports
	required_ioport m_coin;
	required_ioport_array<4> m_inputs_p1;
	required_ioport_array<4> m_inputs_p2;
	optional_ioport m_inputs_p1_extra;
	optional_ioport m_inputs_p2_extra;
	required_ioport_array<2> m_dsw;


	void rombank_w(uint8_t data);
	void msm5205_w(uint8_t data);
	void port_select_w(uint8_t data);
	uint8_t keymatrix_p1_r();
	uint8_t keymatrix_p2_r();
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	void adpcm_reset_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adpcm_int(int state);

private:
	void palette_w(offs_t offset, uint8_t data);
	uint8_t dipsw1_r();
	uint8_t dipsw2_r();
	void gfxreg_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
};

class ojankoy_state : public ojankohs_state
{
public:
	ojankoy_state(const machine_config &mconfig, device_type type, const char *tag) :
		ojankohs_state(mconfig, type, tag, 0x2000, 0x800)
	{ }

	void ojankoy(machine_config &config);

protected:
	ojankoy_state(const machine_config &mconfig, device_type type, const char *tag, uint32_t vramsize, uint32_t pramsize) :
		ojankohs_state(mconfig, type, tag, vramsize, pramsize)
	{ }

	virtual void video_start() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;

	void rombank_adpcm_reset_w(uint8_t data);

private:
	void coinctr_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	void io_map(address_map &map) ATTR_COLD;
};

class ccasino_state : public ojankoy_state
{
public:
	ccasino_state(const machine_config &mconfig, device_type type, const char *tag) :
		ojankoy_state(mconfig, type, tag, 0x2000, 0x800),
		m_extra_dsw(*this, "dsw%u", 3U)
	{ }

	void ccasino(machine_config &config);

protected:
	virtual void machine_start() override { ojankohs_state::machine_start(); }

private:
	required_ioport_array<2> m_extra_dsw;

	uint8_t dipsw3_r();
	uint8_t dipsw4_r();
	void coinctr_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
};

class ojankoc_state : public ojankohs_state
{
public:
	ojankoc_state(const machine_config &mconfig, device_type type, const char *tag) :
		ojankohs_state(mconfig, type, tag, 0x8000, 0x20)
	{ }

	void ojankoc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void ctrl_w(uint8_t data);
	uint8_t keymatrix_p1_r();
	uint8_t keymatrix_p2_r();
	void palette_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void flipscreen(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
};

#endif // MAME_VSYSTEM_OJANKOHS_H
