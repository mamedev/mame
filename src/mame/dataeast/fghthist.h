// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_DATAEAST_FGHTHIST_H
#define MAME_DATAEAST_FGHTHIST_H

#pragma once

#include "deco104.h"
#include "deco146.h"
#include "deco16ic.h"
#include "deco_ace.h"
#include "decospr.h"

#include "decobsmt.h"

#include "cpu/h6280/h6280.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"


class fghthist_common_state : public driver_device
{
protected:
	fghthist_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_sprgen(*this, "spritegen%u", 1)
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_eeprom(*this, "eeprom")
		, m_ioprot(*this, "ioprot")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki%u", 1)
		, m_pf_rowscroll32(*this, "pf%u_rowscroll32", 1)
	{ }

	virtual void video_start() override ATTR_COLD;

	void sound_bankswitch_w(u8 data);
	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void volume_w(u8 data);
	void vblank_ack_w(u32 data);

	template<int Chip> void pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template<int Chip> u32 spriteram_r(offs_t offset);
	template<int Chip> void spriteram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Chip> void buffer_spriteram_w(u32 data);
	void pri_w(u32 data);

	void allocate_spriteram(int chip);

	void common_map(address_map &map) ATTR_COLD;
	void h6280_sound_map(address_map &map) ATTR_COLD;
	void z80_sound_io(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<decospr_device, 2> m_sprgen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<deco_146_base_device> m_ioprot;
	optional_device<ym2151_device> m_ym2151;
	optional_device_array<okim6295_device, 2> m_oki;

	// we use the pointers below to store a 32-bit copy..
	required_shared_ptr_array<u32, 4> m_pf_rowscroll32;

	u32 m_pri = 0;
	std::unique_ptr<u16[]> m_spriteram16[2]{};
	std::unique_ptr<u16[]> m_spriteram16_buffered[2]{};
	std::unique_ptr<u16[]> m_pf_rowscroll[4]{};
};

class fghthist_state : public fghthist_common_state
{
public:
	fghthist_state(const machine_config &mconfig, device_type type, const char *tag)
		: fghthist_common_state(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_paletteram(*this, "paletteram")
		, m_io_in(*this, "IN%u", 0U)
	{ }

	void fghthist(machine_config &config) ATTR_COLD;
	void fghthistu(machine_config &config) ATTR_COLD;
	void fghthisto(machine_config &config) ATTR_COLD;

	void init_fghthist() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<u32> m_paletteram;

	required_ioport_array<2> m_io_in;

	std::unique_ptr<u8[]> m_dirty_palette{};

//  void sound_w(u32 data);
	u32 unk_status_r();

	void allocate_buffered_palette();

	void buffered_palette_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_dma_w(u32 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(fghthist_pri_callback);

	void fghthist_common_map(address_map &map) ATTR_COLD;
	void fghthisto_map(address_map &map) ATTR_COLD;
	void fghthist_map(address_map &map) ATTR_COLD;
	void h6280_sound_custom_latch_map(address_map &map) ATTR_COLD;
};

// nslasher
class nslasher_state : public fghthist_common_state
{
public:
	nslasher_state(const machine_config &mconfig, device_type type, const char *tag)
		: fghthist_common_state(mconfig, type, tag)
		, m_deco_ace(*this, "deco_ace")
	{ }

	void nslasheru(machine_config &config) ATTR_COLD;
	void nslasher(machine_config &config) ATTR_COLD;

	void init_nslasher() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void reorder_tilegfx() ATTR_COLD;

	required_device<deco_ace_device> m_deco_ace;
	bitmap_ind16 m_tilemap_alpha_bitmap;

	void tilemap_color_bank_w(u8 data);
	void sprite1_color_bank_w(u8 data);
	void sprite2_color_bank_w(u8 data);
	u16 nslasher_debug_r();

	void mix_nslasher(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap);
	u32 screen_update_nslasher(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	u16 mix_callback(u16 p, u16 p2);

	void nslasher_common_map(address_map &map) ATTR_COLD;
	void nslasher_map(address_map &map) ATTR_COLD;

};

// tattass
class tattass_state : public nslasher_state
{
public:
	tattass_state(const machine_config &mconfig, device_type type, const char *tag)
		: nslasher_state(mconfig, type, tag)
		, m_decobsmt(*this, "decobsmt")
	{ }

	void tattass(machine_config &config) ATTR_COLD;

	void init_tattass() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<decobsmt_device> m_decobsmt;

	u8 m_tattass_eprom_bit = 0;
	s32 m_last_clock = 0;
	u32 m_buffer = 0U;
	u32 m_buf_ptr = 0;
	s32 m_pending_command = 0;
	s32 m_read_bit_count = 0;
	u32 m_byte_addr = 0;

	void tattass_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void tattass_sound_irq_w(int state);

	void mix_tattass(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap);
	u32 screen_update_tattass(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 port_b_tattass();

	void tattass_map(address_map &map) ATTR_COLD;

};

#endif // MAME_DATAEAST_FGHTHIST_H
