/**********************************************************************

    Intel 8214 Priority Interrupt Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   _B0   1 |*    \_/     | 24  Vcc
                   _B1   2 |             | 23  _ECS
                   _B2   3 |             | 22  _R7
                  _SGS   4 |             | 21  _R6
                  _INT   5 |             | 20  _R5
                  _CLK   6 |    8214     | 19  _R4
                  INTE   7 |             | 18  _R3
                   _A0   8 |             | 17  _R2
                   _A1   9 |             | 16  _R1
                   _A2  10 |             | 15  _R0
                  _ELR  11 |             | 14  ENLG
                   GND  12 |_____________| 13  ETLG

**********************************************************************/

#pragma once

#ifndef __I8214__
#define __I8214__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I8214_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, I8214, _clock)	\
	MCFG_DEVICE_CONFIG(_config)


#define I8214_INTERFACE(name) \
	const i8214_interface (name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i8214_interface

struct i8214_interface
{
	devcb_write_line	m_out_int_func;
	devcb_write_line	m_out_enlg_func;
};


// ======================> i8214_device_config

class i8214_device_config :   public device_config,
                              public i8214_interface
{
    friend class i8214_device;

    // construction/destruction
    i8214_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};



// ======================> i8214_device

class i8214_device :	public device_t
{
    friend class i8214_device_config;

    // construction/destruction
    i8214_device(running_machine &_machine, const i8214_device_config &_config);

public:
	DECLARE_WRITE_LINE_MEMBER( sgs_w );
	DECLARE_WRITE_LINE_MEMBER( etlg_w );
	DECLARE_WRITE_LINE_MEMBER( inte_w );

    UINT8 a_r();
	void b_w(UINT8 data);
	void r_w(UINT8 data);

protected:
    // device-level overrides
    virtual void device_start();

private:
	inline void trigger_interrupt(int level);
	inline void check_interrupt();

	devcb_resolved_write_line	m_out_int_func;
	devcb_resolved_write_line	m_out_enlg_func;

	int m_inte;					// interrupt enable
	int m_int_dis;				// interrupt disable flip-flop
	int m_a;					// request level
	int m_b;					// current status register
	UINT8 m_r;					// interrupt request latch
	int m_sgs;					// status group select
	int m_etlg;					// enable this level group

	const i8214_device_config &m_config;
};


// device type definition
extern const device_type I8214;



#endif
