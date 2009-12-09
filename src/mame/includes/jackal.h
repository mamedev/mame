
#define MASTER_CLOCK         XTAL_18_432MHz
#define SOUND_CLOCK          XTAL_3_579545MHz



typedef struct _jackal_state jackal_state;
struct _jackal_state
{
	/* memory pointers */
	UINT8 *  videoctrl;
	UINT8 *  scrollram;
	UINT8 *  paletteram;

	/* video-related */
	tilemap  *bg_tilemap;

	/* misc */
	int      irq_enable;
	UINT8    *rambank;
	UINT8    *spritebank;

	/* devices */
	const device_config *mastercpu;
	const device_config *slavecpu;
};


/*----------- defined in video/jackal.c -----------*/

void jackal_mark_tile_dirty(running_machine *machine, int offset);

PALETTE_INIT( jackal );
VIDEO_START( jackal );
VIDEO_UPDATE( jackal );
