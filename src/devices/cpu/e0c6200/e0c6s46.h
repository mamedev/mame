// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 family

*/

#ifndef MAME_CPU_E0C6200_E0C6S46_H
#define MAME_CPU_E0C6200_E0C6S46_H

#include "e0c6200.h"

// for the 2 K input ports, use set_input_line(line, state)
enum
{
	E0C6S46_LINE_K00 = 0,
	E0C6S46_LINE_K01,
	E0C6S46_LINE_K02,
	E0C6S46_LINE_K03,
	E0C6S46_LINE_K10,
	E0C6S46_LINE_K11,
	E0C6S46_LINE_K12,
	E0C6S46_LINE_K13
};

enum
{
	E0C6S46_PORT_R0X = 0,
	E0C6S46_PORT_R1X,
	E0C6S46_PORT_R2X,
	E0C6S46_PORT_R3X,
	E0C6S46_PORT_R4X
};

enum
{
	E0C6S46_PORT_P0X = 0,
	E0C6S46_PORT_P1X,
	E0C6S46_PORT_P2X,
	E0C6S46_PORT_P3X
};

// no pinout diagram here, refer to the manual


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	using pixel_delegate = device_delegate<void (int &dx, int &dy)>;

	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// 5 4-bit R output ports
	template <std::size_t Port> auto write_r() { return m_write_r[Port].bind(); }

	// 4 4-bit P I/O ports
	template <std::size_t Port> auto read_p() { return m_read_p[Port].bind(); }
	template <std::size_t Port> auto write_p() { return m_write_p[Port].bind(); }

	// LCD segment outputs: COM0-COM15 as a0-a3, SEG0-SEGx as a4-a10
	auto write_segs() { return m_write_segs.bind(); }

	// LCD contrast (adjusts VL pins overall voltage level)
	auto write_contrast() { return m_write_contrast.bind(); }

	// screen update (optional)
	const u8 *lcd_buffer() { return &m_render_buf[0]; } // get intermediate LCD pixel buffer
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <typename... T> void set_pixel_callback(T &&... args) { m_pixel_cb.set(std::forward<T>(args)...); } // transform pixel x/y

	// OSC3 (set fast oscillator, via resistor)
	void set_osc3(u32 osc) { m_osc3 = osc; }

protected:
	e0c6s46_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_one() override;
	virtual bool check_interrupt() override;

	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);

	required_shared_ptr_array<u8, 2> m_vram;

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 m_irqflag[6];
	u8 m_irqmask[6];
	u8 m_osc;
	u8 m_svd;

	// lcd driver
	u8 m_lcd_control;
	u8 m_lcd_contrast;
	devcb_write8 m_write_segs;
	devcb_write8 m_write_contrast;

	emu_timer *m_lcd_driver;
	TIMER_CALLBACK_MEMBER(lcd_driver_cb);

	std::unique_ptr<u8[]> m_render_buf;
	pixel_delegate m_pixel_cb;

	// i/o ports
	devcb_write8::array<5> m_write_r;
	devcb_read8::array<4> m_read_p;
	devcb_write8::array<4> m_write_p;
	void write_r(u8 port, u8 data);
	void write_r4_out();
	void write_p(u8 port, u8 data);
	u8 read_p(u8 port);

	u8 m_port_r[5];
	u8 m_r_dir;
	u8 m_port_p[4];
	u8 m_p_dir;
	u8 m_p_pullup;
	u8 m_port_k[2];
	u8 m_dfk0;

	// timers
	int m_256_src_pulse;
	emu_timer *m_core_256_handle;
	TIMER_CALLBACK_MEMBER(core_256_cb);

	int m_watchdog_count;
	void clock_watchdog();
	u8 m_clktimer_count;
	void clock_clktimer();

	u8 m_stopwatch_on;
	int m_swl_cur_pulse;
	int m_swl_slice;
	int m_swl_count;
	int m_swh_count;
	void clock_stopwatch();

	u8 m_prgtimer_select;
	u8 m_prgtimer_on;
	int m_prgtimer_src_pulse;
	int m_prgtimer_cur_pulse;
	u8 m_prgtimer_count;
	u8 m_prgtimer_reload;
	emu_timer *m_prgtimer_handle;
	TIMER_CALLBACK_MEMBER(prgtimer_cb);
	bool prgtimer_reset_prescaler();
	void clock_prgtimer();

	u8 m_bz_43_on;
	u8 m_bz_freq;
	u8 m_bz_envelope;
	u8 m_bz_envelope_count;
	u8 m_bz_duty_ratio;
	u8 m_bz_1shot_on;
	bool m_bz_1shot_running;
	u8 m_bz_1shot_count;
	int m_bz_pulse;
	emu_timer *m_buzzer_handle;
	TIMER_CALLBACK_MEMBER(buzzer_cb);
	void schedule_buzzer();
	void reset_buzzer();
	void clock_bz_1shot();
	void clock_bz_envelope();
	void reset_bz_envelope();

	u32 m_osc1;
	u32 m_osc3;
	emu_timer *m_osc_change;
	TIMER_CALLBACK_MEMBER(osc_change);
};

class e0c6s48_device : public e0c6s46_device
{
public:
	e0c6s48_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(E0C6S46, e0c6s46_device)
DECLARE_DEVICE_TYPE(E0C6S48, e0c6s48_device)

#endif // MAME_CPU_E0C6200_E0C6S46_H
