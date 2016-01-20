// license:???
// copyright-holders:Jarek Burczynski
#include "sound/msm5205.h"

class tubep_state : public driver_device
{
public:
	enum
	{
		TIMER_TUBEP_SCANLINE,
		TIMER_RJAMMER_SCANLINE,
		TIMER_SPRITE
	};

	tubep_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_textram(*this, "textram"),
		m_backgroundram(*this, "backgroundram"),
		m_sprite_colorsharedram(*this, "sprite_color"),
		m_rjammer_backgroundram(*this, "rjammer_bgram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_slave(*this, "slave"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_screen(*this, "screen") { }

	UINT8 m_sound_latch;
	UINT8 m_ls74;
	UINT8 m_ls377;
	emu_timer *m_interrupt_timer;
	int m_curr_scanline;
	required_shared_ptr<UINT8> m_textram;
	optional_shared_ptr<UINT8> m_backgroundram;
	required_shared_ptr<UINT8> m_sprite_colorsharedram;
	optional_shared_ptr<UINT8> m_rjammer_backgroundram;
	std::unique_ptr<UINT8[]> m_spritemap;
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
	DECLARE_WRITE8_MEMBER(rjammer_voice_startstop_w);
	DECLARE_WRITE8_MEMBER(rjammer_voice_frequency_select_w);
	DECLARE_WRITE8_MEMBER(ay8910_portA_0_w);
	DECLARE_WRITE8_MEMBER(ay8910_portB_0_w);
	DECLARE_WRITE8_MEMBER(ay8910_portA_1_w);
	DECLARE_WRITE8_MEMBER(ay8910_portB_1_w);
	DECLARE_WRITE8_MEMBER(ay8910_portA_2_w);
	DECLARE_WRITE8_MEMBER(ay8910_portB_2_w);
	DECLARE_MACHINE_START(tubep);
	DECLARE_MACHINE_RESET(tubep);
	DECLARE_VIDEO_START(tubep);
	DECLARE_VIDEO_RESET(tubep);
	DECLARE_PALETTE_INIT(tubep);
	DECLARE_MACHINE_START(rjammer);
	DECLARE_MACHINE_RESET(rjammer);
	DECLARE_PALETTE_INIT(rjammer);
	UINT32 screen_update_tubep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rjammer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(tubep_scanline_callback);
	TIMER_CALLBACK_MEMBER(rjammer_scanline_callback);
	void draw_sprite();
	void tubep_vblank_end();
	void tubep_setup_save_state();
	DECLARE_WRITE_LINE_MEMBER(rjammer_adpcm_vck);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_mcu;
	optional_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;


protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
