#include "video/deco16ic.h"

class sshangha_state : public driver_device
{
public:
	sshangha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		 m_deco_tilegen1(*this, "tilegen1"),
		 m_spriteram(*this, "spriteram"),
		 m_spriteram2(*this, "spriteram2")
		{ }

	UINT16 *m_prot_data;
	UINT16 *m_sound_shared_ram;
	int m_video_control;

	UINT16* m_pf1_rowscroll;
	UINT16* m_pf2_rowscroll;

	UINT16* m_sprite_paletteram;
	UINT16* m_sprite_paletteram2;
	UINT16* m_tile_paletteram1;
	UINT16* m_tile_paletteram2;

	required_device<deco16ic_device> m_deco_tilegen1;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_spriteram2;
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
};


/*----------- defined in video/sshangha.c -----------*/

VIDEO_START( sshangha );
SCREEN_UPDATE_RGB32( sshangha );

