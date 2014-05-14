// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    LH5810/LH5811 Input/Output Port Controller

**********************************************************************/

#pragma once

#ifndef __LH5810__
#define __LH5810__

#include "emu.h"

//*************************************************************************
//  MACROS / CONSTANTS
//*************************************************************************

enum
{
	LH5810_RESET = 4,
	LH5810_U,
	LH5810_L,
	LH5820_F,
	LH5810_OPC,
	LH5810_G,
	LH5810_MSK,
	LH5810_IF,
	LH5810_DDA,
	LH5810_DDB,
	LH5810_OPA,
	LH5810_OPB
};


//*************************************************************************
//  INTERFACE CONFIGURATION MACROS
//*************************************************************************

#define MCFG_LH5810_PORTA_R_CB(_devcb) \
	devcb = &lh5810_device::set_porta_r_callback(*device, DEVCB_##_devcb);

#define MCFG_LH5810_PORTA_W_CB(_devcb) \
	devcb = &lh5810_device::set_porta_w_callback(*device, DEVCB_##_devcb);

#define MCFG_LH5810_PORTB_R_CB(_devcb) \
	devcb = &lh5810_device::set_portb_r_callback(*device, DEVCB_##_devcb);

#define MCFG_LH5810_PORTB_W_CB(_devcb) \
	devcb = &lh5810_device::set_portb_w_callback(*device, DEVCB_##_devcb);

#define MCFG_LH5810_PORTC_W_CB(_devcb) \
	devcb = &lh5810_device::set_portc_w_callback(*device, DEVCB_##_devcb);

#define MCFG_LH5810_OUT_INT_CB(_devcb) \
	devcb = &lh5810_device::set_out_int_callback(*device, DEVCB_##_devcb); //currently unused



//*************************************************************************
//  TYPE DEFINITIONS
//*************************************************************************

// ======================> lh5810_device

class lh5810_device :   public device_t
{
public:
	// construction/destruction
	lh5810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_porta_r_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_porta_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_porta_w_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_porta_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_portb_r_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_portb_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_portb_w_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_portb_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_portc_w_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_portc_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<lh5810_device &>(device).m_out_int_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:

	devcb_read8         m_porta_r_cb;       //port A read
	devcb_write8        m_porta_w_cb;       //port A write
	devcb_read8         m_portb_r_cb;       //port B read
	devcb_write8        m_portb_w_cb;       //port B write
	devcb_write8        m_portc_w_cb;       //port C write

	devcb_write_line    m_out_int_cb;       //IRQ callback

	UINT8 m_reg[0x10];
	UINT8 m_irq;
};


// device type definition
extern const device_type LH5810;

#endif
