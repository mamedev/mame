
#define MASTER_CLOCK         XTAL_18_432MHz
#define SOUND_CLOCK          XTAL_3_579545MHz



class jackal_state : public driver_device
{
public:
	jackal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoctrl;
	UINT8 *  m_scrollram;
	UINT8 *  m_paletteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	int      m_irq_enable;
	UINT8    *m_rambank;
	UINT8    *m_spritebank;

	/* devices */
	device_t *m_mastercpu;
	device_t *m_slavecpu;
};


/*----------- defined in video/jackal.c -----------*/

void jackal_mark_tile_dirty(running_machine &machine, int offset);

PALETTE_INIT( jackal );
VIDEO_START( jackal );
SCREEN_UPDATE( jackal );
