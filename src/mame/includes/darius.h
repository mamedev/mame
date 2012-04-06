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
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE16_MEMBER(darius_watchdog_w);
	DECLARE_READ16_MEMBER(darius_ioc_r);
	DECLARE_WRITE16_MEMBER(darius_ioc_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(adpcm_command_w);
	DECLARE_WRITE8_MEMBER(display_value);
	DECLARE_WRITE8_MEMBER(darius_fm0_pan);
	DECLARE_WRITE8_MEMBER(darius_fm1_pan);
	DECLARE_WRITE8_MEMBER(darius_psg0_pan);
	DECLARE_WRITE8_MEMBER(darius_psg1_pan);
	DECLARE_WRITE8_MEMBER(darius_da_pan);
	DECLARE_READ8_MEMBER(adpcm_command_read);
	DECLARE_READ8_MEMBER(readport2);
	DECLARE_READ8_MEMBER(readport3);
	DECLARE_WRITE8_MEMBER(adpcm_nmi_disable);
	DECLARE_WRITE8_MEMBER(adpcm_nmi_enable);
	DECLARE_WRITE16_MEMBER(darius_fg_layer_w);
};


/*----------- defined in video/darius.c -----------*/


VIDEO_START( darius );
SCREEN_UPDATE_IND16( darius_left );
SCREEN_UPDATE_IND16( darius_middle );
SCREEN_UPDATE_IND16( darius_right );
