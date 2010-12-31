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
    devcb_read8         m_in_a_func;
    devcb_read8         m_in_b_func;
    devcb_write8        m_out_a_func;
    devcb_write8        m_out_b_func;
    devcb_write_line    m_irq_func;
};



// ======================> riot6532_device_config

class riot6532_device_config : public device_config,
                               public riot6532_interface
{
    friend class riot6532_device;

    // construction/destruction
    riot6532_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> riot6532_device

class riot6532_device :  public device_t
{
    friend class riot6532_device_config;

    // construction/destruction
    riot6532_device(running_machine &_machine, const riot6532_device_config &_config);

public:
    UINT8 reg_r(UINT8 offset);
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

    int             m_index;

    riot6532_port   m_port[2];

    devcb_resolved_write_line   m_irq_func;

    UINT8           m_irqstate;
    UINT8           m_irqenable;

    UINT8           m_pa7dir;     /* 0x80 = high-to-low, 0x00 = low-to-high */
    UINT8           m_pa7prev;

    UINT8           m_timershift;
    UINT8           m_timerstate;
    emu_timer *     m_timer;

    const riot6532_device_config &m_config;
};


// device type definition
extern const device_type RIOT6532;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( riot6532_r );
WRITE8_DEVICE_HANDLER( riot6532_w );

void riot6532_porta_in_set(device_t *device, UINT8 data, UINT8 mask);
void riot6532_portb_in_set(device_t *device, UINT8 data, UINT8 mask);

UINT8 riot6532_porta_in_get(device_t *device);
UINT8 riot6532_portb_in_get(device_t *device);

UINT8 riot6532_porta_out_get(device_t *device);
UINT8 riot6532_portb_out_get(device_t *device);

#endif
