// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SN54/74153

    Dual 4-Input Multiplexer

               ___ ___
     EA/1G  1 |*  u   | 16  VCC
      S1/B  2 |       | 15  EB/2G
   I3A/1C3  3 |       | 14  S0/A
   I2A/1C2  4 |       | 13  I3B/2C3
   I1A/1C1  5 |       | 12  I2B/2C2
   I0A/1C0  6 |       | 11  I1B/2C1
     ZA/1Y  7 |       | 10  I0B/2C0
       GND  8 |_______|  9  ZB/2Y

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_74153_H
#define MAME_DEVICES_MACHINE_74153_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ttl153_device : public device_t
{
public:
	// construction/destruction
	ttl153_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto za_cb() { return m_za_cb.bind(); }
	auto zb_cb() { return m_zb_cb.bind(); }

	// select
	void s0_w(int state);
	void s1_w(int state);
	void s_w(uint8_t data);

	// input a
	void i0a_w(int state);
	void i1a_w(int state);
	void i2a_w(int state);
	void i3a_w(int state);
	void ia_w(uint8_t data);

	// input b
	void i0b_w(int state);
	void i1b_w(int state);
	void i2b_w(int state);
	void i3b_w(int state);
	void ib_w(uint8_t data);

	// output
	int za_r();
	int zb_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_a();
	void update_b();

	// callbacks
	devcb_write_line m_za_cb;
	devcb_write_line m_zb_cb;

	// state
	bool m_s[2];
	bool m_ia[4];
	bool m_ib[4];
	bool m_z[2];
};

// device type definition
DECLARE_DEVICE_TYPE(TTL153, ttl153_device)

#endif // MAME_DEVICES_MACHINE_74153_H
