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

#define MCFG_PTM6840_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PTM6840, 0) \
	ptm6840_device::static_set_interface(*device, _interface);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ptm6840_interface

struct ptm6840_interface
{
	double m_internal_clock;
	double m_external_clock[3];

	devcb_write8 m_out_cb[3];		// function to call when output[idx] changes
	devcb_write_line m_irq_cb;	// function called if IRQ line changes
};



// ======================> ptm6840_device

class ptm6840_device :  public device_t,
                        public ptm6840_interface
{
public:
    // construction/destruction
    ptm6840_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const ptm6840_interface &interface);

	int status(int clock) const { return m_enabled[clock]; } // get whether timer is enabled
	int irq_state() const { return m_IRQ; }					// get IRQ state
	UINT16 count(int counter) const { return compute_counter(counter); }	// get counter value
	void set_ext_clock(int counter, double clock);	// set clock frequency
	int ext_clock(int counter) const { return m_external_clock[counter]; }	// get clock frequency

	DECLARE_WRITE8_MEMBER( write );
	void write(offs_t offset, UINT8 data) { write(*machine().memory().first_space(), offset, data); }
	DECLARE_READ8_MEMBER( read );
	UINT8 read(offs_t offset) { return read(*machine().memory().first_space(), offset); }

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

	enum
	{
		PTM_6840_CTRL1   = 0,
		PTM_6840_CTRL2   = 1,
		PTM_6840_STATUS  = 1,
		PTM_6840_MSBBUF1 = 2,
		PTM_6840_LSB1	 = 3,
		PTM_6840_MSBBUF2 = 4,
		PTM_6840_LSB2    = 5,
		PTM_6840_MSBBUF3 = 6,
		PTM_6840_LSB3    = 7,
	};

	devcb_resolved_write8 m_out_func[3];	// function to call when output[idx] changes
	devcb_resolved_write_line m_irq_func;	// function called if IRQ line changes

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
