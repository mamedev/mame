// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/
#ifndef MAME_KANEKO_KANEKO16_H
#define MAME_KANEKO_KANEKO16_H

#pragma once

#include "kaneko_calc3.h"
#include "kaneko_hit.h"
#include "kaneko_spr.h"
#include "kaneko_tmap.h"
#include "kaneko_toybox.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"


class kaneko16_state : public driver_device
{
public:
	kaneko16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ym2149(*this, "ym2149_%u", 1),
		m_oki(*this, "oki%u", 1),
		m_view2(*this, "view2_%u", 0),
		m_kaneko_hit(*this, "kan_hit"),
		m_kaneko_spr(*this, "kan_spr"),
		m_kaneko_calc3(*this, "calc3_prot"),
		m_toybox(*this, "toybox"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bgpalette(*this, "bgpalette"),
		m_eeprom(*this, "eeprom"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog"),
		m_spriteram(*this, "spriteram"),
		m_mainregion(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_mcuram(*this, "mcuram"),
		m_okiregion(*this, "oki%u", 1),
		m_okibank(*this, "okibank%u", 1),
		m_p2_port(*this, "P2"),
		m_fake_port(*this, "FAKE"),
		m_dsw_port(*this, "DSW1"),
		m_eepromout_port(*this, "EEPROMOUT"),
		m_wheel_port(*this, "WHEEL%u", 0U),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd")
	{
	}

	void init_bakubrkr();

	void bakubrkr(machine_config &config);
	void wingforc(machine_config &config);
	void blazeon(machine_config &config);
	void mgcrystl(machine_config &config);

protected:
	required_device<m68000_device> m_maincpu;
	optional_device_array<ym2149_device, 2> m_ym2149;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device_array<kaneko_view2_tilemap_device, 2> m_view2;
	optional_device<kaneko_hit_device> m_kaneko_hit;
	required_device<kaneko16_sprite_device> m_kaneko_spr;
	optional_device<kaneko_calc3_device> m_kaneko_calc3;
	optional_device<kaneko_toybox_device> m_toybox;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_bgpalette;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<watchdog_timer_device> m_watchdog;
	optional_device<buffered_spriteram16_device> m_spriteram;

	required_region_ptr<u16> m_mainregion;
	optional_shared_ptr<u16> m_mainram;
	optional_shared_ptr<u16> m_mcuram;

	optional_memory_region_array<2> m_okiregion;
	optional_memory_bank_array<2> m_okibank;

	optional_ioport m_p2_port;
	optional_ioport m_fake_port;
	optional_ioport m_dsw_port;
	optional_ioport m_eepromout_port;
	optional_ioport_array<3> m_wheel_port;

	u16 m_disp_enable = 0U;

	int m_VIEW2_2_pri = 0;

	virtual void common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize);
	void coin_lockout_w(u8 data);
	void bloodwar_coin_lockout_w(u8 data);

	void display_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0); // (u16 data, u16 mem_mask = ~0);

	template<unsigned Chip> u16 ym2149_r(offs_t offset);
	template<unsigned Chip> void ym2149_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<unsigned Mask> void oki_bank0_w(u8 data);
	template<unsigned Mask> void oki_bank1_w(u8 data);

	DECLARE_MACHINE_RESET(gtmr);
	DECLARE_MACHINE_RESET(mgcrystl);
	virtual void video_start() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template<class BitmapClass>
	u32 screen_update_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	template<class BitmapClass>
	void fill_bitmap(BitmapClass &bitmap, const rectangle &cliprect);

	void gtmr_oki1_map(address_map &map) ATTR_COLD;
	void gtmr_oki2_map(address_map &map) ATTR_COLD;

private:
	optional_device<z80_device> m_audiocpu;
	optional_device<ym2151_device> m_ymsnd;

	u8 eeprom_r();
	void eeprom_w(u8 data);
	void eeprom_cs_w(u8 data);

	void bakubrkr_map(address_map &map) ATTR_COLD;
	void bakubrkr_oki1_map(address_map &map) ATTR_COLD;
	void blazeon_map(address_map &map) ATTR_COLD;
	void blazeon_soundmem(address_map &map) ATTR_COLD;
	void blazeon_soundport(address_map &map) ATTR_COLD;
	void mgcrystl_map(address_map &map) ATTR_COLD;
	void wingforc_soundport(address_map &map) ATTR_COLD;
};

class kaneko16_gtmr_state : public kaneko16_state
{
public:
	kaneko16_gtmr_state(const machine_config &mconfig, device_type type, const char *tag) :
		kaneko16_state(mconfig, type, tag)
	{
	}

	void init_gtmr();
	void bonkadv(machine_config &config);
	void gtmr(machine_config &config);
	void gtmr2(machine_config &config);
	void gtmre(machine_config &config);
	void bloodwar(machine_config &config);

protected:
	virtual void common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize) override;

private:
	u16 gtmr_wheel_r();
	u16 gtmr2_wheel_r();
	u16 gtmr2_IN1_r();

	void bloodwar_map(address_map &map) ATTR_COLD;
	void bonkadv_map(address_map &map) ATTR_COLD;
	void gtmr2_map(address_map &map) ATTR_COLD;
	void gtmr_map(address_map &map) ATTR_COLD;
};

class kaneko16_berlwall_state : public kaneko16_state
{
public:
	kaneko16_berlwall_state(const machine_config &mconfig, device_type type, const char *tag) :
		kaneko16_state(mconfig, type, tag),
		m_bg15_scroll(*this, "bg15_scroll"),
		m_bgpalette(*this, "bgpalette")
	{
	}

	void init_berlwall();
	void init_berlwallk();
	void init_berlwallt();
	void init_berlwall_common();
	void berlwall(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	optional_shared_ptr<u16> m_bg15_scroll;
	required_device<palette_device> m_bgpalette;

	bitmap_ind16 m_bg15_bitmap[32];

	u8 bg15_select_r();
	void bg15_select_w(u8 data);
	u8 bg15_bright_r();
	void bg15_bright_w(u8 data);

	void berlwall_oki_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 berlwall_spriteram_r(offs_t offset);
	void berlwall_spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 berlwall_spriteregs_r(offs_t offset);
	void berlwall_spriteregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u8 m_bg15_select = 0U;
	u8 m_bg15_bright = 0U;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void render_15bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void patch_protection(u32 bra_offset,u16 bra_value,u16 checksum);
	void berlwall_map(address_map &map) ATTR_COLD;
};

class kaneko16_shogwarr_state : public kaneko16_state
{
public:
	kaneko16_shogwarr_state(const machine_config &mconfig, device_type type, const char *tag) :
		kaneko16_state(mconfig, type, tag),
		m_calc3_prot(*this, "calc3_prot")
	{
	}
	void init_shogwarr();
	void init_brapboys();

	void shogwarr(machine_config &config);
	void brapboys(machine_config &config);

private:
	optional_device<kaneko_calc3_device> m_calc3_prot;

	TIMER_DEVICE_CALLBACK_MEMBER(shogwarr_interrupt);

	void shogwarr_oki_bank_w(u8 data);

	void brapboys_oki2_map(address_map &map) ATTR_COLD;
	void shogwarr_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_KANEKO16_H
