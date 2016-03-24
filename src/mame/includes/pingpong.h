// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
class pingpong_state : public driver_device
{
public:
	pingpong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_intenable;
	int m_question_addr_high;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(cashquiz_question_bank_high_w);
	DECLARE_WRITE8_MEMBER(cashquiz_question_bank_low_w);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(pingpong_videoram_w);
	DECLARE_WRITE8_MEMBER(pingpong_colorram_w);
	DECLARE_DRIVER_INIT(cashquiz);
	DECLARE_DRIVER_INIT(merlinmm);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pingpong);
	UINT32 screen_update_pingpong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(pingpong_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(merlinmm_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
