// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    LH5810/LH5811 Input/Output Port Controller

**********************************************************************/

#ifndef MAME_MACHINE_LH5810_H
#define MAME_MACHINE_LH5810_H

#pragma once


//*************************************************************************
//  INTERFACE CONFIGURATION MACROS
//*************************************************************************

#define MCFG_LH5810_PORTA_R_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_porta_r_callback(DEVCB_##_devcb);

#define MCFG_LH5810_PORTA_W_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_porta_w_callback(DEVCB_##_devcb);

#define MCFG_LH5810_PORTB_R_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_portb_r_callback(DEVCB_##_devcb);

#define MCFG_LH5810_PORTB_W_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_portb_w_callback(DEVCB_##_devcb);

#define MCFG_LH5810_PORTC_W_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_portc_w_callback(DEVCB_##_devcb);

#define MCFG_LH5810_OUT_INT_CB(_devcb) \
	devcb = &downcast<lh5810_device &>(*device).set_out_int_callback(DEVCB_##_devcb); //currently unused



//*************************************************************************
//  TYPE DEFINITIONS
//*************************************************************************

// ======================> lh5810_device

class lh5810_device : public device_t
{
public:
	// construction/destruction
	lh5810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_porta_r_callback(Object &&cb) { return m_porta_r_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_porta_w_callback(Object &&cb) { return m_porta_w_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_portb_r_callback(Object &&cb) { return m_portb_r_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_portb_w_callback(Object &&cb) { return m_portb_w_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_portc_w_callback(Object &&cb) { return m_portc_w_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_int_callback(Object &&cb) { return m_out_int_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	devcb_read8         m_porta_r_cb;       //port A read
	devcb_write8        m_porta_w_cb;       //port A write
	devcb_read8         m_portb_r_cb;       //port B read
	devcb_write8        m_portb_w_cb;       //port B write
	devcb_write8        m_portc_w_cb;       //port C write

	devcb_write_line    m_out_int_cb;       //IRQ callback

	uint8_t m_reg[0x10];
	uint8_t m_irq;
};


// device type definition
DECLARE_DEVICE_TYPE(LH5810, lh5810_device)

#endif // MAME_MACHINE_LH5810_H
