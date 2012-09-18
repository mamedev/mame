#include "video/deco16ic.h"

class sshangha_state : public driver_device
{
public:
	sshangha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_prot_data(*this, "prot_data"){ }

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
	int m_video_control;



	DECLARE_WRITE16_MEMBER(sshangha_protection16_w);
	DECLARE_READ16_MEMBER(sshangha_protection16_r);
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
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_sshangha(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
