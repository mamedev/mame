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

#define MCFG_PIA6821_ADD(_tag, _intrf) \
    MCFG_DEVICE_ADD(_tag, PIA6821, 0) \
	pia6821_device::static_set_interface(*device, _intrf);

#define MCFG_PIA6821_MODIFY(_tag, _intrf) \
    MCFG_DEVICE_MODIFY(_tag) \
	pia6821_device::static_set_interface(*device, _intrf);

#define MCFG_PIA6822_ADD(_tag, _intrf) \
    MCFG_DEVICE_ADD(_tag, PIA6822, 0) \
	pia6821_device::static_set_interface(*device, _intrf);

#define MCFG_PIA6822_MODIFY(_tag, _intrf) \
    MCFG_DEVICE_MODIFY(_tag) \
	pia6821_device::static_set_interface(*device, _intrf);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> pia6821_interface

struct pia6821_interface
{
    devcb_read8 m_in_a_cb;
    devcb_read8 m_in_b_cb;
    devcb_read_line m_in_ca1_cb;
    devcb_read_line m_in_cb1_cb;
    devcb_read_line m_in_ca2_cb;
    devcb_read_line m_in_cb2_cb;
    devcb_write8 m_out_a_cb;
    devcb_write8 m_out_b_cb;
    devcb_write_line m_out_ca2_cb;
    devcb_write_line m_out_cb2_cb;
    devcb_write_line m_irq_a_cb;
    devcb_write_line m_irq_b_cb;
};



// ======================> pia6821_device

class pia6821_device :  public device_t,
                        public pia6821_interface
{
public:
    // construction/destruction
    pia6821_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const pia6821_interface &interface);

    DECLARE_READ8_MEMBER( read ) { return reg_r(offset); }
    DECLARE_WRITE8_MEMBER( write ) { reg_w(offset, data); }
    DECLARE_READ8_MEMBER( read_alt ) { return reg_r(((offset << 1) & 0x02) | ((offset >> 1) & 0x01)); }
    DECLARE_WRITE8_MEMBER( write_alt ) { reg_w(((offset << 1) & 0x02) | ((offset >> 1) & 0x01), data); }

    UINT8 port_b_z_mask() const { return ~m_ddr_b; }          // see first note in .c
    void set_port_a_z_mask(UINT8 data) { m_port_a_z_mask = data; }// see second note in .c

	DECLARE_READ8_MEMBER( porta_r );
    UINT8 porta_r() { return porta_r(*memory_nonspecific_space(machine()), 0); }
    DECLARE_WRITE8_MEMBER( porta_w );
    void porta_w(UINT8 data) { porta_w(*memory_nonspecific_space(machine()), 0, data); }
    void set_a_input(UINT8 data, UINT8 z_mask);
    UINT8 a_output();

    DECLARE_READ_LINE_MEMBER( ca1_r );
    DECLARE_WRITE_LINE_MEMBER( ca1_w );

    DECLARE_READ_LINE_MEMBER( ca2_r );
    DECLARE_WRITE_LINE_MEMBER( ca2_w );
    int ca2_output();
    int ca2_output_z();

	DECLARE_READ8_MEMBER( portb_r );
    UINT8 portb_r() { return portb_r(*memory_nonspecific_space(machine()), 0); }
    DECLARE_WRITE8_MEMBER( portb_w );
    void portb_w(UINT8 data) { portb_w(*memory_nonspecific_space(machine()), 0, data); }
    UINT8 b_output();

    DECLARE_READ_LINE_MEMBER( cb1_r );
    DECLARE_WRITE_LINE_MEMBER( cb1_w );

    DECLARE_READ_LINE_MEMBER( cb2_r );
    DECLARE_WRITE_LINE_MEMBER( cb2_w );
    int cb2_output();
    int cb2_output_z();

	int irq_a_state() const { return m_irq_a_state; }
	int irq_b_state() const { return m_irq_b_state; }

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
    UINT8 reg_r(UINT8 offset);
    void reg_w(UINT8 offset, UINT8 data);

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

    // variables that indicate if access a line externally -
    // used to for logging purposes ONLY
    bool m_in_a_pushed;
    bool m_out_a_needs_pulled;
    bool m_in_ca1_pushed;
    bool m_in_ca2_pushed;
    bool m_out_ca2_needs_pulled;
    bool m_in_b_pushed;
    bool m_out_b_needs_pulled;
    bool m_in_cb1_pushed;
    bool m_in_cb2_pushed;
    bool m_out_cb2_needs_pulled;
    bool m_logged_port_a_not_connected;
    bool m_logged_port_b_not_connected;
    bool m_logged_ca1_not_connected;
    bool m_logged_ca2_not_connected;
    bool m_logged_cb1_not_connected;
    bool m_logged_cb2_not_connected;
};


// device type definition
extern const device_type PIA6821;


#endif /* __6821PIA_H__ */
