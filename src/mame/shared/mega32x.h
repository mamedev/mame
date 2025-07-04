// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 32X */
#ifndef MAME_SHARED_MEGA32X_H
#define MAME_SHARED_MEGA32X_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh7604.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"


class sega_32x_device : public device_t, public device_palette_interface, public device_sound_interface, public device_video_interface
{
public:
	void pause_cpu();

	// set some variables at start, depending on region (shall be moved to a device interface?)
	void set_framerate(int rate) { m_framerate = rate; }
	void set_32x_pal(bool pal) { m_32x_pal = pal ? 1 : 0; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }     // this get set at start only
	double get_framerate() { return has_screen() ? screen().frame_period().as_hz() : double(m_framerate); }

	void screen_eof(bool mode3)
	{
		m_32x_vblank_flag = 0;
		m_32x_hcount_compare_val = -1;
		update_total_scanlines(mode3);
	}

	uint16_t m68k_palette_r(offs_t offset);
	void m68k_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_dram_r(offs_t offset);
	void m68k_dram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_dram_overwrite_r(offs_t offset);
	void m68k_dram_overwrite_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_a15106_r();
	void m68k_a15106_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dreq_common_r(address_space &space, offs_t offset);
	void dreq_common_w(address_space &space, offs_t offset, uint16_t data);
	uint16_t m68k_a1511a_r();
	void m68k_a1511a_w(uint16_t data);
	uint16_t m68k_m_hint_vector_r(offs_t offset);
	void m68k_m_hint_vector_w(offs_t offset, uint16_t data);
	uint16_t m68k_MARS_r(offs_t offset);
	uint16_t m68k_a15100_r();
	void m68k_a15100_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_a15102_r();
	void m68k_a15102_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_a15104_r();
	void m68k_a15104_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_m_commsram_r(offs_t offset);
	void m68k_m_commsram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pwm_r(offs_t offset);
	void pwm_w(offs_t offset, uint16_t data);
	void m68k_pwm_w(offs_t offset, uint16_t data);
	uint16_t common_vdp_regs_r(offs_t offset);
	void common_vdp_regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t master_4000_r();
	void master_4000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t slave_4000_r();
	void slave_4000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t common_4002_r();
	void common_4002_w(uint16_t data);
	uint16_t common_4004_r();
	void common_4004_w(uint16_t data);
	uint16_t common_4006_r();
	void common_4006_w(uint16_t data);
	void master_4014_w(uint16_t data);
	void slave_4014_w(uint16_t data);
	void master_4016_w(uint16_t data);
	void slave_4016_w(uint16_t data);
	void master_4018_w(uint16_t data);
	void slave_4018_w(uint16_t data);
	void master_401a_w(uint16_t data);
	void slave_401a_w(uint16_t data);
	void master_401c_w(uint16_t data);
	void slave_401c_w(uint16_t data);
	void master_401e_w(uint16_t data);
	void slave_401e_w(uint16_t data);

	SH2_DMA_FIFO_DATA_AVAILABLE_CB(_32x_fifo_available_callback);

	void render_videobuffer_to_screenbuffer_helper(int scanline);
	void render_videobuffer_to_screenbuffer(int x, uint32_t priority, uint32_t &lineptr);
	int sh2_master_pwmint_enable = 0, sh2_slave_pwmint_enable = 0;

	void check_framebuffer_swap(bool enabled);
	void check_irqs();
	void interrupt_cb(int scanline, int irq6);

	void sh2_main_map(address_map &map) ATTR_COLD;
	void sh2_slave_map(address_map &map) ATTR_COLD;
	void sh2_common_map(address_map &map) ATTR_COLD;

protected:
	sega_32x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_shared_ptr<uint32_t> m_sh2_shared;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 32*32*32/**2*/; }

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	void update_total_scanlines(bool mode3) { m_total_scanlines = mode3 ? (m_base_total_scanlines * 2) : m_base_total_scanlines; }  // this gets set at each EOF

	/* our main vblank handler resets this */
	required_device<m68000_base_device> m_main_cpu;
	required_device<sh7604_device> m_master_cpu;
	required_device<sh7604_device> m_slave_cpu;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_device<timer_device> m_scan_timer;
	memory_bank_creator m_rombank;

	int m_32x_hcount_compare_val = 0;
	int m_32x_vblank_flag = 0;
	int m_sh2_are_running = 0;
	int m_32x_240mode = 0;
	uint16_t m_32x_a1518a_reg = 0;

	sound_stream *m_stream = nullptr;
	TIMER_CALLBACK_MEMBER(handle_pwm_callback);
	void calculate_pwm_timer();
	uint16_t m_pwm_ctrl = 0, m_pwm_cycle = 0, m_pwm_tm_reg = 0;
	static constexpr int PWM_FIFO_SIZE = 3;
	uint16_t m_cur_lch[PWM_FIFO_SIZE]{},m_cur_rch[PWM_FIFO_SIZE]{};
	uint16_t m_pwm_cycle_reg = 0; //used for latching
	uint8_t m_pwm_timer_tick = 0;
	uint8_t m_lch_size = 0, m_rch_size = 0;
	uint16_t m_lch_fifo_state = 0, m_rch_fifo_state = 0;

	void lch_pop();
	void rch_pop();

	uint16_t get_hposition(void);

	emu_timer *m_32x_pwm_timer = nullptr;

private:

	int m_32x_displaymode = 0;
	int m_32x_videopriority = 0;
	uint32_t m_32x_linerender[320+258]{}; // tmp buffer (bigger than it needs to be to simplify RLE decode)

	int m_32x_adapter_enabled = 0;
	int m_32x_access_auth = 0;
	int m_32x_screenshift = 0;

	uint16_t m_32x_68k_a15104_reg = 0;
	int m_sh2_master_vint_enable = 0, m_sh2_slave_vint_enable = 0;
	int m_sh2_master_hint_enable = 0, m_sh2_slave_hint_enable = 0;
	int m_sh2_master_cmdint_enable = 0, m_sh2_slave_cmdint_enable = 0;
	int m_sh2_hint_in_vbl = 0;
	int m_sh2_master_vint_pending = 0;
	int m_sh2_slave_vint_pending = 0;
	int m_32x_fb_swap = 0;
	int m_32x_hcount_reg = 0;

	uint16_t m_32x_autofill_length = 0;
	uint16_t m_32x_autofill_address = 0;
	uint16_t m_32x_autofill_data = 0;
	uint16_t m_a15106_reg = 0;
	uint16_t m_dreq_src_addr[2]{}, m_dreq_dst_addr[2]{}, m_dreq_size = 0;
	uint8_t m_sega_tv = 0;
	uint16_t m_hint_vector[2]{};
	uint16_t m_a15100_reg = 0;
	int m_32x_68k_a15102_reg = 0;

	int m_32x_pal = 0;
	int m_framerate = 0;
	int m_base_total_scanlines = 0;
	int m_total_scanlines = 0;

	uint16_t m_commsram[8]{};

	std::unique_ptr<uint16_t[]> m_32x_dram0;
	std::unique_ptr<uint16_t[]> m_32x_dram1;
	uint16_t *m_32x_display_dram = nullptr, *m_32x_access_dram = nullptr;
	std::unique_ptr<uint16_t[]> m_32x_palette;

	uint16_t m_fifo_block_a[4]{};
	uint16_t m_fifo_block_b[4]{};
	uint16_t* m_current_fifo_block = nullptr;
	uint16_t* m_current_fifo_readblock = nullptr;
	int m_current_fifo_write_pos = 0;
	int m_current_fifo_read_pos = 0;
	int m_fifo_block_a_full = 0;
	int m_fifo_block_b_full = 0;
};


class sega_32x_ntsc_device : public sega_32x_device
{
public:
	template <typename T, typename U>
	sega_32x_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&main_cpu_tag, U &&timer_tag)
		: sega_32x_ntsc_device(mconfig, tag, owner, clock)
	{
		m_main_cpu.set_tag(std::forward<T>(main_cpu_tag));
		m_scan_timer.set_tag(std::forward<U>(timer_tag));
	}

	sega_32x_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

};

class sega_32x_pal_device : public sega_32x_device
{
public:
	template <typename T, typename U>
	sega_32x_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&main_cpu_tag, U &&timer_tag)
		: sega_32x_pal_device(mconfig, tag, owner, clock)
	{
		m_main_cpu.set_tag(std::forward<T>(main_cpu_tag));
		m_scan_timer.set_tag(std::forward<U>(timer_tag));
	}

	sega_32x_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(SEGA_32X_NTSC, sega_32x_ntsc_device)
DECLARE_DEVICE_TYPE(SEGA_32X_PAL,  sega_32x_pal_device)

#endif // MAME_SHARED_MEGA32X_H
