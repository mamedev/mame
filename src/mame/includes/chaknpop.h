// license:BSD-3-Clause
// copyright-holders:BUT


#define MCU_INITIAL_SEED    0x81


class chaknpop_state : public driver_device
{
public:
	chaknpop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mcu_ram(*this, "mcu_ram"),
		m_tx_ram(*this, "tx_ram"),
		m_attr_ram(*this, "attr_ram"),
		m_spr_ram(*this, "spr_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_mcu_ram;
	required_shared_ptr<uint8_t> m_tx_ram;
	required_shared_ptr<uint8_t> m_attr_ram;
	required_shared_ptr<uint8_t> m_spr_ram;

	/* mcu-related */
	uint8_t m_mcu_seed;
	uint8_t m_mcu_select;
	uint8_t m_mcu_result;


	/* video-related */
	tilemap_t  *m_tx_tilemap;
	uint8_t    *m_vram1;
	uint8_t    *m_vram2;
	uint8_t    *m_vram3;
	uint8_t    *m_vram4;
	uint8_t    m_gfxmode;
	uint8_t    m_flip_x;
	uint8_t    m_flip_y;

	DECLARE_WRITE8_MEMBER(coinlock_w);
	DECLARE_READ8_MEMBER(mcu_port_a_r);
	DECLARE_READ8_MEMBER(mcu_port_b_r);
	DECLARE_READ8_MEMBER(mcu_port_c_r);
	DECLARE_WRITE8_MEMBER(mcu_port_a_w);
	DECLARE_WRITE8_MEMBER(mcu_port_b_w);
	DECLARE_WRITE8_MEMBER(mcu_port_c_w);
	DECLARE_READ8_MEMBER(gfxmode_r);
	DECLARE_WRITE8_MEMBER(gfxmode_w);
	DECLARE_WRITE8_MEMBER(txram_w);
	DECLARE_WRITE8_MEMBER(attrram_w);
	DECLARE_WRITE8_MEMBER(unknown_port_1_w);
	DECLARE_WRITE8_MEMBER(unknown_port_2_w);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(chaknpop);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tx_tilemap_mark_all_dirty();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_update_seed(uint8_t data);
};
