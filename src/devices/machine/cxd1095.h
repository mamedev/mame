// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sony CXD1095 CMOS I/O Port Expander

    CXD1095Q: 64-pin quad flat package

       1-2              NC
       3-9              PB1-PB7
        10              GND
     11-18              PC0-PC7
        19              NC

     20-24              PD0-PD4
        25              GND
        26              Vdd
     27-29              PD5-PD7
     30-32              D0-D2

     33-34              NC
     35-39              D3-D7
        40              /CLR
        41              /RST
        42              GND
        43              /WR
        44              /RD
        45              /CS
     46-48              A0-A2
     49-50              PX0-PX1
        51              NC

     52-53              PX2-PX3
     54-56              PA0-PA2
        57              GND
        58              Vdd
     59-63              PA3-PA7
        64              PB0

**********************************************************************/

#ifndef MAME_MACHINE_CXD1095_H
#define MAME_MACHINE_CXD1095_H

#pragma once

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CXD1095_IN_PORTA_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_input_cb(0, DEVCB_##_devcb);
#define MCFG_CXD1095_IN_PORTB_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_input_cb(1, DEVCB_##_devcb);
#define MCFG_CXD1095_IN_PORTC_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_input_cb(2, DEVCB_##_devcb);
#define MCFG_CXD1095_IN_PORTD_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_input_cb(3, DEVCB_##_devcb);
#define MCFG_CXD1095_IN_PORTE_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_input_cb(4, DEVCB_##_devcb);

#define MCFG_CXD1095_OUT_PORTA_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_output_cb(0, DEVCB_##_devcb);
#define MCFG_CXD1095_OUT_PORTB_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_output_cb(1, DEVCB_##_devcb);
#define MCFG_CXD1095_OUT_PORTC_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_output_cb(2, DEVCB_##_devcb);
#define MCFG_CXD1095_OUT_PORTD_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_output_cb(3, DEVCB_##_devcb);
#define MCFG_CXD1095_OUT_PORTE_CB(_devcb) \
	devcb = &downcast<cxd1095_device &>(*device).set_output_cb(4, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cxd1095_device

class cxd1095_device : public device_t
{
public:
	// construction/destruction
	cxd1095_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <class Object> devcb_base &set_input_cb(int p, Object &&cb) { assert(p >= 0 && p < 5); return m_input_cb[p].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_cb(int p, Object &&cb) { assert(p >= 0 && p < 5); return m_output_cb[p].set_callback(std::forward<Object>(cb)); }

	// memory handlers
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// input/output callbacks
	devcb_read8         m_input_cb[5];
	devcb_write8        m_output_cb[5];

	// internal state
	u8                  m_data_latch[5];
	u8                  m_data_dir[5];
};

// device type definition
DECLARE_DEVICE_TYPE(CXD1095, cxd1095_device)

#endif // MAME_MACHINE_CXD1095_H
