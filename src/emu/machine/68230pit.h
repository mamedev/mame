// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
/**********************************************************************

    Motorola MC68230 PI/T Parallell Interface and Timer

**********************************************************************/
#pragma once

#ifndef __68230PTI_H__
#define __68230PTI_H__

#include "emu.h"

#define PIT_68230_PGCR        0x00 /* Port General Control register */
#define PIT_68230_PSRR        0x01 /*   Port Service Request register */
#define PIT_68230_PADDR       0x02 /*   Port A Data Direction register */
#define PIT_68230_PBDDR       0x03 /*   Port B Data Direction register */
#define PIT_68230_PCDDR       0x04 /*   Port C Data Direction register */
#define PIT_68230_PIVR        0x05 /*   Port Interrupt vector register */
#define PIT_68230_PACR        0x06 /*   Port A Control register */
#define PIT_68230_PBCR        0x07 /*   Port B Control register */
#define PIT_68230_PADR        0x08 /*  Port A Data register */
#define PIT_68230_PBDR        0x09 /*  Port B Data register */
#define PIT_68230_PAAR        0x0a /*   Port A Alternate register */
#define PIT_68230_PBAR        0x0b /*   Port B Alternate register */
#define PIT_68230_PCDR        0x0c /*   Port C Data register */
#define PIT_68230_PSR         0x0d /*  Port Status register */
#define PIT_68230_TCR         0x10 /*   Timer Control Register */
#define PIT_68230_TIVR        0x11 /*   Timer Interrupt Vector Register */
#define PIT_68230_CPRH        0x13 /*   Counter Preload Register High */
#define PIT_68230_CPRM        0x14 /*   Counter Preload Register Middle */
#define PIT_68230_CPRL        0x15 /*   Counter Preload Register Low */
#define PIT_68230_CNTRH       0x17 /*   Counter Register High */
#define PIT_68230_CNTRM       0x18 /*   Counter Register Middle */
#define PIT_68230_CNTRL       0x19 /*   Counter Register Low */
#define PIT_68230_TSR         0x1A /*  Timer Status Register */

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pit68230_device

class pit68230_device :  public device_t
{
public:
	// construction/destruction
	pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_internal_clock(device_t &device, double clock) { downcast<pit68230_device &>(device).m_internal_clock = clock; }
	static void set_external_clock(device_t &device, double clock) { downcast<pit68230_device &>(device).m_external_clock = clock; }

	template<class _Object> static devcb_base &set_out0_callback(device_t &device, _Object object) { return downcast<pit68230_device &>(device).m_out0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out1_callback(device_t &device, _Object object) { return downcast<pit68230_device &>(device).m_out1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out2_callback(device_t &device, _Object object) { return downcast<pit68230_device &>(device).m_out2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<pit68230_device &>(device).m_irq_cb.set_callback(object); }

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
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void subtract_from_counter(int counter, int count);
	void tick(int counter, int count);
	void timeout(int idx);

	UINT16 compute_counter(int counter) const;
	void reload_count(int idx);

/*
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
		PTM_6840_LSB3    = 7,
	};
*/
	double m_internal_clock;
	double m_external_clock[3];

	devcb_write8 m_out0_cb;
	devcb_write8 m_out1_cb;
	devcb_write8 m_out2_cb;
	devcb_write_line m_irq_cb;  // function called if IRQ line changes

	UINT8 m_control_reg[3];
	UINT8 m_output[3]; // Output states
	UINT8 m_gate[3];   // Input gate states
	UINT8 m_clk;  // Clock states
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
extern const device_type PIT68230;




#endif // __68230PTI__
