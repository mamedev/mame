// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

#ifndef __KANEKO16_H__
#define __KANEKO16_H__

#include "machine/nvram.h"
#include "video/kan_pand.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"
#include "machine/eepromser.h"
#include "machine/kaneko_calc3.h"
#include "machine/kaneko_toybox.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"


class kaneko16_state : public driver_device
{
public:
	kaneko16_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_ym2149_1(*this, "ym2149_1"),
		m_ym2149_2(*this, "ym2149_2"),
		m_view2_0(*this, "view2_0"),
		m_view2_1(*this, "view2_1"),
		m_kaneko_spr(*this, "kan_spr"),
		m_pandora(*this, "pandora"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_spriteram(*this, "spriteram"),
		m_mainram(*this, "mainram")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	optional_device<ym2149_device> m_ym2149_1;
	optional_device<ym2149_device> m_ym2149_2;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;
	optional_device<kaneko_view2_tilemap_device> m_view2_1;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	optional_device<kaneko_pandora_device> m_pandora;
	required_device<palette_device> m_palette;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_mainram;

	UINT16 m_disp_enable;

	int m_VIEW2_2_pri;


	DECLARE_WRITE16_MEMBER(kaneko16_coin_lockout_w);
	DECLARE_WRITE16_MEMBER(kaneko16_soundlatch_w);
	DECLARE_WRITE16_MEMBER(kaneko16_eeprom_w);

	DECLARE_WRITE16_MEMBER(kaneko16_display_enable);

	DECLARE_READ16_MEMBER(kaneko16_ay1_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay1_YM2149_w);
	DECLARE_READ16_MEMBER(kaneko16_ay2_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay2_YM2149_w);
	DECLARE_WRITE16_MEMBER(bakubrkr_oki_bank_w);
	DECLARE_WRITE8_MEMBER(wingforc_oki_bank_w);

	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);

	DECLARE_DRIVER_INIT(kaneko16);
	DECLARE_DRIVER_INIT(samplebank);


	DECLARE_MACHINE_RESET(gtmr);
	DECLARE_VIDEO_START(kaneko16);
	DECLARE_MACHINE_RESET(mgcrystl);
	UINT32 screen_update_kaneko16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template<class _BitmapClass>
	UINT32 screen_update_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(kaneko16_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(shogwarr_interrupt);

	template<class _BitmapClass>
	void kaneko16_fill_bitmap(palette_device* palette, _BitmapClass &bitmap, const rectangle &cliprect);

	void kaneko16_common_oki_bank_w(  const char *bankname, std::string tag, int bank, size_t fixedsize, size_t bankedsize );
	void kaneko16_unscramble_tiles(const char *region);
	void kaneko16_expand_sample_banks(const char *region);
};

class kaneko16_gtmr_state : public kaneko16_state
{
public:
	kaneko16_gtmr_state(const machine_config &mconfig, device_type type, std::string tag)
		: kaneko16_state(mconfig, type, tag)
	{
	}

	DECLARE_WRITE16_MEMBER(bloodwar_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bloodwar_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(bloodwar_coin_lockout_w);
	DECLARE_READ16_MEMBER(gtmr_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_IN1_r);
	DECLARE_DRIVER_INIT(gtmr);

};



class kaneko16_berlwall_state : public kaneko16_state
{
public:
	kaneko16_berlwall_state(const machine_config &mconfig, device_type type, std::string tag)
		: kaneko16_state(mconfig, type, tag),
		m_bg15_select(*this, "bg15_select"),
		m_bg15_scroll(*this, "bg15_scroll"),
		m_bg15_bright(*this, "bg15_bright"),
		m_bgpalette(*this, "bgpalette")

	{
	}

	optional_shared_ptr<UINT16> m_bg15_select;
	optional_shared_ptr<UINT16> m_bg15_scroll;
	optional_shared_ptr<UINT16> m_bg15_bright;
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

	DECLARE_DRIVER_INIT(berlwall);
	DECLARE_PALETTE_INIT(berlwall);
	DECLARE_VIDEO_START(berlwall);
	UINT32 screen_update_berlwall(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void kaneko16_render_15bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

class kaneko16_shogwarr_state : public kaneko16_state
{
public:
	kaneko16_shogwarr_state(const machine_config &mconfig, device_type type, std::string tag)
		: kaneko16_state(mconfig, type, tag),
		m_calc3_prot(*this, "calc3_prot")
	{
	}

	optional_device<kaneko_calc3_device> m_calc3_prot;

	DECLARE_WRITE16_MEMBER(shogwarr_oki_bank_w);
	DECLARE_WRITE16_MEMBER(brapboys_oki_bank_w);

	DECLARE_DRIVER_INIT(shogwarr);
	DECLARE_DRIVER_INIT(brapboys);
};

#endif
