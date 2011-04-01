class tutankhm_state : public driver_device
{
public:
	tutankhm_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_paletteram;
	UINT8 *  m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	UINT8     m_flip_x;
	UINT8     m_flip_y;

	/* misc */
	UINT8    m_irq_toggle;
	UINT8    m_irq_enable;

	/* devices */
	cpu_device *m_maincpu;
};


/*----------- defined in video/tutankhm.c -----------*/

WRITE8_HANDLER( tutankhm_flip_screen_x_w );
WRITE8_HANDLER( tutankhm_flip_screen_y_w );

SCREEN_UPDATE( tutankhm );
