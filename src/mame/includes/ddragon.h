// license:BSD-3-Clause
// copyright-holders:Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont
/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/
#ifndef MAME_INCLUDES_DDRAGON_H
#define MAME_INCLUDES_DDRAGON_H

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
		, m_rambase(*this, "rambase")
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
	void ddragonb(machine_config &config);
	void ddragonba(machine_config &config);
	void ddragon2(machine_config &config);

	void init_ddragon2();
	void init_ddragon();
	void init_ddragon6809();

	DECLARE_CUSTOM_INPUT_MEMBER(subcpu_bus_free);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_memory_bank m_mainbank;
	optional_shared_ptr<uint8_t> m_rambase;

	/* video-related */
	tilemap_t      *m_fg_tilemap;
	tilemap_t      *m_bg_tilemap;
	uint8_t        m_technos_video_hw;
	uint8_t        m_scrollx_hi;
	uint8_t        m_scrolly_hi;

	/* misc */
	uint8_t        m_ddragon_sub_port;
	uint8_t        m_sprite_irq;
	uint8_t        m_adpcm_sound_irq;
	uint32_t       m_adpcm_pos[2];
	uint32_t       m_adpcm_end[2];
	bool           m_adpcm_idle[2];
	int            m_adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            m_adpcm_addr;
	int            m_i8748_P1;
	int            m_i8748_P2;
	int            m_pcm_shift;
	int            m_pcm_nibble;
	int            m_mcu_command;
#if 0
	int            m_m5205_clk;
#endif

	DECLARE_WRITE8_MEMBER(ddragon_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(ddragon_fgvideoram_w);

	TILEMAP_MAPPER_MEMBER(background_scan);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_16color_tile_info);

	int scanline_to_vcount(int scanline);

	uint32_t screen_update_ddragon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ddragon_base_map(address_map &map);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	optional_shared_ptr<uint8_t> m_comram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollx_lo;
	required_shared_ptr<uint8_t> m_scrolly_lo;

	/* devices */
	optional_device_array<msm5205_device, 2> m_adpcm;

	optional_region_ptr_array<uint8_t, 2> m_adpcm_rom;

	void ddragon_interrupt_ack(offs_t offset, uint8_t data);
	void dd_adpcm_int(int chip);

	/* video/ddragon.c */
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	DECLARE_MACHINE_START(ddragon);
	DECLARE_MACHINE_RESET(ddragon);
	DECLARE_VIDEO_START(ddragon);

	TIMER_DEVICE_CALLBACK_MEMBER(ddragon_scanline);

	DECLARE_WRITE8_MEMBER(ddragon_bankswitch_w);
	DECLARE_READ8_MEMBER(ddragon_interrupt_r);
	DECLARE_WRITE8_MEMBER(ddragon_interrupt_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_ack_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_w);
	DECLARE_READ8_MEMBER(ddragon_hd63701_internal_registers_r);
	DECLARE_WRITE8_MEMBER(ddragon_hd63701_internal_registers_w);
	DECLARE_READ8_MEMBER(ddragon_comram_r);
	DECLARE_WRITE8_MEMBER(ddragon_comram_w);
	DECLARE_WRITE8_MEMBER(dd_adpcm_w);
	DECLARE_READ8_MEMBER(dd_adpcm_status_r);
	DECLARE_WRITE8_MEMBER(ddragonba_port_w);
	DECLARE_WRITE_LINE_MEMBER(dd_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(dd_adpcm_int_2);

	void dd2_map(address_map &map);
	void dd2_sound_map(address_map &map);
	void dd2_sub_map(address_map &map);
	void ddragon_map(address_map &map);
	void ddragonba_sub_map(address_map &map);
	void sound_map(address_map &map);
	void sub_map(address_map &map);
};


class darktowr_state : public ddragon_state
{
public:
	darktowr_state(const machine_config &mconfig, device_type type, const char *tag)
		: ddragon_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_darktowr_bank(*this, "darktowr_bank")
		, m_mcu_port_a_out(0xff)
	{
	}

	void darktowr(machine_config &config);

	void init_darktowr();

private:
	DECLARE_READ8_MEMBER(darktowr_mcu_bank_r);
	DECLARE_WRITE8_MEMBER(darktowr_mcu_bank_w);
	DECLARE_WRITE8_MEMBER(darktowr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mcu_port_a_w);

	void darktowr_map(address_map &map);
	void darktowr_banked_map(address_map &map);

	required_device<m68705p_device> m_mcu;
	optional_device<address_map_bank_device> m_darktowr_bank;

	uint8_t m_mcu_port_a_out;
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
	DECLARE_WRITE8_MEMBER(toffy_bankswitch_w);

	void toffy_map(address_map &map);
};

#endif // MAME_INCLUDES_DDRAGON_H
