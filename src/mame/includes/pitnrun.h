// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Pierpaolo Prazzoli
class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;

	int m_nmi;
	UINT8 m_fromz80;
	UINT8 m_toz80;
	int m_zaccept;
	int m_zready;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	int m_address;
	int m_h_heed;
	int m_v_heed;
	int m_ha;
	int m_scroll;
	int m_char_bank;
	int m_color_select;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[4];
	tilemap_t *m_bg;
	tilemap_t *m_fg;

	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(hflip_w);
	DECLARE_WRITE8_MEMBER(vflip_w);
	DECLARE_READ8_MEMBER(mcu_data_r);
	DECLARE_WRITE8_MEMBER(mcu_data_w);
	DECLARE_READ8_MEMBER(mcu_status_r);
	DECLARE_READ8_MEMBER(m68705_portA_r);
	DECLARE_WRITE8_MEMBER(m68705_portA_w);
	DECLARE_READ8_MEMBER(m68705_portB_r);
	DECLARE_WRITE8_MEMBER(m68705_portB_w);
	DECLARE_READ8_MEMBER(m68705_portC_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(char_bank_select);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(ha_w);
	DECLARE_WRITE8_MEMBER(h_heed_w);
	DECLARE_WRITE8_MEMBER(v_heed_w);
	DECLARE_WRITE8_MEMBER(color_select_w);

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	INTERRUPT_GEN_MEMBER(nmi_source);
	TIMER_CALLBACK_MEMBER(mcu_real_data_r);
	TIMER_CALLBACK_MEMBER(mcu_real_data_w);
	TIMER_CALLBACK_MEMBER(mcu_data_real_r);
	TIMER_CALLBACK_MEMBER(mcu_status_real_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pitnrun);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spotlights();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
