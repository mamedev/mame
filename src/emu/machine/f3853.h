/**********************************************************************

    Fairchild F3853 SRAM interface with integrated interrupt
    controller and timer

    This chip is a timer shift register, basically the same as in the
    F3851.

**********************************************************************/

#pragma once

#ifndef __F3853_H__
#define __F3853_H__

#include "emu.h"




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_F3853_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, F3853, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> f3853_interface

struct f3853_interface
{
    void (*m_interrupt_request)(running_device *device, UINT16 addr, int level);
};


// ======================> f3853_device_config

class f3853_device_config :   public device_config,
							  public f3853_interface
{
    friend class f3853_device;

    // construction/destruction
    f3853_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> f3853_device

class f3853_device :  public device_t
{
    friend class f3853_device_config;

    // construction/destruction
    f3853_device(running_machine &_machine, const f3853_device_config &_config);

public:

	UINT8 f3853_r(UINT32 offset);
	void f3853_w(UINT32 offset, UINT8 data);

	void f3853_set_external_interrupt_in_line(int level);
	void f3853_set_priority_in_line(int level);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( f3853_timer_callback );

private:

	void f3853_set_interrupt_request_line();
	void f3853_timer_start(UINT8 value);
	void f3853_timer();

    UINT8 m_high;
    UINT8 m_low; // Bit 7 is set to 0 for timer interrupts, 1 for external interrupts
    INT32 m_external_enable;
    INT32 m_timer_enable;

    INT32 m_request_flipflop;

    INT32 m_priority_line;				/* inverted level*/
    INT32 m_external_interrupt_line;	/* inverted level */

    emu_timer *m_timer;

	UINT8 m_value_to_cycle[0x100];

    const f3853_device_config &m_config;
};


// device type definition
extern const device_type F3853;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( f3853_r );
WRITE8_DEVICE_HANDLER( f3853_w );

void f3853_set_external_interrupt_in_line(running_device *device, int level);
void f3853_set_priority_in_line(running_device *device, int level);

#endif /* __F3853_H__ */
