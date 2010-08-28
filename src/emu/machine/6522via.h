/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of 6522
    versatile interface adapters.

    This is based on the pre-existing 6821 emulation.

    Written by Mathis Rosenhauer

**********************************************************************/

#pragma once

#ifndef __6522VIA_H__
#define __6522VIA_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_VIA6522_ADD(_tag, _clock, _intrf) \
    MDRV_DEVICE_ADD(_tag, VIA6522, _clock) \
    MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define	VIA_PB	    0
#define	VIA_PA	    1
#define	VIA_DDRB    2
#define	VIA_DDRA    3
#define	VIA_T1CL    4
#define	VIA_T1CH    5
#define	VIA_T1LL    6
#define	VIA_T1LH    7
#define	VIA_T2CL    8
#define	VIA_T2CH    9
#define	VIA_SR     10
#define	VIA_ACR    11
#define	VIA_PCR    12
#define	VIA_IFR    13
#define	VIA_IER    14
#define	VIA_PANH   15


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> via6522_interface

struct via6522_interface
{
    devcb_read8 m_in_a_func;
    devcb_read8 m_in_b_func;
    devcb_read_line m_in_ca1_func;
    devcb_read_line m_in_cb1_func;
    devcb_read_line m_in_ca2_func;
    devcb_read_line m_in_cb2_func;
    devcb_write8 m_out_a_func;
    devcb_write8 m_out_b_func;
    devcb_write_line m_out_ca1_func;
    devcb_write_line m_out_cb1_func;
    devcb_write_line m_out_ca2_func;
    devcb_write_line m_out_cb2_func;
    devcb_write_line m_irq_func;
};


// ======================> via6522_device_config

class via6522_device_config :   public device_config,
                                public via6522_interface
{
    friend class via6522_device;

    // construction/destruction
    via6522_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> via6522_device

class via6522_device :  public device_t
{
    friend class via6522_device_config;
    friend class dart_channel;

    // construction/destruction
    via6522_device(running_machine &_machine, const via6522_device_config &_config);

public:
    UINT8 via_r(UINT32 offset);
    void via_w(UINT32 offset, UINT8 data);

    void via_porta_w(UINT8 data) { m_in_a = data; }

    UINT8 via_portb_r() { return m_in_b; }
    void via_portb_w(UINT8 data) { m_in_b = data; }

    UINT8 via_ca1_r() { return m_in_ca1; }
    void via_ca1_w(UINT8 data);

    UINT8 via_ca2_r() { return m_in_ca2; }
    void via_ca2_w(UINT8 data);

    UINT8 via_cb1_r() { return m_in_cb1; }
    void via_cb1_w(UINT8 data);

    UINT8 via_cb2_r() { return m_in_cb2; }
    void via_cb2_w(UINT8 data);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

    static TIMER_CALLBACK( t1_timeout_callback );
    static TIMER_CALLBACK( t2_timeout_callback );
    static TIMER_CALLBACK( shift_callback );

private:
    attotime cycles_to_time(int c);
    UINT32 time_to_cycles(attotime t);
    UINT16 get_counter1_value();

    void set_int(int data);
    void clear_int(int data);
    void shift();
    void t1_timeout();
    void t2_timeout();

    devcb_resolved_read8 m_in_a_func;
    devcb_resolved_read8 m_in_b_func;
    devcb_resolved_read_line m_in_ca1_func;
    devcb_resolved_read_line m_in_cb1_func;
    devcb_resolved_read_line m_in_ca2_func;
    devcb_resolved_read_line m_in_cb2_func;
    devcb_resolved_write8 m_out_a_func;
    devcb_resolved_write8 m_out_b_func;
    devcb_resolved_write_line m_out_ca1_func;
    devcb_resolved_write_line m_out_cb1_func;
    devcb_resolved_write_line m_out_ca2_func;
    devcb_resolved_write_line m_out_cb2_func;
    devcb_resolved_write_line m_irq_func;

    UINT8 m_in_a;
    UINT8 m_in_ca1;
    UINT8 m_in_ca2;
    UINT8 m_out_a;
    UINT8 m_out_ca2;
    UINT8 m_ddr_a;

    UINT8 m_in_b;
    UINT8 m_in_cb1;
    UINT8 m_in_cb2;
    UINT8 m_out_b;
    UINT8 m_out_cb2;
    UINT8 m_ddr_b;

    UINT8 m_t1cl;
    UINT8 m_t1ch;
    UINT8 m_t1ll;
    UINT8 m_t1lh;
    UINT8 m_t2cl;
    UINT8 m_t2ch;
    UINT8 m_t2ll;
    UINT8 m_t2lh;

    UINT8 m_sr;
    UINT8 m_pcr;
    UINT8 m_acr;
    UINT8 m_ier;
    UINT8 m_ifr;

    emu_timer *m_t1;
    attotime m_time1;
    UINT8 m_t1_active;
    emu_timer *m_t2;
    attotime m_time2;
    UINT8 m_t2_active;

    emu_timer *m_shift_timer;
    UINT8 m_shift_counter;

    const via6522_device_config &m_config;
};


// device type definition
extern const device_type VIA6522;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER(via_r);
WRITE8_DEVICE_HANDLER(via_w);

WRITE8_DEVICE_HANDLER(via_porta_w);

READ8_DEVICE_HANDLER(via_portb_r);
WRITE8_DEVICE_HANDLER(via_portb_w);

READ_LINE_DEVICE_HANDLER(via_ca1_r);
WRITE_LINE_DEVICE_HANDLER(via_ca1_w);

READ_LINE_DEVICE_HANDLER(via_ca2_r);
WRITE_LINE_DEVICE_HANDLER(via_ca2_w);

READ_LINE_DEVICE_HANDLER(via_cb1_r);
WRITE_LINE_DEVICE_HANDLER(via_cb1_w);

READ_LINE_DEVICE_HANDLER(via_cb2_r);
WRITE_LINE_DEVICE_HANDLER(via_cb2_w);

#endif /* __6522VIA_H__ */
