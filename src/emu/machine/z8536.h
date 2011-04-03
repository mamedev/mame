/**********************************************************************

    Zilog Z8536 Counter/Timer and Parallel I/O emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    D4   1 |*    \_/     | 40  D3
                    D5   2 |             | 39  D2
                    D6   3 |             | 38  D1
                    D7   4 |             | 37  D0
                   _RD   5 |             | 36  _CE
                   _WR   6 |             | 35  A1
                   GND   7 |             | 34  A0
                   PB0   8 |             | 33  PA0
                   PB1   9 |             | 32  PA1
                   PB2  10 |    Z8536    | 31  PA2
                   PB3  11 |             | 30  PA3
                   PB4  12 |             | 29  PA4
                   PB5  13 |             | 28  PA5
                   PB6  14 |             | 27  PA6
                   PB7  15 |             | 26  PA7
                  PCLK  16 |             | 25  _INTACK
                   IEI  17 |             | 24  _INT
                   IEO  18 |             | 23  +5 V
                   PC0  19 |             | 22  PC3
                   PC1  20 |_____________| 21  PC2

**********************************************************************/

#pragma once

#ifndef __Z8536__
#define __Z8536__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z8536_ADD(_tag, _clock, _intrf) \
	MCFG_DEVICE_ADD(_tag, Z8536, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#define Z8536_INTERFACE(name) \
	const z8536_interface (name)=



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z8536_interface

struct z8536_interface
{
	devcb_write_line		m_out_int_func;

	devcb_read8				m_in_pa_func;
	devcb_write8			m_out_pa_func;

	devcb_read8				m_in_pb_func;
	devcb_write8			m_out_pb_func;

	devcb_read8				m_in_pc_func;
	devcb_write8			m_out_pc_func;
};


// ======================> z8536_device_config

class z8536_device_config :   public device_config,
                                public z8536_interface
{
    friend class z8536_device;

    // construction/destruction
    z8536_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> z8536_device

class z8536_device :  public device_t
{
    friend class z8536_device_config;

    // construction/destruction
    z8536_device(running_machine &_machine, const z8536_device_config &_config);

public:
    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( intack_w );

	DECLARE_WRITE_LINE_MEMBER( pb0_w );
	DECLARE_WRITE_LINE_MEMBER( pb1_w );
	DECLARE_WRITE_LINE_MEMBER( pb2_w );
	DECLARE_WRITE_LINE_MEMBER( pb3_w );
	DECLARE_WRITE_LINE_MEMBER( pb4_w );
	DECLARE_WRITE_LINE_MEMBER( pb5_w );
	DECLARE_WRITE_LINE_MEMBER( pb6_w );
	DECLARE_WRITE_LINE_MEMBER( pb7_w );
	DECLARE_WRITE_LINE_MEMBER( pc0_w );
	DECLARE_WRITE_LINE_MEMBER( pc1_w );
	DECLARE_WRITE_LINE_MEMBER( pc2_w );
	DECLARE_WRITE_LINE_MEMBER( pc3_w );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_1 = 0;
	static const device_timer_id TIMER_2 = 1;
	static const device_timer_id TIMER_3 = 2;

	inline UINT8 read_register(offs_t offset);
	inline UINT8 read_register(offs_t offset, UINT8 mask);
	inline void write_register(offs_t offset, UINT8 data);
	inline void write_register(offs_t offset, UINT8 data, UINT8 mask);

	inline void count(device_timer_id id);
	inline void trigger(device_timer_id id);
	inline void gate(device_timer_id id);

	devcb_resolved_write_line		m_out_int_func;

	devcb_resolved_read8			m_in_pa_func;
	devcb_resolved_write8			m_out_pa_func;

	devcb_resolved_read8			m_in_pb_func;
	devcb_resolved_write8			m_out_pb_func;

	devcb_resolved_read8			m_in_pc_func;
	devcb_resolved_write8			m_out_pc_func;

	int m_state;
	UINT8 m_register[48];
	UINT8 m_pointer;
	UINT8 m_input[3];
	UINT8 m_output[3];
	UINT8 m_buffer[3];

	// timers
	emu_timer *m_timer[3];

	const z8536_device_config &m_config;
};


// device type definition
extern const device_type Z8536;



#endif
