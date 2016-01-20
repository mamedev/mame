// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Charles MacDonald, David Haywood
#include "video/deco16ic.h"
#include "video/decospr.h"
#include "machine/deco146.h"

class sshangha_state : public driver_device
{
public:
	sshangha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_deco146(*this, "ioprot"),
			m_deco_tilegen1(*this, "tilegen1"),
			m_spriteram(*this, "spriteram"),
			m_spriteram2(*this, "spriteram2"),
		m_sound_shared_ram(*this, "sound_shared"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_sprite_paletteram(*this, "sprite_palram"),
		m_tile_paletteram2(*this, "tile_palram2"),
		m_sprite_paletteram2(*this, "sprite_palram2"),
		m_tile_paletteram1(*this, "tile_palram1"),
		m_prot_data(*this, "prot_data"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette")  { }

	optional_device<deco146_device> m_deco146;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_spriteram2;

	required_shared_ptr<UINT16> m_sound_shared_ram;
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;

	required_shared_ptr<UINT16> m_sprite_paletteram;
	required_shared_ptr<UINT16> m_tile_paletteram2;
	required_shared_ptr<UINT16> m_sprite_paletteram2;
	required_shared_ptr<UINT16> m_tile_paletteram1;

	optional_shared_ptr<UINT16> m_prot_data;

	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;

	int m_video_control;
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	DECLARE_READ16_MEMBER( sshangha_protection_region_8_146_r );
	DECLARE_WRITE16_MEMBER( sshangha_protection_region_8_146_w );
	DECLARE_READ16_MEMBER( sshangha_protection_region_d_146_r );
	DECLARE_WRITE16_MEMBER( sshangha_protection_region_d_146_w );


	DECLARE_READ16_MEMBER(sshanghb_protection16_r);
	DECLARE_READ16_MEMBER(deco_71_r);
	DECLARE_READ8_MEMBER(sshangha_sound_shared_r);
	DECLARE_WRITE8_MEMBER(sshangha_sound_shared_w);
	DECLARE_WRITE16_MEMBER(paletteram16_xbgr_word_be_sprites2_w);
	DECLARE_WRITE16_MEMBER(paletteram16_xbgr_word_be_sprites_w);
	DECLARE_WRITE16_MEMBER(paletteram16_xbgr_word_be_tilelow_w);
	DECLARE_WRITE16_MEMBER(paletteram16_xbgr_word_be_tilehigh_w);
	DECLARE_WRITE16_MEMBER(sshangha_video_w);
	DECLARE_DRIVER_INIT(sshangha);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_sshangha(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void sshangha_set_color_888(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;
};
