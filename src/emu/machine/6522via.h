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

#define MCFG_VIA6522_ADD(_tag, _clock, _intrf) \
    MCFG_DEVICE_ADD(_tag, VIA6522, _clock) \
    MCFG_DEVICE_CONFIG(_intrf)



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
    devcb_read8 m_in_a_cb;
    devcb_read8 m_in_b_cb;
    devcb_read_line m_in_ca1_cb;
    devcb_read_line m_in_cb1_cb;
    devcb_read_line m_in_ca2_cb;
    devcb_read_line m_in_cb2_cb;
    devcb_write8 m_out_a_cb;
    devcb_write8 m_out_b_cb;
    devcb_write_line m_out_ca1_cb;
    devcb_write_line m_out_cb1_cb;
    devcb_write_line m_out_ca2_cb;
    devcb_write_line m_out_cb2_cb;
    devcb_write_line m_irq_cb;
};


// ======================> via6522_device

class via6522_device :  public device_t,
                        public via6522_interface
{
    friend class dart_channel;

public:
    // construction/destruction
    via6522_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

    DECLARE_WRITE8_MEMBER( write_porta ) { m_in_a = data; }

    DECLARE_READ8_MEMBER( read_portb ) { return m_in_b; }
    DECLARE_WRITE8_MEMBER( write_portb ) { m_in_b = data; }

    DECLARE_READ_LINE_MEMBER( read_ca1 ) { return m_in_ca1; }
    DECLARE_WRITE_LINE_MEMBER( write_ca1 );

    DECLARE_READ_LINE_MEMBER( read_ca2 ) { return m_in_ca2; }
    DECLARE_WRITE_LINE_MEMBER( write_ca2 );

    DECLARE_READ_LINE_MEMBER( read_cb1 ) { return m_in_cb1; }
    DECLARE_WRITE_LINE_MEMBER( write_cb1 );

    DECLARE_READ_LINE_MEMBER( read_cb2 ) { return m_in_cb2; }
    DECLARE_WRITE_LINE_MEMBER( write_cb2 );

protected:
    // device-level overrides
    virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_SHIFT = 0;
	static const device_timer_id TIMER_T1 = 1;
	static const device_timer_id TIMER_T2 = 2;
	static const device_timer_id TIMER_CA2 = 3;

    UINT16 get_counter1_value();

	inline void set_irq_line(int state);
    void set_int(int data);
    void clear_int(int data);
    void shift();

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
	int m_irq;

    emu_timer *m_t1;
    attotime m_time1;
    UINT8 m_t1_active;
    emu_timer *m_t2;
    attotime m_time2;
    UINT8 m_t2_active;
	emu_timer *m_ca2_timer;

    emu_timer *m_shift_timer;
    UINT8 m_shift_counter;
};


// device type definition
extern const device_type VIA6522;


#endif /* __6522VIA_H__ */
