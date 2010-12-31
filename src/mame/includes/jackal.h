
#define MASTER_CLOCK         XTAL_18_432MHz
#define SOUND_CLOCK          XTAL_3_579545MHz



class jackal_state : public driver_device
{
public:
	jackal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoctrl;
	UINT8 *  scrollram;
	UINT8 *  paletteram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	int      irq_enable;
	UINT8    *rambank;
	UINT8    *spritebank;

	/* devices */
	device_t *mastercpu;
	device_t *slavecpu;
};


/*----------- defined in video/jackal.c -----------*/

void jackal_mark_tile_dirty(running_machine *machine, int offset);

PALETTE_INIT( jackal );
VIDEO_START( jackal );
VIDEO_UPDATE( jackal );
