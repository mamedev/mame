// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// TC9223P/TC9223F PLL-based frequency synthesizer
#ifndef MAME_MACHINE_TC9223_H
#define MAME_MACHINE_TC9223_H

#pragma once

class tc9223_device : public device_t
{
public:
	tc9223_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void stb_w(int state);
	void dat_w(int state);
	void clk_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 m_shift;
	u8 m_stb, m_clk, m_dat;
};

DECLARE_DEVICE_TYPE(TC9223, tc9223_device)

#endif // MAME_MACHINE_TC9223_H
