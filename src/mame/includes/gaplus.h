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
	DECLARE_READ8_MEMBER(gaplus_spriteram_r);
	DECLARE_WRITE8_MEMBER(gaplus_spriteram_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_2_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_3_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_sreset_w);
	DECLARE_WRITE8_MEMBER(gaplus_freset_w);
	DECLARE_WRITE8_MEMBER(gaplus_customio_3_w);
	DECLARE_READ8_MEMBER(gaplus_customio_3_r);
	DECLARE_READ8_MEMBER(gaplus_videoram_r);
	DECLARE_WRITE8_MEMBER(gaplus_videoram_w);
	DECLARE_WRITE8_MEMBER(gaplus_starfield_control_w);
};


/*----------- defined in machine/gaplus.c -----------*/



/*----------- defined in video/gaplus.c -----------*/

VIDEO_START( gaplus );
PALETTE_INIT( gaplus );
SCREEN_UPDATE_IND16( gaplus );
SCREEN_VBLANK( gaplus );	/* update starfields */
