// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

#ifndef MAME_INCLUDES_KANEKO16_H
#define MAME_INCLUDES_KANEKO16_H

#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"
#include "machine/eepromser.h"
#include "machine/kaneko_calc3.h"
#include "machine/kaneko_toybox.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"


class kaneko16_state : public driver_device
{
public:
	kaneko16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym2149(*this, "ym2149_%u", 1),
		m_oki(*this, "oki%u", 1),
		m_view2(*this, "view2_%u", 0),
		m_kaneko_spr(*this, "kan_spr"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_mainram(*this, "mainram"),
		m_okiregion(*this, "oki%u", 1),
		m_okibank(*this, "okibank%u", 1)
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<ym2149_device, 2> m_ym2149;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device_array<kaneko_view2_tilemap_device, 2> m_view2;
	required_device<kaneko16_sprite_device> m_kaneko_spr;
	required_device<palette_device> m_palette;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_mainram;

	optional_memory_region_array<2> m_okiregion;
	optional_memory_bank_array<2> m_okibank;

	uint16_t m_disp_enable;

	int m_VIEW2_2_pri;

	void kaneko16_common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize);
	DECLARE_WRITE16_MEMBER(kaneko16_coin_lockout_w);
	DECLARE_WRITE16_MEMBER(kaneko16_eeprom_w);

	DECLARE_WRITE16_MEMBER(kaneko16_display_enable);

	template<int Chip> DECLARE_READ16_MEMBER(kaneko16_ay_YM2149_r);
	template<int Chip> DECLARE_WRITE16_MEMBER(kaneko16_ay_YM2149_w);
	template<int Mask> DECLARE_WRITE8_MEMBER(oki_bank0_w);
	template<int Mask> DECLARE_WRITE8_MEMBER(oki_bank1_w);

	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);

	void init_kaneko16();
	void init_bakubrkr();


	DECLARE_MACHINE_RESET(gtmr);
	DECLARE_VIDEO_START(kaneko16);
	DECLARE_MACHINE_RESET(mgcrystl);
	uint32_t screen_update_kaneko16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template<class _BitmapClass>
	uint32_t screen_update_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(kaneko16_interrupt);

	template<class _BitmapClass>
	void kaneko16_fill_bitmap(_BitmapClass &bitmap, const rectangle &cliprect);

	void kaneko16_unscramble_tiles(const char *region);
	void bakubrkr(machine_config &config);
	void wingforc(machine_config &config);
	void blazeon(machine_config &config);
	void mgcrystl(machine_config &config);
	void bakubrkr(address_map &map);
	void bakubrkr_oki1_map(address_map &map);
	void blazeon(address_map &map);
	void blazeon_soundmem(address_map &map);
	void blazeon_soundport(address_map &map);
	void mgcrystl(address_map &map);
	void wingforc_soundport(address_map &map);
	void gtmr_oki1_map(address_map &map);
	void gtmr_oki2_map(address_map &map);
};

class kaneko16_gtmr_state : public kaneko16_state
{
public:
	kaneko16_gtmr_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag)
	{
	}

	DECLARE_WRITE16_MEMBER(bloodwar_coin_lockout_w);
	DECLARE_READ16_MEMBER(gtmr_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_IN1_r);
	void init_gtmr();
	void kaneko16_common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize);

	void bonkadv(machine_config &config);
	void gtmr(machine_config &config);
	void gtmr2(machine_config &config);
	void gtmre(machine_config &config);
	void bloodwar(machine_config &config);
	void bloodwar(address_map &map);
	void bonkadv(address_map &map);
	void gtmr2_map(address_map &map);
	void gtmr_map(address_map &map);
};



class kaneko16_berlwall_state : public kaneko16_state
{
public:
	kaneko16_berlwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_bg15_select(*this, "bg15_select"),
		m_bg15_scroll(*this, "bg15_scroll"),
		m_bg15_bright(*this, "bg15_bright"),
		m_bgpalette(*this, "bgpalette")

	{
	}

	optional_shared_ptr<uint16_t> m_bg15_select;
	optional_shared_ptr<uint16_t> m_bg15_scroll;
	optional_shared_ptr<uint16_t> m_bg15_bright;
	required_device<palette_device> m_bgpalette;

	bitmap_ind16 m_bg15_bitmap[32];

	DECLARE_READ16_MEMBER(kaneko16_bg15_select_r);
	DECLARE_WRITE16_MEMBER(kaneko16_bg15_select_w);
	DECLARE_READ16_MEMBER(kaneko16_bg15_bright_r);
	DECLARE_WRITE16_MEMBER(kaneko16_bg15_bright_w);

	DECLARE_READ16_MEMBER(berlwall_oki_r);
	DECLARE_WRITE16_MEMBER(berlwall_oki_w);

	DECLARE_READ16_MEMBER(berlwall_spriteram_r);
	DECLARE_WRITE16_MEMBER(berlwall_spriteram_w);
	DECLARE_READ16_MEMBER(berlwall_spriteregs_r);
	DECLARE_WRITE16_MEMBER(berlwall_spriteregs_w);

	void init_berlwall();
	void init_berlwallk();
	void init_berlwallt();
	void init_berlwall_common();
	DECLARE_PALETTE_INIT(berlwall);
	DECLARE_VIDEO_START(berlwall);
	uint32_t screen_update_berlwall(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void kaneko16_render_15bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void patch_protection(uint32_t bra_offset,uint16_t bra_value,uint16_t checksum);
	void berlwall(machine_config &config);
	void berlwall(address_map &map);
};

class kaneko16_shogwarr_state : public kaneko16_state
{
public:
	kaneko16_shogwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_calc3_prot(*this, "calc3_prot")
	{
	}

	DECLARE_WRITE16_MEMBER(shogwarr_oki_bank_w);

	void init_shogwarr();
	void init_brapboys();

	TIMER_DEVICE_CALLBACK_MEMBER(shogwarr_interrupt);

	void shogwarr(machine_config &config);
	void brapboys(machine_config &config);
	void brapboys_oki2_map(address_map &map);
	void shogwarr(address_map &map);
private:

	optional_device<kaneko_calc3_device> m_calc3_prot;
};

#endif
