// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP timer device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPTIMER_H
#define MAME_MACHINE_IOPTIMER_H

#pragma once

class iop_timer_device : public device_t
{
public:
	iop_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_int_cb.bind(); }

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	enum timer_ctrl_mask : uint32_t
	{
		CTRL_GATE = (1 << 0),
		CTRL_GATM = (3 << 1),
		CTRL_ZRET = (1 << 3),
		CTRL_CMPE = (1 << 4),
		CTRL_OVFE = (1 << 5),
		CTRL_REPI = (1 << 6),
		CTRL_TOGI = (1 << 7),
		CTRL_INTE = (1 << 10),
		CTRL_CMPF = (1 << 11),
		CTRL_OVFF = (1 << 12),
	};

	enum timer_gate_mode : uint32_t
	{
		GATM_LOW = 0,
		GATM_RISING,
		GATM_FALLING,
		GATM_BOTH
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(compare);
	TIMER_CALLBACK_MEMBER(overflow);

	void set_ctrl(uint32_t data);

	void update_gate();
	void update_interrupts();

	void update_compare_timer();
	void update_overflow_timer();

	void update_count();

	emu_timer *m_compare_timer;
	emu_timer *m_overflow_timer;

	uint32_t m_ctrl;

	attotime m_last_update_time;
	attotime m_elapsed_time;
	bool m_zero_return;
	uint32_t m_count;
	uint32_t m_compare;

	bool m_gate_enable;
	timer_gate_mode m_gate_mode;

	bool m_cmp_int_enabled;
	bool m_cmp_int;

	bool m_ovf_int_enabled;
	bool m_ovf_int;

	bool m_repeat_int;
	bool m_toggle_int;

	bool m_ienable;
	devcb_write_line m_int_cb;
};

DECLARE_DEVICE_TYPE(SONYIOP_TIMER, iop_timer_device)

#endif // MAME_MACHINE_IOPTIMER_H
