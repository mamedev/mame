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
	DECLARE_DRIVER_INIT(magicd2b);
	DECLARE_DRIVER_INIT(magicd2c);
	DECLARE_DRIVER_INIT(saloon);
	DECLARE_DRIVER_INIT(royalcdc);
	DECLARE_DRIVER_INIT(multiwin);
	DECLARE_DRIVER_INIT(mongolnw);
	DECLARE_DRIVER_INIT(soccernw);
	DECLARE_DRIVER_INIT(tabblue);
	DECLARE_DRIVER_INIT(dino4);
	DECLARE_DRIVER_INIT(ctunk);
	DECLARE_DRIVER_INIT(rcdino4);
	DECLARE_DRIVER_INIT(rcdinch);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_VIDEO_START(funworld);
	DECLARE_PALETTE_INIT(funworld);
	DECLARE_VIDEO_START(magicrd2);
	DECLARE_VIDEO_START(chinatow);
	DECLARE_MACHINE_START(lunapark);
	DECLARE_MACHINE_RESET(lunapark);
	uint32_t screen_update_funworld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void royalcd1(machine_config &config);
	void royalcd2(machine_config &config);
	void fw1stpal(machine_config &config);
	void chinatow(machine_config &config);
	void magicrd2(machine_config &config);
	void fw_a7_11(machine_config &config);
	void lunapark(machine_config &config);
	void fw2ndpal(machine_config &config);
	void saloon(machine_config &config);
	void cuoreuno(machine_config &config);
	void funquiz(machine_config &config);
	void rcdino4(machine_config &config);
	void intrgmes(machine_config &config);
	void witchryl(machine_config &config);
	void chinatow_map(address_map &map);
	void cuoreuno_map(address_map &map);
	void funquiz_map(address_map &map);
	void funworld_map(address_map &map);
	void fw_a7_11_map(address_map &map);
	void intergames_map(address_map &map);
	void lunapark_map(address_map &map);
	void magicrd2_map(address_map &map);
	void saloon_map(address_map &map);
	void witchryl_map(address_map &map);
};
