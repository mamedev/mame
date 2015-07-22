// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Angel Kids

*************************************************************************/

class angelkds_state : public driver_device
{
public:
	angelkds_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgtopvideoram(*this, "bgtopvideoram"),
		m_bgbotvideoram(*this, "bgbotvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_subcpu(*this, "sub"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bgtopvideoram;
	required_shared_ptr<UINT8> m_bgbotvideoram;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;

	tilemap_t    *m_tx_tilemap;
	tilemap_t    *m_bgbot_tilemap;
	tilemap_t    *m_bgtop_tilemap;
	int        m_txbank;
	int        m_bgbotbank;
	int        m_bgtopbank;

	UINT8      m_sound[4];
	UINT8      m_sound2[4];
	UINT8      m_layer_ctrl;

	/* devices */
	required_device<cpu_device> m_subcpu;
	DECLARE_WRITE8_MEMBER(angelkds_cpu_bank_write);
	DECLARE_READ8_MEMBER(angelkds_input_r);
	DECLARE_WRITE8_MEMBER(angelkds_main_sound_w);
	DECLARE_READ8_MEMBER(angelkds_main_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_sub_sound_w);
	DECLARE_READ8_MEMBER(angelkds_sub_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_txvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_txbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_layer_ctrl_write);
	DECLARE_WRITE8_MEMBER(angelkds_paletteram_w);
	DECLARE_DRIVER_INIT(angelkds);
	DECLARE_DRIVER_INIT(spcpostn);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bgtop_tile_info);
	TILE_GET_INFO_MEMBER(get_bgbot_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_angelkds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;
};
