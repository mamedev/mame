
class simpsons_state : public driver_device
{
public:
	simpsons_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k053260;
	running_device *k052109;
	running_device *k053246;
	running_device *k053251;
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
