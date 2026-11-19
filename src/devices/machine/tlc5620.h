// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    Texas Instruments TLC5620 quadruple 8-bit digital-to-analog converter

***************************************************************************/

#ifndef MAME_MACHINE_TLC5620_H
#define MAME_MACHINE_TLC5620_H

#pragma once


class tlc5620_device : public device_t
{
public:
	tlc5620_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// an output changed: which one, and its voltage in 256ths of its reference voltage, 0 to 510
	auto out_callback() { return m_out_cb.bind(); }

	void clk_w(int state);
	void data_w(int state);
	void load_w(int state);
	void ldac_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	void update(u32 channel);

	devcb_write16 m_out_cb;

	u16 m_shift;
	u16 m_input[4];     // range and code of each converter as loaded
	u16 m_output[4];    // and as converted
	bool m_clk;
	bool m_data;
	bool m_load;
	bool m_ldac;
};

DECLARE_DEVICE_TYPE(TLC5620, tlc5620_device)

#endif // MAME_MACHINE_TLC5620_H
