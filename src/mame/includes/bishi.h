/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/

#define CPU_CLOCK       (XTAL_24MHz / 2)		/* 68000 clock */
#define SOUND_CLOCK     XTAL_16_9344MHz		/* YMZ280 clock */

class bishi_state : public driver_device
{
public:
	bishi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[4];

	/* misc */
	UINT16     cur_control, cur_control2;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k007232;
	device_t *k056832;
	device_t *k054338;
	device_t *k055555;
};

/*----------- defined in video/bishi.c -----------*/

extern void bishi_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START(bishi);
VIDEO_UPDATE(bishi);
