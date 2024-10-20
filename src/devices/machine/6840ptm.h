// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

***************************************************************************/

#ifndef MAME_MACHINE_6840PTM_H
#define MAME_MACHINE_6840PTM_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ptm6840_device

class ptm6840_device : public device_t
{
public:
	// construction/destruction
	ptm6840_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_external_clocks(double clock0, double clock1, double clock2) { m_external_clock[0] = clock0; m_external_clock[1] = clock1; m_external_clock[2] = clock2; }
	void set_external_clocks(const XTAL &clock0, const XTAL &clock1, const XTAL &clock2) { set_external_clocks(clock0.dvalue(), clock1.dvalue(), clock2.dvalue()); }
	auto o1_callback() { return m_out_cb[0].bind(); }
	auto o2_callback() { return m_out_cb[1].bind(); }
	auto o3_callback() { return m_out_cb[2].bind(); }
	auto irq_callback() { return m_irq_cb.bind(); }

	int status(int idx) const { return m_enabled[idx]; }            // get whether timer is enabled
	int irq_state() const { return m_irq; }                         // get IRQ state
	int count(int idx) const { return compute_counter(idx); }       // get counter value
	void set_ext_clock(int counter, double clock);                  // set clock frequency
	int ext_clock(int idx) const { return m_external_clock[idx]; }  // get clock frequency

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	void set_gate(int idx, int state);
	void set_g1(int state) { set_gate(0, state); }
	void set_g2(int state) { set_gate(1, state); }
	void set_g3(int state) { set_gate(2, state); }

	void set_clock(int idx, int state);
	void set_c1(int state) { set_clock(0, state); }
	void set_c2(int state) { set_clock(1, state); }
	void set_c3(int state) { set_clock(2, state); }

	void update_interrupts();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

private:
	void deduct_from_counter(int idx);
	void tick(int counter);
	TIMER_CALLBACK_MEMBER(state_changed);

	int compute_counter(int idx) const;
	void reload_counter(int idx);
	void update_expiration_for_clock_source(int idx, bool changed_to_external = false, double new_external_clock = 0.0);

	enum
	{
		PTM_6840_CTRL1   = 0,
		PTM_6840_CTRL2   = 1,
		PTM_6840_STATUS  = 1,
		PTM_6840_MSBBUF1 = 2,
		PTM_6840_LSB1    = 3,
		PTM_6840_MSBBUF2 = 4,
		PTM_6840_LSB2    = 5,
		PTM_6840_MSBBUF3 = 6,
		PTM_6840_LSB3    = 7
	};

	enum
	{
		RESET_TIMERS    = 0x01,
		CR1_SELECT      = 0x01,
		T3_PRESCALE_EN  = 0x01,
		INTERNAL_CLK_EN = 0x02,
		COUNT_MODE_8BIT = 0x04,
		MODE_BITS       = 0x38,
		INTERRUPT_EN    = 0x40,
		COUNT_OUT_EN    = 0x80
	};

	enum
	{
		TIMER1_IRQ  = 0x01,
		TIMER2_IRQ  = 0x02,
		TIMER3_IRQ  = 0x04,
		ANY_IRQ     = 0x80
	};

	double m_external_clock[3];

	devcb_write_line::array<3> m_out_cb;
	devcb_write_line m_irq_cb;

	uint8_t m_control_reg[3];
	bool m_output[3]; // Output states
	bool m_gate[3];   // Counter gate states
	bool m_clk[3];    // Clock states
	bool m_enabled[3];
	uint8_t m_mode[3];
	bool m_single_fired[3];
	uint8_t m_t3_divisor;
	uint8_t m_t3_scaler;
	uint8_t m_irq;
	uint8_t m_status_reg;
	uint8_t m_status_read_since_int;
	uint8_t m_lsb_buffer;
	uint8_t m_msb_buffer;

	// Each PTM has 3 timers
	emu_timer *m_timer[3];

	uint16_t m_latch[3];
	uint16_t m_counter[3];
	attotime m_disable_time[3];

	static const char *const opmode[];
};


// device type definition
DECLARE_DEVICE_TYPE(PTM6840, ptm6840_device)

#endif // MAME_MACHINE_6840PTM_H
