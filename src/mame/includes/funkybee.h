

class funkybee_state : public driver_device
{
public:
	funkybee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;

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
};


/*----------- defined in video/funkybee.c -----------*/


PALETTE_INIT( funkybee );
VIDEO_START( funkybee );
SCREEN_UPDATE_IND16( funkybee );
