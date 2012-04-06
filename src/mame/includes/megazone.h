/*************************************************************************

    Megazone

*************************************************************************/

class megazone_state : public driver_device
{
public:
	megazone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *       m_scrollx;
	UINT8 *       m_scrolly;
	UINT8 *       m_videoram;
	UINT8 *       m_colorram;
	UINT8 *       m_videoram2;
	UINT8 *       m_colorram2;
	UINT8 *       m_spriteram;
	size_t        m_spriteram_size;
	size_t        m_videoram_size;
	size_t        m_videoram2_size;

	/* video-related */
	bitmap_ind16      *m_tmpbitmap;
	int           m_flipscreen;

	/* misc */
	int           m_i8039_status;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_daccpu;

	UINT8         m_irq_mask;
	DECLARE_WRITE8_MEMBER(megazone_i8039_irq_w);
	DECLARE_WRITE8_MEMBER(i8039_irqen_and_status_w);
	DECLARE_WRITE8_MEMBER(megazone_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(megazone_flipscreen_w);
};



/*----------- defined in video/megazone.c -----------*/


PALETTE_INIT( megazone );
VIDEO_START( megazone );
SCREEN_UPDATE_IND16( megazone );
