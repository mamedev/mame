// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    74259/9334 8-Bit Addressable Latch

***********************************************************************
                              ____   ____
                      A0   1 |*   \_/    | 16  Vcc
                      A1   2 |           | 15  /C
                      A2   3 |           | 14  /E
                      Q0   4 |  SN74259  | 13  D
                      Q1   5 |  F  9334  | 12  Q7
                      Q2   6 |           | 11  Q6
                      Q3   7 |           | 10  Q5
                     GND   8 |___________|  9  Q4

                              ____   ____
                      Q7   1 |*   \_/    | 16  Vdd
                   RESET   2 |           | 15  Q6
                    DATA   3 |           | 14  Q5
           WRITE DISABLE   4 |           | 13  Q4
                      A0   5 |  CD4099B  | 12  Q3
                      A1   6 |           | 11  Q2
                      A2   7 |           | 10  Q1
                     Vss   8 |___________|  9  Q0

**********************************************************************/

#ifndef MAME_MACHINE_74259_H
#define MAME_MACHINE_74259_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> addressable_latch_device

class addressable_latch_device : public device_t
{
public:
	// static configuration
	template <unsigned Bit> auto q_out_cb() { return m_q_out_cb[Bit].bind(); }
	auto parallel_out_cb() { return m_parallel_out_cb.bind(); }

	// data write handlers
	void write_bit(offs_t offset, bool d);
	void write_abcd(u8 a, bool d);
	void write_d0(offs_t offset, u8 data);
	void write_d1(offs_t offset, u8 data);
	void write_d7(offs_t offset, u8 data);
	void write_a0(offs_t offset, u8 data = 0);
	void write_a3(offs_t offset, u8 data = 0);
	void write_nibble_d0(u8 data);
	void write_nibble_d3(u8 data);
	void clear(u8 data = 0);

	// read handlers (inlined for the sake of optimization)
	int q0_r() { return BIT(m_q, 0); }
	int q1_r() { return BIT(m_q, 1); }
	int q2_r() { return BIT(m_q, 2); }
	int q3_r() { return BIT(m_q, 3); }
	int q4_r() { return BIT(m_q, 4); }
	int q5_r() { return BIT(m_q, 5); }
	int q6_r() { return BIT(m_q, 6); }
	int q7_r() { return BIT(m_q, 7); }
	u8 output_state() const { return m_q; }

	// control inputs
	void enable_w(int state);
	void clear_w(int state);

protected:
	// construction/destruction
	addressable_latch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool clear_active);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void update_bit();
	void clear_outputs(u8 new_q);

	// device callbacks
	devcb_write_line::array<8> m_q_out_cb;      // output line callback array
	devcb_write8        m_parallel_out_cb;  // parallel output option

	// miscellaneous configuration
	bool const          m_clear_active;     // active state of clear line

	// internal state
	u8      m_address;                  // address input
	bool    m_data;                     // data bit input
	u8      m_q;                        // latched output state
	bool    m_enable;                   // enable/load active state
	bool    m_clear;                    // clear/reset active state
};

// ======================> ls259_device

class ls259_device : public addressable_latch_device
{
public:
	ls259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> hc259_device

class hc259_device : public addressable_latch_device
{
public:
	hc259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> hct259_device

class hct259_device : public addressable_latch_device
{
public:
	hct259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> f9334_device

class f9334_device : public addressable_latch_device
{
public:
	f9334_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> cd4099_device

class cd4099_device : public addressable_latch_device
{
public:
	cd4099_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// device type definition
DECLARE_DEVICE_TYPE(LS259, ls259_device)
DECLARE_DEVICE_TYPE(HC259, hc259_device)
DECLARE_DEVICE_TYPE(HCT259, hct259_device)
DECLARE_DEVICE_TYPE(F9334, f9334_device)
DECLARE_DEVICE_TYPE(CD4099, cd4099_device)

#endif // MAME_MACHINE_74259_H
