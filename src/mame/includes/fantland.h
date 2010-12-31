

class fantland_state : public driver_device
{
public:
	fantland_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *    spriteram;   // currently directly used in a 16bit map...
//  UINT8 *    spriteram_2; // currently directly used in a 16bit map...
//  UINT8 *    paletteram;  // currently this uses generic palette handling

	/* misc */
	UINT8      nmi_enable;
	int        old_x[2], old_y[2], old_f[2];
	UINT8      input_ret[2];
	int        adpcm_playing[4];
	int        adpcm_addr[2][4];
	int        adpcm_nibble[4];

	/* devices */
	device_t *audio_cpu;
	device_t *msm1;
	device_t *msm2;
	device_t *msm3;
	device_t *msm4;
};


/*----------- defined in video/fantland.c -----------*/

VIDEO_UPDATE( fantland );
