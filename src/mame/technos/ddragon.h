// license:BSD-3-Clause
// copyright-holders: Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont

/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/
#ifndef MAME_TECHNOS_DDRAGON_H
#define MAME_TECHNOS_DDRAGON_H

#pragma once

#include "cpu/m6805/m68705.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class ddragon_state : public driver_device
{
public:
	ddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_subcpu(*this, "sub")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_soundlatch(*this, "soundlatch")
		, m_mainbank(*this, "mainbank")
		, m_bgvideoram(*this, "bgvideoram")
		, m_fgvideoram(*this, "fgvideoram")
		, m_comram(*this, "comram")
		, m_spriteram(*this, "spriteram")
		, m_scrollx_lo(*this, "scrollx_lo")
		, m_scrolly_lo(*this, "scrolly_lo")
		, m_adpcm(*this, "adpcm%u", 1U)
		, m_adpcm_rom(*this, "adpcm%u", 1U)
	{
	}

	void ddragon(machine_config &config);
	void ddragon6809(machine_config &config);
	void ddragonbl(machine_config &config);
	void ddragonbla(machine_config &config);
	void ddragon2(machine_config &config);

	void init_ddragon2();
	void init_ddragon();
	void init_ddragon6809();

	int subcpu_bus_free_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	optional_memory_bank m_mainbank;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	optional_shared_ptr<uint8_t> m_comram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollx_lo;
	required_shared_ptr<uint8_t> m_scrolly_lo;

	// video-related
	tilemap_t      *m_fg_tilemap = nullptr;
	tilemap_t      *m_bg_tilemap = nullptr;
	uint8_t        m_technos_video_hw = 0;
	uint8_t        m_scrollx_hi = 0;
	uint8_t        m_scrolly_hi = 0;

	// misc
	uint8_t        m_ddragon_sub_port = 0;
	uint8_t        m_sprite_irq = 0;
	uint8_t        m_adpcm_sound_irq = 0;
	uint32_t       m_adpcm_pos[2]{};
	uint32_t       m_adpcm_end[2]{};
	bool           m_adpcm_idle[2]{};
	int            m_adpcm_data[2]{};

	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);

	TILEMAP_MAPPER_MEMBER(background_scan);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_16color_tile_info);

	int scanline_to_vcount(int scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void base_map(address_map &map) ATTR_COLD;

private:
	// devices
	optional_device_array<msm5205_device, 2> m_adpcm;

	optional_region_ptr_array<uint8_t, 2> m_adpcm_rom;

	void interrupt_ack(offs_t offset, uint8_t data);
	template <uint8_t Which> void ddragon_adpcm_int(int state);

	// video/ddragon.cpp
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void bankswitch_w(uint8_t data);
	uint8_t interrupt_r(offs_t offset);
	void interrupt_w(offs_t offset, uint8_t data);
	void ddragon2_sub_irq_ack_w(uint8_t data);
	void ddragon2_sub_irq_w(uint8_t data);
	void sub_port6_w(uint8_t data);
	uint8_t comram_r(offs_t offset);
	void comram_w(offs_t offset, uint8_t data);
	void ddragon_adpcm_w(offs_t offset, uint8_t data);
	uint8_t ddragon_adpcm_status_r();
	void ddragonbla_port_w(uint8_t data);

	void ddragon2_main_map(address_map &map) ATTR_COLD;
	void ddragon2_sound_map(address_map &map) ATTR_COLD;
	void ddragon2_sub_map(address_map &map) ATTR_COLD;
	void ddragon_main_map(address_map &map) ATTR_COLD;
	void ddragonbla_sub_map(address_map &map) ATTR_COLD;
	void ddragon_sound_map(address_map &map) ATTR_COLD;
	void ddragon6809_sound_map(address_map &map) ATTR_COLD;
	void ddragon_sub_map(address_map &map) ATTR_COLD;
	void sub_6309_map(address_map &map) ATTR_COLD;
	void sub_6809_map(address_map &map) ATTR_COLD;
};


class darktowr_state : public ddragon_state
{
public:
	darktowr_state(const machine_config &mconfig, device_type type, const char *tag)
		: ddragon_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_bank(*this, "darktowr_bank")
		, m_rambase(*this, "rambase")
		, m_mcu_port_a_out(0xff)
	{
	}

	void darktowr(machine_config &config);

	void init_darktowr();

private:
	uint8_t mcu_bank_r(offs_t offset);
	void mcu_bank_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void mcu_port_a_w(offs_t offset, uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void banked_map(address_map &map) ATTR_COLD;

	required_device<m68705p_device> m_mcu;
	required_device<address_map_bank_device> m_bank;
	required_shared_ptr<uint8_t> m_rambase;

	uint8_t m_mcu_port_a_out = 0;
};


class toffy_state : public ddragon_state
{
public:
	toffy_state(const machine_config &mconfig, device_type type, const char *tag)
		: ddragon_state(mconfig, type, tag)
	{
	}

	void toffy(machine_config &config);

	void init_toffy();

private:
	void bankswitch_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TECHNOS_DDRAGON_H
