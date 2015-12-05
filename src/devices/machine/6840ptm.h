// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

***************************************************************************/

#pragma once

#ifndef __6840PTM_H__
#define __6840PTM_H__

#include "emu.h"



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PTM6840_INTERNAL_CLOCK(_clk) \
	ptm6840_device::set_internal_clock(*device, _clk);

#define MCFG_PTM6840_EXTERNAL_CLOCKS(_clk0, _clk1, _clk2) \
	ptm6840_device::set_external_clocks(*device, _clk0, _clk1, _clk2);

#define MCFG_PTM6840_OUT0_CB(_devcb) \
	devcb = &ptm6840_device::set_out0_callback(*device, DEVCB_##_devcb);

#define MCFG_PTM6840_OUT1_CB(_devcb) \
	devcb = &ptm6840_device::set_out1_callback(*device, DEVCB_##_devcb);

#define MCFG_PTM6840_OUT2_CB(_devcb) \
	devcb = &ptm6840_device::set_out2_callback(*device, DEVCB_##_devcb);

#define MCFG_PTM6840_IRQ_CB(_devcb) \
	devcb = &ptm6840_device::set_irq_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ptm6840_device

class ptm6840_device :  public device_t
{
public:
	// construction/destruction
	ptm6840_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_internal_clock(device_t &device, double clock) { downcast<ptm6840_device &>(device).m_internal_clock = clock; }
	static void set_external_clocks(device_t &device, double clock0, double clock1, double clock2) { downcast<ptm6840_device &>(device).m_external_clock[0] = clock0; downcast<ptm6840_device &>(device).m_external_clock[1] = clock1; downcast<ptm6840_device &>(device).m_external_clock[2] = clock2; }
	template<class _Object> static devcb_base &set_out0_callback(device_t &device, _Object object) { return downcast<ptm6840_device &>(device).m_out0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out1_callback(device_t &device, _Object object) { return downcast<ptm6840_device &>(device).m_out1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out2_callback(device_t &device, _Object object) { return downcast<ptm6840_device &>(device).m_out2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<ptm6840_device &>(device).m_irq_cb.set_callback(object); }

	int status(int clock) const { return m_enabled[clock]; } // get whether timer is enabled
	int irq_state() const { return m_IRQ; }                 // get IRQ state
	UINT16 count(int counter) const { return compute_counter(counter); }    // get counter value
	void set_ext_clock(int counter, double clock);  // set clock frequency
	int ext_clock(int counter) const { return m_external_clock[counter]; }  // get clock frequency

	DECLARE_WRITE8_MEMBER( write );
	void write(offs_t offset, UINT8 data) { write(machine().driver_data()->generic_space(), offset, data); }
	DECLARE_READ8_MEMBER( read );
	UINT8 read(offs_t offset) { return read(machine().driver_data()->generic_space(), offset); }

	void set_gate(int idx, int state);
	DECLARE_WRITE_LINE_MEMBER( set_g1 );
	DECLARE_WRITE_LINE_MEMBER( set_g2 );
	DECLARE_WRITE_LINE_MEMBER( set_g3 );

	void set_clock(int idx, int state);
	DECLARE_WRITE_LINE_MEMBER( set_c1 );
	DECLARE_WRITE_LINE_MEMBER( set_c2 );
	DECLARE_WRITE_LINE_MEMBER( set_c3 );

	void update_interrupts();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void subtract_from_counter(int counter, int count);
	void tick(int counter, int count);
	void timeout(int idx);

	UINT16 compute_counter(int counter) const;
	void reload_count(int idx);

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

	double m_internal_clock;
	double m_external_clock[3];

	devcb_write8 m_out0_cb;
	devcb_write8 m_out1_cb;
	devcb_write8 m_out2_cb;
	devcb_write_line m_irq_cb;  // function called if IRQ line changes

	UINT8 m_control_reg[3];
	UINT8 m_output[3]; // Output states
	UINT8 m_gate[3];   // Input gate states
	UINT8 m_clk[3];  // Clock states
	UINT8 m_enabled[3];
	UINT8 m_mode[3];
	UINT8 m_fired[3];
	UINT8 m_t3_divisor;
	UINT8 m_t3_scaler;
	UINT8 m_IRQ;
	UINT8 m_status_reg;
	UINT8 m_status_read_since_int;
	UINT8 m_lsb_buffer;
	UINT8 m_msb_buffer;

	// Each PTM has 3 timers
	emu_timer *m_timer[3];

	UINT16 m_latch[3];
	UINT16 m_counter[3];

	static const char *const opmode[];
};


// device type definition
extern const device_type PTM6840;


#endif /* __6840PTM_H__ */
