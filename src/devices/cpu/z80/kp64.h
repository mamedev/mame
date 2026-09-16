// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP64 Timer/Counter Unit

***************************************************************************/

#ifndef MAME_CPU_Z80_KP64_H
#define MAME_CPU_Z80_KP64_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kp64_device : public device_t
{
public:
	// device type constructor
	kp64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto out_callback() { return m_out_callback.bind(); }

	// I/O register interface
	u8 counter_r();
	void counter_w(u8 data);
	u8 status_r();
	void control_w(u8 data);

	// input line interface
	void xclk_w(int state);
	void gate_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// timer callbacks
	TIMER_CALLBACK_MEMBER(count_underflow);
	TIMER_CALLBACK_MEMBER(pulse_off);

	// internal helpers
	void set_out(bool state);
	u16 count_value() const noexcept;
	void reload_count();
	void finish_count();

	// callback objects
	devcb_write_line m_out_callback;

	// internal timer
	emu_timer *m_count_timer;
	emu_timer *m_pulse_timer;

	// input state
	bool m_xclk;
	bool m_gate;

	// internal state
	u16 m_count;
	u16 m_cr;
	u16 m_or;
	u8 m_tmp;
	u8 m_status;
	bool m_read_msb;
	bool m_write_msb;
	bool m_reload;
	bool m_started;
};

// device type declaration
DECLARE_DEVICE_TYPE(KP64, kp64_device)

#endif // MAME_CPU_Z80_KP64_H
