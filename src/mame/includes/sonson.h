/*************************************************************************

    Son Son

*************************************************************************/

class sonson_state : public driver_device
{
public:
	sonson_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_last_irq;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(sonson_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sonson_coin1_counter_w);
	DECLARE_WRITE8_MEMBER(sonson_coin2_counter_w);
	DECLARE_WRITE8_MEMBER(sonson_videoram_w);
	DECLARE_WRITE8_MEMBER(sonson_colorram_w);
	DECLARE_WRITE8_MEMBER(sonson_scrollx_w);
	DECLARE_WRITE8_MEMBER(sonson_flipscreen_w);
};


/*----------- defined in video/sonson.c -----------*/


PALETTE_INIT( sonson );
VIDEO_START( sonson );
SCREEN_UPDATE_IND16( sonson );
