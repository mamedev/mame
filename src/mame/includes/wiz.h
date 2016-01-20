// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Seibu Stinger/Wiz hardware

***************************************************************************/

#include "sound/discrete.h"

class wiz_state : public driver_device
{
public:
	wiz_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
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

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_attrram;
	required_shared_ptr<UINT8> m_attrram2;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	INT32 m_flipx;
	INT32 m_flipy;
	INT32 m_bgcolor;
	UINT8 m_charbank[2];
	UINT8 m_palbank[2];
	UINT8 m_main_nmi_mask;
	UINT8 m_sound_nmi_mask;
	UINT8 m_sprite_bank;

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

	DECLARE_DRIVER_INIT(stinger);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(wiz);
	UINT32 screen_update_wiz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stinger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kungfut(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(wiz_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(wiz_sound_interrupt);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int charbank, int colortype);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int set, int charbank);
};
