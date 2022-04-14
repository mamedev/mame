// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#ifndef MAME_INCLUDES_TUBEP_H
#define MAME_INCLUDES_TUBEP_H

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
		m_textram(*this, "textram"),
		m_backgroundram(*this, "backgroundram"),
		m_sprite_colorsharedram(*this, "sprite_color")
	{ }

	void tubepb(machine_config &config);
	void tubep(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	enum
	{
		TIMER_TUBEP_SCANLINE,
		TIMER_RJAMMER_SCANLINE,
		TIMER_SPRITE
	};

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_slave;
	required_device<m6802_cpu_device> m_mcu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_textram;
	optional_shared_ptr<uint8_t> m_backgroundram;
	required_shared_ptr<uint8_t> m_sprite_colorsharedram;

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
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	void main_cpu_irq_line_clear_w(uint8_t data);
	void second_cpu_irq_line_clear_w(uint8_t data);
	uint8_t tubep_soundlatch_r();
	uint8_t tubep_sound_irq_ack();
	void tubep_textram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(screen_flip_w);
	DECLARE_WRITE_LINE_MEMBER(background_romselect_w);
	DECLARE_WRITE_LINE_MEMBER(colorproms_A4_line_w);
	void tubep_background_a000_w(uint8_t data);
	void tubep_background_c000_w(uint8_t data);
	void tubep_sprite_control_w(offs_t offset, uint8_t data);

	void ay8910_portA_0_w(uint8_t data);
	void ay8910_portB_0_w(uint8_t data);
	void ay8910_portA_1_w(uint8_t data);
	void ay8910_portB_1_w(uint8_t data);
	void ay8910_portA_2_w(uint8_t data);
	void ay8910_portB_2_w(uint8_t data);
	virtual void video_start() override;
	virtual void video_reset() override;
	void tubep_palette(palette_device &palette);
	uint32_t screen_update_tubep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(tubep_scanline_callback);
	TIMER_CALLBACK_MEMBER(rjammer_scanline_callback); // called from common device_timer function so can't be moved out of here yet
	void draw_sprite();
	void tubep_vblank_end();
	void tubep_setup_save_state();

	void nsc_map(address_map &map);

	void tubep_main_map(address_map &map);
	void tubep_main_portmap(address_map &map);
	void tubep_second_map(address_map &map);
	void tubep_second_portmap(address_map &map);
	void tubep_sound_map(address_map &map);
	void tubep_sound_portmap(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};

class rjammer_state : public tubep_state
{
public:
	rjammer_state(const machine_config &mconfig, device_type type, const char *tag) :
		tubep_state(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_adpcm_mux(*this, "adpcm_mux"),
		m_rjammer_backgroundram(*this, "rjammer_bgram")
	{ }

	void rjammer(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void soundlatch_nmi_w(uint8_t data);

	void rjammer_background_LS377_w(uint8_t data);
	void rjammer_background_page_w(uint8_t data);
	void rjammer_voice_startstop_w(uint8_t data);
	void rjammer_voice_frequency_select_w(uint8_t data);

	void rjammer_voice_input_w(uint8_t data);
	void rjammer_voice_intensity_control_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(rjammer_adpcm_vck_w);

	uint32_t screen_update_rjammer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void rjammer_palette(palette_device &palette) const;

	void rjammer_main_map(address_map &map);
	void rjammer_main_portmap(address_map &map);
	void rjammer_second_map(address_map &map);
	void rjammer_second_portmap(address_map &map);
	void rjammer_sound_map(address_map &map);
	void rjammer_sound_portmap(address_map &map);

	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_mux;
	required_shared_ptr<uint8_t> m_rjammer_backgroundram;

	bool m_msm5205_toggle = 0;
};

#endif // MAME_INCLUDES_TUBEP_H
