

class funkybee_state : public driver_device
{
public:
	funkybee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_gfx_bank;
	DECLARE_READ8_MEMBER(funkybee_input_port_0_r);
	DECLARE_WRITE8_MEMBER(funkybee_coin_counter_w);
	DECLARE_WRITE8_MEMBER(funkybee_videoram_w);
	DECLARE_WRITE8_MEMBER(funkybee_colorram_w);
	DECLARE_WRITE8_MEMBER(funkybee_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(funkybee_scroll_w);
	DECLARE_WRITE8_MEMBER(funkybee_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(funkybee_tilemap_scan);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/funkybee.c -----------*/




SCREEN_UPDATE_IND16( funkybee );
