/*************************************************************************

    Darius

*************************************************************************/

#define DARIUS_VOL_MAX    (3*2 + 2)
#define DARIUS_PAN_MAX    (2 + 2 + 1)	/* FM 2port + PSG 2port + DA 1port */

class darius_state : public driver_device
{
public:
	darius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_spriteram;
	UINT16 *    m_fg_ram;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t  *m_fg_tilemap;

	/* misc */
	UINT16     m_cpua_ctrl;
	UINT16     m_coin_word;
	INT32      m_banknum;
	UINT8      m_adpcm_command;
	UINT8      m_nmi_enable;
	UINT32     m_def_vol[0x10];
	UINT8      m_vol[DARIUS_VOL_MAX];
	UINT8      m_pan[DARIUS_PAN_MAX];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_cpub;
	device_t *m_adpcm;
	device_t *m_tc0140syt;
	device_t *m_pc080sn;

	device_t *m_lscreen;
	device_t *m_mscreen;
	device_t *m_rscreen;

	device_t *m_filter0_0l;
	device_t *m_filter0_0r;
	device_t *m_filter0_1l;
	device_t *m_filter0_1r;
	device_t *m_filter0_2l;
	device_t *m_filter0_2r;
	device_t *m_filter0_3l;
	device_t *m_filter0_3r;
	device_t *m_filter1_0l;
	device_t *m_filter1_0r;
	device_t *m_filter1_1l;
	device_t *m_filter1_1r;
	device_t *m_filter1_2l;
	device_t *m_filter1_2r;
	device_t *m_filter1_3l;
	device_t *m_filter1_3r;
	device_t *m_msm5205_l;
	device_t *m_msm5205_r;
};


/*----------- defined in video/darius.c -----------*/

WRITE16_HANDLER( darius_fg_layer_w );

VIDEO_START( darius );
SCREEN_UPDATE( darius );
