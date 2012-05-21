class pooyan_state : public driver_device
{
public:
	pooyan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;

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
