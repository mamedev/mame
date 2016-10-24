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
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	uint8_t io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_input_lines() const override { return 8; }
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_one() override;
	virtual bool check_interrupt() override;

private:
	required_shared_ptr<uint8_t> m_vram1;
	required_shared_ptr<uint8_t> m_vram2;

	uint8_t m_irqflag[6];
	uint8_t m_irqmask[6];
	uint8_t m_osc;
	uint8_t m_svd;

	uint8_t m_lcd_control;
	uint8_t m_lcd_contrast;
	e0c6s46_pixel_update_func m_pixel_update_handler;

	// i/o ports
	devcb_write8 m_write_r0, m_write_r1, m_write_r2, m_write_r3, m_write_r4;
	devcb_read8 m_read_p0, m_read_p1, m_read_p2, m_read_p3;
	devcb_write8 m_write_p0, m_write_p1, m_write_p2, m_write_p3;
	void write_r(uint8_t port, uint8_t data);
	void write_r4_out();
	void write_p(uint8_t port, uint8_t data);
	uint8_t read_p(uint8_t port);

	uint8_t m_port_r[5];
	uint8_t m_r_dir;
	uint8_t m_port_p[4];
	uint8_t m_p_dir;
	uint8_t m_p_pullup;
	uint8_t m_port_k[2];
	uint8_t m_dfk0;

	// timers
	int m_256_src_pulse;
	emu_timer *m_core_256_handle;
	void core_256_cb(void *ptr, int32_t param);

	int m_watchdog_count;
	void clock_watchdog();
	uint8_t m_clktimer_count;
	void clock_clktimer();

	uint8_t m_stopwatch_on;
	int m_swl_cur_pulse;
	int m_swl_slice;
	int m_swl_count;
	int m_swh_count;
	void clock_stopwatch();

	uint8_t m_prgtimer_select;
	uint8_t m_prgtimer_on;
	int m_prgtimer_src_pulse;
	int m_prgtimer_cur_pulse;
	uint8_t m_prgtimer_count;
	uint8_t m_prgtimer_reload;
	emu_timer *m_prgtimer_handle;
	void prgtimer_cb(void *ptr, int32_t param);
	bool prgtimer_reset_prescaler();
	void clock_prgtimer();

	uint8_t m_bz_43_on;
	uint8_t m_bz_freq;
	uint8_t m_bz_envelope;
	uint8_t m_bz_duty_ratio;
	uint8_t m_bz_1shot_on;
	bool m_bz_1shot_running;
	uint8_t m_bz_1shot_count;
	int m_bz_pulse;
	emu_timer *m_buzzer_handle;
	void buzzer_cb(void *ptr, int32_t param);
	void schedule_buzzer();
	void reset_buzzer();
	void clock_bz_1shot();
};



extern const device_type E0C6S46;

#endif /* _E0C6S46_H_ */
