// license:BSD-3-Clause
// copyright-holders:AJR

/***************************************************************************

    74LS157 Quad 2-Line to 1-Line Data Selectors/Multiplexers (TTL)

****************************************************************************
                                ____   ____
                    SELECT   1 |*   \_/    | 16  Vcc
                    A1 IN    2 |           | 15  STROBE
                    B1 IN    3 |           | 14  A4 IN
                    Y1 OUT   4 |           | 13  B4 IN
                    A2 IN    5 |  74LS157  | 12  Y4 OUT
                    B2 IN    6 |           | 11  A3 IN
                    Y2 OUT   7 |           | 10  B3 IN
                    GND      8 |___________|  9  Y3 OUT

***************************************************************************/

#ifndef MAME_MACHINE_74157_H
#define MAME_MACHINE_74157_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ls157_device

class ls157_device : public device_t
{
public:
	// construction/destruction
	ls157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto a_in_callback() { return m_a_in_cb.bind(); }
	auto b_in_callback() { return m_b_in_cb.bind(); }
	auto out_callback() { return m_out_cb.bind(); }

	// data writes
	void a_w(u8 data);
	void b_w(u8 data);
	void ab_w(u8 data);
	void ba_w(u8 data);
	void interleave_w(u8 data);

	// data line writes
	void a0_w(int state);
	void a1_w(int state);
	void a2_w(int state);
	void a3_w(int state);
	void b0_w(int state);
	void b1_w(int state);
	void b2_w(int state);
	void b3_w(int state);

	// control line writes
	void select_w(int state);
	void strobe_w(int state);

	// output read
	u8 output_r();

protected:
	ls157_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal helpers
	void write_a_bit(int bit, bool state);
	void write_b_bit(int bit, bool state);
	void update_output();

	// callbacks & configuration
	devcb_read8     m_a_in_cb;
	devcb_read8     m_b_in_cb;
	devcb_write8    m_out_cb;
	u8              m_data_mask;

	// internal state
	u8              m_a;
	u8              m_b;
	bool            m_select;
	bool            m_strobe;
};

// ======================> ls157_x2_device

class ls157_x2_device : public ls157_device
{
public:
	// construction/destruction
	ls157_x2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> hc157_device

class hc157_device : public ls157_device
{
public:
	// construction/destruction
	hc157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> hct157_device

class hct157_device : public ls157_device
{
public:
	// construction/destruction
	hct157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DECLARE_DEVICE_TYPE(LS157,    ls157_device)
DECLARE_DEVICE_TYPE(LS157_X2, ls157_x2_device)
DECLARE_DEVICE_TYPE(HC157,    hc157_device)
DECLARE_DEVICE_TYPE(HCT157,   hct157_device)

#endif // MAME_MACHINE_74157_H
