

class fuuki16_state : public driver_device
{
public:
	fuuki16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_vram_0;
	UINT16 *    m_vram_1;
	UINT16 *    m_vram_2;
	UINT16 *    m_vram_3;
	UINT16 *    m_vregs;
	UINT16 *    m_priority;
	UINT16 *    m_unknown;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	tilemap_t     *m_tilemap_2;
	tilemap_t     *m_tilemap_3;

	/* misc */
	emu_timer   *m_raster_interrupt_timer;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/fuukifg2.c -----------*/

WRITE16_HANDLER( fuuki16_vram_0_w );
WRITE16_HANDLER( fuuki16_vram_1_w );
WRITE16_HANDLER( fuuki16_vram_2_w );
WRITE16_HANDLER( fuuki16_vram_3_w );

VIDEO_START( fuuki16 );
SCREEN_UPDATE( fuuki16 );
