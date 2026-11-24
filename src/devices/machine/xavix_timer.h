// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_TIMER_H
#define MAME_MACHINE_XAVIX_TIMER_H

#pragma once

class xavix_timer_device : public device_t
{
public:
	xavix_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_restart_on_run(bool restart) { m_restart_on_run = restart; }
	auto irq_callback() { return m_irq_cb.bind(); }
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct counter_state
	{
		u8 control;
		u8 count;
		u32 prescaler_count;
	};

	u8 status_r();
	void ctrl_w(u8 data);
	u8 reload_r();
	void reload_w(u8 data);
	u8 prescaler_r();
	void prescaler_w(u8 data);
	u8 count_r();
	u8 mode_r();
	void mode_w(u8 data);
	u32 divisor() const;
	u32 terminal_ticks(u8 count) const;
	u64 current_clock() const;
	counter_state current_counter() const;
	void synchronize();
	void update_irq();
	void schedule_timer();
	TIMER_CALLBACK_MEMBER(timer_expired);

	devcb_write_line m_irq_cb;
	bool m_restart_on_run;
	emu_timer *m_timer;
	u8 m_ctrl;
	u8 m_reload;
	u8 m_prescaler;
	u8 m_count;
	u8 m_mode;
	u32 m_prescaler_count;
	u64 m_last_clock;
};

DECLARE_DEVICE_TYPE(XAVIX_TIMER, xavix_timer_device)

#endif // MAME_MACHINE_XAVIX_TIMER_H
