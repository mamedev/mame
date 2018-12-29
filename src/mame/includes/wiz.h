// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Seibu Stinger/Wiz hardware

***************************************************************************/
#ifndef MAME_INCLUDES_WIZ_H
#define MAME_INCLUDES_WIZ_H

#pragma once

#include "sound/discrete.h"
#include "emupal.h"

class wiz_state : public driver_device
{
public:
	wiz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_attrram(*this, "attrram"),
		m_attrram2(*this, "attrram2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void wiz(machine_config &config);
	void kungfut(machine_config &config);
	void scion(machine_config &config);
	void stinger(machine_config &config);

	void init_stinger();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_attrram;
	required_shared_ptr<uint8_t> m_attrram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	int32_t m_flipx;
	int32_t m_flipy;
	int32_t m_bgcolor;
	uint8_t m_charbank[2];
	uint8_t m_palbank[2];
	uint8_t m_main_nmi_mask;
	uint8_t m_sound_nmi_mask;
	uint8_t m_sprite_bank;

	int m_dsc0;
	int m_dsc1;

	DECLARE_READ8_MEMBER(wiz_protection_r);
	DECLARE_WRITE8_MEMBER(wiz_coin_counter_w);
	DECLARE_WRITE8_MEMBER(wiz_main_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_palette_bank_w);
	DECLARE_WRITE8_MEMBER(wiz_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(wiz_bgcolor_w);
	DECLARE_WRITE8_MEMBER(wiz_char_bank_w);
	DECLARE_WRITE8_MEMBER(wiz_flipx_w);
	DECLARE_WRITE8_MEMBER(wiz_flipy_w);
	DECLARE_WRITE8_MEMBER(stinger_explosion_w);
	DECLARE_WRITE8_MEMBER(stinger_shot_w);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	void wiz_palette(palette_device &palette) const;
	uint32_t screen_update_wiz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stinger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kungfut(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(wiz_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(wiz_sound_interrupt);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int charbank, int colortype);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int set, int charbank);

	void decrypted_opcodes_map(address_map &map);
	void kungfut_main_map(address_map &map);
	void kungfut_sound_map(address_map &map);
	void stinger_main_map(address_map &map);
	void stinger_sound_map(address_map &map);
	void wiz_main_map(address_map &map);
};

#endif // MAME_INCLUDES_WIZ_H
