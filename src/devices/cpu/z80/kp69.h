// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP69 Interrupt Controller

***************************************************************************/

#ifndef MAME_CPU_Z80_KP69_H
#define MAME_CPU_Z80_KP69_H

#pragma once

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kp69_device : public device_t, public device_z80daisy_interface
{
public:
	// construction/destruction
	kp69_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback to CPU
	auto int_callback() { return m_int_callback.bind(); }

	// read handlers
	u8 isrl_r();
	u8 isrh_r();
	u8 imrl_r();
	u8 imrh_r();

	// write handlers
	void lerl_pgrl_w(u8 data);
	void lerh_pgrh_w(u8 data);
	void imrl_w(u8 data);
	void ivr_imrh_w(u8 data);

	// interrupt inputs
	template <int N> DECLARE_WRITE_LINE_MEMBER(ir_w)
	{
		static_assert(N >= 0 && N < 16, "Invalid level");
		set_input_level(N, state);
	}

	void add_to_state(device_state_interface &state, int index);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	// internal helpers
	bool int_active() const;
	void set_int(bool active);
	void set_input_level(int level, bool state);
	void set_irr(u16 data);
	void set_isr(u16 data);
	void set_imr(u16 data);
	void set_ler(u16 data);
	void set_pgr(u16 data);

	// callback object
	devcb_write_line m_int_callback;

	// internal state
	u16 m_input_levels;
	u16 m_irr;
	u16 m_isr;
	bool m_illegal_state;
	u8 m_ivr;
	bool m_ivr_written;
	u16 m_imr;
	u16 m_ler;
	u16 m_pgr;
	bool m_int_active;
};

// device type declaration
DECLARE_DEVICE_TYPE(KP69, kp69_device)

#endif // MAME_CPU_Z80_KP69_H
