// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina,David Haywood


class freekick_state : public driver_device
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_freek_tilemap;

	/* misc */
	int        m_inval;
	int        m_outval;
	int        m_cnt;   // used by oigas
	int        m_romaddr;
	int        m_spinner;
	int        m_nmi_en;
	int        m_ff_data;
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(spinner_select_w);
	DECLARE_READ8_MEMBER(spinner_r);
	DECLARE_WRITE8_MEMBER(pbillrd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(oigas_5_w);
	DECLARE_READ8_MEMBER(oigas_3_r);
	DECLARE_READ8_MEMBER(oigas_2_r);
	DECLARE_READ8_MEMBER(freekick_ff_r);
	DECLARE_WRITE8_MEMBER(freekick_ff_w);
	DECLARE_WRITE8_MEMBER(freek_videoram_w);
	DECLARE_WRITE8_MEMBER(snd_rom_addr_l_w);
	DECLARE_WRITE8_MEMBER(snd_rom_addr_h_w);
	DECLARE_READ8_MEMBER(snd_rom_r);
	DECLARE_DRIVER_INIT(gigas);
	DECLARE_DRIVER_INIT(gigasb);
	DECLARE_DRIVER_INIT(pbillrds);
	TILE_GET_INFO_MEMBER(get_freek_tile_info);
	virtual void video_start() override;
	DECLARE_MACHINE_START(pbillrd);
	DECLARE_MACHINE_RESET(freekick);
	DECLARE_MACHINE_START(freekick);
	DECLARE_MACHINE_START(oigas);
	DECLARE_MACHINE_RESET(oigas);
	UINT32 screen_update_pbillrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_freekick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gigas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(freekick_irqgen);
	void gigas_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pbillrd_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void freekick_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_memory_bank m_bank1, m_bank1d;
};
