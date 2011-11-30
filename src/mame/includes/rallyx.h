struct jungler_star
{
	int x, y, color;
};

#define JUNGLER_MAX_STARS 1000

class rallyx_state : public driver_device
{
public:
	rallyx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;
	UINT8 *  m_radarattr;
	UINT8 *  m_radarx;
	UINT8 *  m_radary;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int      m_last_bang;
	int      m_spriteram_base;
	int      m_stars_enable;
	int      m_total_stars;
	UINT8    m_drawmode_table[4];
	struct jungler_star m_stars[JUNGLER_MAX_STARS];

	/* devices */
	cpu_device *m_maincpu;
	device_t *m_samples;

	UINT8    m_main_irq_mask;
};


/*----------- defined in video/rallyx.c -----------*/

WRITE8_HANDLER( rallyx_videoram_w );
WRITE8_HANDLER( rallyx_scrollx_w );
WRITE8_HANDLER( rallyx_scrolly_w );
WRITE8_HANDLER( tactcian_starson_w );

PALETTE_INIT( rallyx );
PALETTE_INIT( jungler );
VIDEO_START( rallyx );
VIDEO_START( jungler );
VIDEO_START( locomotn );
VIDEO_START( commsega );
SCREEN_UPDATE( rallyx );
SCREEN_UPDATE( jungler );
SCREEN_UPDATE( locomotn );
