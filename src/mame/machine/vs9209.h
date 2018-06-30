// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    VS9209 (4L01F1429) QFP80 I/O chip

**********************************************************************/

#ifndef MAME_MACHINE_VS9209_H
#define MAME_MACHINE_VS9209_H

#pragma once

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VS9209_IN_PORTA_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(0, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTB_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(1, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTC_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(2, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTD_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(3, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTE_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(4, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTF_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(5, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTG_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(6, DEVCB_##_devcb);
#define MCFG_VS9209_IN_PORTH_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_input_cb(7, DEVCB_##_devcb);

#ifdef VS9209_PROBABLY_NONEXISTENT_OUTPUTS
#define MCFG_VS9209_OUT_PORTA_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(0, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTB_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(1, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTC_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(2, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTD_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(3, DEVCB_##_devcb);
#endif
#define MCFG_VS9209_OUT_PORTE_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(4, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTF_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(5, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTG_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(6, DEVCB_##_devcb);
#define MCFG_VS9209_OUT_PORTH_CB(_devcb) \
	devcb = &downcast<vs9209_device &>(*device).set_output_cb(7, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vs9209_device

class vs9209_device : public device_t
{
public:
	// construction/destruction
	vs9209_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template<class Object> devcb_base &set_input_cb(int p, Object &&obj)
	{
		assert(p >= 0 && p < 8);
		return m_input_cb[p].set_callback(std::forward<Object>(obj));
	}
	template<class Object> devcb_base &set_output_cb(int p, Object &&obj)
	{
		assert(p >= 0 && p < 8);
		return m_output_cb[p].set_callback(std::forward<Object>(obj));
	}

	// memory handlers
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// input/output callbacks
	devcb_read8         m_input_cb[8];
	devcb_write8        m_output_cb[8];

	// internal state
	u8                  m_data_latch[8];
	u8                  m_data_dir[8];
};

// device type definition
DECLARE_DEVICE_TYPE(VS9209, vs9209_device)

#endif // MAME_MACHINE_VS9209_H
