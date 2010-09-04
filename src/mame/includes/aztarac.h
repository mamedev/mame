/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

class aztarac_state : public driver_device
{
public:
	aztarac_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
};

/*----------- defined in audio/aztarac.c -----------*/

READ16_HANDLER( aztarac_sound_r );
WRITE16_HANDLER( aztarac_sound_w );

READ8_HANDLER( aztarac_snd_command_r );
READ8_HANDLER( aztarac_snd_status_r );
WRITE8_HANDLER( aztarac_snd_status_w );

INTERRUPT_GEN( aztarac_snd_timed_irq );


/*----------- defined in video/aztarac.c -----------*/

extern UINT16 *aztarac_vectorram;

WRITE16_HANDLER( aztarac_ubr_w );

VIDEO_START( aztarac );

