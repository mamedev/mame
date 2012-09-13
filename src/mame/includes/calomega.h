class calomega_state : public driver_device
{
public:
	calomega_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	UINT8 m_tx_line;
	UINT8 m_rx_line;
	int m_s903_mux_data;
	int m_s905_mux_data;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(calomega_videoram_w);
	DECLARE_WRITE8_MEMBER(calomega_colorram_w);
	DECLARE_WRITE_LINE_MEMBER(tx_rx_clk);
	DECLARE_READ8_MEMBER(s903_mux_port_r);
	DECLARE_WRITE8_MEMBER(s903_mux_w);
	DECLARE_READ8_MEMBER(s905_mux_port_r);
	DECLARE_WRITE8_MEMBER(s905_mux_w);
	DECLARE_READ8_MEMBER(pia0_ain_r);
	DECLARE_READ8_MEMBER(pia0_bin_r);
	DECLARE_WRITE8_MEMBER(pia0_aout_w);
	DECLARE_WRITE8_MEMBER(pia0_bout_w);
	DECLARE_WRITE8_MEMBER(pia0_ca2_w);
	DECLARE_READ8_MEMBER(pia1_ain_r);
	DECLARE_READ8_MEMBER(pia1_bin_r);
	DECLARE_WRITE8_MEMBER(pia1_aout_w);
	DECLARE_WRITE8_MEMBER(pia1_bout_w);
	DECLARE_WRITE8_MEMBER(ay_aout_w);
	DECLARE_WRITE8_MEMBER(ay_bout_w);
	DECLARE_WRITE8_MEMBER(lamps_903a_w);
	DECLARE_WRITE8_MEMBER(lamps_903b_w);
	DECLARE_WRITE8_MEMBER(lamps_905_w);
	DECLARE_READ_LINE_MEMBER(acia_rx_r);
	DECLARE_WRITE_LINE_MEMBER(acia_tx_w);
	DECLARE_DRIVER_INIT(elgrande);
	DECLARE_DRIVER_INIT(standard);
	DECLARE_DRIVER_INIT(comg080);
	DECLARE_DRIVER_INIT(jjpoker);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/calomega.c -----------*/



SCREEN_UPDATE_IND16( calomega );
