// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  IC Designs 2061ASC-1 Programmable Clock Generator

***************************************************************************/

#ifndef MAME_MACHINE_ICD2061A_H
#define MAME_MACHINE_ICD2061A_H

#pragma once

class icd2061a_device : public device_t
{
public:
	icd2061a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto vclkout_changed() { return m_vclkout_changed_cb.bind(); }
	auto mclkout_changed() { return m_mclkout_changed_cb.bind(); }
	auto errout() { return m_errout_cb.bind(); }

	void init0_w(int state) { m_init0 = state; }
	void init1_w(int state) { m_init1 = state; }
	void outdis_w(int state) { m_outdis = state; }
	void pwrdwn_w(int state) { m_pwrdwn = state; }
	void intclk_w(int state) { m_intclk = state; }

	void set_featclock(const uint32_t clock);

	void data_w(int state);
	void clk_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	enum : uint8_t {
		CLOCKGEN_UNLOCK = 0,
		CLOCKGEN_START_BIT,
		CLOCKGEN_DATA,
	};

	enum : int8_t {
		VCLKOUT_REG0 = 0,
		VCLKOUT_REG1,
		VCLKOUT_REG2,
		VCLKOUT_HIGH_Z,
		VCLKOUT_FORCED_HIGH,
		VCLKOUT_FEATCLK,

		MCLKOUT_MREG = 0,
		MCLKOUT_HIGH_Z,
		MCLKOUT_PWRDWN,
	};

	enum {
		REG0 = 0,
		REG1,
		REG2,
		MREG,
	};

	TIMER_CALLBACK_MEMBER( watchdog_callback );
	TIMER_CALLBACK_MEMBER( update_clock_callback );

	void update_clock();

	emu_timer *m_watchdog_timer;
	emu_timer *m_update_timer;

	devcb_write32 m_vclkout_changed_cb, m_mclkout_changed_cb;
	devcb_write_line m_errout_cb;

	uint8_t m_init0, m_init1;
	uint8_t m_sel0, m_sel1;
	uint8_t m_outdis;
	uint8_t m_pwrdwn;
	uint8_t m_intclk;
	int8_t m_vclkout_select, m_mclkout_select;

	uint8_t m_state;
	uint8_t m_unlock_step;
	uint8_t m_cur_bit;
	uint8_t m_data, m_data_prev;
	uint8_t m_clk;
	uint32_t m_cmd;

	uint32_t m_regs[4];
	uint32_t m_reg_clocks[4];
	uint8_t m_prescale[4];
	uint8_t m_powerdown_mode;
	uint8_t m_muxref_vclkout_source;
	uint8_t m_timeout_interval;
	uint8_t m_muxref_adjust;
	uint8_t m_powerdown_divisor;

	uint32_t m_featclock;
	uint32_t m_vclkout_clock, m_mclkout_clock;
};


DECLARE_DEVICE_TYPE(ICD2061A, icd2061a_device)

#endif // MAME_MACHINE_ICD2061A_H
