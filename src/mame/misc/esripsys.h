// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Entertainment Sciences RIP System hardware

*************************************************************************/

#ifndef MAME_MISC_ESRIPSYS_H
#define MAME_MISC_ESRIPSYS_H

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


class esripsys_state : public driver_device
{
public:
	esripsys_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_framecpu(*this, "frame_cpu"),
		m_videocpu(*this, "video_cpu"),
		m_gamecpu(*this, "game_cpu"),
		m_soundcpu(*this, "sound_cpu"),
		m_tms(*this, "tms5220nl"),
		m_pal_ram(*this, "pal_ram"),
		m_dac(*this, "dac"),
		m_screen(*this, "screen")
	{ }

	void esripsys(machine_config &config);

	void init_esripsys();

	DECLARE_INPUT_CHANGED_MEMBER(keypad_interrupt);
	DECLARE_INPUT_CHANGED_MEMBER(coin_interrupt);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_framecpu;
	required_device<esrip_device> m_videocpu;
	required_device<cpu_device> m_gamecpu;
	required_device<cpu_device> m_soundcpu;
	required_device<tms5220_device> m_tms;
	required_shared_ptr<uint8_t> m_pal_ram;
	required_device<dac_word_interface> m_dac;
	required_device<screen_device> m_screen;

	uint8_t m_g_iodata = 0U;
	uint8_t m_g_ioaddr = 0U;
	uint8_t m_coin_latch = 0U;
	uint8_t m_keypad_status = 0U;
	uint8_t m_g_status = 0U;
	uint8_t m_f_status = 0U;
	int m_io_firq_status = 0;
	uint8_t m_cmos_ram_a2_0 = 0U;
	uint8_t m_cmos_ram_a10_3 = 0U;
	std::unique_ptr<uint8_t[]> m_cmos_ram;
	uint8_t m_u56a = 0U;
	uint8_t m_u56b = 0U;
	uint8_t m_g_to_s_latch1 = 0U;
	uint8_t m_g_to_s_latch2 = 0U;
	uint8_t m_s_to_g_latch1 = 0U;
	uint8_t m_s_to_g_latch2 = 0U;
	uint8_t m_dac_msb = 0U;
	uint8_t m_tms_data = 0U;
	std::unique_ptr<uint8_t[]> m_fdt_a;
	std::unique_ptr<uint8_t[]> m_fdt_b;

	struct line_buffer_t
	{
		std::unique_ptr<uint8_t[]> colour_buf;
		std::unique_ptr<uint8_t[]> intensity_buf;
		std::unique_ptr<uint8_t[]> priority_buf;
	};

	struct line_buffer_t m_line_buffer[2]{};
	int m_fasel = 0;
	int m_fbsel = 0;
	int m_hblank = 0;
	int m_frame_vbl = 0;
	int m_12sel = 0;
	int m_video_firq_en = 0;
	emu_timer *m_hblank_end_timer = nullptr;
	emu_timer *m_hblank_start_timer = nullptr;
	std::unique_ptr<uint8_t[]> m_fig_scale_table;
	std::unique_ptr<uint8_t[]> m_scale_table;
	int m_video_firq = 0;
	uint8_t m_bg_intensity = 0;

	void uart_w(offs_t offset, uint8_t data);
	uint8_t uart_r();
	uint8_t g_status_r();
	void g_status_w(uint8_t data);
	uint8_t f_status_r();
	void f_status_w(uint8_t data);
	void frame_w(uint8_t data);
	uint8_t fdt_r(offs_t offset);
	void fdt_w(offs_t offset, uint8_t data);
	uint16_t fdt_rip_r(offs_t offset);
	void fdt_rip_w(offs_t offset, uint16_t data);
	uint8_t rip_status_in();
	void g_iobus_w(uint8_t data);
	uint8_t g_iobus_r();
	void g_ioadd_w(uint8_t data);
	uint8_t s_200e_r();
	void s_200e_w(uint8_t data);
	void s_200f_w(uint8_t data);
	uint8_t s_200f_r();
	uint8_t tms5220_r(offs_t offset);
	void tms5220_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void esripsys_bg_intensity_w(uint8_t data);
	void ptm_irq(int state);
	void esripsys_dac_w(offs_t offset, uint8_t data);
	uint32_t screen_update_esripsys(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(esripsys_vblank_irq);
	TIMER_CALLBACK_MEMBER(delayed_bank_swap);
	TIMER_CALLBACK_MEMBER(hblank_start_callback);
	TIMER_CALLBACK_MEMBER(hblank_end_callback);
	ESRIP_DRAW(esripsys_draw);
	void frame_cpu_map(address_map &map) ATTR_COLD;
	void game_cpu_map(address_map &map) ATTR_COLD;
	void sound_cpu_map(address_map &map) ATTR_COLD;
	void video_cpu_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_ESRIPSYS_H
