// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Fairchild F3853 SRAM interface with integrated interrupt
    controller and timer

    This chip is a timer shift register, basically the same as in the
    F3851.

****************************************************************************
                            _____   _____
                   Vgg   1 |*    \_/     | 40  Vdd
                   PHI   2 |             | 39  ROMC4
                 WRITE   3 |             | 38  ROMC3
              _INT REQ   4 |             | 37  ROMC2
               _PRI IN   5 |             | 36  ROMC1
            _RAM WRITE   6 |             | 35  ROMC0
              _EXT INT   7 |             | 34  CPU READ
                 ADDR7   8 |             | 33  REG DR
                 ADDR6   9 |             | 32  ADDR15
                 ADDR5  10 |    F3853    | 31  ADDR14
                 ADDR4  11 |             | 30  ADDR13
                 ADDR3  12 |             | 29  ADDR12
                 ADDR2  13 |             | 28  ADDR11
                 ADDR1  14 |             | 27  ADDR10
                 ADDR0  15 |             | 26  ADDR9
                   DB0  16 |             | 25  ADDR8
                   DB1  17 |             | 24  DB7
                   DB2  18 |             | 23  DB6
                   DB3  19 |             | 22  DB5
                   Vss  20 |_____________| 21  DB4

***************************************************************************/

#pragma once

#ifndef __F3853_H__
#define __F3853_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_F3853_EXT_INPUT_CB(_class, _method) \
	f3853_device::set_interrupt_req_callback(*device, f3853_interrupt_req_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef device_delegate<void (UINT16 addr, int level)> f3853_interrupt_req_delegate;

#define F3853_INTERRUPT_REQ_CB(_name) void _name(UINT16 addr, int level)


// ======================> f3853_device

class f3853_device :  public device_t
{
public:
	// construction/destruction
	f3853_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void set_interrupt_req_callback(device_t &device, f3853_interrupt_req_delegate callback) { downcast<f3853_device &>(device).m_interrupt_req_cb = callback; }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_external_interrupt_in_line(int level);
	void set_priority_in_line(int level);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

	static TIMER_CALLBACK( f3853_timer_callback );

private:

	void set_interrupt_request_line();
	void timer_start(UINT8 value);
	void timer();

	f3853_interrupt_req_delegate m_interrupt_req_cb;
	UINT8 m_high;
	UINT8 m_low; // Bit 7 is set to 0 for timer interrupts, 1 for external interrupts
	INT32 m_external_enable;
	INT32 m_timer_enable;

	INT32 m_request_flipflop;

	INT32 m_priority_line;              /* inverted level*/
	INT32 m_external_interrupt_line;    /* inverted level */

	emu_timer *m_timer;

	UINT8 m_value_to_cycle[0x100];
};


// device type definition
extern const device_type F3853;

#endif /* __F3853_H__ */
