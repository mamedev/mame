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
#include "cpu/z80/z80daisy.h"



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
	devcb_write_line        m_out_int_cb;

	devcb_read8             m_in_pa_cb;
	devcb_write8            m_out_pa_cb;

	devcb_read8             m_in_pb_cb;
	devcb_write8            m_out_pb_cb;

	devcb_read8             m_in_pc_cb;
	devcb_write8            m_out_pc_cb;
};


// ======================> z8536_device

class z8536_device :  public device_t,
						public device_z80daisy_interface,
						public z8536_interface
{
public:
	// construction/destruction
	z8536_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	int intack_r();

	DECLARE_WRITE_LINE_MEMBER( pa0_w );
	DECLARE_WRITE_LINE_MEMBER( pa1_w );
	DECLARE_WRITE_LINE_MEMBER( pa2_w );
	DECLARE_WRITE_LINE_MEMBER( pa3_w );
	DECLARE_WRITE_LINE_MEMBER( pa4_w );
	DECLARE_WRITE_LINE_MEMBER( pa5_w );
	DECLARE_WRITE_LINE_MEMBER( pa6_w );
	DECLARE_WRITE_LINE_MEMBER( pa7_w );

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

private:
	static const device_timer_id TIMER_1 = 0;
	static const device_timer_id TIMER_2 = 1;
	static const device_timer_id TIMER_3 = 2;

	static const int PORT_A = 0;
	static const int PORT_B = 1;
	static const int PORT_C = 2;

	inline void get_interrupt_vector();
	inline void check_interrupt();

	inline UINT8 read_register(offs_t offset);
	inline UINT8 read_register(offs_t offset, UINT8 mask);
	inline void write_register(offs_t offset, UINT8 data);
	inline void write_register(offs_t offset, UINT8 data, UINT8 mask);

	inline bool counter_enabled(device_timer_id id);
	inline bool counter_external_output(device_timer_id id);
	inline bool counter_external_count(device_timer_id id);
	inline bool counter_external_trigger(device_timer_id id);
	inline bool counter_external_gate(device_timer_id id);
	inline bool counter_gated(device_timer_id id);
	inline void count(device_timer_id id);
	inline void trigger(device_timer_id id);
	inline void gate(device_timer_id id, int state);
	inline void match_pattern(int port);
	inline void external_port_w(int port, int bit, int state);

	devcb_resolved_write_line       m_out_int_func;

	devcb_resolved_read8            m_in_pa_func;
	devcb_resolved_write8           m_out_pa_func;

	devcb_resolved_read8            m_in_pb_func;
	devcb_resolved_write8           m_out_pb_func;

	devcb_resolved_read8            m_in_pc_func;
	devcb_resolved_write8           m_out_pc_func;

	// interrupt state
	int m_int;

	// register state
	int m_state;
	UINT8 m_register[48];
	UINT8 m_pointer;

	// input/output port state
	UINT8 m_input[3];
	UINT8 m_output[3];
	UINT8 m_buffer[3];
	UINT8 m_match[3];

	// timers
	emu_timer *m_timer;
	UINT16 m_counter[3];
};


// device type definition
extern const device_type Z8536;



#endif
