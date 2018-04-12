// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega Model 1/2 I/O RAM Abstraction

    This device handles the 16 bytes of dual-port RAM that are used to
    communicate with the I/O board. It should go away once we properly
    emulate the I/O boards.

***************************************************************************/

#ifndef MAME_MACHINE_M1IO_H
#define MAME_MACHINE_M1IO_H

#pragma once

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M1IO_AN0_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 0);

#define MCFG_M1IO_AN1_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 1);

#define MCFG_M1IO_AN2_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 2);

#define MCFG_M1IO_AN3_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 3);

#define MCFG_M1IO_AN4_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 4);

#define MCFG_M1IO_AN5_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 5);

#define MCFG_M1IO_AN6_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 6);

#define MCFG_M1IO_AN7_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_an_callback(DEVCB_##_devcb, 7);

#define MCFG_M1IO_DI0_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_di_callback(DEVCB_##_devcb, 0);

#define MCFG_M1IO_DI1_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_di_callback(DEVCB_##_devcb, 1);

#define MCFG_M1IO_DI2_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_di_callback(DEVCB_##_devcb, 2);

#define MCFG_M1IO_DO_CB(_devcb) \
	devcb = &downcast<m1io_device &>(*device).set_do_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m1io_device : public device_t
{
public:
	// construction/destruction
	m1io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_an_callback(Object &&cb, int index)
	{ return m_an_cb[index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_di_callback(Object &&cb, int index)
	{ return m_di_cb[index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_do_callback(Object &&cb)
	{ return m_do_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_an_cb[8];
	devcb_read8 m_di_cb[3];
	devcb_write8 m_do_cb;

	uint8_t m_out;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_M1IO, m1io_device)

#endif // MAME_MACHINE_M1IO_H
