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

	required_shared_ptr<uint8_t> m_nvram;

	/* machine state */
	uint8_t  m_a2d_select;
	uint8_t  m_nvram_enabled;
	emu_timer *m_interrupt_timer;

	/* video state */
	required_shared_ptr<uint8_t> m_foreground_bank;
	required_shared_ptr<uint8_t> m_video_off;
	required_shared_ptr<uint8_t> m_backgroundram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_foregroundram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_smoothing_table;
	uint32_t m_vscroll;
	uint32_t m_hscroll;

	/* audio state */
	uint8_t  m_audio_latch;
	uint8_t  m_audio_ack_latch;
	required_shared_ptr<uint8_t> m_audio_comm_stat;
	required_shared_ptr<uint8_t> m_speech_data;
	uint8_t  m_speech_strobe_state;
	void main_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rom_banksel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t a2d_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void a2d_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jedi_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nvram_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nvram_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jedi_vscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jedi_hscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value jedi_audio_comm_stat_r(ioport_field &field, void *param);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jedi_audio_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jedi_audio_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t audio_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t jedi_audio_ack_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void audio_ack_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void speech_strobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t speech_ready_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void speech_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void sound_reset() override;
	void video_start_jedi();
	uint32_t screen_update_jedi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void generate_interrupt(void *ptr, int32_t param);
	void delayed_audio_latch_w(void *ptr, int32_t param);
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
