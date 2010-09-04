/*************************************************************************

    Atari Return of the Jedi hardware

*************************************************************************/


/* oscillators and clocks */
#define JEDI_MAIN_CPU_OSC		(XTAL_10MHz)
#define JEDI_AUDIO_CPU_OSC		(XTAL_12_096MHz)
#define JEDI_MAIN_CPU_CLOCK		(JEDI_MAIN_CPU_OSC / 4)
#define JEDI_AUDIO_CPU_CLOCK	(JEDI_AUDIO_CPU_OSC / 8)
#define JEDI_POKEY_CLOCK		(JEDI_AUDIO_CPU_CLOCK)
#define JEDI_TMS5220_CLOCK		(JEDI_AUDIO_CPU_OSC / 2 / 9) /* div by 9 is via a binary counter that counts from 7 to 16 */


class jedi_state : public driver_device
{
public:
	jedi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8> m_nvram;

	/* machine state */
	UINT8  a2d_select;
	UINT8  nvram_enabled;
	emu_timer *interrupt_timer;

	/* video state */
	UINT8 *foregroundram;
	UINT8 *backgroundram;
	UINT8 *spriteram;
	UINT8 *paletteram;
	UINT8 *foreground_bank;
	UINT8 *video_off;
	UINT8 *smoothing_table;
	UINT32 vscroll;
	UINT32 hscroll;

	/* audio state */
	UINT8  audio_latch;
	UINT8  audio_ack_latch;
	UINT8 *audio_comm_stat;
	UINT8 *speech_data;
	UINT8  speech_strobe_state;
};


/*----------- defined in audio/jedi.c -----------*/

MACHINE_CONFIG_EXTERN( jedi_audio );

WRITE8_HANDLER( jedi_audio_reset_w );
WRITE8_HANDLER( jedi_audio_latch_w );
READ8_HANDLER( jedi_audio_ack_latch_r );
CUSTOM_INPUT( jedi_audio_comm_stat_r );


/*----------- defined in video/jedi.c -----------*/

MACHINE_CONFIG_EXTERN( jedi_video );

WRITE8_HANDLER( jedi_vscroll_w );
WRITE8_HANDLER( jedi_hscroll_w );
