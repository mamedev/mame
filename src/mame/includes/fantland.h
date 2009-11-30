

typedef struct _fantland_state fantland_state;
struct _fantland_state
{
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
	const device_config *audio_cpu;
	const device_config *msm1;
	const device_config *msm2;
	const device_config *msm3;
	const device_config *msm4;
};


/*----------- defined in video/fantland.c -----------*/

VIDEO_UPDATE( fantland );
