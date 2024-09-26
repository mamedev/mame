// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_DATAEAST_DEC8_H
#define MAME_DATAEAST_DEC8_H

#pragma once

#include "decbac06.h"
#include "deckarn.h"
#include "decmxc06.h"
#include "decrmc3.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"

#include "screen.h"
#include "tilemap.h"


class dec8_state_base : public driver_device
{
protected:
	dec8_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_spriteram(*this, "spriteram") ,
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_tilegen(*this, "tilegen%u", 1),
		m_mainbank(*this, "mainbank"),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(audiocpu_nmi_clear);

	void buffer_spriteram16_w(u8 data);
	void sound_w(u8 data);
	void main_irq_on_w(u8 data);
	void main_irq_off_w(u8 data);
	void main_firq_off_w(u8 data);
	void sub_irq_on_w(u8 data);
	void sub_irq_off_w(u8 data);
	void sub_firq_off_w(u8 data);
	void flip_screen_w(u8 data);
	void bg_ram_w(offs_t offset, u8 data);
	u8 bg_ram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);

	void set_screen_raw_params_data_east(machine_config &config);

	void allocate_buffered_spriteram16();
	void dec8_s_map(address_map &map) ATTR_COLD;
	void oscar_s_map(address_map &map) ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device_array<deco_bac06_device, 2> m_tilegen;

	/* memory regions */
	required_memory_bank m_mainbank;

	/* memory pointers */
	required_shared_ptr<u8> m_videoram;
	optional_shared_ptr<u8> m_bg_ram;

	std::unique_ptr<u16[]>   m_buffered_spriteram16; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fix_tilemap = nullptr;
	int      m_scroll[4]{};
	int      m_game_uses_priority = 0;

	/* misc */
	bool     m_coin_state = false;

	emu_timer *m_m6502_timer = nullptr;
};

// with I8751 MCU
class dec8_mcu_state_base : public dec8_state_base
{
protected:
	dec8_mcu_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		dec8_state_base(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_coin_port(*this, "I8751")
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(mcu_irq_clear);

	u8 i8751_h_r();
	u8 i8751_l_r();
	void i8751_w(offs_t offset, u8 data);

	u8 i8751_port0_r();
	void i8751_port0_w(u8 data);
	u8 i8751_port1_r();
	void i8751_port1_w(u8 data);

	void i8751_reset_w(u8 data);

	required_device<i8751_device> m_mcu;

	/* ports */
	required_ioport m_coin_port;

	// MCU communication
	u8  m_i8751_p2 = 0;
	int m_i8751_port0 = 0;
	int m_i8751_port1 = 0;
	int m_i8751_return = 0;
	int m_i8751_value = 0;

	emu_timer *m_i8751_timer = nullptr;
};

// with unique sprite hardware
class srdarwin_state : public dec8_mcu_state_base
{
public:
	srdarwin_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec8_mcu_state_base(mconfig, type, tag)
	{
	}

	void srdarwin(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void mcu_to_main_w(u8 data);
	void srdarwin_videoram_w(offs_t offset, u8 data);
	void control_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap);

	void main_map(address_map &map) ATTR_COLD;
};

// with 'karnov' sprite hardware
class lastmisn_state : public dec8_mcu_state_base
{
public:
	lastmisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec8_mcu_state_base(mconfig, type, tag),
		m_spritegen_krn(*this, "spritegen_krn"),
		m_nmigate(*this, "nmigate")
	{
	}

	void garyoret(machine_config &config);
	void ghostb(machine_config &config);
	void lastmisn(machine_config &config);
	void meikyuh(machine_config &config);
	void shackled(machine_config &config);

	void init_meikyuhbl();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void ghostb_bank_w(u8 data);
	void gondo_scroll_w(offs_t offset, u8 data);
	void gondo_mcu_to_main_w(u8 data);

	TILE_GET_INFO_MEMBER(get_gondo_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_tile_info);

	DECLARE_VIDEO_START(lastmisn);
	uint32_t screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void screen_vblank(int state);

	required_device<deco_karnovsprites_device> m_spritegen_krn;
	optional_device<input_merger_device> m_nmigate;

private:
	void lastmisn_control_w(u8 data);
	void shackled_control_w(u8 data);
	void lastmisn_scrollx_w(u8 data);
	void lastmisn_scrolly_w(u8 data);
	void shackled_mcu_to_main_w(u8 data);

	TILEMAP_MAPPER_MEMBER(lastmisn_scan_rows);

	TILE_GET_INFO_MEMBER(get_ghostb_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_lastmisn_tile_info);
	TILE_GET_INFO_MEMBER(get_lastmisn_fix_tile_info);

	DECLARE_VIDEO_START(garyoret);
	DECLARE_VIDEO_START(ghostb);
	DECLARE_VIDEO_START(shackled);

	uint32_t screen_update_ghostb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void shackled_coin_irq(int state);

	void lastmisn_map(address_map &map) ATTR_COLD;
	void lastmisn_sub_map(address_map &map) ATTR_COLD;
	void garyoret_map(address_map &map) ATTR_COLD;
	void meikyuh_map(address_map &map) ATTR_COLD;
	void shackled_map(address_map &map) ATTR_COLD;
	void shackled_sub_map(address_map &map) ATTR_COLD;
	void ym3526_s_map(address_map &map) ATTR_COLD;

	bool     m_secclr = false;
	bool     m_nmi_enable = false;
};

// with rotary joystick
class gondo_state : public lastmisn_state
{
public:
	gondo_state(const machine_config &mconfig, device_type type, const char *tag) :
		lastmisn_state(mconfig, type, tag),
		m_analog_io(*this, "AN%u", 0U),
		m_in_io(*this, "IN%u", 0U)
	{
	}

	void gondo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	template<unsigned Which> u8 player_io_r(offs_t offset);

	uint32_t screen_update_gondo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void gondo_colpri_cb(u32 &colour, u32 &pri_mask);

	void gondo_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_analog_io;
	required_ioport_array<4> m_in_io;
};

// with MXC06 sprite hardware
class oscar_state : public dec8_state_base
{
public:
	oscar_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec8_state_base(mconfig, type, tag),
		m_spritegen_mxc(*this, "spritegen_mxc")
	{
	}

	void cobracom(machine_config &config);
	void oscar(machine_config &config);
	void oscarbl(machine_config &config);

private:
	void bank_w(u8 data);

	TILE_GET_INFO_MEMBER(get_cobracom_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_oscar_fix_tile_info);

	DECLARE_VIDEO_START(cobracom);
	DECLARE_VIDEO_START(oscar);

	uint32_t screen_update_cobracom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_oscar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void coin_irq(int state);
	void coin_clear_w(u8 data);
	void cobracom_colpri_cb(u32 &colour, u32 &pri_mask);
	void oscar_tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags);

	void cobra_map(address_map &map) ATTR_COLD;
	void oscar_map(address_map &map) ATTR_COLD;
	void oscarbl_s_opcodes_map(address_map &map) ATTR_COLD;
	void oscar_sub_map(address_map &map) ATTR_COLD;

	required_device<deco_mxc06_device> m_spritegen_mxc;
};

// with MSM5205 ADPCM
class csilver_state : public lastmisn_state
{
public:
	csilver_state(const machine_config &mconfig, device_type type, const char *tag) :
		lastmisn_state(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_soundbank(*this, "soundbank")
	{
	}

	void csilver(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void scroll_w(offs_t offset, u8 data);
	void control_w(u8 data);
	void adpcm_data_w(u8 data);
	void sound_bank_w(u8 data);
	void mcu_to_main_w(u8 data);
	u8 adpcm_reset_r();
	void adpcm_int(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<msm5205_device> m_msm;
	required_memory_bank m_soundbank;

	int      m_toggle = 0;
	int      m_msm5205next = 0;
};

#endif // MAME_DATAEAST_DEC8_H
