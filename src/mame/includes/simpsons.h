
typedef struct _simpsons_state simpsons_state;
struct _simpsons_state
{
	/* memory pointers */
	UINT8 *    ram;
	UINT8 *    xtraram;
	UINT16 *   spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        sprite_colorbase, layer_colorbase[3];
	int        layerpri[3];

	/* misc */
	int        firq_enabled;
	int        video_bank;
	//int        nmi_enabled;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k053260;
	const device_config *k052109;
	const device_config *k053246;
	const device_config *k053251;
};

/*----------- defined in machine/simpsons.c -----------*/

WRITE8_HANDLER( simpsons_eeprom_w );
WRITE8_HANDLER( simpsons_coin_counter_w );
READ8_HANDLER( simpsons_sound_interrupt_r );
READ8_DEVICE_HANDLER( simpsons_sound_r );
MACHINE_RESET( simpsons );
MACHINE_START( simpsons );

/*----------- defined in video/simpsons.c -----------*/

void simpsons_video_banking( running_machine *machine, int select );
VIDEO_UPDATE( simpsons );

extern void simpsons_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void simpsons_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
