// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    Motorola MC14411 Bit Rate Generator

***********************************************************************
                            _____   _____
                     F1  1 |*    \_/     | 24  VDD
                     F3  2 |             | 23  Rate Select A
                     F5  3 |             | 22  Rate Select B
                     F7  4 |             | 21  Xtal In
                     F8  5 |             | 20  Xtal Out
                    F10  6 |             | 19  F16
                     F9  7 |   MC14411   | 18  F15
                    F11  8 |             | 17  F2
                    F14  9 |             | 16  F4
                 Reset* 10 |             | 15  F6
              Not Used  11 |             | 14  F12
                   VSS  12 |_____________| 13  F13

**********************************************************************/

#ifndef MAME_MACHINE_MC14411_H
#define MAME_MACHINE_MC14411_H

#pragma once

class mc14411_device : public device_t
{
public:
	// timer indices
	enum timer_id : int
	{
		TIMER_F1,
		TIMER_F2,
		TIMER_F3,
		TIMER_F4,
		TIMER_F5,
		TIMER_F6,
		TIMER_F7,
		TIMER_F8,
		TIMER_F9,
		TIMER_F10,
		TIMER_F11,
		TIMER_F12,
		TIMER_F13,
		TIMER_F14,
		TIMER_F15,
		TIMER_F16
	};

	// rate select inputs
	enum
	{
		RSA = 0x01,
		RSB = 0x02
	};

	// construction/destruction
	mc14411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <std::size_t Line> auto out_f() { return m_out_fx_cbs[Line-1].bind(); }

	void reset_w(int state);
	void rate_select_w(uint8_t data);
	void rsa_w(int state);
	void rsb_w(int state);

	void timer_enable(timer_id i, bool enable);
	void timer_disable_all();

protected:
	mc14411_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(reset_tick);

private:
	void arm_timer(int i);

	struct fx_timer
	{
		emu_timer *timer;
		bool state;
		bool enabled;
	};

	fx_timer m_fx_timers[16];
	emu_timer *m_reset_timer;

	devcb_write_line::array<16> m_out_fx_cbs;

	uint32_t m_divider; // main divider to use, 0-3 column index into counter_divider
	uint32_t m_reset;   // Reset line state

	// divider matrix
	static const int s_counter_divider[16];
	static const int s_divider_select[4];
};

DECLARE_DEVICE_TYPE(MC14411, mc14411_device)

#endif // MAME_MACHINE_MC14411_H
