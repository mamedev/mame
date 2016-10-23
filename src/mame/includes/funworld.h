// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Peter Ferrie
class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(questions_r);
	DECLARE_WRITE8_MEMBER(question_bank_w);
	DECLARE_WRITE8_MEMBER(funworld_videoram_w);
	DECLARE_WRITE8_MEMBER(funworld_colorram_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_a_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_b_w);
	DECLARE_WRITE_LINE_MEMBER(pia1_ca2_w);
	DECLARE_READ8_MEMBER(funquiz_ay8910_a_r);
	DECLARE_READ8_MEMBER(funquiz_ay8910_b_r);
	DECLARE_READ8_MEMBER(chinatow_r_32f0);
	void init_magicd2b();
	void init_magicd2c();
	void init_saloon();
	void init_royalcdc();
	void init_multiwin();
	void init_mongolnw();
	void init_soccernw();
	void init_tabblue();
	void init_dino4();
	void init_ctunk();
	void init_rcdino4();
	void init_rcdinch();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void video_start_funworld();
	DECLARE_PALETTE_INIT(funworld);
	void video_start_magicrd2();
	void video_start_chinatow();
	void machine_start_lunapark();
	void machine_reset_lunapark();
	uint32_t screen_update_funworld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
