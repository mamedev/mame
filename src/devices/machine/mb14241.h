// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef MAME_MACHINE_MB14241_H
#define MAME_MACHINE_MB14241_H

#pragma once

class mb14241_device : public device_t
{
public:
	mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void shift_count_w(u8 data);
	void shift_data_w(u8 data);
	u8 shift_result_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u16 m_shift_data;  // 15 bits only
	u8 m_shift_count;  // 3 bits
};

DECLARE_DEVICE_TYPE(MB14241, mb14241_device)

#endif // MAME_MACHINE_MB14241_H
