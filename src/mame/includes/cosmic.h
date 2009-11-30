/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/

#define COSMICG_MASTER_CLOCK     XTAL_9_828MHz
#define Z80_MASTER_CLOCK         XTAL_10_816MHz


typedef struct _cosmic_state cosmic_state;
struct _cosmic_state
{
	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        spriteram;
	size_t         videoram_size;
	size_t         spriteram_size;

	/* video-related */
	pen_t          (*map_color)(running_machine *machine, UINT8 x, UINT8 y);
	int            color_registers[3];
	int            background_enable;
	int            magspot_pen_mask;

	/* sound-related */
	int            sound_enabled;
	int            march_select, gun_die_select, dive_bomb_b_select;

	/* misc */
	UINT32         pixel_clock;

	/* devices */
	const device_config *samples;
	const device_config *dac;
};


/*----------- defined in video/cosmic.c -----------*/

WRITE8_HANDLER( cosmic_color_register_w );
WRITE8_HANDLER( cosmic_background_enable_w );

PALETTE_INIT( panic );
PALETTE_INIT( cosmica );
PALETTE_INIT( cosmicg );
PALETTE_INIT( magspot );
PALETTE_INIT( nomnlnd );

VIDEO_UPDATE( panic );
VIDEO_UPDATE( magspot );
VIDEO_UPDATE( devzone );
VIDEO_UPDATE( cosmica );
VIDEO_UPDATE( cosmicg );
VIDEO_UPDATE( nomnlnd );
