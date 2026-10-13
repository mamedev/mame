// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 7360/8360 Text Edit Device (TED) emulation

****************************************************************************
                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A13
                  _IRQ   8 |             | 33  A12
                    LP   9 |             | 32  A11
                   _CS  10 |   MOS7360   | 31  A10
                   R/W  11 |             | 30  A9
                    BA  12 |             | 29  A8
                   Vdd  13 |             | 28  A7
                 COLOR  14 |             | 27  A6
                 S/LUM  15 |             | 26  A5
                   AEC  16 |             | 25  A4
                   PH0  17 |             | 24  A3
                  PHIN  18 |             | 23  A2
                 PHCOL  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

***************************************************************************/

#ifndef MAME_SOUND_MOS7360_H
#define MAME_SOUND_MOS7360_H

#pragma once


class mos7360_device :  public device_t,
						public device_memory_interface,
						public device_sound_interface,
						public device_video_interface
{
public:
	mos7360_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto write_irq_callback() { return m_write_irq.bind(); }
	auto write_ba_callback() { return m_write_ba.bind(); }
	auto read_k_callback() { return m_read_k.bind(); }

	virtual space_config_vector memory_space_config() const override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t bus_r();
	bool rom() const { return m_rom; }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void configure_screen(screen_device &screen, bool ntsc) const ATTR_COLD;

	void mos7360_videoram_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned HTOTAL = 456;
	static constexpr unsigned HVISIBLE = 384;
	static constexpr unsigned PAL_LINES = 312;
	static constexpr unsigned PAL_VBEND = 22;
	static constexpr unsigned PAL_VBSTART = 310;
	static constexpr unsigned NTSC_LINES = 262;
	static constexpr unsigned NTSC_VBEND = 19;
	static constexpr unsigned NTSC_VBSTART = 261;

	struct character
	{
		uint8_t attr;
		uint8_t ch;
		uint8_t bitmap;
		uint8_t flags;
	};

	void sync();
	void cpu_access(uint8_t data, bool write);
	void catch_up(uint64_t slot);
	void half_cycle_pre();
	void half_cycle_post();
	uint64_t cpu_slot() const;
	void cpu_cycle(uint8_t data, bool write);
	bool cpu_slot_free(bool write) const;
	attotime slot_delay(uint64_t slot) const;
	uint64_t next_even_slot() const;
	void update_ba();
	void arm_ba(uint64_t slot);
	TIMER_CALLBACK_MEMBER(ba_tick);

	void even_events();
	void odd_events();
	void process_events(uint32_t events);
	void vertical_events();
	void check_raster_irq();
	void check_dma_reset();
	void update_irq();
	void update_mode();
	void set_ntsc(bool ntsc);
	void update_timing();

	uint8_t fetch_dma(offs_t addr);
	uint8_t fetch_bitmap(offs_t addr);
	void fetch_idle();
	uint8_t reg_r(offs_t offset);
	void render(int load, bool force_blank);

	void soundport_w(int offset, int data);
	void update_sound_outputs();

	const address_space_config m_videoram_space_config;

	devcb_write_line m_write_irq;
	devcb_write_line m_write_ba;
	devcb_read8 m_read_k;

	required_device<cpu_device> m_cpu;
	memory_passthrough_handler m_bus_tap;
	bool m_bus_tap_installed;

	sound_stream *m_stream;
	bitmap_rgb32 m_bitmap;
	const rgb_t *m_palette;

	// scheduling
	bool m_catching_up;
	uint64_t m_slot;
	uint64_t m_cpu_base;
	bool m_pre_done;
	bool m_cpu_waiting;
	bool m_cpu_access;
	uint8_t m_cpu_data;
	uint64_t m_cpu_prev_slot;
	emu_timer *m_ba_timer;
	bool m_ba;
	bool m_ba_armed;
	uint64_t m_ba_slot;
	uint64_t m_ba_rise_slot;

	// registers
	uint8_t m_reg[0x20];
	uint8_t m_bus;
	bool m_rom;
	bool m_ntsc;

	// counters
	uint8_t m_col;
	uint16_t m_line;
	uint16_t m_saved_line;
	uint16_t m_line_d1;
	uint8_t m_char_line;
	uint8_t m_prev_char_line;
	uint16_t m_char_pos;
	uint16_t m_saved_char_pos;
	uint16_t m_char_pos_reload;
	uint8_t m_char_col;
	uint16_t m_dma_pos;
	uint16_t m_dma_pos_reload;
	uint16_t m_dma_base;
	uint16_t m_bitmap_base;
	uint16_t m_charset_base;
	uint16_t m_cursor_pos;
	uint16_t m_irq_line;
	uint8_t m_refresh;

	// state flags
	bool m_frozen;
	bool m_frozen_half;
	uint8_t m_flash;
	bool m_render_window;
	bool m_inc_char_line;
	uint8_t m_bitmap_disable;
	bool m_display_window;
	bool m_display_active;
	bool m_hblank;
	bool m_vblank;
	bool m_prev_raster_irq;
	bool m_irq;
	bool m_dma_enabled;
	uint8_t m_single;
	bool m_ext_single;
	uint8_t m_dma_flags;
	bool m_dma_active;
	bool m_inc_dma_pos;
	bool m_inc_char_pos;
	bool m_cpu_running;
	bool m_cpu_halted;
	uint32_t m_events_even;
	uint32_t m_events_odd;
	uint32_t m_events_now;

	// timers
	bool m_t1_run, m_t2_run, m_t3_run;
	uint16_t m_t1, m_t1_reload, m_t2, m_t3;

	// video pipeline
	uint8_t m_attr_buf[64];
	uint8_t m_attr_tmp[64];
	uint8_t m_char_buf[64];
	bool m_shift_enable;
	uint8_t m_mode;
	uint8_t m_render_mode;
	uint8_t m_char_mask;
	character m_shift;
	character m_current;
	character m_next;
	uint8_t m_color_latch[5];
	uint8_t m_hscroll;
	uint8_t m_vscroll;
	uint16_t m_beam_x;
	uint16_t m_beam_y;
	bool m_vsync_pending;

	// sound
	uint16_t m_osc_accu[2];
	uint16_t m_osc_reload[2];
	uint8_t m_osc_sign[2];
	uint8_t m_osc_out[2];
	uint8_t m_noise_sr;
};


DECLARE_DEVICE_TYPE(MOS7360, mos7360_device)

#endif // MAME_SOUND_MOS7360_H
