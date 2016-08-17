// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

Crazy Ballooon

*************************************************************************/

#include "sound/discrete.h"
#include "sound/sn76477.h"

#define CRBALOON_MASTER_XTAL    (XTAL_9_987MHz)


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_pc3092_data(*this, "pc3092_data"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_pc3092_data;
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_sn;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;

	UINT16 m_collision_address;
	UINT8 m_collision_address_clear;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(pc3092_w);
	DECLARE_READ8_MEMBER(pc3259_r);
	DECLARE_WRITE8_MEMBER(port_sound_w);
	DECLARE_WRITE8_MEMBER(crbaloon_videoram_w);
	DECLARE_WRITE8_MEMBER(crbaloon_colorram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(pc3092_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(crbaloon);
	UINT32 screen_update_crbaloon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	DECLARE_WRITE8_MEMBER(crbaloon_audio_set_music_freq);
	DECLARE_WRITE8_MEMBER(crbaloon_audio_set_music_enable);
	DECLARE_WRITE8_MEMBER(crbaloon_audio_set_laugh_enable);
	UINT16 crbaloon_get_collision_address();
	void crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear);
	void draw_sprite_and_check_collision(bitmap_ind16 &bitmap);
	void pc3092_reset(void);
	void pc3092_update();
	void pc3259_update(void);
	void crbaloon_audio_set_explosion_enable(int enabled);
	void crbaloon_audio_set_breath_enable(int enabled);
	void crbaloon_audio_set_appear_enable(int enabled);
};


/*----------- defined in audio/crbaloon.c -----------*/

MACHINE_CONFIG_EXTERN( crbaloon_audio );
