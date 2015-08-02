// license:BSD-3-Clause
// copyright-holders:David Haywood

class fitfight_state : public driver_device
{
public:
	fitfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fof_100000(*this, "fof_100000"),
		m_fof_600000(*this, "fof_600000"),
		m_fof_700000(*this, "fof_700000"),
		m_fof_800000(*this, "fof_800000"),
		m_fof_900000(*this, "fof_900000"),
		m_fof_a00000(*this, "fof_a00000"),
		m_fof_bak_tileram(*this, "fof_bak_tileram"),
		m_fof_mid_tileram(*this, "fof_mid_tileram"),
		m_fof_txt_tileram(*this, "fof_txt_tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fof_100000;
	required_shared_ptr<UINT16> m_fof_600000;
	required_shared_ptr<UINT16> m_fof_700000;
	required_shared_ptr<UINT16> m_fof_800000;
	required_shared_ptr<UINT16> m_fof_900000;
	required_shared_ptr<UINT16> m_fof_a00000;
	required_shared_ptr<UINT16> m_fof_bak_tileram;
	required_shared_ptr<UINT16> m_fof_mid_tileram;
	required_shared_ptr<UINT16> m_fof_txt_tileram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t  *m_fof_bak_tilemap;
	tilemap_t  *m_fof_mid_tilemap;
	tilemap_t  *m_fof_txt_tilemap;

	/* misc */
	int      m_bbprot_kludge;
	UINT16   m_fof_700000_data;
	DECLARE_READ16_MEMBER(fitfight_700000_r);
	DECLARE_READ16_MEMBER(histryma_700000_r);
	DECLARE_READ16_MEMBER(bbprot_700000_r);
	DECLARE_WRITE16_MEMBER(fitfight_700000_w);
	DECLARE_READ8_MEMBER(snd_porta_r);
	DECLARE_READ8_MEMBER(snd_portb_r);
	DECLARE_READ8_MEMBER(snd_portc_r);
	DECLARE_WRITE8_MEMBER(snd_porta_w);
	DECLARE_WRITE8_MEMBER(snd_portb_w);
	DECLARE_WRITE8_MEMBER(snd_portc_w);
	DECLARE_WRITE16_MEMBER(fof_bak_tileram_w);
	DECLARE_WRITE16_MEMBER(fof_mid_tileram_w);
	DECLARE_WRITE16_MEMBER(fof_txt_tileram_w);

	DECLARE_READ16_MEMBER( hotmindff_unk_r );
	DECLARE_DRIVER_INIT(hotmindff);
	DECLARE_DRIVER_INIT(fitfight);
	DECLARE_DRIVER_INIT(histryma);
	DECLARE_DRIVER_INIT(bbprot);
	TILE_GET_INFO_MEMBER(get_fof_bak_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_txt_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_fitfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(snd_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
