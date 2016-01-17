// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Knuckle Joe

*************************************************************************/

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"),
		m_soundcpu(*this, "soundcpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollregs;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tile_bank;
	int         m_sprite_bank;
	int        m_flipscreen;

	/* misc */
	UINT8      m_port1;
	UINT8      m_port2;

	/* devices */
	required_device<cpu_device> m_soundcpu;
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_WRITE8_MEMBER(kncljoe_videoram_w);
	DECLARE_WRITE8_MEMBER(kncljoe_control_w);
	DECLARE_WRITE8_MEMBER(kncljoe_scroll_w);
	DECLARE_WRITE8_MEMBER(m6803_port1_w);
	DECLARE_WRITE8_MEMBER(m6803_port2_w);
	DECLARE_READ8_MEMBER(m6803_port1_r);
	DECLARE_READ8_MEMBER(m6803_port2_r);
	DECLARE_WRITE8_MEMBER(unused_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(kncljoe);
	UINT32 screen_update_kncljoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_nmi);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
