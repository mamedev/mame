// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Fire Trap

***************************************************************************/
#include "sound/msm5205.h"

class firetrap_state : public driver_device
{
public:
	firetrap_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bg1videoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t       *m_fg_tilemap;
	tilemap_t       *m_bg1_tilemap;
	tilemap_t       *m_bg2_tilemap;
	UINT8         m_scroll1_x[2];
	UINT8         m_scroll1_y[2];
	UINT8         m_scroll2_x[2];
	UINT8         m_scroll2_y[2];

	/* misc */
	int           m_sound_irq_enable;
	int           m_nmi_enable;
	int           m_i8751_return;
	int           m_i8751_current_command;
	int           m_i8751_init_ptr;
	int           m_msm5205next;
	int           m_adpcm_toggle;
	int           m_coin_command_pending;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(firetrap_nmi_disable_w);
	DECLARE_WRITE8_MEMBER(firetrap_bankselect_w);
	DECLARE_READ8_MEMBER(firetrap_8751_bootleg_r);
	DECLARE_READ8_MEMBER(firetrap_8751_r);
	DECLARE_WRITE8_MEMBER(firetrap_8751_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_command_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_2400_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_bankselect_w);
	DECLARE_WRITE8_MEMBER(firetrap_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(firetrap_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrolly_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrolly_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(firetrap);
	UINT32 screen_update_firetrap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(firetrap_irq);
	inline void get_bg_tile_info(tile_data &tileinfo, int tile_index, UINT8 *bgvideoram, int gfx_region);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(firetrap_adpcm_int);
};
