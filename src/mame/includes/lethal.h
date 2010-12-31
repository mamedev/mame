/*************************************************************************

    Lethal Enforcers

*************************************************************************/

class lethal_state : public driver_device
{
public:
	lethal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[4], sprite_colorbase;

	/* misc */
	UINT8      cur_control2;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k054539;
	device_t *k056832;
	device_t *k053244;
	device_t *k054000;
};

/*----------- defined in video/lethal.c -----------*/

extern void lethalen_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void lethalen_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

WRITE8_HANDLER(lethalen_palette_control);

VIDEO_START(lethalen);
VIDEO_UPDATE(lethalen);
