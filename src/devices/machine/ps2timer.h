// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE timer device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_PS2TIMER_H
#define MAME_MACHINE_PS2TIMER_H

#pragma once

class ps2_timer_device : public device_t
{
public:
	ps2_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool can_hold)
		: ps2_timer_device(mconfig, tag, owner, clock)
	{
		m_can_hold = can_hold;
	}

	ps2_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	enum timer_mode_mask : uint32_t
	{
		MODE_CLKS = (3 << 0),
		MODE_GATE = (1 << 2),
		MODE_GATS = (1 << 3),
		MODE_GATM = (3 << 4),
		MODE_ZRET = (1 << 6),
		MODE_CUE  = (1 << 7),
		MODE_CMPE = (1 << 8),
		MODE_OVFE = (1 << 9),
		MODE_EQUF = (1 << 10),
		MODE_OVFF = (1 << 11),
	};

	enum timer_gate_mode : uint32_t
	{
		GATM_LOW = 0,
		GATM_RISING,
		GATM_FALLING,
		GATM_BOTH
	};

	enum timer_clock_select : uint32_t
	{
		CLKS_BUSCLK1 = 0,
		CLKS_BUSCLK16,
		CLKS_BUSCLK256,
		CLKS_HBLNK
	};

	enum timer_gate_select : uint32_t
	{
		GATS_HBLNK = 0,
		GATS_VBLNK
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(compare);
	TIMER_CALLBACK_MEMBER(overflow);

	void update_gate();
	void update_interrupts();

	void update_compare_timer();
	void update_overflow_timer();

	void update_count();
	void update_hold();

	void set_mode(uint32_t data);

	emu_timer *m_compare_timer;
	emu_timer *m_overflow_timer;

	uint32_t m_mode;

	attotime m_last_enable_time;
	attotime m_last_update_time;
	attotime m_elapsed_time;
	bool m_enabled;
	bool m_zero_return;
	uint32_t m_count;
	uint32_t m_compare;

	bool m_can_hold;
	uint32_t m_hold;

	timer_clock_select m_clk_select;

	bool m_gate_enable;
	timer_gate_select m_gate_select;
	timer_gate_mode m_gate_mode;

	bool m_cmp_int_enabled;
	bool m_cmp_int;

	bool m_ovf_int_enabled;
	bool m_ovf_int;
};

DECLARE_DEVICE_TYPE(SONYPS2_TIMER, ps2_timer_device)

#endif // MAME_MACHINE_PS2TIMER_H
