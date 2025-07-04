// license:BSD-3-Clause
// copyright-holders: R. Belmont
/***************************************************************************

  Cypress Semiconductor ICD2053B Programmable Clock Generator

***************************************************************************/

#ifndef MAME_MACHINE_ICD2053B_H
#define MAME_MACHINE_ICD2053B_H

#pragma once

class icd2053b_device : public device_t
{
public:
	icd2053b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clkout_changed() { return m_clkout_changed_cb.bind(); }

	void data_w(int state);
	void clk_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write32 m_clkout_changed_cb;

	u32 m_shifter, m_shifter_last3, m_shift_pos;
	u8 m_pll_control;
	int m_prev_shift_clock, m_shifter_expected, m_datalatch;
};

DECLARE_DEVICE_TYPE(ICD2053B, icd2053b_device)

#endif // MAME_MACHINE_ICD2053B_H
