// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Uki
/***************************************************************************

    Aeroboto

***************************************************************************/

class aeroboto_state : public driver_device
{
public:
	aeroboto_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_hscroll(*this, "hscroll"),
		m_tilecolor(*this, "tilecolor"),
		m_spriteram(*this, "spriteram"),
		m_vscroll(*this, "vscroll"),
		m_starx(*this, "starx"),
		m_stary(*this, "stary"),
		m_bgcolor(*this, "bgcolor"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_mainram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_hscroll;
	required_shared_ptr<UINT8> m_tilecolor;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_vscroll;
	required_shared_ptr<UINT8> m_starx;
	required_shared_ptr<UINT8> m_stary;
	required_shared_ptr<UINT8> m_bgcolor;

	/* stars layout */
	UINT8 * m_stars_rom;
	int     m_stars_length;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_charbank;
	int     m_starsoff;
	int     m_sx;
	int     m_sy;
	UINT8   m_ox;
	UINT8   m_oy;

	/* misc */
	int m_count;
	int m_disable_irq;
	DECLARE_READ8_MEMBER(aeroboto_201_r);
	DECLARE_READ8_MEMBER(aeroboto_irq_ack_r);
	DECLARE_READ8_MEMBER(aeroboto_2973_r);
	DECLARE_WRITE8_MEMBER(aeroboto_1a2_w);
	DECLARE_READ8_MEMBER(aeroboto_in0_r);
	DECLARE_WRITE8_MEMBER(aeroboto_3000_w);
	DECLARE_WRITE8_MEMBER(aeroboto_videoram_w);
	DECLARE_WRITE8_MEMBER(aeroboto_tilecolor_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_aeroboto(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(aeroboto_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
