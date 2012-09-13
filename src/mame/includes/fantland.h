

class fantland_state : public driver_device
{
public:
	fantland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram", 0),
		m_spriteram2(*this, "spriteram2", 0){ }

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
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_spriteram2;
	DECLARE_WRITE8_MEMBER(fantland_nmi_enable_w);
	DECLARE_WRITE16_MEMBER(fantland_nmi_enable_16_w);
	DECLARE_WRITE8_MEMBER(fantland_soundlatch_w);
	DECLARE_WRITE16_MEMBER(fantland_soundlatch_16_w);
	DECLARE_READ16_MEMBER(spriteram_16_r);
	DECLARE_READ16_MEMBER(spriteram2_16_r);
	DECLARE_WRITE16_MEMBER(spriteram_16_w);
	DECLARE_WRITE16_MEMBER(spriteram2_16_w);
	DECLARE_WRITE8_MEMBER(borntofi_nmi_enable_w);
	DECLARE_READ8_MEMBER(borntofi_inputs_r);
	DECLARE_WRITE8_MEMBER(borntofi_msm5205_w);
	DECLARE_CUSTOM_INPUT_MEMBER(wheelrun_wheel_r);
	DECLARE_MACHINE_START(fantland);
	DECLARE_MACHINE_RESET(fantland);
	DECLARE_MACHINE_START(borntofi);
	DECLARE_MACHINE_RESET(borntofi);
};


/*----------- defined in video/fantland.c -----------*/

SCREEN_UPDATE_IND16( fantland );
