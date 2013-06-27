/***************************************************************************

  RIOT 6532 emulation

***************************************************************************/

#pragma once

#ifndef __RIOT6532_H__
#define __RIOT6532_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RIOT6532_ADD(_tag, _clock, _intrf) \
	MCFG_DEVICE_ADD(_tag, RIOT6532, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> riot6532_interface

struct riot6532_interface
{
	devcb_read8         m_in_a_cb;
	devcb_read8         m_in_b_cb;
	devcb_write8        m_out_a_cb;
	devcb_write8        m_out_b_cb;
	devcb_write_line    m_irq_cb;
};



// ======================> riot6532_device

class riot6532_device :  public device_t,
							public riot6532_interface
{
public:
	// construction/destruction
	riot6532_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 reg_r(UINT8 offset, bool debugger_access = false);
	void reg_w(UINT8 offset, UINT8 data);

	void porta_in_set(UINT8 data, UINT8 mask);
	void portb_in_set(UINT8 data, UINT8 mask);

	UINT8 porta_in_get();
	UINT8 portb_in_get();

	UINT8 porta_out_get();
	UINT8 portb_out_get();

	void timer_end();

protected:
	class riot6532_port
	{
	public:
		UINT8                   m_in;
		UINT8                   m_out;
		UINT8                   m_ddr;
		devcb_resolved_read8    m_in_func;
		devcb_resolved_write8   m_out_func;
	};

	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

	static TIMER_CALLBACK( timer_end_callback );

private:
	void update_irqstate();
	UINT8 apply_ddr(const riot6532_port *port);
	void update_pa7_state();
	UINT8 get_timer();

	riot6532_port   m_port[2];

	devcb_resolved_write_line   m_irq_func;

	UINT8           m_irqstate;
	UINT8           m_irqenable;
	int             m_irq;

	UINT8           m_pa7dir;     /* 0x80 = high-to-low, 0x00 = low-to-high */
	UINT8           m_pa7prev;

	UINT8           m_timershift;
	UINT8           m_timerstate;
	emu_timer *     m_timer;
};


// device type definition
extern const device_type RIOT6532;

#endif
