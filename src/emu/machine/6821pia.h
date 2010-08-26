/**********************************************************************

    Motorola 6821 PIA interface and emulation

    Notes:
        * get_port_b_z_mask() gives the caller the bitmask that shows
          which bits are high-impendance when reading port B, and thus
          neither 0 or 1. get_output_cb2_z() returns the same info
          for the CB2 pin.
        * set_port_a_z_mask allows the input callback to indicate
          which port A bits are disconnected. For these bits, the
          read operation will return the output buffer's contents.
        * The 'alt' interface functions are used when the A0 and A1
          address bits are swapped.
        * All 'int' data or return values are bool, and should be
          converted to bool at some point.

**********************************************************************/

#pragma once

#ifndef __6821PIA_H__
#define __6821PIA_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIA6821_ADD(_tag, _intrf) \
    MDRV_DEVICE_ADD(_tag, PIA6821, 0) \
    MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6821_MODIFY(_tag, _intrf) \
    MDRV_DEVICE_MODIFY(_tag) \
    MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_ADD(_tag, _intrf) \
    MDRV_DEVICE_ADD(_tag, PIA6822, 0) \
    MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_MODIFY(_tag, _intrf) \
    MDRV_DEVICE_MODIFY(_tag) \
    MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> pia6821_interface

struct pia6821_interface
{
    devcb_read8 m_in_a_func;
    devcb_read8 m_in_b_func;
    devcb_read_line m_in_ca1_func;
    devcb_read_line m_in_cb1_func;
    devcb_read_line m_in_ca2_func;
    devcb_read_line m_in_cb2_func;
    devcb_write8 m_out_a_func;
    devcb_write8 m_out_b_func;
    devcb_write_line m_out_ca2_func;
    devcb_write_line m_out_cb2_func;
    devcb_write_line m_irq_a_func;
    devcb_write_line m_irq_b_func;
};



// ======================> pia6821_device_config

class pia6821_device_config : public device_config,
                              public pia6821_interface
{
    friend class pia6821_device;

    // construction/destruction
    pia6821_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> pia6821_device

class pia6821_device :  public device_t
{
    friend class pia6821_device_config;

    // construction/destruction
    pia6821_device(running_machine &_machine, const pia6821_device_config &_config);

public:

    UINT8 reg_r(UINT8 offset);
    void reg_w(UINT8 offset, UINT8 data);

    UINT8 alt_r(UINT8 offset);
    void alt_w(UINT8 offset, UINT8 data);

    UINT8 get_port_b_z_mask();          /* see first note in .c */
    void set_port_a_z_mask(UINT8 data); /* see second note in .c */

    UINT8 porta_r();
    void porta_w(UINT8 data);
    void set_input_a(UINT8 data, UINT8 z_mask);
    UINT8 get_output_a();

    UINT8 ca1_r();
    void ca1_w(UINT8 state);

    UINT8 ca2_r();
    void ca2_w(UINT8 state);
    int get_output_ca2();
    int get_output_ca2_z();

    UINT8 portb_r();
    void portb_w(UINT8 data);
    UINT8 get_output_b();

    UINT8 cb1_r();
    void cb1_w(UINT8 state);

    UINT8 cb2_r();
    void cb2_w(UINT8 state);
    int get_output_cb2();
    int get_output_cb2_z();

    int get_irq_a();
    int get_irq_b();

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

private:

    void update_interrupts();

    UINT8 get_in_a_value();
    UINT8 get_in_b_value();

    UINT8 get_out_a_value();
    UINT8 get_out_b_value();

    void set_out_ca2(int data);
    void set_out_cb2(int data);

    UINT8 port_a_r();
    UINT8 ddr_a_r();
    UINT8 control_a_r();

    UINT8 port_b_r();
    UINT8 ddr_b_r();
    UINT8 control_b_r();

    void send_to_out_a_func(const char* message);
    void send_to_out_b_func(const char* message);

    void port_a_w(UINT8 data);
    void ddr_a_w(UINT8 data);

    void port_b_w(UINT8 data);
    void ddr_b_w(UINT8 data);

    void control_a_w(UINT8 data);
    void control_b_w(UINT8 data);

    devcb_resolved_read8 m_in_a_func;
    devcb_resolved_read8 m_in_b_func;
    devcb_resolved_read_line m_in_ca1_func;
    devcb_resolved_read_line m_in_cb1_func;
    devcb_resolved_read_line m_in_ca2_func;
    devcb_resolved_read_line m_in_cb2_func;
    devcb_resolved_write8 m_out_a_func;
    devcb_resolved_write8 m_out_b_func;
    devcb_resolved_write_line m_out_ca2_func;
    devcb_resolved_write_line m_out_cb2_func;
    devcb_resolved_write_line m_irq_a_func;
    devcb_resolved_write_line m_irq_b_func;

    UINT8 m_in_a;
    UINT8 m_in_ca1;
    UINT8 m_in_ca2;
    UINT8 m_out_a;
    UINT8 m_out_ca2;
    UINT8 m_port_a_z_mask;
    UINT8 m_ddr_a;
    UINT8 m_ctl_a;
    UINT8 m_irq_a1;
    UINT8 m_irq_a2;
    UINT8 m_irq_a_state;

    UINT8 m_in_b;
    UINT8 m_in_cb1;
    UINT8 m_in_cb2;
    UINT8 m_out_b;
    UINT8 m_out_cb2;
    UINT8 m_last_out_cb2_z;
    UINT8 m_ddr_b;
    UINT8 m_ctl_b;
    UINT8 m_irq_b1;
    UINT8 m_irq_b2;
    UINT8 m_irq_b_state;

    /* variables that indicate if access a line externally -
       used to for logging purposes ONLY */
    UINT8 m_in_a_pushed;
    UINT8 m_out_a_needs_pulled;
    UINT8 m_in_ca1_pushed;
    UINT8 m_in_ca2_pushed;
    UINT8 m_out_ca2_needs_pulled;
    UINT8 m_in_b_pushed;
    UINT8 m_out_b_needs_pulled;
    UINT8 m_in_cb1_pushed;
    UINT8 m_in_cb2_pushed;
    UINT8 m_out_cb2_needs_pulled;
    UINT8 m_logged_port_a_not_connected;
    UINT8 m_logged_port_b_not_connected;
    UINT8 m_logged_ca1_not_connected;
    UINT8 m_logged_ca2_not_connected;
    UINT8 m_logged_cb1_not_connected;
    UINT8 m_logged_cb2_not_connected;

    const pia6821_device_config &m_config;
};


// device type definition
extern const device_type PIA6821;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( pia6821_r );
WRITE8_DEVICE_HANDLER( pia6821_w );

READ8_DEVICE_HANDLER( pia6821_alt_r );
WRITE8_DEVICE_HANDLER( pia6821_alt_w );

UINT8 pia6821_get_port_b_z_mask(running_device *device);  /* see first note */
void pia6821_set_port_a_z_mask(running_device *device, UINT8 data);  /* see second note */

READ8_DEVICE_HANDLER( pia6821_porta_r );
WRITE8_DEVICE_HANDLER( pia6821_porta_w );
void pia6821_set_input_a(running_device *device, UINT8 data, UINT8 z_mask);
UINT8 pia6821_get_output_a(running_device *device);

READ_LINE_DEVICE_HANDLER( pia6821_ca1_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_ca1_w );

READ_LINE_DEVICE_HANDLER( pia6821_ca2_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_ca2_w );
int pia6821_get_output_ca2(running_device *device);
int pia6821_get_output_ca2_z(running_device *device);

READ8_DEVICE_HANDLER( pia6821_portb_r );
WRITE8_DEVICE_HANDLER( pia6821_portb_w );
UINT8 pia6821_get_output_b(running_device *device);

READ_LINE_DEVICE_HANDLER( pia6821_cb1_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_cb1_w );

READ_LINE_DEVICE_HANDLER( pia6821_cb2_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_cb2_w );
int pia6821_get_output_cb2(running_device *device);
int pia6821_get_output_cb2_z(running_device *device);

int pia6821_get_irq_a(running_device *device);
int pia6821_get_irq_b(running_device *device);


#endif /* __6821PIA_H__ */
