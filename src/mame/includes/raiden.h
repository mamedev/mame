// license:???
// copyright-holders:Oliver Bergmann, Bryan McPhail, Randy Mongenel
/*******************************************************************************

    Seibu Raiden hardware

*******************************************************************************/

#include "audio/seibu.h"
#include "video/bufsprite.h"

class raiden_state : public driver_device
{
public:
	raiden_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_shared_ram(*this, "shared_ram"),
		m_videoram(*this, "videoram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<UINT16> m_shared_ram;
	required_shared_ptr<UINT16> m_videoram;
	optional_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;

	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	UINT8 m_bg_layer_enabled;
	UINT8 m_fg_layer_enabled;
	UINT8 m_tx_layer_enabled;
	UINT8 m_sp_layer_enabled;
	UINT8 m_flipscreen;
	UINT16 m_raidenb_scroll_ram[6];

	DECLARE_WRITE16_MEMBER(raiden_background_w);
	DECLARE_WRITE16_MEMBER(raiden_foreground_w);
	DECLARE_WRITE16_MEMBER(raiden_text_w);
	DECLARE_WRITE8_MEMBER(raiden_control_w);
	DECLARE_WRITE8_MEMBER(raidenb_control_w);
	DECLARE_WRITE16_MEMBER(raidenb_layer_enable_w);
	DECLARE_WRITE16_MEMBER(raidenb_layer_scroll_w);
	DECLARE_DRIVER_INIT(raidenu);
	DECLARE_DRIVER_INIT(raidenk);
	DECLARE_DRIVER_INIT(raiden);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	virtual void video_start() override;
	DECLARE_VIDEO_START(raidenb);

	UINT32 screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *scrollregs);
	UINT32 screen_update_raiden(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_raidenb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(raiden_interrupt);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask);
	void common_decrypt();
};
