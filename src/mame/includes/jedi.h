// license:BSD-3-Clause
// copyright-holders:Dan Boris, Aaron Giles
/*************************************************************************

    Atari Return of the Jedi hardware

*************************************************************************/


/* oscillators and clocks */
#define JEDI_MAIN_CPU_OSC       (XTAL_10MHz)
#define JEDI_AUDIO_CPU_OSC      (XTAL_12_096MHz)
#define JEDI_MAIN_CPU_CLOCK     (JEDI_MAIN_CPU_OSC / 4)
#define JEDI_AUDIO_CPU_CLOCK    (JEDI_AUDIO_CPU_OSC / 8)
#define JEDI_POKEY_CLOCK        (JEDI_AUDIO_CPU_CLOCK)
#define JEDI_TMS5220_CLOCK      (JEDI_AUDIO_CPU_OSC / 2 / 9) /* div by 9 is via a binary counter that counts from 7 to 16 */


class jedi_state : public driver_device
{
public:
	jedi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_nvram(*this, "nvram") ,
		m_foreground_bank(*this, "foreground_bank"),
		m_video_off(*this, "video_off"),
		m_backgroundram(*this, "backgroundram"),
		m_paletteram(*this, "paletteram"),
		m_foregroundram(*this, "foregroundram"),
		m_spriteram(*this, "spriteram"),
		m_smoothing_table(*this, "smoothing_table"),
		m_audio_comm_stat(*this, "audio_comm_stat"),
		m_speech_data(*this, "speech_data"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen") { }

	required_shared_ptr<UINT8> m_nvram;

	/* machine state */
	UINT8  m_a2d_select;
	UINT8  m_nvram_enabled;
	emu_timer *m_interrupt_timer;

	/* video state */
	required_shared_ptr<UINT8> m_foreground_bank;
	required_shared_ptr<UINT8> m_video_off;
	required_shared_ptr<UINT8> m_backgroundram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_foregroundram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_smoothing_table;
	UINT32 m_vscroll;
	UINT32 m_hscroll;

	/* audio state */
	UINT8  m_audio_latch;
	UINT8  m_audio_ack_latch;
	required_shared_ptr<UINT8> m_audio_comm_stat;
	required_shared_ptr<UINT8> m_speech_data;
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
	DECLARE_CUSTOM_INPUT_MEMBER(jedi_audio_comm_stat_r);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(jedi_audio_reset_w);
	DECLARE_WRITE8_MEMBER(jedi_audio_latch_w);
	DECLARE_READ8_MEMBER(audio_latch_r);
	DECLARE_READ8_MEMBER(jedi_audio_ack_latch_r);
	DECLARE_WRITE8_MEMBER(audio_ack_latch_w);
	DECLARE_WRITE8_MEMBER(speech_strobe_w);
	DECLARE_READ8_MEMBER(speech_ready_r);
	DECLARE_WRITE8_MEMBER(speech_reset_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void sound_reset() override;
	DECLARE_VIDEO_START(jedi);
	UINT32 screen_update_jedi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(generate_interrupt);
	TIMER_CALLBACK_MEMBER(delayed_audio_latch_w);
	void get_pens(pen_t *pens);
	void do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background_and_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
};

/*----------- defined in audio/jedi.c -----------*/
MACHINE_CONFIG_EXTERN( jedi_audio );

/*----------- defined in video/jedi.c -----------*/
MACHINE_CONFIG_EXTERN( jedi_video );
