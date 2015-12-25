// license:???
// copyright-holders:David Haywood,insideoutboy, Pierpaolo Prazzoli
/***************************************************************************

    Popper

***************************************************************************/

class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ol_videoram(*this, "ol_videoram"),
		m_videoram(*this, "videoram"),
		m_ol_attribram(*this, "ol_attribram"),
		m_attribram(*this, "attribram"),
		m_spriteram(*this, "spriteram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ol_videoram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_ol_attribram;
	required_shared_ptr<UINT8> m_attribram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_p123_tilemap;
	tilemap_t *m_p0_tilemap;
	tilemap_t *m_ol_p123_tilemap;
	tilemap_t *m_ol_p0_tilemap;
	INT32 m_flipscreen;
	INT32 m_e002;
	INT32 m_gfx_bank;
	rectangle m_tilemap_clip;

	/* devices */
	required_device<cpu_device> m_audiocpu;

	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(popper_input_ports_r);
	DECLARE_READ8_MEMBER(popper_soundcpu_nmi_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(popper_ol_videoram_w);
	DECLARE_WRITE8_MEMBER(popper_videoram_w);
	DECLARE_WRITE8_MEMBER(popper_ol_attribram_w);
	DECLARE_WRITE8_MEMBER(popper_attribram_w);
	DECLARE_WRITE8_MEMBER(popper_flipscreen_w);
	DECLARE_WRITE8_MEMBER(popper_e002_w);
	DECLARE_WRITE8_MEMBER(popper_gfx_bank_w);
	TILE_GET_INFO_MEMBER(get_popper_p123_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_p0_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_ol_p123_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_ol_p0_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(popper);
	UINT32 screen_update_popper(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
