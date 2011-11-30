#define MAX_STARS			250

struct star {
	float x,y;
	int col,set;
};


class gaplus_state : public driver_device
{
public:
	gaplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_customio_3;
	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	tilemap_t *m_bg_tilemap;
	UINT8 m_starfield_control[4];
	int m_total_stars;
	struct star m_stars[MAX_STARS];
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;
};


/*----------- defined in machine/gaplus.c -----------*/

WRITE8_HANDLER( gaplus_customio_3_w );
READ8_HANDLER( gaplus_customio_3_r );


/*----------- defined in video/gaplus.c -----------*/

READ8_HANDLER( gaplus_videoram_r );
WRITE8_HANDLER( gaplus_videoram_w );
WRITE8_HANDLER( gaplus_starfield_control_w );
VIDEO_START( gaplus );
PALETTE_INIT( gaplus );
SCREEN_UPDATE( gaplus );
SCREEN_EOF( gaplus );	/* update starfields */
