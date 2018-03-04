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
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<0>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<1>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<2>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<3>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<4>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<5>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<6>(DEVCB_##_devcb);
#define MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_q_out_cb<7>(DEVCB_##_devcb);

#define MCFG_ADDRESSABLE_LATCH_PARALLEL_OUT_CB(_devcb) \
	devcb = &downcast<addressable_latch_device &>(*device).set_parallel_out_cb(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> addressable_latch_device

class addressable_latch_device : public device_t
{
public:
	// static configuration
	template<unsigned Bit, class Object> devcb_base &set_q_out_cb(Object &&cb) { return m_q_out_cb[Bit].set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_parallel_out_cb(Object &&cb) { return m_parallel_out_cb.set_callback(std::forward<Object>(cb)); }

	// data write handlers
	void write_bit(offs_t offset, bool d);
	void write_abcd(u8 a, bool d);
	DECLARE_WRITE8_MEMBER(write_d0);
	DECLARE_WRITE8_MEMBER(write_d1);
	DECLARE_WRITE8_MEMBER(write_d7);
	DECLARE_WRITE8_MEMBER(write_a0);
	DECLARE_WRITE8_MEMBER(write_a3);
	DECLARE_WRITE8_MEMBER(write_nibble_d0);
	DECLARE_WRITE8_MEMBER(write_nibble_d3);
	DECLARE_WRITE8_MEMBER(clear);

	// read handlers (inlined for the sake of optimization)
	DECLARE_READ_LINE_MEMBER(q0_r) { return BIT(m_q, 0); }
	DECLARE_READ_LINE_MEMBER(q1_r) { return BIT(m_q, 1); }
	DECLARE_READ_LINE_MEMBER(q2_r) { return BIT(m_q, 2); }
	DECLARE_READ_LINE_MEMBER(q3_r) { return BIT(m_q, 3); }
	DECLARE_READ_LINE_MEMBER(q4_r) { return BIT(m_q, 4); }
	DECLARE_READ_LINE_MEMBER(q5_r) { return BIT(m_q, 5); }
	DECLARE_READ_LINE_MEMBER(q6_r) { return BIT(m_q, 6); }
	DECLARE_READ_LINE_MEMBER(q7_r) { return BIT(m_q, 7); }
	u8 output_state() const { return m_q; }

	// control inputs
	DECLARE_WRITE_LINE_MEMBER(enable_w);
	DECLARE_WRITE_LINE_MEMBER(clear_w);

protected:
	// construction/destruction
	addressable_latch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool clear_active);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal helpers
	void update_bit();
	void clear_outputs(u8 new_q);

	// device callbacks
	devcb_write_line    m_q_out_cb[8];      // output line callback array
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
	ls259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> hc259_device

class hc259_device : public addressable_latch_device
{
public:
	hc259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> hct259_device

class hct259_device : public addressable_latch_device
{
public:
	hct259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> f9334_device

class f9334_device : public addressable_latch_device
{
public:
	f9334_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> cd4099_device

class cd4099_device : public addressable_latch_device
{
public:
	cd4099_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type definition
DECLARE_DEVICE_TYPE(LS259, ls259_device)
DECLARE_DEVICE_TYPE(HC259, hc259_device)
DECLARE_DEVICE_TYPE(HCT259, hct259_device)
DECLARE_DEVICE_TYPE(F9334, f9334_device)
DECLARE_DEVICE_TYPE(CD4099, cd4099_device)

#endif // MAME_MACHINE_74259_H
