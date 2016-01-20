// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    Munch Mobile

*************************************************************************/

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_sprite_xpos(*this, "sprite_xpos"),
		m_sprite_tile(*this, "sprite_tile"),
		m_sprite_attr(*this, "sprite_attr"),
		m_videoram(*this, "videoram"),
		m_status_vram(*this, "status_vram"),
		m_vreg(*this, "vreg"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_sprite_xpos;
	required_shared_ptr<UINT8> m_sprite_tile;
	required_shared_ptr<UINT8> m_sprite_attr;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_status_vram;
	required_shared_ptr<UINT8> m_vreg;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	int          m_palette_bank;
	int          m_flipscreen;

	/* misc */
	int          m_nmi_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(mnchmobl_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_soundlatch_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_palette_bank_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_flipscreen_w);
	DECLARE_READ8_MEMBER(munchmo_ay1reset_r);
	DECLARE_READ8_MEMBER(munchmo_ay2reset_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(munchmo);
	UINT32 screen_update_mnchmobl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mnchmobl_vblank_irq);
	INTERRUPT_GEN_MEMBER(mnchmobl_sound_irq);
	void draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
