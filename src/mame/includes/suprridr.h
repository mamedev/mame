// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

class suprridr_state : public driver_device
{
public:
	suprridr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_nmi_enable;
	UINT8 m_sound_data;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_noscroll;
	UINT8 m_flipx;
	UINT8 m_flipy;

	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_WRITE8_MEMBER(coin_lock_w);
	DECLARE_WRITE8_MEMBER(flipx_w);
	DECLARE_WRITE8_MEMBER(flipy_w);
	DECLARE_WRITE8_MEMBER(fgdisable_w);
	DECLARE_WRITE8_MEMBER(fgscrolly_w);
	DECLARE_WRITE8_MEMBER(bgscrolly_w);
	DECLARE_WRITE8_MEMBER(bgram_w);
	DECLARE_WRITE8_MEMBER(fgram_w);
	DECLARE_READ8_MEMBER(sound_data_r);

	DECLARE_CUSTOM_INPUT_MEMBER(control_r);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	INTERRUPT_GEN_MEMBER(main_nmi_gen);
	TIMER_CALLBACK_MEMBER(delayed_sound_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(suprridr);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int is_screen_flipped();
};
