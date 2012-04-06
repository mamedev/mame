class pooyan_state : public driver_device
{
public:
	pooyan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT8    m_irq_toggle;
	UINT8    m_irq_enable;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(pooyan_videoram_w);
	DECLARE_WRITE8_MEMBER(pooyan_colorram_w);
	DECLARE_WRITE8_MEMBER(pooyan_flipscreen_w);
};


/*----------- defined in video/pooyan.c -----------*/


PALETTE_INIT( pooyan );
VIDEO_START( pooyan );
SCREEN_UPDATE_IND16( pooyan );
