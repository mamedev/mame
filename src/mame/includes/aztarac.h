/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

class aztarac_state : public driver_device
{
public:
	aztarac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
	int m_sound_status;
	UINT16 *m_vectorram;
	int m_xcenter;
	int m_ycenter;
	DECLARE_READ16_MEMBER(nvram_r);
	DECLARE_READ16_MEMBER(joystick_r);
	DECLARE_WRITE16_MEMBER(aztarac_ubr_w);
};

/*----------- defined in audio/aztarac.c -----------*/

READ16_HANDLER( aztarac_sound_r );
WRITE16_HANDLER( aztarac_sound_w );

READ8_HANDLER( aztarac_snd_command_r );
READ8_HANDLER( aztarac_snd_status_r );
WRITE8_HANDLER( aztarac_snd_status_w );

INTERRUPT_GEN( aztarac_snd_timed_irq );


/*----------- defined in video/aztarac.c -----------*/


VIDEO_START( aztarac );

