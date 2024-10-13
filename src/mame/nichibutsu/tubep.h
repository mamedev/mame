// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#ifndef MAME_NICHIBUTSU_TUBEP_H
#define MAME_NICHIBUTSU_TUBEP_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "machine/gen_latch.h"
#include "machine/74157.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"

class tubep_state : public driver_device
{
public:
	tubep_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_slave(*this, "slave"),
		m_mcu(*this, "mcu"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_bgram(*this, "bgram"),
		m_sprite_cram(*this, "sprite_cram"),
		m_textram(*this, "textram"),
		m_bgrom(*this, "background"),
		m_spriterom(*this, "sprites"),
		m_textrom(*this, "text")
	{ }

	void tubepb(machine_config &config);
	void tubep(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(assert_sprite_int);

	void tubep_main_map(address_map &map) ATTR_COLD;
	void tubep_main_portmap(address_map &map) ATTR_COLD;
	void tubep_second_map(address_map &map) ATTR_COLD;
	void tubep_second_portmap(address_map &map) ATTR_COLD;
	void tubep_sound_map(address_map &map) ATTR_COLD;
	void tubep_sound_portmap(address_map &map) ATTR_COLD;

	void nsc_map(address_map &map) ATTR_COLD;

	void coin1_counter_w(int state);
	void coin2_counter_w(int state);
	void main_cpu_irq_line_clear_w(uint8_t data);
	void second_cpu_irq_line_clear_w(uint8_t data);
	uint8_t tubep_soundlatch_r();
	uint8_t tubep_sound_irq_ack();
	void tubep_textram_w(offs_t offset, uint8_t data);
	void screen_flip_w(int state);
	void background_romselect_w(int state);
	void colorproms_A4_line_w(int state);
	void background_a000_w(uint8_t data);
	void background_c000_w(uint8_t data);
	void sprite_control_w(offs_t offset, uint8_t data);

	void ay8910_portA_0_w(uint8_t data);
	void ay8910_portB_0_w(uint8_t data);
	void ay8910_portA_1_w(uint8_t data);
	void ay8910_portB_1_w(uint8_t data);
	void ay8910_portA_2_w(uint8_t data);
	void ay8910_portB_2_w(uint8_t data);
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;
	void palette_init(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual TIMER_CALLBACK_MEMBER(scanline_callback);
	void draw_sprite();
	void vblank_end();
	void setup_save_state();

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_slave;
	required_device<m6802_cpu_device> m_mcu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_sprite_cram;
	required_shared_ptr<uint8_t> m_textram;
	required_region_ptr<uint8_t> m_bgrom;
	required_region_ptr<uint8_t> m_spriterom;
	required_region_ptr<uint8_t> m_textrom;

	emu_timer *m_interrupt_timer = nullptr;
	emu_timer *m_sprite_timer = nullptr;
	int m_curr_scanline = 0;
	std::unique_ptr<uint8_t[]> m_spritemap;
	uint8_t m_prom2[32];
	uint32_t m_romD_addr = 0;
	uint32_t m_romEF_addr = 0;
	uint32_t m_E16_add_b = 0;
	uint32_t m_HINV = 0;
	uint32_t m_VINV = 0;
	uint32_t m_XSize = 0;
	uint32_t m_YSize = 0;
	uint32_t m_mark_1 = 0;
	uint32_t m_mark_2 = 0;
	uint32_t m_colorram_addr_hi = 0;
	uint32_t m_ls273_g6 = 0;
	uint32_t m_ls273_j6 = 0;
	uint32_t m_romHI_addr_mid = 0;
	uint32_t m_romHI_addr_msb = 0;
	uint8_t m_DISP = 0;
	uint8_t m_background_romsel = 0;
	uint8_t m_color_A4 = 0;
	uint8_t m_ls175_b7 = 0;
	uint8_t m_ls175_e8 = 0;
	uint8_t m_ls377_data = 0;
	uint32_t m_page = 0;
};

class rjammer_state : public tubep_state
{
public:
	rjammer_state(const machine_config &mconfig, device_type type, const char *tag) :
		tubep_state(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_adpcm_mux(*this, "adpcm_mux")
	{ }

	void rjammer(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual TIMER_CALLBACK_MEMBER(scanline_callback) override;

	void soundlatch_nmi_w(uint8_t data);

	void background_ls377_w(uint8_t data);
	void background_page_w(uint8_t data);
	void voice_startstop_w(uint8_t data);
	void voice_frequency_select_w(uint8_t data);

	void voice_input_w(uint8_t data);
	void voice_intensity_control_w(uint8_t data);

	void adpcm_vck_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void palette_init(palette_device &palette) const;

	void rjammer_main_map(address_map &map) ATTR_COLD;
	void rjammer_main_portmap(address_map &map) ATTR_COLD;
	void rjammer_second_map(address_map &map) ATTR_COLD;
	void rjammer_second_portmap(address_map &map) ATTR_COLD;
	void rjammer_sound_map(address_map &map) ATTR_COLD;
	void rjammer_sound_portmap(address_map &map) ATTR_COLD;

	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_mux;

	bool m_msm5205_toggle = 0;
};

#endif // MAME_NICHIBUTSU_TUBEP_H
