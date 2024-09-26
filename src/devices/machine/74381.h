// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    74381.h
    SN74S381 ALU / Function Generator

***************************************************************************/

#ifndef MAME_MACHINE_74381_H
#define MAME_MACHINE_74381_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sn74s381_device

class sn74s381_device : public device_t
{
public:
	// construction/destruction
	sn74s381_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void a_w(uint8_t data);
	void b_w(uint8_t data);
	void s_w(uint8_t data);
	void cn_w(int state);

	auto f() { return m_f_out.bind(); }
	auto p() { return m_p_out.bind(); }
	auto g() { return m_g_out.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update();

	uint8_t m_a;
	uint8_t m_b;
	uint8_t m_s;
	uint8_t m_cn;

	uint8_t m_f;
	bool m_p;
	bool m_g;

	devcb_write8 m_f_out;
	devcb_write_line m_p_out;
	devcb_write_line m_g_out;
};


// device type definition
DECLARE_DEVICE_TYPE(SN74S381, sn74s381_device)

#endif // MAME_MACHINE_74381_H
