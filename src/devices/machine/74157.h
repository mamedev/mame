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

#pragma once

#ifndef DEVICES_MACHINE_74157_H
#define DEVICES_MACHINE_74157_H



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_74157_OUT_CB(_devcb) \
	devcb = &ls157_device::set_out_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ls157_device

class ls157_device : public device_t
{
public:
	// construction/destruction
	ls157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	ls157_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

	// static configuration
	template<class _Object> static devcb_base &set_out_callback(device_t &device, _Object object) { return downcast<ls157_device &>(device).m_out_cb.set_callback(object); }

	// data writes
	DECLARE_WRITE8_MEMBER(a_w);
	void a_w(u8 data);
	DECLARE_WRITE8_MEMBER(b_w);
	void b_w(u8 data);
	DECLARE_WRITE8_MEMBER(ab_w);
	void ab_w(u8 data);
	DECLARE_WRITE8_MEMBER(ba_w);
	void ba_w(u8 data);
	DECLARE_WRITE8_MEMBER(interleave_w);
	void interleave_w(u8 data);

	// line writes
	DECLARE_WRITE_LINE_MEMBER(select_w);
	DECLARE_WRITE_LINE_MEMBER(strobe_w);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal helpers
	void update_output();

	// callbacks
	devcb_write8    m_out_cb;

	// internal state
	u8              m_a;
	u8              m_b;
	bool            m_select;
	bool            m_strobe;
};

class hct157_device : public ls157_device
{
public:
	// construction/destruction
	hct157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type LS157;
extern const device_type HCT157;


#endif
