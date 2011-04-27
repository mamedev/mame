

class fantland_state : public driver_device
{
public:
	fantland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_spriteram;   // currently directly used in a 16bit map...
//  UINT8 *    m_spriteram_2; // currently directly used in a 16bit map...
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling

	/* misc */
	UINT8      m_nmi_enable;
	int        m_old_x[2];
	int        m_old_y[2];
	int        m_old_f[2];
	UINT8      m_input_ret[2];
	int        m_adpcm_playing[4];
	int        m_adpcm_addr[2][4];
	int        m_adpcm_nibble[4];

	/* devices */
	device_t *m_audio_cpu;
	device_t *m_msm1;
	device_t *m_msm2;
	device_t *m_msm3;
	device_t *m_msm4;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
};


/*----------- defined in video/fantland.c -----------*/

SCREEN_UPDATE( fantland );
