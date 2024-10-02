// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    NEC µPD7001 CMOS Serial I/O Analog-to-Digital Converter

**********************************************************************
                              ___    ___
                    _EOC   1 |*  \__/   | 16  Vdd
                      DL   2 |          | 15  Vref
                      SI   3 |          | 14  AGND
                    _SCK   4 |          | 13  A3
                      SO   5 |  D7001C  | 12  A2
                     _CS   6 |          | 11  A1
                     CL0   7 |          | 10  A0
                     CL1   8 |__________| 9   Vss

*********************************************************************/

#ifndef MAME_MACHINE_UPD7001_H
#define MAME_MACHINE_UPD7001_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7001_device

class upd7001_device : public device_t
{
public:
	// device type constructors
	upd7001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	upd7001_device(const machine_config &mconfig, const char *tag, device_t *owner, double r, double c)
		: upd7001_device(mconfig, tag, owner, 0U)
	{
		set_rc(r, c);
	}

	// input callback configuration
	auto a0_callback() { return m_an_callback[0].bind(); }
	auto a1_callback() { return m_an_callback[1].bind(); }
	auto a2_callback() { return m_an_callback[2].bind(); }
	auto a3_callback() { return m_an_callback[3].bind(); }

	// output callback configuration
	auto eoc_callback() { return m_eoc_callback.bind(); }

	// misc. configuration
	void set_rc(double res, double cap) { assert(!configured()); m_res = res; m_cap = cap; }

	// serial interface
	void cs_w(int state);
	void sck_w(int state);
	void si_w(int state) { m_si = state; }
	void dl_w(int state);
	int so_r() { return m_oe ? m_so : 1; }
	int eoc_r() { return m_eoc_active ? 0 : 1; }
	int eoc_so_r() { return eoc_r() && so_r(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	// timing helpers
	TIMER_CALLBACK_MEMBER(conversion_done);
	TIMER_CALLBACK_MEMBER(output_enabled);

	// callback objects
	devcb_read8::array<4> m_an_callback;
	devcb_write_line m_eoc_callback;

	// timing parameters
	double m_res;
	double m_cap;
	attotime m_fck_rc;

	// internal timers
	emu_timer *m_conv_timer;
	emu_timer *m_scsk_timer;

	// internal state
	bool m_cs_active;
	bool m_eoc_active;
	bool m_oe;
	bool m_sck;
	bool m_si;
	bool m_so;
	bool m_dl;
	u8 m_sr;
	u8 m_mpx;
};

// device type declaration
DECLARE_DEVICE_TYPE(UPD7001, upd7001_device)

#endif // MAME_MACHINE_UPD7001_H
