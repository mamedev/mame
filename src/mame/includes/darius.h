/*************************************************************************

    Darius

*************************************************************************/

#define DARIUS_VOL_MAX    (3*2 + 2)
#define DARIUS_PAN_MAX    (2 + 2 + 1)	/* FM 2port + PSG 2port + DA 1port */

class darius_state : public driver_device
{
public:
	darius_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    spriteram;
	UINT16 *    fg_ram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t  *fg_tilemap;

	/* misc */
	UINT16     cpua_ctrl;
	UINT16     coin_word;
	INT32      banknum;
	UINT8      adpcm_command;
	UINT8      nmi_enable;
	UINT32     def_vol[0x10];
	UINT8      vol[DARIUS_VOL_MAX];
	UINT8      pan[DARIUS_PAN_MAX];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *cpub;
	device_t *adpcm;
	device_t *tc0140syt;
	device_t *pc080sn;

	device_t *lscreen;
	device_t *mscreen;
	device_t *rscreen;

	device_t *filter0_0l;
	device_t *filter0_0r;
	device_t *filter0_1l;
	device_t *filter0_1r;
	device_t *filter0_2l;
	device_t *filter0_2r;
	device_t *filter0_3l;
	device_t *filter0_3r;
	device_t *filter1_0l;
	device_t *filter1_0r;
	device_t *filter1_1l;
	device_t *filter1_1r;
	device_t *filter1_2l;
	device_t *filter1_2r;
	device_t *filter1_3l;
	device_t *filter1_3r;
	device_t *msm5205_l;
	device_t *msm5205_r;
};


/*----------- defined in video/darius.c -----------*/

WRITE16_HANDLER( darius_fg_layer_w );

VIDEO_START( darius );
VIDEO_UPDATE( darius );
