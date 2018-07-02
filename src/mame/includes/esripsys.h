// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Entertainment Sciences RIP System hardware

*************************************************************************/

#ifndef MAME_INCLUDES_ESRIPSYS_H
#define MAME_INCLUDES_ESRIPSYS_H

#pragma once

#include "cpu/esrip/esrip.h"
#include "sound/dac.h"
#include "sound/tms5220.h"
#include "screen.h"

/* TODO */
#define ESRIPSYS_PIXEL_CLOCK    (XTAL(25'000'000) / 2)
#define ESRIPSYS_HTOTAL         (512 + 141 + 2)
#define ESRIPSYS_HBLANK_START   (512)
#define ESRIPSYS_HBLANK_END     (0)
#define ESRIPSYS_VTOTAL         (384 + 20)
#define ESRIPSYS_VBLANK_START   (384)
#define ESRIPSYS_VBLANK_END     (0)

#define CMOS_RAM_SIZE           (2048)
#define FDT_RAM_SIZE            (2048 * sizeof(uint16_t))

struct line_buffer_t
{
	std::unique_ptr<uint8_t[]> colour_buf;
	std::unique_ptr<uint8_t[]> intensity_buf;
	std::unique_ptr<uint8_t[]> priority_buf;
};

class esripsys_state : public driver_device
{
public:
	esripsys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_framecpu(*this, "frame_cpu"),
			m_videocpu(*this, "video_cpu"),
			m_gamecpu(*this, "game_cpu"),
			m_soundcpu(*this, "sound_cpu"),
			m_tms(*this, "tms5220nl"),
			m_pal_ram(*this, "pal_ram"),
			m_dac(*this, "dac"),
			m_screen(*this, "screen") { }

	void esripsys(machine_config &config);

	void init_esripsys();

	DECLARE_INPUT_CHANGED_MEMBER(keypad_interrupt);
	DECLARE_INPUT_CHANGED_MEMBER(coin_interrupt);

private:
	required_device<cpu_device> m_framecpu;
	required_device<esrip_device> m_videocpu;
	required_device<cpu_device> m_gamecpu;
	required_device<cpu_device> m_soundcpu;
	required_device<tms5220_device> m_tms;

	uint8_t m_g_iodata;
	uint8_t m_g_ioaddr;
	uint8_t m_coin_latch;
	uint8_t m_keypad_status;
	uint8_t m_g_status;
	uint8_t m_f_status;
	int m_io_firq_status;
	uint8_t m_cmos_ram_a2_0;
	uint8_t m_cmos_ram_a10_3;
	std::unique_ptr<uint8_t[]> m_cmos_ram;
	uint8_t m_u56a;
	uint8_t m_u56b;
	uint8_t m_g_to_s_latch1;
	uint8_t m_g_to_s_latch2;
	uint8_t m_s_to_g_latch1;
	uint8_t m_s_to_g_latch2;
	uint8_t m_dac_msb;
	uint8_t m_tms_data;
	std::unique_ptr<uint8_t[]> m_fdt_a;
	std::unique_ptr<uint8_t[]> m_fdt_b;
	struct line_buffer_t m_line_buffer[2];
	int m_fasel;
	int m_fbsel;
	int m_hblank;
	required_shared_ptr<uint8_t> m_pal_ram;
	int m_frame_vbl;
	int m_12sel;
	int m_video_firq_en;
	emu_timer *m_hblank_end_timer;
	emu_timer *m_hblank_start_timer;
	std::unique_ptr<uint8_t[]> m_fig_scale_table;
	std::unique_ptr<uint8_t[]> m_scale_table;
	int m_video_firq;
	uint8_t m_bg_intensity;
	DECLARE_WRITE8_MEMBER(uart_w);
	DECLARE_READ8_MEMBER(uart_r);
	DECLARE_READ8_MEMBER(g_status_r);
	DECLARE_WRITE8_MEMBER(g_status_w);
	DECLARE_READ8_MEMBER(f_status_r);
	DECLARE_WRITE8_MEMBER(f_status_w);
	DECLARE_WRITE8_MEMBER(frame_w);
	DECLARE_READ8_MEMBER(fdt_r);
	DECLARE_WRITE8_MEMBER(fdt_w);
	DECLARE_READ16_MEMBER( fdt_rip_r );
	DECLARE_WRITE16_MEMBER( fdt_rip_w );
	DECLARE_READ8_MEMBER(rip_status_in);
	DECLARE_WRITE8_MEMBER(g_iobus_w);
	DECLARE_READ8_MEMBER(g_iobus_r);
	DECLARE_WRITE8_MEMBER(g_ioadd_w);
	DECLARE_READ8_MEMBER(s_200e_r);
	DECLARE_WRITE8_MEMBER(s_200e_w);
	DECLARE_WRITE8_MEMBER(s_200f_w);
	DECLARE_READ8_MEMBER(s_200f_r);
	DECLARE_READ8_MEMBER(tms5220_r);
	DECLARE_WRITE8_MEMBER(tms5220_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(esripsys_bg_intensity_w);
	DECLARE_WRITE_LINE_MEMBER(ptm_irq);
	DECLARE_WRITE8_MEMBER(esripsys_dac_w);
	virtual void video_start() override;
	uint32_t screen_update_esripsys(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(esripsys_vblank_irq);
	TIMER_CALLBACK_MEMBER(delayed_bank_swap);
	TIMER_CALLBACK_MEMBER(hblank_start_callback);
	TIMER_CALLBACK_MEMBER(hblank_end_callback);
	required_device<dac_word_interface> m_dac;
	required_device<screen_device> m_screen;
	ESRIP_DRAW(esripsys_draw);
	void frame_cpu_map(address_map &map);
	void game_cpu_map(address_map &map);
	void sound_cpu_map(address_map &map);
	void video_cpu_map(address_map &map);
};

#endif // MAME_INCLUDES_ESRIPSYS_H
