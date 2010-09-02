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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *cpub;
	running_device *adpcm;
	running_device *tc0140syt;
	running_device *pc080sn;

	running_device *lscreen;
	running_device *mscreen;
	running_device *rscreen;

	running_device *filter0_0l;
	running_device *filter0_0r;
	running_device *filter0_1l;
	running_device *filter0_1r;
	running_device *filter0_2l;
	running_device *filter0_2r;
	running_device *filter0_3l;
	running_device *filter0_3r;
	running_device *filter1_0l;
	running_device *filter1_0r;
	running_device *filter1_1l;
	running_device *filter1_1r;
	running_device *filter1_2l;
	running_device *filter1_2r;
	running_device *filter1_3l;
	running_device *filter1_3r;
	running_device *msm5205_l;
	running_device *msm5205_r;
};


/*----------- defined in video/darius.c -----------*/

WRITE16_HANDLER( darius_fg_layer_w );

VIDEO_START( darius );
VIDEO_UPDATE( darius );
