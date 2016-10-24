// license:GPL-2.0+
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

	tubep_state(const machine_config &mconfig, device_type type, const char *tag)
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

	uint8_t m_sound_latch;
	uint8_t m_ls74;
	uint8_t m_ls377;
	emu_timer *m_interrupt_timer;
	int m_curr_scanline;
	required_shared_ptr<uint8_t> m_textram;
	optional_shared_ptr<uint8_t> m_backgroundram;
	required_shared_ptr<uint8_t> m_sprite_colorsharedram;
	optional_shared_ptr<uint8_t> m_rjammer_backgroundram;
	std::unique_ptr<uint8_t[]> m_spritemap;
	uint8_t m_prom2[32];
	uint32_t m_romD_addr;
	uint32_t m_romEF_addr;
	uint32_t m_E16_add_b;
	uint32_t m_HINV;
	uint32_t m_VINV;
	uint32_t m_XSize;
	uint32_t m_YSize;
	uint32_t m_mark_1;
	uint32_t m_mark_2;
	uint32_t m_colorram_addr_hi;
	uint32_t m_ls273_g6;
	uint32_t m_ls273_j6;
	uint32_t m_romHI_addr_mid;
	uint32_t m_romHI_addr_msb;
	uint8_t m_DISP;
	uint8_t m_background_romsel;
	uint8_t m_color_A4;
	uint8_t m_ls175_b7;
	uint8_t m_ls175_e8;
	uint8_t m_ls377_data;
	uint32_t m_page;
	void tubep_LS259_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void main_cpu_irq_line_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void second_cpu_irq_line_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tubep_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tubep_sound_irq_ack(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tubep_sound_unknown(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_LS259_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rjammer_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rjammer_voice_input_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_voice_intensity_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_background_romselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_colorproms_A4_line_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_background_a000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_background_c000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tubep_sprite_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_background_LS377_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_background_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_voice_startstop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rjammer_voice_frequency_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portA_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portB_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portA_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portB_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portA_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portB_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_tubep();
	void machine_reset_tubep();
	void video_start_tubep();
	void video_reset_tubep();
	void palette_init_tubep(palette_device &palette);
	void machine_start_rjammer();
	void machine_reset_rjammer();
	void palette_init_rjammer(palette_device &palette);
	uint32_t screen_update_tubep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rjammer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tubep_scanline_callback(void *ptr, int32_t param);
	void rjammer_scanline_callback(void *ptr, int32_t param);
	void draw_sprite();
	void tubep_vblank_end();
	void tubep_setup_save_state();
	void rjammer_adpcm_vck(int state);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_mcu;
	optional_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;


protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
