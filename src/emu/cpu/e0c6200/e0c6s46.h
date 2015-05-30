// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

#ifndef _E0C6S46_H_
#define _E0C6S46_H_

#include "e0c6200.h"


// I/O ports setup

// 5 4-bit R output ports
#define MCFG_E0C6S46_WRITE_R_CB(R, _devcb) \
	e0c6s46_device::set_write_r##R##_callback(*device, DEVCB_##_devcb);

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
	hmcs40_cpu_device::set_read_r##P##_callback(*device, DEVCB_##_devcb);
#define MCFG_E0C6S46_WRITE_P_CB(R, _devcb) \
	e0c6s46_device::set_write_r##P##_callback(*device, DEVCB_##_devcb);

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
#define MCFG_E0C6S46_PIXEL_UPDATE_CB(_cb) \
	e0c6s46_device::static_set_pixel_update_cb(*device, _cb);

typedef void (*e0c6s46_pixel_update_func)(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state);
#define E0C6S46_PIXEL_UPDATE_CB(name) void name(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state)


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_write_r0_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_r0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r1_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_r1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r2_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_r2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r3_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_r3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r4_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_r4.set_callback(object); }

	template<class _Object> static devcb_base &set_read_p0_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_read_p0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_p1_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_read_p1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_p2_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_read_p2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_p3_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_read_p3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_p0_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_p0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_p1_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_p1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_p2_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_p2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_p3_callback(device_t &device, _Object object) { return downcast<e0c6s46_device &>(device).m_write_p3.set_callback(object); }

	static void static_set_pixel_update_cb(device_t &device, e0c6s46_pixel_update_func _cb) { downcast<e0c6s46_device &>(device).m_pixel_update_handler = _cb; }

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_input_lines() const { return 8; }
	virtual void execute_set_input(int line, int state);
	virtual void execute_one();
	virtual bool check_interrupt();

private:
	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_vram2;

	UINT8 m_irqflag[6];
	UINT8 m_irqmask[6];
	UINT8 m_osc;
	UINT8 m_svd;

	UINT8 m_lcd_control;
	UINT8 m_lcd_contrast;
	e0c6s46_pixel_update_func m_pixel_update_handler;

	// i/o ports
	devcb_write8 m_write_r0, m_write_r1, m_write_r2, m_write_r3, m_write_r4;
	devcb_read8 m_read_p0, m_read_p1, m_read_p2, m_read_p3;
	devcb_write8 m_write_p0, m_write_p1, m_write_p2, m_write_p3;
	void write_r(UINT8 port, UINT8 data);
	void write_r4_out();
	void write_p(UINT8 port, UINT8 data);
	UINT8 read_p(UINT8 port);

	UINT8 m_port_r[5];
	UINT8 m_r_dir;
	UINT8 m_port_p[4];
	UINT8 m_p_dir;
	UINT8 m_p_pullup;
	UINT8 m_port_k[2];
	UINT8 m_dfk0;

	// timers
	int m_256_src_pulse;
	emu_timer *m_core_256_handle;
	TIMER_CALLBACK_MEMBER(core_256_cb);

	int m_watchdog_count;
	void clock_watchdog();
	UINT8 m_clktimer_count;
	void clock_clktimer();

	UINT8 m_stopwatch_on;
	int m_swl_cur_pulse;
	int m_swl_slice;
	int m_swl_count;
	int m_swh_count;
	void clock_stopwatch();

	UINT8 m_prgtimer_select;
	UINT8 m_prgtimer_on;
	int m_prgtimer_src_pulse;
	int m_prgtimer_cur_pulse;
	UINT8 m_prgtimer_count;
	UINT8 m_prgtimer_reload;
	emu_timer *m_prgtimer_handle;
	TIMER_CALLBACK_MEMBER(prgtimer_cb);
	bool prgtimer_reset_prescaler();
	void clock_prgtimer();

	UINT8 m_bz_43_on;
	UINT8 m_bz_freq;
	UINT8 m_bz_envelope;
	UINT8 m_bz_duty_ratio;
	UINT8 m_bz_1shot_on;
	bool m_bz_1shot_running;
	UINT8 m_bz_1shot_count;
	int m_bz_pulse;
	emu_timer *m_buzzer_handle;
	TIMER_CALLBACK_MEMBER(buzzer_cb);
	void schedule_buzzer();
	void reset_buzzer();
	void clock_bz_1shot();
};



extern const device_type E0C6S46;

#endif /* _E0C6S46_H_ */
