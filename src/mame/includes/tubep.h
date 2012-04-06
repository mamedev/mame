class tubep_state : public driver_device
{
public:
	tubep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_sound_latch;
	UINT8 m_ls74;
	UINT8 m_ls377;
	emu_timer *m_interrupt_timer;
	int m_curr_scanline;
	UINT8 *m_rjammer_backgroundram;
	UINT8 *m_backgroundram;
	UINT8 *m_textram;
	UINT8 *m_sprite_colorsharedram;
	UINT8 *m_spritemap;
	UINT8 m_prom2[32];
	UINT32 m_romD_addr;
	UINT32 m_romEF_addr;
	UINT32 m_E16_add_b;
	UINT32 m_HINV;
	UINT32 m_VINV;
	UINT32 m_XSize;
	UINT32 m_YSize;
	UINT32 m_mark_1;
	UINT32 m_mark_2;
	UINT32 m_colorram_addr_hi;
	UINT32 m_ls273_g6;
	UINT32 m_ls273_j6;
	UINT32 m_romHI_addr_mid;
	UINT32 m_romHI_addr_msb;
	UINT8 m_DISP;
	UINT8 m_background_romsel;
	UINT8 m_color_A4;
	UINT8 m_ls175_b7;
	UINT8 m_ls175_e8;
	UINT8 m_ls377_data;
	UINT32 m_page;
	DECLARE_WRITE8_MEMBER(tubep_LS259_w);
	DECLARE_WRITE8_MEMBER(main_cpu_irq_line_clear_w);
	DECLARE_WRITE8_MEMBER(tubep_soundlatch_w);
	DECLARE_WRITE8_MEMBER(second_cpu_irq_line_clear_w);
	DECLARE_READ8_MEMBER(tubep_soundlatch_r);
	DECLARE_READ8_MEMBER(tubep_sound_irq_ack);
	DECLARE_WRITE8_MEMBER(tubep_sound_unknown);
	DECLARE_WRITE8_MEMBER(rjammer_LS259_w);
	DECLARE_WRITE8_MEMBER(rjammer_soundlatch_w);
	DECLARE_READ8_MEMBER(rjammer_soundlatch_r);
	DECLARE_WRITE8_MEMBER(rjammer_voice_input_w);
	DECLARE_WRITE8_MEMBER(rjammer_voice_intensity_control_w);
	DECLARE_WRITE8_MEMBER(tubep_textram_w);
	DECLARE_WRITE8_MEMBER(tubep_background_romselect_w);
	DECLARE_WRITE8_MEMBER(tubep_colorproms_A4_line_w);
	DECLARE_WRITE8_MEMBER(tubep_background_a000_w);
	DECLARE_WRITE8_MEMBER(tubep_background_c000_w);
	DECLARE_WRITE8_MEMBER(tubep_sprite_control_w);
	DECLARE_WRITE8_MEMBER(rjammer_background_LS377_w);
	DECLARE_WRITE8_MEMBER(rjammer_background_page_w);
};


/*----------- defined in video/tubep.c -----------*/

void tubep_vblank_end(running_machine &machine);
PALETTE_INIT( tubep );
SCREEN_UPDATE_IND16( tubep );
PALETTE_INIT( rjammer );
SCREEN_UPDATE_IND16( rjammer );
VIDEO_START( tubep );
VIDEO_RESET( tubep );





