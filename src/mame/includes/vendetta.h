/*************************************************************************

    Vendetta

*************************************************************************/

class vendetta_state : public driver_device
{
public:
	vendetta_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* misc */
	int        irq_enabled;
	offs_t     video_banking_base;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k053260;
	device_t *k052109;
	device_t *k053246;
	device_t *k053251;
	device_t *k054000;
};

/*----------- defined in video/vendetta.c -----------*/

extern void vendetta_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void esckids_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void vendetta_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_UPDATE( vendetta );
