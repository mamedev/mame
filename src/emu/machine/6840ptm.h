/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

***************************************************************************/

#pragma once

#ifndef __6840PTM_H__
#define __6840PTM_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PTM6840_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, PTM6840, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> ptm6840_interface

struct ptm6840_interface
{
	double m_internal_clock;
	double m_external_clock[3];

	devcb_write8 m_out_func[3];		// function to call when output[idx] changes
	devcb_write_line m_irq_func;	// function called if IRQ line changes
};



// ======================> ptm6840_device_config

class ptm6840_device_config : public device_config,
                               public ptm6840_interface
{
    friend class ptm6840_device;

    // construction/destruction
    ptm6840_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> ptm6840_device

class ptm6840_device :  public device_t
{
    friend class ptm6840_device_config;

    // construction/destruction
    ptm6840_device(running_machine &_machine, const ptm6840_device_config &_config);

public:

	int ptm6840_get_status(int clock);		// get whether timer is enabled
	int ptm6840_get_irq();					// get IRQ state
	UINT16 ptm6840_get_count(int counter);	// get counter value
	void ptm6840_set_ext_clock(int counter, double clock);	// set clock frequency
	int ptm6840_get_ext_clock(int counter);	// get clock frequency

	void ptm6840_set_g1(UINT32 offset, UINT8 data);	// set gate1 state
	void ptm6840_set_g2(UINT32 offset, UINT8 data);	// set gate2 state
	void ptm6840_set_g3(UINT32 offset, UINT8 data);	// set gate3 state
	void ptm6840_set_c1(UINT32 offset, UINT8 data);	// set clock1 state
	void ptm6840_set_c2(UINT32 offset, UINT8 data);	// set clock2 state
	void ptm6840_set_c3(UINT32 offset, UINT8 data);	// set clock3 state

	void ptm6840_write(UINT32 offset, UINT8 data);
	UINT8 ptm6840_read(UINT32 offset);

	void ptm6840_set_gate(int state, int idx);
	void ptm6840_set_clock(int state, int idx);

	void update_interrupts();

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( ptm6840_timer1_cb );
	static TIMER_CALLBACK( ptm6840_timer2_cb );
	static TIMER_CALLBACK( ptm6840_timer3_cb );

private:

	void subtract_from_counter(int counter, int count);
	void ptm_tick(int counter, int count);

	void ptm6840_timeout(int idx);

	UINT16 compute_counter(int counter);
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

	double m_internal_clock;
	double m_external_clock[3];

	devcb_resolved_write8 m_out_func[3];	// function to call when output[idx] changes
	devcb_resolved_write_line m_irq_func;	// function called if IRQ line changes

	UINT8 m_control_reg[3];
	UINT8 m_output[3]; /* Output states */
	UINT8 m_gate[3];   /* Input gate states */
	UINT8 m_clk[3];  /* Clock states */
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

	/* Each PTM has 3 timers */
	emu_timer *m_timer[3];

	UINT16 m_latch[3];
	UINT16 m_counter[3];

    const ptm6840_device_config &m_config;

	static const char *const opmode[];
};


// device type definition
extern const device_type PTM6840;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

int ptm6840_get_status( device_t *device, int clock );	// get whether timer is enabled
int ptm6840_get_irq( device_t *device );					// get IRQ state
UINT16 ptm6840_get_count( device_t *device, int counter );// get counter value
void ptm6840_set_ext_clock( device_t *device, int counter, double clock ); // set clock frequency
int ptm6840_get_ext_clock( device_t *device, int counter );// get clock frequency

WRITE8_DEVICE_HANDLER( ptm6840_set_g1 );	// set gate1 state
WRITE8_DEVICE_HANDLER( ptm6840_set_g2 );	// set gate2 state
WRITE8_DEVICE_HANDLER( ptm6840_set_g3 );	// set gate3 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c1 );	// set clock1 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c2 );	// set clock2 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c3 );	// set clock3 state

WRITE8_DEVICE_HANDLER( ptm6840_write );
READ8_DEVICE_HANDLER( ptm6840_read );

#endif /* __6840PTM_H__ */
