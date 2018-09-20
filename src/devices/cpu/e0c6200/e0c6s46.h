// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

#ifndef MAME_CPU_E0C6200_E0C6S46_H
#define MAME_CPU_E0C6200_E0C6S46_H

#include "e0c6200.h"


// I/O ports setup

// 5 4-bit R output ports
#define MCFG_E0C6S46_WRITE_R_CB(R, _devcb) \
	downcast<e0c6s46_device &>(*device).set_write_r##R##_callback(DEVCB_##_devcb);

enum
{
	E0C6S46_PORT_R0X = 0,
	E0C6S46_PORT_R1X,
	E0C6S46_PORT_R2X,
	E0C6S46_PORT_R3X,
	E0C6S46_PORT_R4X
};

// 4 4-bit P I/O ports
#define MCFG_E0C6S46_READ_P_CB(R, _devcb) \
	downcast<hmcs40_cpu_device &>(*device).set_read_r##P##_callback(DEVCB_##_devcb);
#define MCFG_E0C6S46_WRITE_P_CB(R, _devcb) \
	downcast<e0c6s46_device &>(*device).set_write_r##P##_callback(DEVCB_##_devcb);

enum
{
	E0C6S46_PORT_P0X = 0,
	E0C6S46_PORT_P1X,
	E0C6S46_PORT_P2X,
	E0C6S46_PORT_P3X
};

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


// lcd driver
#define MCFG_E0C6S46_PIXEL_UPDATE_CB(_class, _method) \
	downcast<e0c6s46_device &>(*device).set_pixel_update_cb(e0c6s46_device::pixel_update_delegate(&_class::_method, #_class "::" #_method, this));

#define E0C6S46_PIXEL_UPDATE(name) void name(bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state)


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state)> pixel_update_delegate;

	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <class Object> devcb_base &set_write_r0_callback(Object &&cb) { return m_write_r0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_r1_callback(Object &&cb) { return m_write_r1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_r2_callback(Object &&cb) { return m_write_r2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_r3_callback(Object &&cb) { return m_write_r3.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_r4_callback(Object &&cb) { return m_write_r4.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_read_p0_callback(Object &&cb) { return m_read_p0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_p1_callback(Object &&cb) { return m_read_p1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_p2_callback(Object &&cb) { return m_read_p2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_p3_callback(Object &&cb) { return m_read_p3.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_p0_callback(Object &&cb) { return m_write_p0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_p1_callback(Object &&cb) { return m_write_p1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_p2_callback(Object &&cb) { return m_write_p2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_p3_callback(Object &&cb) { return m_write_p3.set_callback(std::forward<Object>(cb)); }

	template <typename Object> void set_pixel_update_cb(Object &&cb) { m_pixel_update_cb = std::forward<Object>(cb); }

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_input_lines() const override { return 8; }
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_one() override;
	virtual bool check_interrupt() override;

	void e0c6s46_data(address_map &map);
	void e0c6s46_program(address_map &map);

private:
	required_shared_ptr<u8> m_vram1;
	required_shared_ptr<u8> m_vram2;

	u8 m_irqflag[6];
	u8 m_irqmask[6];
	u8 m_osc;
	u8 m_svd;

	u8 m_lcd_control;
	u8 m_lcd_contrast;
	pixel_update_delegate m_pixel_update_cb;

	// i/o ports
	devcb_write8 m_write_r0, m_write_r1, m_write_r2, m_write_r3, m_write_r4;
	devcb_read8 m_read_p0, m_read_p1, m_read_p2, m_read_p3;
	devcb_write8 m_write_p0, m_write_p1, m_write_p2, m_write_p3;
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
};


DECLARE_DEVICE_TYPE(E0C6S46, e0c6s46_device)

#endif // MAME_CPU_E0C6200_E0C6S46_H
