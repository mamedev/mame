// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

    Custom 100-pin QFP LSI. Supports 7 8-bit input/output ports and can
    directly interact with dual port RAM. Also supports a master/slave
    configuration where one controller acts as master and sends commands
    and data over a serial link to another controller.

    TODO:
    - Serial
    - Slave mode
    - and probably lots more

***************************************************************************/

#ifndef MAME_MACHINE_315_5338A_H
#define MAME_MACHINE_315_5338A_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_315_5338A_READ_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_read_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_WRITE_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_write_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT0_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out0_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN1_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in1_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN2_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in2_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN3_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in3_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN4_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in4_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT4_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out4_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT5_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out5_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN6_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in6_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5338a_device : public device_t
{
public:
	// construction/destruction
	sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_read_callback(Object &&cb)
	{ return m_read_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_write_callback(Object &&cb)
	{ return m_write_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_out0_callback(Object &&cb)
	{ return m_out0_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in1_callback(Object &&cb)
	{ return m_in1_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in2_callback(Object &&cb)
	{ return m_in2_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in3_callback(Object &&cb)
	{ return m_in3_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in4_callback(Object &&cb)
	{ return m_in4_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_out4_callback(Object &&cb)
	{ return m_out4_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_out5_callback(Object &&cb)
	{ return m_out5_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in6_callback(Object &&cb)
	{ return m_in6_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;

	devcb_write8 m_out0_cb;
	devcb_read8 m_in1_cb;
	devcb_read8 m_in2_cb;
	devcb_read8 m_in3_cb;
	devcb_read8 m_in4_cb;
	devcb_write8 m_out4_cb;
	devcb_write8 m_out5_cb;
	devcb_read8 m_in6_cb;

	uint8_t m_port0;
	uint8_t m_config;
	uint8_t m_serial_output;
	uint16_t m_address;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device)

#endif // MAME_MACHINE_315_5338A_H
