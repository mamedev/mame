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
	jedi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8> m_nvram;

	/* machine state */
	UINT8  m_a2d_select;
	UINT8  m_nvram_enabled;
	emu_timer *m_interrupt_timer;

	/* video state */
	UINT8 *m_foregroundram;
	UINT8 *m_backgroundram;
	UINT8 *m_spriteram;
	UINT8 *m_paletteram;
	UINT8 *m_foreground_bank;
	UINT8 *m_video_off;
	UINT8 *m_smoothing_table;
	UINT32 m_vscroll;
	UINT32 m_hscroll;

	/* audio state */
	UINT8  m_audio_latch;
	UINT8  m_audio_ack_latch;
	UINT8 *m_audio_comm_stat;
	UINT8 *m_speech_data;
	UINT8  m_speech_strobe_state;
	DECLARE_WRITE8_MEMBER(main_irq_ack_w);
	DECLARE_WRITE8_MEMBER(rom_banksel_w);
	DECLARE_READ8_MEMBER(a2d_data_r);
	DECLARE_WRITE8_MEMBER(a2d_select_w);
	DECLARE_WRITE8_MEMBER(jedi_coin_counter_w);
	DECLARE_WRITE8_MEMBER(nvram_data_w);
	DECLARE_WRITE8_MEMBER(nvram_enable_w);
	DECLARE_WRITE8_MEMBER(jedi_vscroll_w);
	DECLARE_WRITE8_MEMBER(jedi_hscroll_w);
};


/*----------- defined in audio/jedi.c -----------*/

MACHINE_CONFIG_EXTERN( jedi_audio );

WRITE8_HANDLER( jedi_audio_reset_w );
WRITE8_HANDLER( jedi_audio_latch_w );
READ8_HANDLER( jedi_audio_ack_latch_r );
CUSTOM_INPUT( jedi_audio_comm_stat_r );


/*----------- defined in video/jedi.c -----------*/

MACHINE_CONFIG_EXTERN( jedi_video );

