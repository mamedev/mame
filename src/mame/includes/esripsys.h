// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Entertainment Sciences RIP System hardware

*************************************************************************/

#ifndef _ESRIPSYS_H_
#define _ESRIPSYS_H_

#pragma once

#include "cpu/esrip/esrip.h"
#include "sound/dac.h"
#include "sound/tms5220.h"

/* TODO */
#define ESRIPSYS_PIXEL_CLOCK    (XTAL_25MHz / 2)
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
	void uart_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t uart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t g_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void g_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t f_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void f_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void frame_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fdt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t fdt_rip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fdt_rip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t rip_status_in(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void g_iobus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t g_iobus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void g_ioadd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t s_200e_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void s_200e_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s_200f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t s_200f_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tms5220_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tms5220_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void esripsys_bg_intensity_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void keypad_interrupt(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin_interrupt(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void ptm_irq(int state);
	void esripsys_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_esripsys();
	virtual void video_start() override;
	uint32_t screen_update_esripsys(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void esripsys_vblank_irq(device_t &device);
	void delayed_bank_swap(void *ptr, int32_t param);
	void hblank_start_callback(void *ptr, int32_t param);
	void hblank_end_callback(void *ptr, int32_t param);
	required_device<dac_word_interface> m_dac;
	required_device<screen_device> m_screen;
	ESRIP_DRAW(esripsys_draw);
};

#endif // _ESRIPSYS_H_
