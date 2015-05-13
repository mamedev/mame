// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

#ifndef _E0C6S46_H_
#define _E0C6S46_H_

#include "e0c6200.h"

#define MCFG_E0C6S46_PIXEL_UPDATE_CB(_cb) \
	e0c6s46_device::static_set_pixel_update_cb(*device, _cb);


typedef void (*e0c6s46_pixel_update_func)(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state);
#define E0C6S46_PIXEL_UPDATE_CB(name) void name(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, int contrast, int seg, int com, int state)


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
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
	
	UINT8 m_port_k[2];
	UINT8 m_dfk0;
	
	UINT8 m_lcd_control;
	UINT8 m_lcd_contrast;
	e0c6s46_pixel_update_func m_pixel_update_handler;

	int m_watchdog_count;
	void clock_watchdog();
	
	UINT8 m_clktimer_count;
	emu_timer *m_clktimer_handle;
	TIMER_CALLBACK_MEMBER(clktimer_cb);

	UINT8 m_stopwatch_on;
	int m_swl_src_pulse;
	int m_swl_cur_pulse;
	int m_swl_slice;
	int m_swl_count;
	int m_swh_count;
	emu_timer *m_stopwatch_handle;
	TIMER_CALLBACK_MEMBER(stopwatch_cb);
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
};



extern const device_type E0C6S46;

#endif /* _E0C6S46_H_ */
