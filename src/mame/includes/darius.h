/*************************************************************************

    Darius

*************************************************************************/

#define DARIUS_VOL_MAX    (3*2 + 2)
#define DARIUS_PAN_MAX    (2 + 2 + 1)	/* FM 2port + PSG 2port + DA 1port */

typedef struct _darius_state darius_state;
struct _darius_state
{
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
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *cpub;
	const device_config *adpcm;
	const device_config *tc0140syt;
	const device_config *pc080sn;

	const device_config *lscreen;
	const device_config *mscreen;
	const device_config *rscreen;

	const device_config *filter0_0l;
	const device_config *filter0_0r;
	const device_config *filter0_1l;
	const device_config *filter0_1r;
	const device_config *filter0_2l;
	const device_config *filter0_2r;
	const device_config *filter0_3l;
	const device_config *filter0_3r;
	const device_config *filter1_0l;
	const device_config *filter1_0r;
	const device_config *filter1_1l;
	const device_config *filter1_1r;
	const device_config *filter1_2l;
	const device_config *filter1_2r;
	const device_config *filter1_3l;
	const device_config *filter1_3r;
	const device_config *msm5205_l;
	const device_config *msm5205_r;
};


/*----------- defined in video/darius.c -----------*/

WRITE16_HANDLER( darius_fg_layer_w );

VIDEO_START( darius );
VIDEO_UPDATE( darius );
