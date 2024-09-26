// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Fairchild 4702B Programmable Bit Rate Generator

*****************************************************************************
                              ____   ____
                       Q0  1 |*   \_/    | 16  Vdd
                       Q1  2 |           | 15  Im
                       Q2  3 |           | 14  S0
                     _Ecp  4 |           | 13  S1
                       CP  5 |  4702BPC  | 12  S2
                       Ox  6 |           | 11  S3
                       Ix  7 |           | 10  Z
                      Vss  8 |___________|  9  CO

****************************************************************************/

#ifndef MAME_MACHINE_F4702_H
#define MAME_MACHINE_F4702_H

#pragma once


// ======================> f4702_device

class f4702_device : public device_t, public device_execute_interface
{
public:
	f4702_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto s_callback() { return m_s_callback.bind(); }
	auto z_callback() { return m_z_callback.bind(); }

	// external rate input
	void im_w(int state);

	// reset control (optional)
	void reset_counters();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;

private:
	// internal helpers
	bool z_output() const;

	// input & output callbacks
	devcb_read8 m_s_callback;
	devcb_write8 m_z_callback;

	// internal counters
	u16 m_main_counter;
	u8 m_div_200_50;
	u8 m_div_134_5;
	u8 m_div_110;
	u8 m_div_1800;

	// miscellaneous state
	bool m_im;
	u8 m_s;
	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(F4702, f4702_device)

#endif // MAME_MACHINE_F4702_H

